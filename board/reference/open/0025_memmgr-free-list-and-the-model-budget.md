Type: bug
Title: The memory manager preserves original allocation, relocation and swap behavior
Parent: 0001

## Contract

Load the original model/sprite set under the same memory configuration, preserve contents through
resize/compaction, and implement the original exhaustion/swap outcomes without stand-ins.

## Evidence

- Sorted lists have low(base=0) and high(base=ffff) sentinels; a self-linked low sentinel caused
  infinite insertion. Free total must equal the sum of free-node sizes.
- Model-budget/free-list recovery landed; patch 586 restored driver/sprite shrink operations,
  allowing M2CON.MRL to load. Patch 591 preserved relocation ES; patch 596 fixed purge accounting.
- The remaining 0c21 fallback is still a probe stub. Original 1c68 swaps to EMS (pool 16f6),
  1fcc to XMS (pool 1718); 0d2e brings a block back. Captured oracle pools were empty, explaining
  failure in those samples, not proving the services can be omitted for all configurations.
- The recorded conventional heap start differs by 0xad paragraphs between port and oracle.
  Overlay lookup must honor resized block ranges, not stale original load sizes.


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


- Fresh accepted639 production diagnostics now reproduce the startup
  failure on both targets without supplying allocator/CF results:
  `tools/oracle/memmgr_startup_port_case.json`; reproduce with
  `python3 -B tools/capture_memmgr_startup_ports.py --native /tmp/wasm-fist-639-complete/production/native --wasm /tmp/wasm-fist-639-complete/production/wasm/fistrun.js --node /usr/bin/node --output /tmp/wasm-fist-memmgr-port-parent`.
  Actual Native constructor and first6e95 have zero descriptors; the
  real6e95→3322→3661→0f64 stack raises37fd. Both complete600-ms outputs
  have39 identical frame records/end bytes, but the actual final current
  task is private rather than the engine far task and has WORD00ff/reason37fd.
  Native GDB observes its private memory directly; WASM's unchanged
  JS/WASM producer exposes its live linear heap through an exit observer.
  The WASM g_mem address is derived from a unique exact tick12016MiB
  guest/linear pair, then used for the same immutable producer's complete
 600-ms heap. This is an address/state diagnostic, not a matched original
  CPU boundary. The generated error helper uses an undefined1 pointer and
  writes only a BYTE; original0f64 stores WORDffff. Source0009's nonlocal
  continuation remains required. The original source receipt now also
  records healthy KDV task status from its whole-memory snapshot.

- Candidate startup/shared-task regression has three reaching parent
  failures with zero setup errors: Native lacks the eight actual startup
  allocations and both targets own the wrong KDV task. Isolated Native/
  WASM experiments using the real84c0 before KDV plus the engine far task
  remove the false37fd/00ff state and retain all complete600-ms parent
  frame/end bytes. Source first KDV still has an additional sound-bank allocation;
  no full table/GP/IRQ/time or production correction is accepted from this
  experiment. Keep the actual startup allocation, task ownership and
  follow-up error/audio contracts distinct. The earlier sky attribution is
  disproved: original77e2 calls6032 with the image's85a4 `dsounds.bin`, then
  actual36bf receives EDX85b0, ECX134240, EBX4 and AL3 and returns at7832. A fresh
  original baseline/observer pair preserves every39 frame/27518 mixed
  sample/end600ms byte. The earlier sky-named diagnostic is superseded by
  0003's portable `sound_bank_startup_case.json`: ten actual device/free/
  query/allocation/full-bank/checkpoint/KDV boundaries and complete paged
  asset verification. Sound-bank startup belongs to0003, not the sky
  renderer. Restore the actual service and DOS
  size/read contracts instead of inserting a fitted allocation.

- Accepted production step640 drives the existing84c0 in the module
  constructor and binds both KDV OPEN paths to the actual engine far task
  through one owner; the private task and filename copy are removed.
  `tests/test_memmgr_startup.py` has two passing methods (parent: three
  failures, no setup errors); all six previous loader methods still pass.
  The frozen1055-input unfiltered gate `run.p0jEr5` under
  `/tmp/wasm-fist-640-complete` passes130 tests, exact patches, sequential
  Native/WASM builds, six startup cases and all178 existing flows, durable0.
  All419 original hashes/inventory remain unchanged. Receipt:
  `tools/oracle/memmgr_initialization_production_case.json`; reproduce with
  `FIST_WORKDIR=/tmp/wasm-fist-startup-check bash tools/check_flow.sh`. Actual Native constructor descriptors match
  the source slot/size/alignment/AL flags and zero-size checkpoint. Both
  complete600-ms outputs use the engine task with status/reason0; Native
  has no0f64 callback. All2100 fresh30000-ms frame/end bytes and43 sound
  register rows retain639 output. Full47 packets remain unmasked: Native
  op68's upper word changes `f6680005` to `e6480005`, whereas WASM retains
  `00000005`; full GP transport is still unaccepted. The same28 original
  pixel failures start817 and strict comparison rejects missing final PCM.
  Missing-HIGH production still first differs at560798us/F36/palettebyte5:
  manualop44 bypasses6032, now with no false MEMMGR error. Existing map-load
  reinitialization, original77e2 sound-bank allocation/device work, actual
  file-service/nonlocal errors and complete original output remain open.

## Next

1. Compare current allocation/free/open traces under matched memory availability. Check list order,
   backlinks, sentinel ends, descriptor ownership, total=sum(free sizes), and moved-block contents.
2. Resolve conventional-memory/PSP/environment layout differences from the original loader's
   AH=48 answers before changing the budget. Do not increase memory to hide missing compaction.
3. Replace 0c21's stub with the original EMS/XMS probe and swap contracts. Begin with empty pools
   (same failure as the oracle); then capture supported nonempty configurations before adding support.
4. Test allocate→resize→move→release and memory pressure during cockpit switching. Compare model
   opens and loaded bytes, including detail variants; protect audio/video overlay coexistence.

## Accept

Allocation decisions, free accounting, relocated contents and failure/swap results match the
original. Both targets load the same assets without corruption; INDIA3 and UKRAINE2 regressions pass.
Board:0011 owns the missing framebuffer check in audio flows; board:0027 owns switched consoles.
