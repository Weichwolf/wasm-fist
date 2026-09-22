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
