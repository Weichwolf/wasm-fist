Type: bug
Status: open
Parent: 0001
Note: renumbered from 0024 (commit 430604b filed it as 0024; that number belongs to the closed
      guest-RAM oracle capture item).
Title: every mission loads every model and sprite sheet the original loads -- the CRT memory manager
       measures free memory and can insert into its free list

An LD_PRELOAD open-trace of the original entering AZER1 (tools/oracle/opentrace.c via
tools/oracle/trace_click3.sh 160 100 205 128 40 186 55 12 25) opens, after the map: 41 models x
{.MAL .M00 .M08}, then every .M16, then every .M32 -- 160 model files -- then MSN2.MS3, MSPRITE5.BIN,
MSPRITE1.BIN, M1CON.MRL.  The port (FIST_OPENLOG=1, same clicks) opens M1_A.MAL/.M00/.M08,
M1_B.MAL/.M00/.M08, then ONLY the .MAL of each remaining model, no TREES.MAL at all, no .M16/.M32,
no RESOURCE.RES, 532.PAL, DSOUNDS.BIN, WVSOUNDS.BIN.

## Why: the model memory budget is a constant

    42cc  mov ax,0xffff ; mov bx,0x157c ; mov cx,ds ; lcall [0xe8]      "how much memory is there"
          word[0x6d9b] = max(word[0x6d9b], BX) ; word[0x6d9d] = word[0x6d9b] - 0x1280
    4397  sub [0x6d9d],ax ; jb 43a2                                     per model in the 4308 walk

DGROUP:0xe8 = 0f69:20ef = FUN_1000_177f (installed by far reloc section si=0xec; the oracle DGROUP
sample tools/oracle/samples/dgroup_0x0_0x100.bin reads `20ef 2382` there).  177f arms `ss:0x1744`,
calls the allocator 184b, and on failure returns the LARGEST FREE BLOCK in BX from ss:[0x16d8].  In
the port: patch 383 dropped the section-0xec install at 06bc ("a no-op"), 177f was never promoted,
and 42cc's decompile used the INPUT bx (0x157c) as the answer -- budget 0x2fc paragraphs = 12 KB =
two meshes, on any machine.  scratch/parked/571-177f-memmgr-vector-model-budget.diff fixes all three
(promotes 177f with BX through g_fist_177f_bx, reinstates the section, threads the arguments at the
four `lcall [0xe8]` sites 42cc/4386/e813/e8df).

## Why it is parked: the free-list insert never terminates

With 571 the `ax=0xffff` request runs the allocator's whole strategy chain -- first-fit, 0bd1, 0bef,
0c21 (the block-relocating probe the shim reports as "unimplemented deep follow-on"), 0c7d, then
0c7f, which DISCARDS a discardable block and reinserts it into the free list through 1110/1114 --
and 1114's sorted insert (`cmp ax,[0] ; ja next` / `jb prev`) spins: the port's free list is ONE
self-linked sentinel (`3a04: base=0 next=prev=3a04`), so the forward walk never finds a node with
base >= 0x4b15.  Patch 025 built 1114 for the boot alloc path only and documented the follow-on
("its other 16 folded callers need the same cursor treatment when the free path is exercised").
The original's list layout at this point -- the arena header 1467 writes word[0]=own seg, and
whatever sentinels 0748/0d70/0dc2 maintain -- has to be read from the asm, not guessed.

## What this gates

- every unit, tree, wreck, shell and effect mesh (the "chrome but no content" windshield of
  board:0001/0002 -- the port has been drawing objects from records that never held a model);
- the sprite blitter hang after the DEFEAT verdict (patch 569's finding: zero-width sprite headers
  from unloaded sheets), i.e. the mission-over exit of the self-play sweep (board:0017);
- the two menu-time 0x1000-paragraph allocations at e813/e8df (they trapped silently until now).

## Order of work

