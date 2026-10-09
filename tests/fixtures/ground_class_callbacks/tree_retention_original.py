"""Read-only saved-tree health retention through actual import/full readiness."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_object_pool_oracle import REGISTRY
from original_unit_oracle import DGROUP
from test_original_command_boundary import chunks
from test_original_mission_ready import complete_expected
from test_units import records_from_scenario

ROOT=Path(__file__).resolve().parents[3]
REVIEW=None
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--review-dir',type=Path,required=True)
REVIEW=parser.parse_args().review_dir.resolve()
if not REVIEW.is_relative_to(Path('/tmp')):
 parser.error('Disposable review evidence must live under /tmp')
REVIEW.mkdir(parents=True,exist_ok=True)
MODEL=Path(__file__).resolve().parents[2]/'physical_contact_contract.py'
path=ROOT/'armoredfist/FISTDATA/TRAIN1.FSG'
data=path.read_bytes()
pin=json.loads((ROOT/'tests/scenario_originals.json').read_bytes())[path.name]
assert (len(data),hashlib.sha256(data).hexdigest())==(pin['size'],pin['sha256'])
assert path.stat().st_mode&0o222==0
blocks=chunks(data)
registry,generation,original=next(record for record in records_from_scenario(data)
    if struct.unpack_from('<H',record[2])[0]==21)
oracle=OriginalMissionReadyOracle()
counts={'restored_payloads':0,'complete_ready_returns':0,'height_transfers':0,'rng_draws':0}
digest=hashlib.sha256()
for link,variant,damage in itertools.product((0,2),range(4),range(256)):
    saved=bytearray(original)
    saved[0x16]&=223
    saved[0x19]=variant
    saved[0x1a]=damage
    saved=bytes(saved)
    machine,objects=oracle.prepare_saved([(registry,generation,saved)],
        (1,2,32768,65535),variant,link,(blocks[b'PATH'],blocks[b'PINF']))
    assert len(objects)==1 and set(objects)=={0}
    index,value,pointer,size=objects[0]
    assert (index,value,size)==(registry,generation,55)
    expected_restored=saved[:2]+struct.pack('<H',0)+saved[4:]
    assert bytes(machine.mem_read(DGROUP+pointer,55))==expected_restored
    counts['restored_payloads']+=1
    pixels=bytes((31,67,123,241)) if link==0 else bytes((241,123,67,31))
    before,actual,transfers=oracle.reset(machine,pixels)
    expected,consumed,released,artillery=complete_expected(before,transfers)
    assert consumed==2 and released==[] and artillery==[[],[]]
    assert len(transfers)==1 and transfers[0][:2]==(registry,pointer)
    assert actual[pointer:pointer+55]==expected[pointer:pointer+55]
    assert actual[pointer+0x1a]==damage
    assert actual[0x1f82:0x1f8c]==expected[0x1f82:0x1f8c]
    assert actual[0xdfbc:0xe3ad]==expected[0xdfbc:0xe3ad]
    assert actual[0x930a:0x930c]==expected[0x930a:0x930c]==struct.pack('<H',1)
    assert oracle.orders.blocks(machine)==(blocks[b'PATH'],blocks[b'PINF'])
    for address,size in ((0xa022,150*55),(0xc05c,32*251)):
        assert actual[address:address+size]==expected[address:address+size]
    counts['complete_ready_returns']+=1
    counts['height_transfers']+=len(transfers)
    counts['rng_draws']+=consumed
    digest.update(bytes([link,variant,damage])+actual[pointer:pointer+55])
assert path.read_bytes()==data and path.stat().st_mode&0o222==0
receipt={'success':True,'cases':counts['complete_ready_returns'],'counts':counts,
    'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'model_sha256':hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scenario':path.name,'scenario_sha256':pin['sha256'],'source_tree_registry':registry,
    'scope':'Retention reference only: first actual TRAIN1 tree, all256 saved+1a bytes, four authored variants, both link boundaries and declared original two-by-two height planes. Actual saved allocation/continuation/order import/full d755 readiness and genuine kernel op54 transfer. Independent entire restored/tree/all-arena payloads, RNG/pool/census and standard readiness return ABI; unchanged actual PATH/PINF. No independent whole memory/eleven-register ABI, full all47/contact/source-producer/lifetime/C/class/game acceptance.'}
(REVIEW/'tree-retention-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
