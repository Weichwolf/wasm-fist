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

- Patch639 now consumes the original6032 CF at6044 and606a from the
  existing5cc2/5d50 owner. Four real-DOS-I/O parent target failures become
  four corrected find/open cases; all eight existing complete-file cases
  retain their expectations. The six loader methods exercise12 Native/WASM
  cases and observe actual0f64 entry before its unaccepted body. The frozen
  1046-input unfiltered `run.fWjgrl` passes128 tests, exact patches, both
  sequential builds, six startup cases and all178 flows with durable exit0;
  all419 original hashes/inventory are unchanged. Every2100 frame/end byte,
  43 sound-register rows and all47 unmasked service packets retain638's
  behavior. Original palettes/layouts/times match, but28 pixel failures
  still start817/11704156us/byte8754; final mixed port PCM remains absent.
  Native/WASM full op68 inboxes remain f6680005/00000005. Fresh complete
  missing-HIGH production captures still first fail at560798us/F36/palette
  byte5, with engine task0/reason37fd in separate tick120 diagnostics:
  manual op44 still bypasses6032. Receipt:
  `tools/oracle/file_carry_production_case.json`; reproduce with
  `FIST_WORKDIR=/tmp/wasm-fist-file-carry-check bash tools/check_flow.sh`.
  This accepts both loader CF branches only. Actual op44, resource archive
  paths, full GP/configuration transport, task WORD/error stack/main-loop
  continuation, clocks/IRQ and complete original output remain open.

- Original find/open error continuation is now recovered beyond0f5d:
  `tools/oracle/file_error_main_case.json`, reproduced by
  `python3 -B tools/oracle/capture_file_error_main.py --output /tmp/wasm-fist-file-error-main-source`.
  The GDB extension reuses the existing file-error CF/body/page-walk owner;
  its separate caller trace retains281 actual fetches and eight full
  GP/segments/raw+lazyflags/control/time plus whole16MiB boundaries per
  failure. Actuale339 jumps through the original DGROUP:58 vector into
  runtime-relocated CRT314. Whole RAM CRT-entry→00e0-resume changes only
  two stack WORDs and BYTE[DGROUP:6a]=ff. The saved near-return word and
  PUSHSS/POPDS overwrite follow the fetched instructions; f69e rebases SS
  while preserving the stack's physical address. Full register upper
  halves are preserved, with AX/BX/SP derived from the original segment
  arithmetic rather than copied observed constants. Actual5c5f executes
  STC, returning CF1 to00e5: JAE skips00d8 and reaches00e7 shutdown.
  Both independent complete error pairs retain38 frames/27518 mixed
  samples/end600ms. Relocated code is checked against FIST.DAT image bytes.
  This accepts original continuation evidence only. The port still lacks
  the nonlocal service boundary, CRT/main-loop resume, real poll carry and
  full GP/flags/stack/instruction/device-time transport. Returning normally
  from6032, or using AL0 as the carry result, does not implement this path.

- Accepted patch641 restores actual0f64's WORDffff task store without
  changing the byte-addressed pointer owner. The reaching regression has
  two methods/four Native/WASM parent failures and no setup errors; all15
  file-test methods pass after the fix. Real DOS find/open/CF and actual
  post-store observation preserve existing command expectations and guarded
  destinations; the exact4096-byte task dump changes only its firstWORD.
  Real task inputs stay zero, with bytes2..3 guarded: original5ce5 reads
  TCB+496 as an alternate drive. The earlier whole-task A5 guard incorrectly
  changed that input and is superseded. Receipt:
  `tools/oracle/file_error_status_production_case.json`; reproduce with
  `FIST_WORKDIR=/tmp/wasm-fist-error-status-check bash tools/check_flow.sh`.
  The frozen1061-input unfiltered `run.RGoCka` under
  `/tmp/wasm-fist-641-complete` passes132 tests, exact patches, sequential
  Native/WASM builds, six startup cases and all178 flows with durable0.
  All419 original hashes/inventory stay unchanged. The independentf2d01ac
  source-only sound-bank commit changes none of these running inputs.
  Every2100 fresh30000-ms frame/end byte,43 sound-register rows and all47
  unmasked service packets retain640 behavior. Native/WASM op68 remains
  e6480005/00000005. All original palettes/layouts/times match, but28 pixel
  failures still start817/11704156us/byte8754 and final mixed port PCM is
  absent. Complete600-ms startup captures retain healthy shared task0 and
  no Native0f64 callback. Missing-HIGH production still first differs at
  560798us/F36/palettebyte5, with actual task/reason0: manualop44 bypasses
  6032. This accepts the task-store width only; no nonlocal service/CRT/
  main-loop continuation, full GP/flags/stack/IRQ/time or original full
  output acceptance follows.

