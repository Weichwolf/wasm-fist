
*** TRAP-TRACE METHOD EXHAUSTED (systematic sweep, this loop) ***
Ran FIST_TRACE_TRAPS across the three major surface paths -- menu (cold), mission (AZER1 drive),
and editor (FSG round-trip). Result: the ONLY real un-seeded-function trap was SOUNDDVR+0xfab
(31x, fixed by patch 408). All three paths show the SAME 4 residual traps -- 0x00000 (null),
0x01b31, 0x1360f, 0x13f7f -- each firing ONCE and each "no FUN_ there" (verified mid-function /
null computed targets, not function entries), safely handled by the honest trap trampoline. So the
un-seeded-function class is EXHAUSTED; the trap-trace method (which cleanly yielded patch 408) has
no more tractable wins. The remaining incompleteness on every surface is the DEEP shared root: the
engine's main GAME/SCREEN LOOP does not fully run in the port -- only the extender render pipeline
+ coop-tick drive the 159 frame-pinned flows, so the screen objects' enter/update methods (which
start music, run menu logic, drive the flight-model camera/sim) are not dispatched. That is the
next frontier and it is NOT a trap or an un-seeded function -- it is an unreached code path in the
engine's state/screen manager. Landed this session: patch 407 (windshield 6980 SMC) + patch 408
(audio 0xfab instrument-apply), both verify.sh-both verified (159/0, native==wasm).
