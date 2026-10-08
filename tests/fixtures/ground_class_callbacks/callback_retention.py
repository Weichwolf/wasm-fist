"""Complete class start/readiness payloads retain the three recovered byte counters."""
import hashlib
import json
from pathlib import Path
import struct

from original_mission_ready_oracle import RESET_BANK
from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle
from test_original_mission_ready import ground_reset
from test_vehicle_start import initialized

FIELDS = (0x37, 0xa6, 0x52)
oracle = OriginalVehicleStartOracle()
digest = hashlib.sha256()
counts = {'class_start': 0, 'readiness': 0}
for link in (0, 2):
    records = []
    for kind in range(4):
        for value in range(256):
            raw = bytearray((i * 37 + value) & 255 for i in range(251))
            struct.pack_into('<H', raw, 0, kind)
            for field, byte in zip(FIELDS, (value, (value * 13) & 255, (255 - value))):
                raw[field] = byte
            records.append((len(records), 1, bytes(raw)))
    expected, random = initialized(records, (1, 2, 32768, 65535), 0, link)
    actual, end = oracle.initialize(records, (1, 2, 32768, 65535), 0, link)
    assert actual == expected and end == random
    for original, result in zip(records, actual):
        assert all(original[2][field] == result[2][field] for field in FIELDS)
        digest.update(result[2])
        counts['class_start'] += 1
    machine = oracle.machine((1, 2, 32768, 65535), 0, link)
    di, *_ = oracle.machine_registers()
    for _, _, raw in records:
        machine.mem_write(DGROUP + 0x7000, raw)
        machine.reg_write(di, 0x7000)
        oracle.call(machine, RESET_BANK[struct.unpack_from('<H', raw)[0]])
        actual = bytes(machine.mem_read(DGROUP + 0x7000, 251))
        assert actual == ground_reset(raw, link)
        assert all(raw[field] == actual[field] for field in FIELDS)
        assert machine.reg_read(di) == 0x7000
        assert oracle.random_state(machine) == ([1, 2, 32768, 65535], 0)
        digest.update(actual)
        counts['readiness'] += 1
assert counts == {'class_start': 2048, 'readiness': 2048}
receipt = {
    'success': True, 'cases': 4096, 'counts': counts,
    'scope': 'All four complete original class-start/readiness payloads, both declared link inputs, all values of three recovered byte counters, return ABI and RNG checked. Not full class/global/scheduling acceptance.',
    'fields': list(FIELDS), 'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'reproduce': 'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-callback-retention.py'}
Path('/tmp/wasm-fist-0119-class-research/callback-retention.json').write_text(json.dumps(receipt, indent=2)+'\n')
print(json.dumps(receipt), flush=True)
