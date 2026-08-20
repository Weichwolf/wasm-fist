Type: bug
Area: engine
Tags: instrument

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