- Source-only dispatcher stack proof at parent04e571d:
  `tools/oracle/file_service_stack_case.json` and
  `python3 -B tools/oracle/capture_file_service_stack.py --output /tmp/wasm-fist-file-service-stack-source`.
  Fresh four isolated600-ms original runs plus `--verify-only` pass. Both
  find/open observations retain all38 frames/27518 mixed samples/end600ms
  against their complete error baselines. The observer shares0009's existing
  error/main state, RAM and guest-page translation owners. Nine actual
  fetch boundaries begin at0f30, not the nominal decompiler label0f23;
  no execution or time is inferred for the preceding marker padding.
  Actual0f30 saves DWORD ESP at0f60, MOVZX EBX/BX selects44, DWORD[cb3+44]
  supplies near handler10da, and DWORD[caf] receives that target. The
  dispatcher then loads the shared task and its full DWORD inbox before
  near CALL[caf] and handler10da's near CALL7660. Eight individual fetched
  instructions preserve complete GP/segments/raw+lazyflags/control state
  except their specified GP changes; whole16MiB transitions contain only
  the exact saved-stack/handler DWORD stores and near return pushes.
  The loader6032 is12 stack bytes below entry. Errorf57 restores actual
  entry ESP; f5d consumes its unchanged8-byte far frame, whose CS DWORD
  has nonzero upper padding. The first caller selects the low CS WORD and
  advances ESP by8. Original incoming EBXf0010044 becomes selector44 only
  at MOVZX; handler inboxf0010005 is retained in full. These are observed
  values, not replacement constants. Complete error/CRT/main continuation
  proof is embedded. Four negative probes independently reject a corrupt
  saved ESP store, corrupt near return push, corrupt fetched CALL opcode
  and missing full RAM boundary. Canonical raw proof:
  `/tmp/wasm-fist-file-service-stack-public/`; compact negative receipt:
  `/tmp/wasm-fist-file-service-stack-negative/proof.json`. No port dispatch,
  nonlocal continuation, instruction/device time or complete output
  acceptance follows from this original evidence.

- Actual production op44 FILEMGR step at parent28f4cc1:
  `tests/test_detail_service.py` executes the actual shim branch and real
  6032/DOS/clock against isolated LOW/MEDIUM/HIGH files. Eight reaching
  parent failures become eight successful target/settings cases, verifying
  complete guarded2052-byte tables, real size/handle/byte count and six
  DOS commands. Original7660 filename offsets and default6877 sky selection
  feed the loader; map-load consumes those settings. The selector remains
  available to e339's actual task-error test.
  The frozen1070-input full gate
  `/tmp/wasm-fist-detail-filemgr-candidate/verify/run.lJ9mV1` passes133 tests
  in283.688s, exact patches, sequential Native/WASM builds, six startup
  cases and all178 existing flows with zero failures; gate/runner and all
  additional600-ms/30s captures terminate0. Staged engine/extender bytes
  equal641, all419 originals remain unchanged. Portable scoped receipt:
  `tools/oracle/detail_service_production_case.json`.
  Complete normal30s streams retain every parent641 frame/end byte, all43
  sound-register rows and47 unmasked packets. Both2100-frame streams agree,
  all original palettes/layouts/times match and the first10s video is equal;
  the same28 pixel failures begin817/11704156us/byte8754. Strict comparison
  still rejects missing final mixedPCM. Fresh actual missing-HIGH runs now
  reach reason1 and WORD taskffff on both targets, versus parent641's0/0.
  Their tick120 diagnostics do not equal the original CPU boundary;
  complete error output still has39 versus38 frames, first failure
  560798us/F36/palettebyte5. Ordinaryf64 return/CRT restart remains wrong.
  This accepts normal detail FILEMGR/table/size/handle/sky ownership only.
  Complete incoming/returned GP, raw/lazy flags, stack and instruction/device
  time are open; CX/DX/DI gate lanes do not prove original incoming values.
  Engine6de2 still drops the returned EBX and independent configuration AX.

## Next

1. Consume0025's accepted640 startup allocations/shared task and preserve
   the reaching regressions for the removed false normal37fd. Drive
   actual op44 FILEMGR/full returned EBX and0f64→0f57→0f5d/main-loop
   continuation. Preserve638 DTA, accepted639 CF branches and641's original
   WORDffff task store;0014 owns the independent configuration AX
   input. Require complete original error output as well as the normal
   sequence; a normal error-helper return does not implement the source.
   Consume the actual0f30 dispatcher/near-stack/full-inbox proof above;
   do not use a far16:16 callback, invent an ESP, clear register upper
   halves or charge time for unobserved0f23 marker padding.
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
