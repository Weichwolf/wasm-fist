#!/usr/bin/env python3
"""Pinned prepared-world inputs for complete shared remaining-ground consumption.

The inventory command verifies complete original mission preparation and optionally
its current native observation. It does not claim station/support child acceptance.
Only one decoded height/detail and one prepared world are retained at a time.
"""
import argparse
import dataclasses
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import time

from mission_ready_contract import original_prepare, observed_world
from orders_contract import scenario_order_blocks
from original_asset_oracle import OriginalAssetOracle
from original_heightfield_oracle import OriginalHeightfieldOracle
from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_object_pool_oracle import LONG_BASE, SHORT_BASE
from original_unit_oracle import DGROUP
from roster_promotion_contract import word
from test_mission_world import install, world_lines
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / 'armoredfist/FISTDATA'
SEEDS = (1, 2, 32768, 65535)
DETAILS = (512, 1024, 2048, 4096)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def checked_file(name, metadata):
    path = DIRECTORY / name
    data = path.read_bytes()
    if digest(data) != metadata['sha256'] or ('size' in metadata and len(data) != metadata['size']):
        raise AssertionError(f'Pinned original differs: {name}')
    if path.stat().st_mode & 0o222:
        raise AssertionError(f'Original is writable: {name}')
    return data


def height_name(data):
    offset, names = 0, []
    while offset < len(data):
        if len(data) - offset < 6:
            raise AssertionError('Incomplete scenario envelope')
        tag, size = struct.unpack_from('<4sH', data, offset)
        if offset + size + 6 > len(data):
            raise AssertionError('Incomplete scenario payload')
        if tag == b'BINF':
            names.append(data[offset + 6:offset + 22].split(b'\0')[0]
                         .replace(b' ', b'').decode('ascii').upper())
        offset += size + 6
    if len(names) != 1:
        raise AssertionError('Expected exactly one authored mission terrain name')
    return names[0]


@dataclasses.dataclass(frozen=True)
class PreparedWorld:
    name: str
    terrain: str
    side: int
    pixels: bytes
    scenario: bytes
    data: bytes
    objects: dict
    before_observation: str
    after_observation: str

    def physical(self):
        used = self.data[0xe2f7:0xe2f7 + 150] + self.data[0xe38d:0xe38d + 32]
        return {slot: (SHORT_BASE + slot * 55 if slot < 150 else LONG_BASE + (slot - 150) * 251)
                for slot, alive in enumerate(used) if alive}

    def ground_actors(self):
        return {slot: pointer for slot, pointer in self.physical().items() if word(self.data, pointer) < 4}

    def artillery(self, side):
        count, base = (0x9ccb, 0x9ccf) if side == 0 else (0x9ccd, 0x9cd7)
        length = word(self.data, count)
        if length > 4:
            raise AssertionError('Prepared resource list exceeds the recovered side contract')
        physical = {pointer: slot for slot, pointer in self.physical().items()}
        result = []
        for index in range(length):
            pointer = word(self.data, base + index * 2)
            if pointer not in physical or word(self.data, pointer) != 27 or word(self.data, pointer + 31) != 5:
                raise AssertionError('Actual preparation did not publish a live five-round artillery resource')
            result.append((physical[pointer], self.data[pointer + 25], word(self.data, pointer + 31)))
        return result


