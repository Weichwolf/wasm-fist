"""Used selector safety, genuine invalid DOS jumps and ignored-input boundaries."""
import collections
import hashlib
import json
from pathlib import Path
import struct

from manual_parent_original import setup, run, snapshot, REGISTERS, expected_memory
from manual_control_contract import predict, DRIVE_BANK, WEAPON_BANK
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from manual_turret_contract import predict as turret
from unicorn import UcError

ROOT = Path('/tmp/wasm-fist-0125-review')
oracle, machine, actors = setup()
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
digest = hashlib.sha256()
negatives = []

def prepare(kind, mode, action, flags=0, selected=True):
    actor = actors[kind]
    machine.mem_write(0, base)
    machine.mem_write(DGROUP+actor, snapshot(kind, action, 160, flags, 0))
    machine.mem_write(DGROUP+0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP+0x8b43, struct.pack('<H', mode))
    machine.mem_write(DGROUP+0x9000, struct.pack('<H', 0xeff0))
    values = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444, actor,
              0x1c00, 0x1c00, 0x9000, 0)
    for register, value in zip(REGISTERS, values):
        machine.reg_write(register, value)
    return actor, values

# Independent typed-bank domain rejects all unsafe used offsets before mutations.
# This is the proposed C repair boundary; DOS itself has no such checks.
for kind in range(4):
    for mode in range(65536):
        raw = snapshot(kind, 0, 0, 0, 0)
        valid = (mode & 32767) < 6
        try:
            result = predict(raw, True, mode, 0, (0, 18, 18), 0)
        except ValueError:
            assert not valid
            counts['rejected_drive_words'] += 1
        else:
            assert valid and result[3]['drive_mode'] == (mode & 32767)
            counts['accepted_drive_words'] += 1
        assert raw == snapshot(kind, 0, 0, 0, 0)
        digest.update(struct.pack('<HB?', mode, kind, valid))
    for action in range(256):
        raw = snapshot(kind, action, 0, 1, 0)
        try:
            predict(raw, True, 65535, 0, (0, 18, 18), 0)
        except ValueError:
            assert action not in (0, 2, 4, 6, 8)
            counts['rejected_action_bytes'] += 1
        else:
            assert action in (0, 2, 4, 6, 8)
            counts['accepted_action_bytes'] += 1
        digest.update(bytes((kind, action)))

for kind in range(4):
    # Word wrap aliases outside the complete bank can still accidentally RET.
    # Explicitly prove successful DOS completion is not sufficient domain evidence.
    for mode in (6, 32774, 4096, 65535):
        actor, values = prepare(kind, mode, 0)
        before = bytes(machine.mem_read(0, 0x60000))
        expected = bytearray(before)
        struct.pack_into('<H', expected, DGROUP+0x8ffe, 0xa59a)
        address = (0x976c+((mode*2)&65535)) & 65535
        assert struct.unpack_from('<H', before, DGROUP+address)[0] == (0xa59b if mode in (6, 32774) else 0)
        assert oracle.image[struct.unpack_from('<H', before, DGROUP+address)[0]] == 0xc3
        OriginalGroundManeuverOracle.execute(machine, 0xa57a, 0xeff0)
        assert bytes(machine.mem_read(0, 0x60000)) == expected
        assert tuple(machine.reg_read(r) for r in REGISTERS) == (values[0], 0, *values[2:9], 0x9002, 0)
        counts['out_of_bank_accidental_returns'] += 1
    for mode, action in ((0, 255),):
        actor, _ = prepare(kind, mode, action)
        before = bytes(machine.mem_read(0, 0x60000))
        pointer = struct.unpack_from('<H', before, DGROUP+(
            (0x976c+((mode*2)&65535))&65535 if action == 0 else 0x9778+action))[0]
        assert pointer not in (DRIVE_BANK if action == 0 else WEAPON_BANK)
        try:
            OriginalGroundManeuverOracle.execute(machine, 0xa57a, 0xeff0)
        except (UcError, RuntimeError) as error:
            negatives.append({'kind': kind, 'mode': mode, 'action': action,
                              'resolved_pointer': hex(pointer), 'error': str(error),
                              'post_memory_sha256': hashlib.sha256(bytes(machine.mem_read(0, 0x60000))).hexdigest()})
            counts['actual_invalid_failures'] += 1
        else:
            raise AssertionError(('Expected the pinned malformed case to fail, without accepting a partial return', kind, mode, action))
    # Mode7 resolves to the genuine left-turret wrapper instead of a drive
    # callback. After inhibition clears, holding it with action2 turns twice.
    actor, _ = prepare(kind, 7, 2, flags=1)
    run(machine, actor, True, 7)
    assert not int.from_bytes(machine.mem_read(DGROUP+actor+0x40, 2), 'little') & 1
    counts['inhibited_bad_mode_then_turn'] += 1
    try:
        run(machine, actor, True, 7)
    except ValueError:
        counts['next_used_bad_mode_rejected'] += 1
    else:
        raise AssertionError('The formerly ignored drive word became used without rejection')
    before = bytes(machine.mem_read(0, 0x60000))
    assert struct.unpack_from('<H', before, DGROUP+0x976c+14)[0] == 0xa59c
    # Independent valid one-turn full state/stack model, then the additional
    # identical left helper. The second callback overwrites all first-call
    # stack scratch here (control flag and target are already cleared).
    expected, ax, bx, _ = expected_memory(before, actor, True, 0)
    raw = bytes(expected[DGROUP+actor:DGROUP+actor+251])
    twice, _, _ = turret(raw, 88, False)
    expected[DGROUP+actor:DGROUP+actor+251] = twice
    OriginalGroundManeuverOracle.execute(machine, 0xa57a, 0xeff0)
    assert bytes(machine.mem_read(0, 0x60000)) == expected
    assert tuple(machine.reg_read(r) for r in REGISTERS) == (ax, bx, 0x1111, 0x2222, 0x3333, 0x4444,
                                                            actor, 0x1c00, 0x1c00, 0x9002, 0)
    counts['actual_held_bad_mode_double_turns'] += 1
    digest.update(twice)
assert counts == {'rejected_drive_words': 262096, 'accepted_drive_words': 48,
                  'rejected_action_bytes': 1004, 'accepted_action_bytes': 20,
                  'out_of_bank_accidental_returns': 16, 'actual_invalid_failures': 4,
                  'inhibited_bad_mode_then_turn': 4, 'next_used_bad_mode_rejected': 4,
                  'actual_held_bad_mode_double_turns': 4}, counts
receipt = {'success': True, 'cases': sum(counts.values()), 'counts': dict(counts),
           'negative_evidence': negatives, 'output_sha256': digest.hexdigest(),
           'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
           'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
           'contract_sha256': hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest(),
           'scope': 'Proposed typed complete-bank used-domain repair proved independently on all mode words/action bytes. Genuine unchanged DOS action255 fetch failures retained (no partial returns accepted); mode6/32774/4096/65535 accidental RETs explicitly distinguished from valid bank entries. Inhibited mode7 becomes used after manual turn clears inhibition and provably executes the wrong callback, causing a double turret turn; whole memory and eleven-register ABI independently match that erroneous composition. Whole unused/inhibited domains are separately required by full parent fixture. This checkpoint does not claim implemented C transactions or full UI/hardware acceptance.'}
(ROOT/'manual-invalid-selectors.json').write_text(json.dumps(receipt, indent=2)+'\n')
print(json.dumps(receipt), flush=True)
