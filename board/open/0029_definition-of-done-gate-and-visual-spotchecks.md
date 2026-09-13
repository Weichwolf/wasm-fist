Type: feature
Status: open
Parent: 0000
Title: the port passes the definition-of-done gate -- 10x consecutive wasm matrix + visual spot-checks vs the original

The goal's two-part correctness proof (CLAUDE.md "What decides done"):

  1. the full test matrix runs error-free TEN consecutive times over `tools/wasm_gate.sh`
     (one failure resets the counter to zero);
  2. at representative points of every surface, render spot-checks are compared VISUALLY against the
     original (screenshot comparison, not only a byte hash).

## Proof (2): visual spot-checks vs the DOSBox original -- the static surfaces are DONE

Every `ref/*_native320.png` is a GENUINE 1:1 unscaled 320x200 DOSBox capture of the original
(`tools/refcapture_*.sh`), so the verify matrix's per-flow comparison IS a port-vs-original check, not
merely native-vs-wasm.  Confirmed directly this round:

  - All 29 menu / screen / dialog / settings surfaces with an oracle ref (mainmenu, about, review,
    selplayer[+ok/+kkr], battles[+select/+cancel/+scroll/+page/+ok], campaigns[+select/+ok/+cancel],
    and every settings sub-state: sky, detail med/low, sound-fx med/off, music off, smoke off,
    joystick + flightstick/tmfcs/ch/tmwcs) render **pixel-identical** to their DOSBox capture on the
    port -- 29/29 identical, 0 differ (rendered on /tmp/fist_native, hashed vs ref).
  - A port-vs-original montage (main menu, battles, cockpit chrome) was eyeballed: indistinguishable.

So for the menu/screen/dialog/settings class, "the matrix passes" is already "the player cannot tell
the port from the original".  What remains for proof (2): the DYNAMIC surfaces -- the in-mission
windshield voxel band (board:0002, oracle-framebuffer congruence) and the cockpit instruments/radar
(frame-timed; verified per-tick against g_mem in board:0017, but not yet as a still-frame screenshot
against a tick-aligned oracle capture) -- and the vehicle cockpits beyond the M1/M2 (board:0027).

## Proof (1): the 10x wasm endurance

`tools/wasm_gate.sh` grinds 10 consecutive clean `verify.sh wasm` runs (178 flows each; one FAIL
resets to 0).  Run on the 609 tree.  DONE when the log reaches "10 consecutive clean wasm runs
achieved" AND proof (2)'s dynamic surfaces are closed.

DONE when: proof (1) reaches 10/10 on a tree where every surface named in the goal is implemented,
and proof (2) covers every surface class (static: done; dynamic windshield + cockpits: open).