1. Read the MEMMGR init (0748's arena setup, 1467, the 0x16d4/0x16f6/0x1718 control frames) and
   establish the original's free-list sentinel layout; fix the port's init to match.
2. Rebuild 1114's sorted insert + coalesce from the asm against that layout; then the 16 folded
   callers patch 025 listed.
3. Implement 0c21 (the relocation probe: 1c68/1fcc) faithfully or prove the 0xffff query never
   reaches it in the original.
4. Unpark 571; confirm FIST_OPENLOG matches the open-trace file for file; gate.

## Step 1 measured: the original's list layout (RAM dump scratch/oracle/player.ram.bin, mid-mission)

    ctl0 DGROUP:0x16d4  [4]=1474 free-total  [0xa]=446e alloc head  [0xc]=4a70 free head  [0x10]=001e
      free:  4a70 base=0000 size=0 (LOW sentinel) -> 4a34 base=576b size=3 -> 454a base=8b8e size=1471
             -> 4a6c base=ffff size=0 (HIGH sentinel) -> 0000
      alloc: 446e base=4470 size=0903 -> 4a58 -> 4a56 -> 4a6e -> 4a46 -> 4a44 -> ...
    ctl1 DGROUP:0x16f6  free: 4a64 base=0000 -> 4a62 base=ffff -> 0     alloc: 4a68 base=0000 -> 4a66 base=ffff -> 0
    ctl2 DGROUP:0x1718  free: 4a5c base=0000 -> 4a5a base=ffff -> 0     alloc: 4a60 base=0000 -> 4a5e base=ffff -> 0

Every list, free and allocated, in every arena, is bracketed by a LOW sentinel (base 0x0000, size 0,
owner = itself) and a HIGH sentinel (base 0xffff, size 0), prev of the low and next of the high being
0.  That is what makes 1114's walk terminate: `ja next` stops at 0xffff, `jb prev` stops at 0x0000,
and the new node is spliced between.  The port's free list at the spin is the low sentinel alone,
linked to itself (`3a04: base=0 next=prev=3a04`) -- its arena init never creates the high sentinel
(or the pair for the allocated list).  The node fields, confirmed: [0]=base seg, [2]=flags, [4]=size
(paragraphs), [6:8]=owner far ptr (a sentinel owns itself), [0xc]=next, [0xe]=prev, [0x10]=refcount.

Budget numbers from the same dump: word[0x6d9b]=0x4656 (the largest free block 42cc measured, 287 KB),
word[0x6d9d]=0x1e72 (0x33d6 - the 0x1564 paragraphs the models took), word[0x454]=0x396f.  The port's
DOS layer (INT 21h AH=48h) reports 0xe820 paragraphs free (929 KB) where DOSBox gives the original
~287 KB usable -- enough to load everything, but word[0x454] and any memory-dependent behaviour will
differ until the shim's DOS memory map reproduces DOSBox's (memsize=16, ~600 KB conventional minus
the loader).

## Step 2 measured: the insert is fine, 0c7f's unlink was the corruption -- and the meshes still do not load

gdb free-list dumps at every strategy of the 0xffff request: the list is intact through 0bd1 (which
discards cached blocks -- free total 0x15b7 -> 0x3a08 -- the strategy the original runs for the same
query), 0c21 (stub), 0c7d, and into 0c7f; 0c7f's C read `ctl[0xc]` (the free-list head) where the asm
`8e 06 0c 00` is `mov es,[ds:0xc]`, the NODE's next -- it spliced the LOW sentinel into the allocated
chain and the following 1110 insert spun on it.  scratch/parked/572-0c7f-unlink-uses-the-node-links.diff
fixes it; with 571+572 the mission loads and the budget reads word[0x6d9b]=0xe820, [0x6d9d]=0xd5a0.

The meshes STILL do not load, and the trace shows why, one level down:

    183f idx=0 lod=8 -> 19d1 (M1_A.MAL opened) -> 1a45('00') M1_A.M00 opened -> 1a45('08') opened
    183f idx=4 lod=8 -> 19d1 (M1_C.MAL opened) -> 1a45('00') NO open -> 1a45('08') NO open

1a45 (the streaming mesh loader, asm 0x1a45-0x1b8x) builds the name, opens through [0x38c]=276e,
then its C tests a fabricated `uVar22 = 0xd91b < param_2*2` (Ghidra's rendering of the open's `jb
1a42`) -- true for every model, because param_2 carries the name index in its high word -- and takes
the fail path 1a2b, whose close `lcall [0x390]` the C calls with no slot.  So M1_A/M1_B's four mesh
opens each took one of 276e's FOUR open-file slots (cs:0x2b6a..0x2b72) and never released it; from
M1_C on, 276e finds no free slot and returns before DOS.  No mesh has ever been loaded in the port.

Next: rebuild 1a45 from the asm (CF from 276e via g_fist_cf; the 16-byte header read via [0x394];
the mesh allocation via [0xe4] = FUN_1000_1774 (bytes -> paragraphs, then 177f) into DGROUP:0x3ad6;
the chunked `rep movsw`/[0x394] reads into that segment (1afe..1b33); the per-model pointer table
0x27f4[idx] and 0x276c[idx]; the sub-block walk at 1b50 with the 0x300 stride; the close with the
slot in BX), and confirm 276e's slot release on close.  Then the .M16/.M32 passes should follow from
the table's lod bits, and FIST_OPENLOG should match the oracle open-trace file for file.

## Step 3 landed (patches 571/572/573, 2026-09-11)

The chain rebuilt from the asm -- 183f/1692/1735/19d1/1a45, 1774 -> 177f, the FILEMGR leaves with DS and
CF, extender ops 0x3c/0x4c in the gate, INT 21h AH=43 as an existence probe -- and two things the loads
uncovered: the resource relocation callback 15c2 (1541's [S+4]) had never been promoted, so every
MEMMGR block move left the sprite directory [0x4f0] stale, and three arg-less cockpit blits (7fcd,
5a1d, 3823) drew garbage records over live blocks (board:0014 lists the 118 remaining sites).

Measured (FIST_OPENLOG vs the oracle open trace): the 160 model files open in the original's order,
file for file; the self-play runs to DEFEAT at 3:38 without a crash or hang (the original: 5:49).

Still open here:
- the original opens BURM_D2.KLC/BURM_C2.KLC five times between the models and MSN2.MS3 -- the TCB's
  default map names (DGROUP:0xea7c "5.SKY 502.PAL BURM_D2.KLC BURM_C2.KLC WVSOUNDS.BIN").  Measured
  (FIST_EXTLOG): at that point the engine posts extender op 0x60 sixteen times, which the gate does
  not handle.  Op 0x60 = ext 0x8650: from the posted position (TCB+0xd2/+0xd6, <<13 >> (32-detail))
  it samples a (4<<(detail-9))-square window of the heightmap for its min/max; when the range is <= 9
  it decodes the KLC named at TCB+0x5a through 643c into [0x8644] and resamples it with bc06/bed2 to
  the map's detail -- a local terrain patch.  The original's five BURM pairs are the five posts that
  passed the range test.  Belongs to board:0009 (the extender services); it changes what the voxel
  renderer sees around those five objects.
- the budget: the port's DOS AH=48 reports 0xe820 paragraphs, the original ~0x4656; word[0x454] and any
  memory-dependent branch differ until the shim's conventional-memory map matches DOSBox's.
- 0c21 (the block-relocating probe) is still a stub that answers "not found"; the 0xffff query runs it.
- 0d2e (the resource touch/reload, f69:169e) is still Ghidra's: it writes DGROUP:2/6/8 for the node's
  fields, dispatches [bp+0x18] under CS 0x1000 instead of 0f69 (ctl1/ctl2's 262a/29bf are the reload
  methods, ctl0's 1026 is `stc ; ret`), and calls 0d70 with the wrong argument.  Reached when a
  descriptor carries flag 0x10 (discarded) -- not yet on AZER1.
- the port additionally opens MSPRITE2.BIN/M2CON.MRL/CCV.MRL when the player's vehicle dies (7eb7's
  view reset with the descriptor 0x8f82 unallocated); whether the original does the same after its
  1:38 death is unmeasured (open3.log ends at the spawn).

## The out-of-memory path is the MEMMGR's swap (2026-09-12, board:0027's root)

The second cockpit's console (M2CON.MRL, 0x663 paragraphs, loaded by 84c3 -> 0310 -> 26fc -> 250d
-> 184b when the player's vehicle is lost) comes out torn because its block is handed to the next
allocation while it is in use.  Measured with the MEMMGR lists dumped at each step ($SC/mmfree.py):
before the load the free list holds only a 1-paragraph and a 0x281-paragraph block; 184b -> 0a31
then splits the ALLOCATED 0x1000-paragraph block at 0x5c54 (owner DGROUP:0x157c, the model budget)
into a 0x663 node owned by the descriptor 0x8f7e and a 0x99d remainder -- and leaves both on the
FREE list.  2004's next 182a (0xc00 bytes for the child 0x8f8c) takes the 0x5c54 node, and the
compiled-sprite records `03 05 40 01` overwrite the console's RLE stream before 0340 decodes it.

The asm behind it (0x10c21-0x10c7a): when no free block fits, 0c21 walks the allocated list for a
node whose owner flag has bit 2 set and bit 6 clear and runs 1c68 (1d5c/1d9c: copy the block's
paragraphs elsewhere, mark the node 0x30, drop the owner) and, failing that, 1fcc (211f/2149, then a
far call through DGROUP:0xd242 with the block's size and segments in DGROUP:0xd328..0xd336 -- the
move service outside conventional memory) -- the block is SWAPPED OUT so its paragraphs can be
reused; 1345/0d70 and the flag-0x10 reload (0d2e, the ctl1/ctl2 methods 262a/29bf) bring it back
when its owner touches it.  The port's 0c21 answers "not found" (the message
`[memmgr] FUN_1000_0c21 block-relocating probe (1c68/1fcc) reached -- unimplemented`), and
0a31's carve then runs with no room taken -- the corrupt split above.  1c68, 1cba, 1d51, 1d5c,
1d9c, 1df2, 1dfa, 1eab, 1eb4, 1f3b, 1f43, 1fcc, 204f, 211f, 2149 and the [0xd242] service are
Ghidra's (DS-relative node fields rendered as DAT_1000_c000..).  This is the next MEMMGR item: the
swap-out/swap-in pair, faithful to the asm, with the moved paragraphs kept where the original keeps
them (the [0xd242] service's memory), so a mission that fills conventional memory -- every one, once
the player's vehicle is lost -- loads its later resources into blocks that stay valid.
