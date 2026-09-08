Type: bug
Area: engine
Tags: instrument
Depends: 0024
Title: INDIA3 spawns its cockpit crash-free like every other battle

INDIA3 spawns its op-0x2c cockpit crash-free like every other FSG battle (34/35
already do). Currently INDIA3 is the sole crasher: the cockpit-instrument
bbox-analyzer (2004) allocs buf1(0xc00) OK then buf2(0x140) fails on a fragmented
near-heap free pool — a deterministic near-heap OOM after the mga-icall SEGV chain
was ported (patches 400-404).

## Comments

At op-0x18 INDIA3 == AZER3 byte-identical (free=1681); the divergence is an
INDIA3-specific over-consumption in the synchronous post-map cockpit-setup cascade
(reaches 2004 via 84c3, children 0x8f8c/0x8fa8/0x8fc4). Pool size, patch 401/332,
and the allocator (0a31/1040/0c7d) are all asm-verified faithful. The open datum
is whether the ORIGINAL has more near-heap headroom at 84c3-entry — an oracle-only
question (guest phys 0x2d190+0x16d8). Shim env FIST_DBG_MM dumps the pool state.

## The crash is GONE at HEAD; the item now turns on native<->wasm identity

INDIA3 no longer crashes. Measured at HEAD (patches through 557):

    FIST_FSG_BATTLE=INDIA3 FIST_DUMPTICK=20000
      -> rc=0, [outcome] a294=100 a296=7 loaded=1 peak_a296=10   (mission loaded and simulating)

    the matrix's own op-0x2c flow (FIST_MISSFB2C=1 FIST_MISSFB_N=1)
      -> rc=0, frame dumped: mode=0x13 nonzero=56728/64000 distinct-indices=182

i.e. a real rendered cockpit, not a blank frame, and no near-heap OOM. A first attempt to check this
was WORTHLESS and is recorded so it is not repeated: run with `FIST_DUMPTICK=4000` the process is still
in the TITLE.KDV intro and never reaches the cockpit at all, so "rc=0" there proves nothing.

Which patch fixed it is NOT established. The stated cause was a fragmented near-heap free pool at
84c3-entry, and several allocation-adjacent corruptions have since been removed -- the 48 aliasing
byte variables of patch 552, the DGROUP:0x1549 run of 553, and the two out-of-bounds writes of 554/555
-- any of which could have been consuming or corrupting the pool. Attributing it would need a bisect.

`tools/verify.sh` now carries `mission-cockpit-india3`, which completes the 47-mission cockpit set (it
was the only exclusion). THIS ITEM STAYS OPEN until that flow passes native<->wasm: the crash going
away is necessary, not sufficient, and the matrix run that checks identity has not completed yet.
