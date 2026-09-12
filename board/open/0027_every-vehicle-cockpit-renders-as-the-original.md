Type: feature
Status: open
Parent: 0002
Title: every vehicle's cockpit renders as the original's -- the M2/M3 Bradley, the T-80, the helicopters, not only the M1

The M1 cockpit (MSPRITE1.BIN + M1CON.MRL, the paint chain 77dc -> 795c -> a84c) is the one every
flow and every screenshot check has exercised.  The game has more: when the player's vehicle is lost
the view switches to the next vehicle of the platoon (a93e -> a5c4's flash -> 500f/5087/5fca -> the
new type's cockpit), and missions start the player in other types.  On AZER1 the switch lands in an
M2/M3 Bradley cockpit (MSPRITE2.BIN + M2CON.MRL: the analog MPH/TEMP/OIL/FUEL dials, the TOW/HEI/AP/MG
ammo boxes at the left, the map at the right) -- the oracle's 60-second frame of the watch run
(scratch/oracle/watch_c252/w.final.png) shows it whole, with a tank on the windshield.

What the port shows after the switch (FIST_DUMPTICK 1900 on AZER1, SYRIA1 from tick ~1700): the M2
console's left half at its place and again ~140 pixels to the right, the map inset missing, the HUD
texts (PL:1, GOALS REMAINING, the range) where they belong, the op-0x24 render surface still carrying
the M1 console (FIST_MISSFB frame 1500) -- the M2 console is painted onto the presented frame with the
wrong geometry while the render surface never receives it.  The palette under it was red until patch
585 (a5c4's dropped immediates); with the fade right the console pixels index entries the M2 palette
never reached (saturated red/green/blue), so the palette upload of the new console is missing too.

The original's path, read off the block trace of the switch (scratch/oracle/regtrace_0660/blk.txt
after the 0660 hit, first executions in order): 500f 5087 5fca 7ea6 77c3 7eb7 7f19 7f5f 7f3e 84c3(0f69)
MGA 2595 7eba(0f69) 7f44 8390(0f69) 7f82 852f(0f69) 7fcd 7fff 801e 862b(0f69) 7fb5 8568(0f69) 8152
819c 7fc1 8668(0f69) 7fc7 8682(0f69) MGA 2a39/2a51/2a7a 82ee 831f 8243 82a1 81e6 8329 834a 8450 853f
8558 8024 804b 8072 031c 848c 84c5 8503 80eb 85bd 8573 7f53 842d(0f69) 7f59 8463(0f69) 83df ... --
the 0x7exx-0x85xx family is the Bradley cockpit's element builders and paint methods, none of which
the M1 flows run.  Each is to be checked against its asm the way the M1 chain was (base-losses,
register arguments, store widths), the console load (c560/0310, the .MRL palette upload c5b0) for
the second cockpit included, until the switch frame is pixel-identical to the oracle's.  Then the same
for the T-80 (TRAIN missions / the Soviet campaign's player) and the helicopter views.

The torn console's cause is not in the paint chain: the M2CON.MRL block is handed out again before
0340 decodes it, because the load runs the MEMMGR's out-of-memory path, which the port has as a
stub (board:0025, "The out-of-memory path is the MEMMGR's swap").  The paint chain runs in the
original's order (500f 5087 5fca 466c 7ea6 1cdb 77c3 7eb7 7f19 bedc 7f5f 84c3 0310 2595x3 7eba 0340
8390 852f 7fcd 7fff 801e 862b 8568 8152 8668 8682 2a39 82ee 8243 82a1 8329 8450 8558 8024 031c 848c
85bd 842d 8463 83df, gdb-traced) and the decoder 0340 is byte-exact on M2CON.MRL (a Python PackBits
of the file equals the oracle's static console pixels); what it decodes is the sprite records 2004
wrote over the stream.  Blocked on 0025's swap.

Unblocked by the MEMMGR resize (patch 586, board:0025): the heap the original has at the switch is
the heap the port has, M2CON.MRL loads into the free tail, the Bradley console renders.  Patch 587
then took the four Bradley paint methods that still carried Ghidra's arguments (82ee's fill, 852f's
label and its AH background byte, 862b's needles, 8682's mirrored glyph).  82ee's was the last static
artefact: `xor al,al ; lea bx,[bx+4] ; lcall [0x60a]` passed as `(0)` fills a rect read from
DGROUP:0 in a stack colour -- four full-height columns of index 224 every 64 pixels over the whole
cockpit (a gdb watchpoint on (118,150) named MGA 1091 <- 82ee).

Measured on the tick-2100 AZER1 frame against the oracle's 60-second frame (scratch/oracle/watch_c252,
the two are not the same instant): columns 0..255 of the console rows differ only in the TEMP/OIL
needles (rows 131-135), the compass strip (rows 111-124, cols 156-169), a 25-pixel gauge pointer at
(83..89, 104..108), the mouse pointer the scripted run parks at (40,186) (MGA 0db8/0e3b, the cursor
save/restore -- not the console), and the map inset's frame at cols 243-255; columns 256..319 are the
map inset and the radar (dynamic).  What remains for the Bradley is the tick-aligned oracle
comparison (board:0017's congruence run) for the dynamic instruments and the radar sweep; then the
T-80 and the helicopters.  The display list the cockpit walks at tick 2100 (43 nodes: 7f5f 7f44
7f82 7fcd 7fff 801e 7fb5 8152 819c 7fc1 7fc7 82ee 8243x4 82a1x4 81e6x2 8329x2 8450x2 853f 8558 8024
804b 8072 848c 84c5 8503 80ebx3 85bd 8573 7f53 7f59) contains none of the arg-less driver dispatches
board:0014 lists -- the Bradley's methods are the 0x1000-cluster 852f/862b/8682 family, now done.
