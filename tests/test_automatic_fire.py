#!/usr/bin/env python3
"""Shared complete automatic fire, ordered SAM racks and physical constructor."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from automatic_fire_contract import FIRE_THRESHOLDS, MISSILE_MESSAGES, automatic_fire
from original_unit_oracle import DGROUP
from roster_promotion_contract import store, word
from test_ground_maneuver import physical_context
from test_target_discovery import NONE, physical, pointer
from test_units import snapshot
from test_vehicle_motion import start
from test_vehicle_start import state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = REVIEW = None
ORACLE = ORIGINALS = False
SEEDS = (1, 2, 32768, 65535)
ACTOR = pointer(150)
ENTRIES = {0xafa2: 67, 0xaf97: 68, 0x8711: 69, 0x96c0: 69}


def output(before, after, actor, effect, status=0):
    addresses, registry = physical_context(after)
    slot = physical(actor)
    raw = bytearray(after[actor:actor + 251])
    kind = word(raw, 0)
    store(raw, 0x9d, 0)  # Source near words never become runtime candidates.
    binding = next(((index, value) for index, (bound, value) in enumerate(registry)
                    if bound == slot), (0, 0))
    branch = effect['branch']
    failure = branch.removeprefix('missile_')
    failed = failure in ('empty', 'target_rejected', 'capacity') and word(before, 0x6d34) == actor
    notice_kind = {'empty': 1, 'target_rejected': 2, 'capacity': 3}.get(failure, 0)
    notice = (120, 28, 0, 0, notice_kind) if failed else (77, NONE, 255, 1, 0)
    sound = effect['request'] is not None
    rack = effect['rack'] if effect['rack'] is not None else 255
    states = tuple(raw[0xb7:0xb9]) if kind in (1, 3) else (0, 0)
    values = (status, int(branch == 'fire_requested'), int(branch == 'rack_loading'),
              int(branch == 'missile_launched'), int(failed), rack, *states, *notice,
              word(after, 0x9fdd), int(sound), 7 if sound else 0, 0, 0)
    result = 'fire ' + ' '.join(map(str, values)) + '\n' + state_lines([(*binding, raw)])
    if effect['allocation'] is not None:
        address, missile_slot, index = effect['allocation']
        body = after[address:address + 55]
        values = (15, missile_slot, index, 1, *struct.unpack_from('<3iH', body, 4),
                  physical(word(body, 0x2b)), physical(word(body, 0x1a)), word(body, 0x12),
                  word(body, 0x14), word(body, 0x26), word(body, 0x28), body[22], body[23],
                  *struct.unpack_from('<5h', body, 0x1c), body[24], body[25], body[0x2a])
        result += 'missile ' + ' '.join(map(str, values)) + '\n'
    result += f'counts {word(after, 0xe294)} {word(after, 0xe296)}\nslots'
    for index in range(182):
        result += f' {int(pointer(index) in addresses)}:{word(after, pointer(index)) if pointer(index) in addresses else 0}'
    result += '\nregistry' + ''.join(f' {bound}:{value}' for bound, value in registry) + '\n'
    return result


def fixture(baseline, kind=0, *, behavior=0, target_flags=0, phase=0, distance=0, control=128,
            actual=0, requested=0, states=(1, 1), counts=(1, 1), selected=False,
            audible=False, present=True, target_kind=5, pose=(0, 0, 0), heading=0,
            actor_flags=8, retained=12345, opaque=None, fill=0, orphan=False):
    data = bytearray(baseline)
    data[0xdfbc:0xdfbc + 728] = bytes(728)
    data[0xe2f7:0xe2f7 + 150] = bytes(150)
    data[0xe38d:0xe38d + 32] = bytes(32)
    raw = bytearray(start(kind))
    raw[22], raw[0x92] = actor_flags, 93
    raw[0xb5:0xb9] = bytes((*counts, *states))
    struct.pack_into('<3iH', raw, 4, *pose, heading)
    for offset, value in ((0x40, control), (0x97, pointer(0) if present else 0),
                          (0x99, distance), (0x89, actual), (0x8b, requested)):
        store(raw, offset, value)
    if opaque is not None:
        store(raw, 0x97, opaque)
    data[ACTOR:ACTOR + 251] = raw
    target = bytearray(snapshot(target_kind, flags=target_flags))
    target_slot = 151 if target_kind in (0, 1, 2, 3, 19) else 0
    target_pointer = pointer(target_slot)
    data[target_pointer:target_pointer + len(target)] = target
    if present and opaque is None:
        store(data, ACTOR + 0x97, target_pointer)
    data[0xe38d] = 1
    if not orphan:
        struct.pack_into('<HH', data, 0xdfbc + 150 * 4, ACTOR, 1)
    table = 0xe38d if target_slot >= 150 else 0xe2f7
    data[table + (target_slot - 150 if target_slot >= 150 else target_slot)] = 1
    struct.pack_into('<HH', data, 0xdfbc, target_pointer, 7)
    for index in range(fill):
        address = pointer(index + 1)
        body = snapshot(18, flags=0)
        data[address:address + 55] = body
        data[0xe2f7 + index + 1] = 1
        struct.pack_into('<HH', data, 0xdfbc + (index + 1) * 4, address, index + 1)
    short_count = sum(data[0xe2f7:0xe2f7 + 150])
    extended_count = sum(data[0xe38d:0xe38d + 32])
    for offset, value in ((0xe294, short_count), (0xe296, extended_count), (0xe298, short_count),
                          (0x9796, 0x8000), (0x8000, behavior), (0x978c, phase),
                          (0x6d34, ACTOR if selected else 0), (0x9fdf, ACTOR if audible else 0),
                          (0x9fdd, retained), (0x969e, 77), (0x96a0, NONE), (0x7ae0, 0)):
        store(data, offset, value)
    struct.pack_into('<5H', data, 0x1f82, 0x1f8a, *SEEDS)
    return data


class Collector:
    def __init__(self, commands, temporary, owner=None):
        self.commands, self.owner = commands, owner
        self.machine = owner.fresh() if owner else None
        self.path = pathlib.Path(temporary) / 'cases.bin'
        self.cases, self.expected = [], []
        self.count = self.batches = self.canonical_count = self.original_count = 0
        self.branches = collections.Counter()
        self.digest = hashlib.sha256()
        self.canonical = None
        self.side, self.pixels = 1, b'\0'

    def begin(self, name=None, side=1, pixels=b'\0'):
        self.flush()
        self.canonical = ROOT / 'armoredfist/FISTDATA' / name if name else None
        self.side, self.pixels = side, pixels

    def enqueue(self, before, actor=ACTOR, entry=0xafa2, *, original_after=None, invalid=False):
        if self.owner and original_after is None and not invalid:
            self.machine.mem_write(DGROUP, bytes(before))
            original_after, _ = self.owner.observe(self.machine, actor, entry=entry)
        after, effect = automatic_fire(before, actor, entry=entry) if not invalid else (before,
            {'branch': 'invalid', 'rack': None, 'allocation': None, 'request': None})
        if original_after is not None:
            self.original_count += 1
            # Stack scratch and stack return words are outside the logical fixture.
            if after[:0x8fc0] != original_after[:0x8fc0] or after[0x9002:] != original_after[0x9002:]:
                raise AssertionError('Complete independent fire model differs from unchanged original')
        addresses, registry = physical_context(before)
        target = word(before, actor + 0x97)
        target_slot = addresses.get(target, NONE)
        seeds = struct.unpack_from('<4H', before, 0x1f84)
        cursor = ((word(before, 0x1f82) - 0x1f84) // 2 + 1) % 4
        header = struct.pack('<6H3B4HH', addresses[actor], physical(word(before, 0x6d34)),
                             word(before, 0x978c), physical(word(before, 0x9fdf)),
                             word(before, 0x9fdd), target_slot, 0, 0, cursor, *seeds, len(addresses))
        body = header + struct.pack('<B3H', ENTRIES[entry], word(before, word(before, 0x9796)), NONE, NONE)
        body += b''.join(struct.pack('<HH', *value) for value in registry)
        for address, slot in sorted(addresses.items(), key=lambda item: item[1]):
            size = 251 if word(before, address) in (0, 1, 2, 3, 19) else 55
            body += struct.pack('<H', slot) + before[address:address + size]
        self.cases.append(body)
        self.expected.append(output(before, after, actor, effect, -1 if invalid else 0))
        self.count += 1
        self.canonical_count += self.canonical is not None
        self.branches[effect['branch']] += 1
        if len(self.cases) >= 1024:
            self.flush()
        return after, effect

    def flush(self):
        if not self.cases:
            return
        self.path.write_bytes(struct.pack('<II', self.side, len(self.cases)) + self.pixels + b''.join(self.cases))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command, '--automatic-fire']
            if self.canonical:
                args.append(str(self.canonical))
            result = subprocess.run([*args, str(self.path)], capture_output=True, text=True, timeout=180)
            if result.returncode or result.stdout != expected:
                actual, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((index for index, (left, right) in enumerate(zip(actual, desired))
                                 if left != right), min(len(actual), len(desired)))
                raise AssertionError(f'{args[:2]} exit {result.returncode}: {result.stderr}; '
                                     f'full fire output differs at line {mismatch}: '
                                     f'{actual[mismatch:mismatch+1]} != {desired[mismatch:mismatch+1]}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.cases, self.expected = [], []


class FireTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='fist-automatic-fire-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        commands = []
        if TARGET in ('all', 'native'):
            commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_target_discovery_probe')])
        if TARGET in ('all', 'wasm'):
            commands.append(['node', str(BUILD / 'wasm/fist_target_discovery_probe.js')])
        owner = None
        cls.baseline = bytes(65536)
        if ORACLE:
            from original_automatic_fire_oracle import OriginalAutomaticFireOracle
            owner = OriginalAutomaticFireOracle()
            cls.baseline = bytes(owner.fresh().mem_read(DGROUP, 65536))
        cls.baseline = bytearray(cls.baseline)
        cls.baseline[0x9952:0x9956] = bytes(FIRE_THRESHOLDS)
        struct.pack_into('<HH', cls.baseline, 0xea2c, 0, 0x4000)
        cls.collector = Collector(commands, cls.temp.name, owner)

    def add(self, kind=0, entry=0xafa2, invalid=False, **options):
        return self.collector.enqueue(fixture(self.baseline, kind, **options), entry=entry, invalid=invalid)

    def tearDown(self):
        self.collector.flush()

    def test_01_complete_words_and_actor_flag_admission(self):
        for value in range(65536):
            kind = value % 4
            self.add(kind, actual=(value * 137) & 65535, requested=(value * 138) & 65535)
            self.add(kind, distance=value, behavior=value % 3,
                     phase=(0, 4, 40, 80, 140, 252)[value % 6], control=0)
            self.add(kind, distance=201, phase=value, actor_flags=8 if value & 256 else 0,
                     behavior=value % 3, control=0)
            self.add(kind, distance=201, phase=252, control=value)
            self.add(kind, behavior=value, distance=200, control=0)
            self.add(kind, behavior=3, opaque=value)
        for kind, entry, flags, target_flags, phase in itertools.product(
                range(4), (0xaf97, 0xafa2), range(256), (0, 16), range(4)):
            self.add(kind, entry, actor_flags=flags, target_flags=target_flags, phase=phase, control=0)
        for kind, entry, distance, difference, behavior in itertools.product(range(4),
                (0xaf97, 0xafa2), (0, 199, 200, 201, 65535),
                (0, 181, 182, 183, 65353, 65354, 65355, 65535), range(4)):
            self.add(kind, entry, distance=distance, requested=difference, behavior=behavior, control=0)

    def test_02_complete_rack_pairs_and_reserve_target_domains(self):
        for kind, first, second in itertools.product((1, 3), range(256), range(256)):
            reserves = (0, 1, 255)
            self.add(kind, states=(first, second), counts=(reserves[first % 3], reserves[second % 3]),
                     target_flags=16, selected=True, audible=True)
        for kind, rack, reserve, other, selected, audible in itertools.product(
                (1, 3), (0, 1), range(256), (0, 1, 255), (False, True), (False, True)):
            self.add(kind, states=(4, 1) if rack == 0 else (1, 4),
                     counts=(reserve, other) if rack == 0 else (other, reserve),
                     target_flags=16, selected=selected, audible=audible)
        for kind, reserve, flags, present in itertools.product((1, 3), (0, 1, 255), range(256), (False, True)):
            self.add(kind, 0x8711 if kind == 1 else 0x96c0, states=(4, 4), counts=(reserve, reserve),
                     target_flags=flags, present=present, selected=True, audible=True)

    def test_03_all_allocated_target_classes_and_orphans(self):
        for kind, target_kind, flags in itertools.product((1, 3), range(28), range(256)):
            self.add(kind, target_kind=target_kind, target_flags=flags, states=(4, 4),
                     counts=(1, 255), selected=True, audible=True)
        for kind, target_kind in itertools.product((1, 3), range(28)):
            self.add(kind, target_kind=target_kind, target_flags=16, states=(4, 4), orphan=True)

    def test_04_physical_capacity_holes_reserved_registry_and_wrapped_constructor(self):
        for kind, fill, counts, selected, audible in itertools.product((1, 3), (0, 118, 119, 120, 148, 149),
                ((1, 1), (255, 0)), (False, True), (False, True)):
            self.add(kind, fill=fill, counts=counts, states=(4, 4), target_flags=16,
                     selected=selected, audible=audible)
        for kind, pose, heading in itertools.product((1, 3),
                ((0, 0, 0), (-2**31, 2**31 - 1, 2**31 - 1), (2**31 - 1, -2**31, -2**31)),
                (0, 1, 16384, 32768, 65535)):
            self.add(kind, pose=pose, heading=heading, states=(4, 4), target_flags=16, audible=True)
        for kind in (1, 3):
            for hole in (1, 75, 149):
                data = fixture(self.baseline, kind, fill=149, states=(4, 4), target_flags=16)
                data[0xe2f7 + hole] = 0
                struct.pack_into('<HH', data, 0xdfbc + hole * 4, 0, 0)
                store(data, 0xe294, 149)
                self.collector.enqueue(data)
            data = fixture(self.baseline, kind, states=(4, 4), counts=(1, 2), target_flags=16,
                           selected=True, audible=True)
            for _ in range(4):
                data, _ = self.collector.enqueue(data)
        data = fixture(self.baseline, 1, states=(4, 4), target_flags=16)
        struct.pack_into('<HH', data, 0xdfbc + 4, 0, 65535)
        self.collector.enqueue(data)

    def test_05_used_probability_and_opaque_reference_failures(self):
        for behavior in (4, 255, 256, 32768, 65535):
            self.add(1, behavior=behavior, distance=201, control=0, invalid=True)
        for kind in range(4):
            self.add(kind, opaque=12345, invalid=True)
            self.add(kind, entry=0xaf97, opaque=12345, invalid=kind in (1, 3))
        for kind in (1, 3):
            entry = 0x8711 if kind == 1 else 0x96c0
            self.add(kind, entry, opaque=12345, states=(4, 4), target_flags=16, invalid=True)
            self.add(kind, entry, opaque=12345, states=(1, 1), target_flags=16)
            self.add(kind, entry, opaque=12345, counts=(0, 0), states=(4, 4), target_flags=16, selected=True)

    def test_06_all_original_prepared_missions(self):
        if not ORIGINALS:
            return
        from orders_contract import scenario_order_blocks
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        from test_units import records_from_scenario
        owner = self.collector.owner
        if owner is None:
            self.fail('Original prepared-world acceptance requires --oracle')
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            offset, height_name = 0, None
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size + 6
            self.assertEqual(offset, len(data))
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual((len(manifest), len(grouped)), (47, 8))
        worlds = actors_total = 0
        for height_name, missions in sorted(grouped.items()):
            info = terrains[height_name]
            encoded = (directory / height_name).read_bytes()
            self.assertEqual(hashlib.sha256(encoded).hexdigest(), info['sha256'])
            decoded = decoder.klc(encoded)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed, side)
                for name, data in missions:
                    machine, objects = owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                            scenario_order_blocks(data))
                    owner.reset(machine, pixels, side=side)
                    actors = [address for _, _, address, _ in objects.values() if word(owner.raw(machine, address), 0) < 4]
                    prepared = bytes(machine.mem_read(DGROUP, 65536))
                    for entry in (0xaf97, 0xafa2):
                        self.collector.begin(name, side, pixels)
                        machine.mem_write(DGROUP, prepared)
                        for actor in actors:
                            current = bytearray(machine.mem_read(DGROUP, 65536))
                            descriptor = word(current, 0x85a0 + current[actor + 27] * 2)
                            store(current, 0x9796, descriptor)
                            machine.mem_write(DGROUP, bytes(current))
                            actual, _ = owner.observe(machine, actor, entry=entry)
                            self.collector.enqueue(current, actor, entry, original_after=actual)
                    worlds += 1
                    actors_total += len(actors)
            print(f'Complete shared fire corpus: {height_name}', flush=True)
        self.collector.begin()
        self.assertEqual((worlds, actors_total, self.collector.canonical_count), (188, 3840, 7680))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--oracle', action='store_true')
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args, remaining = parser.parse_known_args()
    TARGET, BUILD, NATIVE_PROBE = args.target, args.build_root, args.native_probe
    ORACLE, ORIGINALS, REVIEW = args.oracle, args.originals, args.review_dir
    if ORIGINALS and not ORACLE:
        parser.error('--originals requires --oracle')
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__, *remaining], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped
    if success and REVIEW:
        collector = FireTests.collector
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'shared-automatic-fire.json').write_text(json.dumps({
            'success': True, 'target': TARGET, 'groups': program.result.testsRun,
            'skips': 0, 'calls': collector.count, 'original_returns': collector.original_count,
            'canonical_returns': collector.canonical_count, 'branches': dict(collector.branches),
            'output_sha256': collector.digest.hexdigest(), 'complete_wasm_streak': 0,
            'scope': 'Complete shared child fire/readiness/constructor; parent, flight and PCM remain open'
        }, indent=2) + '\n')
    raise SystemExit(not success)