class Corpus:
    def __init__(self):
        self.manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        self.terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        if sorted(p.name for p in DIRECTORY.glob('*.FSG')) != sorted(self.manifest) or len(self.manifest) != 47:
            raise AssertionError('Missing, extra or unpinned mission inputs')
        self.grouped = {}
        for name, info in sorted(self.manifest.items()):
            terrain = height_name(checked_file(name, info))
            if terrain not in self.terrains:
                raise AssertionError('Mission references an unpinned height source')
            self.grouped.setdefault(terrain, []).append(name)
        if len(self.grouped) != 8:
            raise AssertionError('Expected all eight authored height sources')
        self.owner = OriginalMissionReadyOracle()
        self.decoder, self.scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()

    def worlds(self):
        for terrain, names in sorted(self.grouped.items()):
            metadata = self.terrains[terrain]
            source = checked_file(terrain, metadata)
            decoded = self.decoder.klc(source)
            if digest(decoded) != metadata['decoded_sha256']:
                raise AssertionError('Complete original height decode differs')
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in DETAILS:
                actual_side, pixels = self.scaler.resample(metadata['width'], plane, [side])
                if actual_side != side or len(pixels) != side * side:
                    raise AssertionError('Incomplete original height publication')
                for name in names:
                    scenario = checked_file(name, self.manifest[name])
                    records, orders = records_from_scenario(scenario), scenario_order_blocks(scenario)
                    status, installed = install(records, SEEDS, 3, 0)
                    if status != 0:
                        raise AssertionError('Authored mission cannot be installed by the delivered contract')
                    before = world_lines(installed, orders)
                    machine, objects = self.owner.prepare_saved(records, SEEDS, 3, 0, orders)
                    if world_lines(observed_world(self.owner, machine, objects), orders) != before:
                        raise AssertionError('Complete original saved installation differs')
                    prepared = original_prepare(self.owner, machine, objects, side, pixels)
                    after = world_lines(prepared, orders)
                    yield PreparedWorld(name, terrain, side, pixels, scenario,
                                        bytes(machine.mem_read(DGROUP, 65536)), objects, before, after)
            checked_file(terrain, metadata)
        for name, info in self.manifest.items():
            checked_file(name, info)


def inventory(native_probe=None):
    begin = time.monotonic()
    corpus = Corpus()
    records, actor_count, resource_count, native_count = {}, 0, 0, 0
    with tempfile.TemporaryDirectory(prefix='fist-support-corpus-', dir='/tmp') as temporary:
        request = pathlib.Path(temporary) / 'prepare.bin'
        for world in corpus.worlds():
            actors = world.ground_actors()
            resources = [world.artillery(side) for side in range(2)]
            if native_probe is not None:
                request.write_bytes(struct.pack('<I4HBBH', world.side, *SEEDS, 3, 0, 1)
                                    + world.pixels + bytes(4))
                result = subprocess.run([str(native_probe), str(DIRECTORY / world.name), str(request)],
                                        capture_output=True, text=True, timeout=180)
                expected = 'status 0\n' + world.before_observation + 'prepare 0\n' + world.after_observation
                if result.returncode or result.stdout != expected:
                    actual_lines, expected_lines = result.stdout.splitlines(), expected.splitlines()
                    mismatch = next((i for i, (a, b) in enumerate(zip(actual_lines, expected_lines)) if a != b),
                                    min(len(actual_lines), len(expected_lines)))
                    raise AssertionError(f'{world.name}/{world.side}: complete native preparation differs '
                                         f'at line {mismatch}: {actual_lines[mismatch:mismatch+1]!r} versus '
                                         f'{expected_lines[mismatch:mismatch+1]!r}; {result.stderr}')
                native_count += 1
            actor_count += len(actors)
            resource_count += sum(map(len, resources))
            records.setdefault(world.name, {})[str(world.side)] = {
                'terrain': world.terrain, 'ground_actors': len(actors),
                'prepared_data_sha256': digest(world.data), 'installed_height_sha256': digest(world.pixels),
                'complete_observation_sha256': digest(world.after_observation.encode()), 'artillery': resources}
            print(f'Prepared corpus: {world.name} {world.side}: {len(actors)} ground actors, '
                  f'{list(map(len, resources))} artillery', flush=True)
    if len(records) != 47 or any(set(v) != set(map(str, DETAILS)) for v in records.values()) or actor_count != 3840:
        raise AssertionError('Incomplete prepared-world inventory')
    if native_probe is not None and native_count != 188:
        raise AssertionError('Incomplete native producer observation')
    return {'success': True, 'scope': 'Complete original prepared input inventory and optional native producer '
            'observation only; station/support consuming and captured-lifetime acceptance remain open',
            'worlds': 188, 'ground_actors': actor_count, 'artillery_resources': resource_count,
            'native_preparations': native_count, 'seconds': time.monotonic() - begin, 'corpus': records}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', action='store_true', required=True)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--result-json', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.result_json.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    args.result_json.unlink(missing_ok=True)
    result = inventory(args.native_probe)
    args.result_json.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'corpus'}, sort_keys=True), flush=True)
