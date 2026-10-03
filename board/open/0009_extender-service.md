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
  returns EAX=`804`, ECX/EDX=`0`, EBX=`5`; actual de89 retains EAX=`804`.
  The caller subsequently loads/shifts its configuration WORDs before e2df
  posts op68 with full EBX=`5` and AX=`2`. Two natural
  handlers and the caller path are in a complete 32,009-fetch source trace.
  Three fresh 600-ms runs retain all39 original frames/27518 mixed samples.
  Source receipt: `tools/oracle/detail_loader_case.json`; reproduce with
  `python3 -B tools/oracle/capture_detail_loader.py --output
  /tmp/wasm-fist-detail-loader-source`. This accepts no port behavior:
  the shim still seeds a host DTA outside guest RAM and uses manual fread
  for op44; 0014 owns the stale caller/result transport. Loader raw flags,
  errors, instruction/device time and complete output remain unresolved.

- Consume0014's accepted patch637 saved config/overlay handles and existing
  read-CF lane. The actual startup no longer leaks SOUND.CFG/SOUNDDVR/
  MGAVIDEO. Seven real-DOS tests turn16 reaching parent failures into20
  target cases; the unfiltered124-test/six-startup/full178-flow gate passes0.
  Receipt: `tools/oracle/file_close_production_case.json`. op68 now carries
  low handle5, but its Native/WASM upperDWORD still differs. Actual op44
  FILEMGR/DTA initialization and full returned register transport remain
  unresolved; this does not accept the generic service or full output.

- Correct the earlier de89 EAX attribution with
  `tools/oracle/detail_return_case.json`. The old detail-loader fixture's
  `returned` row at6e00 is after MOV AX,[8b49], not the leaf return.
  Actual6032/76fc/e339/de89 retain EAX804 through the true6dfd caller
  boundary. The first configuration WORD2 shifts to1 for c008. A separate
  MOV AX,[8b4b] at6e05 loads4 and SHR AX,1 gives2 before e2df/op68.
  Full EBX5 and ECX/EDX0 persist through this path. The fresh complete
  32,009-fetch trace and39-frame/27518-mixed-sample baseline/caller pair
  preserve the600-ms endpoint. Reproduce:
  `python3 -B tools/oracle/capture_detail_return.py --output
  /tmp/wasm-fist-detail-return-source`. This source-only correction does
  not accept port transport, raw flags, whole memory or instruction/device
  time. Recover the actual configuration input instead of fitting AL2.

- Actual7660 detail/sky producer operands now have five fresh original
  cases in `tools/oracle/detail_operands_case.json`: natural, LOW, MEDIUM,
  HIGH and sky-off, with two reached6032 calls each. The selected files'
  complete2052 bytes match the module tables; returned EAX804/EBX5 and
  ECX/EDX0 are observed in every case. The default sky branch stores6877
  in3958 and BYTE1 in395c; nonzero sky selects689a and copies its BYTE.
  Full GP/segments/raw+lazyflags/controls are recorded at handler entry,
  loader entry and handler return. Controlled TCB writes affect exactly
  the requested detail/sky bytes and preserve architectural state/time.
  All39 frames/27518 mixed samples/end600ms equal a fresh unobserved
  baseline. Reproduce `python3 -B tools/oracle/capture_detail_operands.py
  --output /tmp/wasm-fist-detail-operands-source-final`. This source-only
  evidence does not accept port transport, loader interval memory/CF/errors,
  original instruction/device time or complete frame/audio sequences.

