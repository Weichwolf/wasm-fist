"""Complete original navigation throttle after explicit target-loss repair.

Full ad2f and its gear callback execute unchanged. Declared +90=2 preserves
manual gear selection; no unrelated voice/device path is replaced.
"""
import hashlib
import struct
import time

from ground_bearing_contract import bearing
from original_ground_bearing_oracle import OriginalGroundBearingOracle
from original_ground_command_oracle import OriginalGroundCommandOracle
from original_ground_goal_oracle import OriginalGroundGoalOracle
from original_unit_oracle import DGROUP
from test_ground_goal import assign, sample
from test_original_ground_bearing import actor


def verify_navigation():
    from unicorn.x86_const import UC_X86_REG_DI
    owner = OriginalGroundBearingOracle()
    machine = owner.machine((1, 2, 32768, 65535), 3)
    dispatch = (0xad62, 0xad8c, 0xae06, 0xaddb, 0xae07, 0xae25, 0xae26, 0xae2c)
    throttles = (96, 160, 208, 224)
    if struct.unpack_from('<8H', owner.image, DGROUP + 0x97f0) != dispatch:
        raise AssertionError('Original complete throttle bank differs')
    if struct.unpack_from('<4H', owner.image, DGROUP + 0x992c) != throttles:
        raise AssertionError('Original leader throttle choices differ')
    cycles = OriginalGroundCommandOracle().throttle_cycles()
    if len(cycles) != 64:
        raise AssertionError('Original throttle UI choices are incomplete')
    returns = 0
    digest = hashlib.sha256()
    started = time.monotonic()

    def throttle(raw, choice):
        nonlocal returns
        machine.mem_write(DGROUP + 0x7000, bytes(raw))
        machine.mem_write(DGROUP + 0x9796, struct.pack('<H', 0x85b6))
        machine.mem_write(DGROUP + 0x85bc, struct.pack('<H', choice))
        machine.reg_write(UC_X86_REG_DI, 0x7000)
        before = bytes(machine.mem_read(DGROUP, 65536))
        owner.call(machine, 0xad2f)
        after = bytes(machine.mem_read(DGROUP, 65536))
        if machine.reg_read(UC_X86_REG_DI) != 0x7000:
            raise AssertionError('Original throttle changed actor identity')
        offset = 0
        for begin, end in ((0x7057, 0x7059), (0x8fc0, 0x9002), (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original throttle changed unrelated actor/target/orders/RNG')
            offset = end
        flags, = struct.unpack_from('<H', raw, 0x40)
        distance, = struct.unpack_from('<H', raw, 0x53)
        if raw[0x43] == 0:
            speed = 0 if not flags & 2 else (80 if distance <= 8 else throttles[choice])
        elif not flags & 2 or distance <= 3 or distance == 65535:
            speed = 0
        else:
            speed = next(speed for limit, speed in
                         ((8, 16), (32, 32), (48, 128), (80, 240), (65535, 272))
                         if distance <= limit)
        expected = bytearray(raw)
        struct.pack_into('<H', expected, 0x57, speed)
        if after[0x7000:0x70fb] != expected:
            raise AssertionError('Complete original navigation throttle return differs')
        digest.update(expected)
        returns += 1
        return bytes(expected)

    for mode in (0, 2):
        for distance in range(65536):
            raw = bytearray(actor(distance % 4, flags=3, mode=mode))
            raw[0x90] = 2
            struct.pack_into('<H', raw, 0x53, distance)
            throttle(raw, distance % 4)
        for flags in range(65536):
            raw = bytearray(actor(flags % 4, flags=flags, mode=mode))
            raw[0x90] = 2
            struct.pack_into('<H', raw, 0x53, flags)
            throttle(raw, flags % 4)
        print(f'Complete original ad2f navigation {mode}: all range/control words', flush=True)

    goals = OriginalGroundGoalOracle()
    cases = stopped = 0
    for kind in range(4):
        for mode in (4, 6):
            for member in range(4):
                for presence in (0, 1, 2, 3):
                    for count in (0, 1, 32):
                        case = sample(kind, mode=mode, member=member, presence=presence,
                                      count=count, formation=5, flags=1)
                        raw, leader, present, descriptor, route, seeds, cursor = case
                        raw = bytearray(raw)
                        raw[0x43] = 0 if member == 0 else 2
                        raw[0x90] = 2
                        # Deliberate repair input, not an original null-target
                        # result: invalidate the old goal before shared assignment.
                        raw[0x40] &= 255 ^ 2
                        struct.pack_into('<H', raw, 0x97, 0)
                        fixed = (bytes(raw), leader, present, descriptor, route, seeds, cursor)
                        result = goals.cases([fixed])[0]
                        if result != assign(fixed)[1]:
                            raise AssertionError('Original repaired navigation goal differs')
                        machine.mem_write(DGROUP + 0x7000, result)
                        result = owner.direction(machine, 0x7000, raw[0x1b])
                        if result != bearing(assign(fixed)[1], descriptor):
                            raise AssertionError('Original repaired navigation bearing differs')
                        output = throttle(result, 0)
                        cases += 1
                        stopped += struct.unpack_from('<H', output, 0x57)[0] == 0
    return {'scope': 'full original ad2f navigation modes; declared manual gear +90=2',
            'dispatch': list(dispatch), 'leader_throttles': list(throttles), 'ui_cycles': len(cycles),
            'complete_ad2f_returns': returns, 'deliberate_repair_goal_bearing_throttle_cases': cases,
            'stopped_without_goal': stopped, 'output_sha256': digest.hexdigest(),
            'seconds': time.monotonic() - started, 'rng_unchanged': True}
