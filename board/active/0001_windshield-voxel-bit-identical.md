
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

*** BROWSER NOW LIVE-RENDERS THE IN-MISSION VIEW (cockpit + windshield) -- 3 web-harness fixes ***
Pushed the browser from "mission loads to the PL:1 screen" to LIVE in-mission rendering with a working
palette.  Root causes found + fixed (all __EMSCRIPTEN__/g_web_mode-gated -> the verified native+node-wasm
matrix is behaviorally untouched; native build confirmed OK):
1. TICK-HOLD STARVED THE SIM.  native_main.c:561 held fist_wasm_tick during mission-load-pre-cockpit
   (frozen-c452, a deterministic-CAPTURE aid for FIST_MISSFB node-wasm).  For LIVE play the sim must
   advance, so d549 never reached 0x1c -> stuck at PL:1.  Fix: `|| g_web_mode` -> the browser ticks
   normally in-mission (op 0x1c / 459a sim now fires; the tank spawns).
2. IN-MISSION PUMP STARVATION (the deep one).  Confirmed by the pre-existing FIST_R3D_DUMP note
   ("the mission loop does not re-enter fist_timer_pump"): unlike the menu's constant in(0x3da)
   busy-wait, the mission sim rarely polls ports, so ticks/palette-fade/frame-post all stall (frames
   froze ~192, palette stayed black).  Native drives these off the async SIGALRM timer; wasm has no
   in-mission timer.  Fix: hook the per-frame cockpit render op (op 0x24 in fist_extender_gate) to
   drive fist_vga_service_retrace() (palette upload + fade ramp) + fist_web_pump_input() +
   fist_web_post_frame() -- one pump/post per render, the natural in-mission heartbeat.
3. PALETTE LAGGED THE RENDER.  In-mission the retrace-poll DAC upload may not have run when the web
   frame is posted -> g_pal black though the render + MGAVIDEO buffer are valid.  Added
   fist_web_force_palette() (FIST_PALNOW logic: word[DGROUP:0x782] -> g_pal) in the web post.
RESULT (headless-chromium CDP, AZER1 auto-driven via ENV FIST_MOUSE): the mission renders LIVE in the
browser -- palette populated (palSum 0->28221, 719/768 entries), the windshield terrain (the 78.7%
voxel, static-3a24 as expected) + cockpit chrome panels compositing, frames advancing off the op-0x24
seam.  Captured frame is MID-COMPOSITION (fbNonzero ~24.5k vs native's full ~51k) -- the remaining
polish is posting at RENDER-COMPLETE (frame-end op), not mid-display-list.  NOTE: worker.js currently
carries a TEMP FIST_MOUSE auto-drive for this test -- must be reverted before commit (interactive
default).  The 3 shim fixes are correct-in-principle + g_web_mode-gated; run verify.sh both (mission
flows byte-identity) before committing.

CHROME-COMPLETENESS ROOT (browser, next iter): the cockpit dashboard is M1CON.MRL (the M1 tank
CONsole display-list resource; menu uses MAINMENU.MRL, mission uses M1CON).  Native op-0x24 frame is
full+persistent (post#1=51237, post#50=54826 nonzero -> chrome drawn once at mission-screen-enter,
persists while the windshield redraws on top).  The browser reaches only ~24-27k = windshield + a few
M1CON elements (bottom-left panel) -> the M1CON console paint chain does NOT fully iterate in-mission
under wasm (same class as the in-mission pump/state-manager starvation).  NEXT: runtime-compare the
browser 0xA0000 chrome region vs native to see which M1CON elements are missing, trace the console
paint (display-list element methods, si=0x174 vectors), find why it stops partway in wasm.  Do this
with a browser run AFTER verify (no CPU contention).  verify.sh both in flight = 40/0 so far (the
g_web_mode-gated web fixes are provably invisible to the g_web_mode=0 verify path).

DISPROVEN (this iter): the M1CON console-partial is NOT the tick-hold.  Tested reverting the
`|| g_web_mode` tick override (keep the frozen-c452 hold for web too, like the node-wasm FIST_MISSFB
capture that reaches the FULL 51k console): the browser console stayed PARTIAL (~27k) AND, worse, went
BLACK (fbNonzero~73) once d549==0x1c released the hold and the sim free-ran at ~1500fps.  So the
committed free-tick (9ab60c6) is the better state (stable partial 27k).  Conclusion: console-
completeness + live-sim-stability are the DEEP wasm in-mission execution frontier (the sim/HUD paint
diverges from native under wasm's cooperative cadence), NOT a tick-hold tweak.  node-wasm FIST_MISSFB
(g_web_mode=0) reaches the full 51k console at op-0x24 post#1, but that path captures-and-exits; the
LIVE g_web_mode loop reaches only ~27k -> the divergence is in the live in-mission loop, not the
op-0x24 render itself.  Reverted the experiment; 9ab60c6 stands.

*** BROWSER -O2: ~5.6x faster in-mission, BYTE-IDENTICAL (commit ef0f518) ***
The engine C (decompile + shim) was built -O0 -g.  Tested -O2: the mission-cockpit op-0x24 frame
(voxel render + 459a sim -- the hardest path) is BYTE-IDENTICAL native(-O0) == node-wasm(-O2), and a
full-matrix verify (NATIVE=-O0 vs OUTJS=-O2) runs clean (front-end + settings so far).  So the
machine-generated decompile is well-behaved under -O2 (no UB exploited; -fno-strict-aliasing).
Switched build_web.sh -> -O2: measured in-mission fps 0.4 -> 2.26 (headless-chromium), menu snappier.
Kept build.sh/native at -O0 -g (gdb tracing).  FOLLOW-UP OPTION: if the -O2 full-matrix verify passes
159/0, -O2 could be adopted for build.sh + native too (faster verify/DoD-gate + native), trading -g.
The in-mission render is still ~2.3fps (compute-bound even at -O2) + partial M1CON console + static
3a24 terrain -- those remain the deep frontiers, but the mission is now WATCHABLE in the browser.

-O2 ADOPTION COMPLETE (verify 159/0): the full-matrix verify with NATIVE=-O0 vs OUTJS=-O2 passed
159 PASS / 0 FAIL -- -O2 wasm is byte-identical to -O0 native across EVERY flow.  So build.sh (node-
wasm + DoD-gate build) now defaults to -O2 (FIST_DEBUG=1 restores -O0 -g); build_web.sh already -O2.
The DoD 10x wasm gate + the wasm side of verify now run ~5x faster with zero output change.  CAVEAT:
32-bit NATIVE stays -O0 -- -O2 HANGS the x86 native build on the mission-cockpit (a decompile-UB the
wasm/LLVM backend does not hit; -O2 wasm is fine, -O2 x86 diverges).  So native keeps -O0 -g (also the
reference + gdb-traceable).  Net: browser watchable (5.6x) + DoD gate 5x faster, all byte-verified.