- Patch638 restores the original DWORD module DTA offset080b in927 and
  resolves every patched DTA+1a field access through the existing
  `fist_ext_addr` owner. The host-only buffer is removed. Four reaching
  parent failures (Native invalid pointer/WASM unrelated query size) become
  eight target cases with complete HIGH.DTL/DSOUNDS.BIN contents and
  unchanged legacy expectations. Actual Native constructor and both
  production startup dumps at cooperative tick120 retain080b; the initial
  reserved buffer/code bytes match the image. These dumps diagnose DTA
  storage/addressing, not original clock or whole-memory equality.
  The frozen1030-input unfiltered `run.6bEeNV` passes126 tests, exact
  patches, sequential builds, six startup cases and all178 existing flows,
  durable0; all419 original hashes/inventory are unchanged. Complete fresh
  30000-ms captures retain every2100 parent637 frame/end byte and43 sound
  register rows on both targets. All47 unmasked packets are recorded:
  Native op68 changes `f6e80005` to `f6680005`; WASM retains `00000005`.
  The upper-word producer is still unaccepted. Original palettes/layouts/
  times match;28 pixel failures start817/11704156us/byte8754, and strict
  comparison rejects absent final mixed port PCM. Receipt/reproduction:
  `tools/oracle/dta_production_case.json`; `python3 -B
  tools/capture_dta_startup.py --native /tmp/wasm-fist-638-complete/production/native
  --wasm /tmp/wasm-fist-638-complete/production/wasm/fistrun.js --node /usr/bin/node
  --output /tmp/wasm-fist-dta-startup`. Recreate the unfiltered gate with
  `FIST_WORKDIR=/tmp/wasm-fist-dta-check bash tools/check_flow.sh`.
  This accepts storage/addressing only. Actual op44 FILEMGR dispatch,
  resource archive paths, CF/errors, full GP/flags/farstack/IRQ/time and
  complete original output remain open; configuration AX/returned EBX
  transport belongs to0014 with the original operands above.

- Missing HIGH.DTL is now an original-backed reaching error case, not a
  hypothetical service inventory entry. `tools/oracle/file_error_case.json`
  proves CF1 at6044, the5e3a error branch, and nonlocal38e8→0f64→0f57→0f5d.
  Four observed0f64 fetches store DWORD reason1 at0d82, load full TCB EBX
  and store WORDffff in its status, preserving every other GP/segment/
  raw+lazyflag/control field. Complete16-MiB interval writes contain only
  those reason/status changes. The next fetch restores ESP from0f60;
  the nested6032/7660 stack is abandoned. A normal helper return cannot
  implement this contract. A fresh unobserved/observed error pair retains
  all38 frames/27518 mixed samples/end600ms. Reproduce with `python3 -B
  tools/oracle/capture_file_error.py --output /tmp/wasm-fist-file-error-source-final`.
  Both accepted638 production targets still expose the failure: complete
  error captures first differ at560798us/F event36/palette byte5 (170/0),
  and independent cooperative tick120 memory diagnostics retain task0/
  reason37fd rather thanffff/1. Their corrected DTA080b is preserved.
  These memory ticks are not a matched original CPU boundary. Receipt:
  `tools/oracle/file_error_port_case.json`; reproduce with `python3 -B
  tools/capture_file_error_ports.py --native /tmp/wasm-fist-638-complete/production/native
  --wasm /tmp/wasm-fist-638-complete/production/wasm/fistrun.js --node /usr/bin/node
  --original /tmp/wasm-fist-file-error-source-final/baseline/sequence
  --output /tmp/wasm-fist-file-error-ports-final`. This source/diagnostic
  step accepts no port fix, error inventory, caller/IRQ/device time or
  complete original frame/audio behavior.


- The second6032 CF boundary is now observed in the original too. The
  shared `capture_file_error.py --failure open` / `file_error.gdb` removes
  only the isolated HIGH.DTL at6065 after successful find-first. Both the
  control-only baseline and observed run preserve the entire16MiB RAM,
  architectural state and time across that filesystem action. Actual606a
  has CF1 before the3f read, branches to5e3a, and reaches the same nonlocal
  reason1/taskffff/ESP-restoration sequence. All38 frame records,27518
  mixed PCM samples and end600ms match between the pair. Whole-memory
  error writes and all preserved register/flag/control fields are verified
  again for both find/open cases. Receipts: `tools/oracle/file_error_case.json`
  and `tools/oracle/file_open_error_case.json`. Reproduce with `python3 -B
  tools/oracle/capture_file_error.py --failure find --output /tmp/wasm-fist-file-find-error-source`
  and `--failure open --output /tmp/wasm-fist-file-open-error-source`.
  `file_error_port_case.json` is freshly rebound to the find receipt using
  unchanged638 binaries; both still fail at560798us/F36/palette byte5 and
  retain task0/reason37fd in the independent tick120 diagnostics. This
  accepts original find/open branch evidence only, not a port correction.

