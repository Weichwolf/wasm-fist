"""Complete manual banks on all pinned saved actors; no full-world claim."""
import collections
from manual_control_contract import predict
import hashlib
import itertools
import json
from pathlib import Path
import struct

from manual_parent_original import setup, run
from original_unit_oracle import DGROUP
from test_units import records_from_scenario

ROOT = Path('/tmp/wasm-fist-0125-review')
PROJECT = Path.cwd()
manifest_path = PROJECT / 'tests/scenario_originals.json'
manifest = json.loads(manifest_path.read_bytes())
directory = PROJECT / 'armoredfist/FISTDATA'
assert len(manifest) == 47
assert sorted(p.name for p in directory.glob('*.FSG')) == sorted(manifest)
oracle, machine, actors = setup()
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
kinds = collections.Counter()
saved_actions = collections.Counter()
digest = hashlib.sha256()
files = {}
for name, metadata in sorted(manifest.items()):
    path = directory / name
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == metadata['sha256']
    assert not path.stat().st_mode & 0o222
    inventory = 0
    for _, _, source in records_from_scenario(data):
        kind, = struct.unpack_from('<H', source)
        if kind >= 4:
            continue
        assert len(source) == 251
        actor = actors[kind]
        saved_actions[source[0xa0]] += 1
        kinds[kind] += 1
        inventory += 1
        for mode, action, admission in itertools.product(range(6), (0, 2, 4, 6, 8),
                                                         ('selected', 'inhibited', 'unselected')):
            machine.mem_write(0, base)
            raw = bytearray(source)
            # Preserve the real allocated physical index, as original restoration does.
            raw[2:4] = base[DGROUP+actor+2:DGROUP+actor+4]
            raw[0xa0] = action
            flags, = struct.unpack_from('<H', raw, 0x40)
            if admission != 'unselected':
                flags = flags | 1 if admission == 'inhibited' else flags & ~1
                struct.pack_into('<H', raw, 0x40, flags)
            machine.mem_write(DGROUP+actor, bytes(raw))
            machine.mem_write(DGROUP+0x452, struct.pack('<H', 65535))
            machine.mem_write(DGROUP+0x9600, struct.pack('<3H', 0, 18, 363))
            machine.mem_write(DGROUP+0x9746, struct.pack('<H', 32768))
            actual, _ = run(machine, actor, admission != 'unselected', mode)
            digest.update(name.encode()+bytes((kind, mode, action))+admission.encode()+raw+actual)
            counts[admission] += 1
    assert hashlib.sha256(path.read_bytes()).hexdigest() == metadata['sha256']
    files[name] = inventory
    print(name, inventory, dict(counts), flush=True)
assert sum(files.values()) == 960 and len(files) == 47
assert kinds == {0: 179, 1: 131, 2: 396, 3: 254}
assert saved_actions == {0: 960}
assert counts == {'selected': 28800, 'inhibited': 28800, 'unselected': 28800}
receipt = {'success': True, 'cases': sum(counts.values()), 'counts': dict(counts),
           'files': files, 'actors': 960, 'classes': dict(kinds),
           'saved_action_bytes': dict(saved_actions), 'output_sha256': digest.hexdigest(),
           'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
           'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
           'contract_sha256': hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest(),
           'manifest_sha256': hashlib.sha256(manifest_path.read_bytes()).hexdigest(),
           'scope': 'All 47 read-only pinned authored saved sources / 960 ground actors through complete unchanged manual parent and both banks, selected/inhibited/unselected. Whole 0x60000 memory and eleven-register ABI independently predicted on genuinely allocated matching-class actors. Saved payloads preserve actual constructor physical index; explicit caller admission/actions/clock/control inputs above. No full mission preparation, full class or device acceptance.'}
(ROOT/'manual-parent-corpus.json').write_text(json.dumps(receipt, indent=2)+'\n')
print(json.dumps(receipt), flush=True)
