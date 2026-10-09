"""All pinned prepared missions at four details consume original physical contacts."""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct

from physical_contact_contract import predict
from ground_maneuver_contract import motion_obstacle
from orders_contract import scenario_order_blocks
from original_asset_oracle import OriginalAssetOracle
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_heightfield_oracle import OriginalHeightfieldOracle
from original_unit_oracle import DGROUP
from test_units import records_from_scenario
from test_original_ground_maneuver import SEEDS
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS, UC_X86_REG_EFLAGS)

ROOT = Path(__file__).resolve().parents[3]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--review-dir', type=Path, required=True)
REVIEW = parser.parse_args().review_dir.resolve()
if not REVIEW.is_relative_to(Path('/tmp')):
    parser.error('Disposable review evidence must live under /tmp')
REVIEW.mkdir(parents=True, exist_ok=True)
MODEL = Path(__file__).resolve().parents[2] / 'physical_contact_contract.py'
REGISTERS = (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
    UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)
owner = OriginalGroundManeuverOracle()
decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
directory = ROOT / 'armoredfist/FISTDATA'
assert len(manifest) == 47
assert sorted(p.name for p in directory.glob('*.FSG')) == sorted(manifest)
grouped = {}
for name, info in manifest.items():
    data = (directory / name).read_bytes()
    assert (len(data), hashlib.sha256(data).hexdigest()) == (info['size'], info['sha256'])
    offset = 0
    height_name = None
    while offset < len(data):
        tag, size = struct.unpack_from('<4sH', data, offset)
        if tag == b'BINF':
            height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
        offset += size + 6
    assert offset == len(data) and height_name in terrains
    grouped.setdefault(height_name, []).append((name, data))
assert len(grouped) == 8
counts = collections.Counter()
branches = collections.Counter()
missions = {}
digest = hashlib.sha256()


def check(machine, baseline, actor, selected, *, forced_tree=False):
    machine.mem_write(0, baseline)
    if forced_tree:
        # Declared consuming overlap in a genuinely restored/prepared world,
        # retaining the real tree payload/registry/RNG/height rather than copying
        # an authored fixture into a synthetic allocation.
        registry = [struct.unpack_from('<H', baseline, DGROUP + 0xdfbc + i * 4)[0] for i in range(182)]
        eligible = [p for p in registry if p not in (0, actor) and baseline[DGROUP + p + 0x16] & 64 and not baseline[DGROUP + p + 0x16] & 16]
        tree = next((p for p in eligible if struct.unpack_from('<H', baseline, DGROUP + p)[0] == 21), eligible[0] if eligible else None)
        if tree is not None:
            machine.mem_write(DGROUP + actor + 4, baseline[DGROUP + tree + 4:DGROUP + tree + 12])
            counts['overlap_candidate_type_' + str(struct.unpack_from('<H', baseline, DGROUP + tree)[0])] += 1
        else:
            counts['authored_no_obstacle_contexts'] += 1
        machine.mem_write(DGROUP + actor + 0x62, bytes([baseline[DGROUP + actor + 0x62] & 253]))
        machine.mem_write(DGROUP + actor + 0x93, b'\0')
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
    # Genuine prepared worlds start with no installed sound owner or prior
    # damage source. Keep those actual producer states, never substitute calls.
    assert struct.unpack_from('<H', baseline, DGROUP + 0x9fdf)[0] == 0
    assert struct.unpack_from('<H', baseline, DGROUP + 0xe3b2)[0] == 0
    entry = 0x1a0a4 if selected else 0xa631
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if selected else struct.pack('<H', 0xeff0))
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for reg, value in zip(REGISTERS, initial):
        machine.reg_write(reg, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial, near_obstacle=motion_obstacle)
    owner.execute(machine, entry, 0xeff0, 0xf69 if selected else 0)
    actual = bytes(machine.mem_read(0, 0x60000))
    if effect['near_obstacle_calls']:
        common_sp = 0x9000 if selected else 0x8ffc
        lo, hi = DGROUP + common_sp - 38, DGROUP + common_sp
        assert actual[:lo] == expected[:lo] and actual[hi:] == expected[hi:], ('semantic memory', actor, [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b and not lo <= i < hi][:24])
        compared = (1, 2, 4, 6, 7, 8, 9, 10)
    else:
        assert actual == expected, ('full memory', actor, [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:24])
        compared = tuple(range(11))
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    assert [observed[i] for i in compared] == [abi[i] for i in compared], ('ABI', actor, effect, observed, abi)
    counts['returns'] += 1
    counts['consuming_overlap_returns' if forced_tree else 'retained_saved_returns'] += 1
    counts['scratch_excluded_returns' if effect['near_obstacle_calls'] else 'full_memory_returns'] += 1
    for key in ('geometry_calls', 'registry_visits', 'high_word_skips', 'near_obstacle_calls', 'prediction_calls', 'prediction_samples', 'prediction_hits', 'tree_calls', 'tree_released'):
        counts[key] += effect[key]
    branches[effect['branch']] += 1
    digest.update(repr((actor, selected, forced_tree)).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


for height_name, mission_data in grouped.items():
    info = terrains[height_name]
    source = (directory / height_name).read_bytes()
    assert hashlib.sha256(source).hexdigest() == info['sha256']
    decoded = decoder.klc(source)
    assert hashlib.sha256(decoded).hexdigest() == info['decoded_sha256']
    plane = decoded.split(b'\n', 1)[1][768:]
    for side in (512, 1024, 2048, 4096):
        actual_side, pixels = scaler.resample(info['width'], plane, [side])
        assert actual_side == side
        for name, data in mission_data:
            machine, objects = owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0, scenario_order_blocks(data))
            owner.reset(machine, pixels, side=side)
            baseline = bytes(machine.mem_read(0, 0x60000))
            actors = [p for _, _, p, _ in objects.values() if int.from_bytes(baseline[DGROUP + p:DGROUP + p + 2], 'little') < 4]
            count_before = counts['returns']
            for actor in actors:
                for selected in (False, True):
                    check(machine, baseline, actor, selected)
                    check(machine, baseline, actor, selected, forced_tree=True)
            assert counts['returns'] - count_before == len(actors) * 4
            missions.setdefault(name, {})[str(side)] = {'ground_actors': len(actors), 'contact_returns': len(actors) * 4}
            counts['prepared_worlds'] += 1
            counts['ground_actors'] += len(actors)
    print(f'Original contact worlds: {height_name}, four details', flush=True)
assert counts['prepared_worlds'] == 188 and counts['ground_actors'] == 3840
assert counts['returns'] == 15360
assert counts['tree_calls'] > 0 and counts['prediction_calls'] > 0
assert sum(value for key, value in counts.items() if key.startswith('overlap_candidate_type_')) + counts['authored_no_obstacle_contexts'] == counts['consuming_overlap_returns']
assert len(missions) == 47 and all(set(details) == {'512', '1024', '2048', '4096'} for details in missions.values())
result = {'success': True, 'cases': counts['returns'], 'counts': dict(counts),
    'branches': dict(branches), 'missions': missions, 'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(owner.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scope': 'All47 original missions, eight genuine heights, four prepared details, every3840 retained ground context and selected/unselected contacts plus declared overlaps against actual prepared trees or other eligible obstacles, with explicit authored no-obstacle contexts; no tree is invented for treeless missions. Whole image/eleven register comparison except explicitly38-byte numeric stack and three scratch registers on predictive continuations. Initial null damage/sound producer states are preserved. This is contact/world coupling, not full class/tick/battle/source-sequence/PCM/shared C acceptance.'}
(REVIEW / 'contact-world-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
