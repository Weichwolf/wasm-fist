
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

Game-loop probe (this loop): instrumented FUN_0000_459a (the per-tick sim / main game loop, asm
0x459a) -- it NEVER runs in the menu (0 [looptrace] hits). But 459a is the MISSION sim (called from
FUN_0000_87a, the game-entry that also does the be67 sound-register + FUN_0000_5d12), so its
absence in the menu is EXPECTED. The menu-music gap is therefore in the MENU SCREEN HANDLER path
(distinct from 459a) which also does not dispatch the music-start. Net, confirmed from multiple
angles: the port runs the EXTENDER render pipeline (159 frame-pinned flows pass) + limited engine
setup, but NOT the engine's full screen/state manager loop that would dispatch per-screen methods
(menu music 9f1d/bde4, menu logic) NOR the mission game loop 459a (flight-model sim). Both the
windshield (flight-model camera via 459a/PM-gate) and the audio (menu-music via the screen handler)
share this root: the engine's game/screen-manager loop is not driven in the port. This is a focused
multi-session architectural effort (the engine main loop + screen/state manager + the real->PM
gate), NOT a trap, un-seeded function, or base-loss. The tractable loop-win classes (trap-trace ->
patch 408; SMC-freeze -> patch 407) are EXHAUSTED for this session. Landed + verify-both-verified
this session: patch 407 (windshield 6980 SMC) + patch 408 (audio 0xfab instrument-apply).

*** BROWSER PLAYABILITY MILESTONE (web/ harness) -- the wasm build runs + is navigable in a browser ***
Built a browser harness (web/index.html + worker.js + tools/build_web.sh + tools/serve_web.py; shim
accessors fist_web_* in native_main.c/fist_vga.c, __EMSCRIPTEN__/g_web_mode-gated so native+node builds
are unaffected). The engine runs BLOCKING in a Web Worker (no ASYNCIFY -- it inflates a giant decompiled
function past the browser's wasm local-count limit); each ~frame the pump posts the 320x200 framebuffer
+ palette to the main thread (canvas render), and polls a SharedArrayBuffer for live mouse/keyboard
(one queued event per frame -> press/release land on different frames = real clicks; mouse X is the
mode-13h virtual 0..639 space). VERIFIED end-to-end via headless chromium (CDP): the full FRONT-END is
navigable in the browser -- main menu renders live, click SETTINGS -> settings screen (CONTROL/DISPLAY/
SOUND), click BATTLES -> SELECT BATTLE dialog (AZER1..CYPRUS1), select+OK -> mission briefing
("BLOODFEUD!"), ACCEPT -> the mission LOADS (frames resume after the load, shows the "PL:1" load
screen). So the browser build reaches in-mission; the LIVE in-mission windshield terrain (voxel via the
flight-model 459a + 3a24 producer) is the same deep frontier already mapped -- everything UP TO it works
interactively in the browser. This is a real, visible deliverable toward "play it in the browser".
Bonus finding: the settings screen shows MUSIC=OFF selected -> that config default (not a bug) is why
menu music was silent; clicking MUSIC ON would exercise the (patch-408-fixed) OPL playback.
