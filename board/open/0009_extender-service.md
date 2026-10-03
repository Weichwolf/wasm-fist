Type: feature
Title: Every extender service the original exposes is implemented faithfully
Parent: 0001

## Contract

Dispatch extender operations with the original argument, result, register and memory effects.
Implement reachable services; distinguish an original no-op from an unimplemented return-zero shim.

## Evidence

The 32-bit service table is image offset 0xcb3, one dword per op/4; board:0021 preserves the map.
The old “0x8799 is the missing generic gate / intro is blank” narrative is superseded: 0x8799 is
inside graphics machinery, while op 0x78's table entry is 0x11dd. Intro and several later services
have implementations. Current code handles 0x08, 0x44, 0x50, 0x5c and 0x60; the old missing-op
inventory must not be used as a patch list.

- Original detail-loader provenance, production base `c4cf756`: startup `2b:a88`
  loads full EDX=`0000080b`, then `a8d` stores that DWORD in `ds:927`.
  The paging-aware physical address is `130927`, not the guest flat address.
  Both MOVs preserve every other GP/segment/control/raw+lazyflag field;
  complete 16-MiB snapshots differ only at the pointer slot. The natural
  `7660` op44 handler loads HIGH.DTL through actual6032. At `604a`, loading
  the full DTA pointer changes EBX=`f0010000` to `0000080b`; WORD MOV BX
  at `607c` then produces `00000005`. This is a FILEMGR operand contract,
  not a generic mode-gate rule for clearing register upper words. The loader
  returns EAX=`804`, ECX/EDX=`0`, EBX=`5`; actual de89 returns EAX=`2`
  independently before e2df posts op68 with full EBX=`5`. Two natural
  handlers and the caller path are in a complete 32,009-fetch source trace.
  Three fresh 600-ms runs retain all39 original frames/27518 mixed samples.
  Source receipt: `tools/oracle/detail_loader_case.json`; reproduce with
  `python3 -B tools/oracle/capture_detail_loader.py --output
  /tmp/wasm-fist-detail-loader-source`. This accepts no port behavior:
  the shim still seeds a host DTA outside guest RAM and uses manual fread
  for op44; 0014 owns the stale caller/result transport. Loader raw flags,
  errors, instruction/device time and complete output remain unresolved.

## Next

1. Build a current inventory from the original table and the shim's branches. For each op record
   implemented/original-no-op/missing, the asm contract and a reaching scenario.
2. Run `FIST_OPHIST` across boot, missions, editor, campaign and link surfaces. A missing op in one
   battle's trace proves nothing about other surfaces.
3. Audit the remaining candidates first: 0x04/10c9, 0x68/76fd, 0x6c/77e2, 0x74/6f17, 0x7c/77a4.
   Confirm current handling and reachability before implementing. Mission sound 0x64 belongs to 0003.
4. Recommendation: keep dispatch and operation bodies separate, with explicit TCB inputs/outputs;
   port one observed operation at a time and compare its memory writes and return lanes.

## Accept

Every exposed operation is accounted for by asm and a test or a documented original no-op.
No missing reachable operation is silently accepted as success. Both targets and original traces match.