- Pending patch639 reuses the existing5cc2/5d50 CF owner at6044 and606a.
  Real DOS I/O produces four parent target failures (find returns after
  consuming stale DTA; failed open issues3f). The candidate passes all six
  file-loader methods on both targets, preserving legacy file-content
  expectations. Tests observe actual0f64 entry before its unaccepted body.
  The frozen1046-input128-test/full178-flow gate is still running under
  `/tmp/wasm-fist-639-complete`; no production acceptance yet. Actual op44
  dispatch, nonlocal continuation and original output remain required.


- Original startup/KDV ownership is now observed end-to-end before any
  port repair: `tools/oracle/memmgr_startup_case.json`, reproduced with
  `python3 -B tools/oracle/capture_memmgr_startup.py --output /tmp/wasm-fist-memmgr-startup-source`.
  Actual84c0 starts with an empty registry and makes eight36bf calls;
  every EDX slot, ECX size, EBX alignment, AL flag and returned descriptor
  is recorded. The eighth is the zero-sized bc98 checkpoint. Whole16MiB
  tcb-clear→init-return writes contain only the TCB+488 zero-store.
  First6e95→3322→3661 sees nine registered blocks including the moved
  checkpoint, returns its found index in ESI and frees only that suffix.
  Actual3661 does not store the found index in2f60; the port's extra store
  is a transport shim, not an original memory write. The engine's far
  task at its actual DGROUP:ea2c and extender[0c93] resolve to the same
  physical task at KDV entry/lookup/free. The old Native comment claiming
  a separate original KDV TCB is disproved. All39 complete frame records,
  27518 mixed PCM samples and end600ms match a fresh unobserved original
  baseline. Snapshot GP/segments/raw+lazyflags/control/time and whole RAM
  are retained. Native638 independently reaches6e95→3322→3661→0f64(37fd)
  with zero registered blocks (`/tmp/wasm-fist-normal-file-error-caller/errors.jsonl`);
  original ownership comes from84c0 before program execution, whereas
  current Native setup runs it only at map-load. This accepts source
  startup/task ownership evidence only. Port initialization, full GP
  transport, false-error removal, nonlocal continuation, clocks/IRQ and
  complete original output remain open.

## Next

1. Recover the missing-file producer through actual6032/5cc2 and the
   nonlocal error boundary above. Preserve638's DTA storage/addressing;
   do not force CF clear, return ordinary success after a task error, or
   fit the observed palette deadline. Wire actual op44 FILEMGR and full
   returned EBX;0014 owns the independent configuration AX input.
2. Build a current inventory from the original table and the shim's branches. For each op record
   implemented/original-no-op/missing, the asm contract and a reaching scenario.
3. Run `FIST_OPHIST` across boot, missions, editor, campaign and link surfaces. A missing op in one
   battle's trace proves nothing about other surfaces.
4. Audit the remaining candidates first: 0x04/10c9, 0x68/76fd, 0x6c/77e2, 0x74/6f17, 0x7c/77a4.
   Confirm current handling and reachability before implementing. Mission sound 0x64 belongs to 0003.
5. Recommendation: keep dispatch and operation bodies separate, with explicit TCB inputs/outputs;
   port one observed operation at a time and compare its memory writes and return lanes.

## Accept

Every exposed operation is accounted for by asm and a test or a documented original no-op.
No missing reachable operation is silently accepted as success. Both targets and original traces match.
