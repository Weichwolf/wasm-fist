#!/usr/bin/env python3
"""Reject complete checkpoint evidence with missing output or coherent CPU/RAM defects."""
from pathlib import Path
import argparse,copy,json,os,shutil,struct,tempfile
from check_device_checkpoint_transitions import check,digest
from verify_device_checkpoint import physical_owner

def negatives(root,repo,output):
 source=check(root,repo);output.mkdir(parents=True,exist_ok=False)
 rows=[json.loads(line) for line in (root/'source/prefix-fetches.jsonl').read_text().splitlines()]
 q=rows[242];memory=(root/'source/3322.memory').read_bytes()
 physical=physical_owner(repo)((q['segments'][3]['base']+0x2f60)&0xffffffff,memory,q)
 cases=('missing-entry-memory','short-terminal-memory','missing-mixed-PCM','extra-index-RAM-publication','lost-INC-carry','changed-INC-var2','corrupted-mid-register','wrong-RET-stack-width')
 results=[]
 for kind in cases:
  with tempfile.TemporaryDirectory(dir=output,prefix=kind+'-') as temporary:
   clone=Path(temporary);shutil.copytree(root/'source',clone/'source',copy_function=os.link,ignore=shutil.ignore_patterns('game'))
   shutil.copytree(root/'baseline',clone/'baseline',copy_function=os.link,ignore=shutil.ignore_patterns('game'))
   for name in ('source.exit','baseline.exit','original-hashes.json','producers.json','instructions-probe'):shutil.copy2(root/name,clone/name)
   altered=copy.deepcopy(rows)
   if kind=='missing-entry-memory':(clone/'source/3322.memory').unlink()
   elif kind=='short-terminal-memory':
    p=clone/'source/780e.memory';data=p.read_bytes();p.unlink();p.write_bytes(data[:-1])
   elif kind=='missing-mixed-PCM':(clone/'source/sequence.pcm').unlink()
   elif kind=='extra-index-RAM-publication':
    p=clone/'source/780e.memory';data=bytearray(p.read_bytes());struct.pack_into('<I',data,physical,7);p.unlink();p.write_bytes(data)
   elif kind=='lost-INC-carry':
    for q in altered[253:257]:q['cpu_regs.flags']&=~1
   elif kind=='changed-INC-var2':
    for q in altered[253:255]:q['lflags.var2.dword[0]']=1
   elif kind=='corrupted-mid-register':
    for q in altered[296:301]:q['registers'][1]|=0x10000
   elif kind=='wrong-RET-stack-width':altered[-1]['registers'][4]-=2
   else:raise AssertionError(kind)
   if altered!=rows:
    p=clone/'source/prefix-fetches.jsonl';p.unlink();p.write_text(''.join(json.dumps(q)+'\n' for q in altered))
    for p in (clone/'source').glob('*.json'):
     selected=[q for q in altered if q['cpu_regs.ip.dword[0]']==int(p.stem,16)]
     assert len(selected)==1;p.unlink();p.write_text(json.dumps(selected[0],indent=2)+'\n')
   try:check(clone,repo)
   except (AssertionError,FileNotFoundError,ValueError) as error:
    results.append(dict(negative=kind,rejected=True,error_type=type(error).__name__,reason=str(error)))
   else:raise AssertionError('Accepted corrupted checkpoint evidence: '+kind)
 proof=dict(scope='Eight coherent CPU/RAM/missing-output negatives for complete original checkpoint source contract; no port implementation acceptance.',cases=results,source_proof_sha256=digest(root/'proof.json'),transition_proof_sha256=digest(root/'transitions-proof.json'),verifier_sha256=digest(repo/'tools/oracle/verify_device_checkpoint.py'),transition_checker_sha256=digest(repo/'tools/oracle/check_device_checkpoint_transitions.py'),negative_checker_sha256=digest(__file__),retired_clone_directories=True)
 (output/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS:all eight checkpoint evidence corruptions rejected; temporary clones removed')
 return proof


def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--repo',type=Path,required=True)
 parser.add_argument('--source',type=Path,required=True)
 parser.add_argument('--output',type=Path,required=True)
 args=parser.parse_args()
 assert args.output.resolve().is_relative_to(Path('/tmp'))
 return negatives(args.source.resolve(strict=True),args.repo.resolve(strict=True),args.output.resolve())


if __name__=='__main__':main()
