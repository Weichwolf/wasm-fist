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
