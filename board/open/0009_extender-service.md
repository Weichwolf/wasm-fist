Type: feature
Area: extender-shim
Parent: 0003

The Doug-Huffman extender TASK-EXECUTION service (real-mode->PM callback gate, linear 0x8799) is
implemented faithfully in the platform shim, so every engine `extender op` renders/executes bit-identically
native==wasm and against the DOSBox/QEMU oracle -- unblocking the intro animation render, audio
full-duration (board:0003), and any task-driven screen.

## The interface (asm-anchored, build/fist.c + re_out/fist_ext.c)

The engine posts an extender op through a single funnel FUN_0000_e339 (asm 0xe339):
  - task base   = word[DGROUP:0xea2c] : word[DGROUP:0xea2e]   (DAT_2000_aa2c : aa2e)
  - op code     = word[DGROUP:0xea10] (DAT_2000_aa10)
  - command arg = dword[task + 0x3f2]  (set by the caller just before)
  - gate ptr    = dword[DGROUP:0xea16] (DAT_2000_aa16) = 0762:1179 = linear 0x8799
  e339: pushes [ea1a..ea1e]+0, `call [ea16]` (the gate), then IFF word[task]!=0 && aa10!=0 far-jmps
        [DGROUP:0x58] (the task's own dispatch tail).  Returns AX from the gate.

The op posters (all `store arg->task+0x3f2 ; aa10=OP ; call e339`):
  op 0x04  e37* (task re-execute)           op 0x64  e2c2 (post frame draw command)
  op 0x0c  (dcd*)                           op 0x68  e2df
  op 0x10/0x5c (dcd4, cond)                 op 0x70  (e714-area setup, LAST intro setup op)
  op 0x1c  e459 (LOADING screen)            op 0x74  (setup)
  op 0x24  windshield poster (board:0001)   op 0x78  d97e (TASK-EXECUTE / present a frame)
  op 0x2c  cockpit poster (board:0002)      op 0x44  (setup)
Current state: fist_icall_far(0x8799) TRAPS to 0 (task_seg stays 0:0, so e339's tail is skipped and the
gate is a no-op).  op 0x24/0x2c already render via a SEPARATE shim path (native_main.c g_web_mode &&
op==0x24, and the mission-cockpit flows) -- so the windshield/cockpit posters are partially served
OUTSIDE the gate.  op 0x78 (the intro anim-script frame execute) is NOT served -> the intro renders BLANK.

## Why this is the critical path

- AUDIO full-duration (board:0003): the intro-script loop e584 calls d97e (op 0x78) per frame; d97e's
  return is the gate's AX.  Patch 411 (`return FUN_0000_e339()`, asm-faithful, VERIFIED audio-intro
  native==wasm diff=0/381008 B) cannot land alone because op 0x78 is a stub -> wasm plays a BLANK intro
  and, under async(native)-vs-coop(wasm), misses the tick-pin -> battles-cancel-briefing regressed.
  Land patch 411 TOGETHER with a real op-0x78 render and the intro plays+renders identically on both.
- INTRO visual completeness: op 0x78 renders the 573-frame TITLE animation (the KDV decode+blit machinery
  already exists -- the intro flow uses FIST_KDV frame 385; op 0x78 must connect the intro task's posted
  frame commands to that render, blitting to 0xA0000).
- The gate is the CENTRAL display/task dispatch (12+ ops) -> a faithful implementation is reusable across
  every task-driven screen, not just the intro.

## Approach

1. Implement the gate (linear 0x8799) in the extender shim (re_out/fist_ext.c / the extender/overlay
   loader is hand-written per CLAUDE.md) as a real op dispatcher keyed on aa10, operating on the task
   struct at aa2c:aa2e with the command at task+0x3f2.  Seed task_seg so e339's tail (`far-jmp [0x58]`)
   also runs where the original does.
2. Start with op 0x78 (task-execute/present): decode the posted frame draw-command (the e584 script
   record: word[STRSEG:cur] threshold, byte[cur+5] = the frame/sprite id posted via e2c2 op 0x64) and
   blit it -- reuse the existing sprite/KDV blit path.  Return the deterministic status (0 = continue).
3. Verify: intro renders (native==wasm framebuffer, and AE vs a DOSBox intro-frame oracle); then land
   patch 411 and extend the audio flow across the intro (proven native==wasm diff=0 once both play it).
4. Re-gate 10x on the full matrix (the gate touches the boot path of every flow).

## Board coupling

board:0003 (audio) is BLOCKED on op 0x78 here.  board:0001/0002 (windshield/terrain) use op 0x24/0x2c via
the separate shim path; folding them into a unified gate dispatcher is a later consolidation, not required
first.  This is a focused, asm-anchored, multi-step shim implementation -- the documented "extender-service
frontier" made first-class.

CORRECTION (see board:0003's tick-regime finding): op 0x78 is NOT the audio blocker.  Audio full-duration
native==wasm is blocked by the async-vs-coop TICK REGIME (audio diff=0 was achieved WITH a blank intro
under COOP_TICK+patch411).  op 0x78 here is for the intro's VISUAL fidelity (real frames vs blank) and the
DD2 "every screen renders" completeness -- a parallel frontier, not a prerequisite for audio.  Patch 411
still lands with the tick-regime unification (board:0003), independently of this render.

GATE ENTRY LOCATED + DISASSEMBLED (recon for the implement session; read-only, matrix-safe).  The op
posters d97e (op 0x78) and e2c2 (op 0x64) are TRIVIAL pass-throughs -- each does
`*(task+0x3f2)=arg; aa10=OP; return/call FUN_0000_e339()`; all real work is in the GATE.  The gate lives
in the 32-bit extender image re_out/fist_image.bin (49040 B, app-relative flat base 0, so app-off ==
linear).  linear 0x8799 = fist_image.bin offset 0x8799; disassembled as i386 it is coherent 32-bit code:
    87ab: mov esi,[0xc93]      ; [0xc93] = the TCB (task control block) pointer (extract_image.py note)
    87b1: add esi,0x6a
    87b4: call 0x643c ; 87b9: call 0x4a3c
    87be: mov ecx,0xa ; 87c3: sub ecx,[0x8490] ; 87c9: je 0x87ef ; 87cb: ja 0x87e0
So the gate is a 32-bit PM dispatcher: it loads the TCB, then branches on a value at [0x8490] (a state/op
selector) via a compare ladder (je/ja at 0x87c9/0x87cb ...).  The implement session's entry points:
disassemble fist_image.bin from 0x8799 tracing the [0x8490] compare-ladder to the op-0x78 arm, map the TCB
struct fields it touches (base [0xc93], +0x6a), and the callees 0x643c/0x4a3c; then reimplement that arm
in the extender shim to decode the posted frame-command (task+0x3f2) and blit via the existing KDV/sprite
path.  fist_icall_far(0x8799) currently traps to 0 (the whole gate is a no-op) -- this is the single seam
to implement.  This is a DEDICATED multi-step 32-bit-extender recovery + shim build (touches every flow's
boot path -> full 10x re-gate), not a tail-of-session change; the entry map here is its ready starting point.

GATE BLIT MACHINERY MAPPED (deeper recon, read-only).  From 0x8799 the gate is a SCALED FRAME BLIT:
  - [0x8490] : scale/mode state selector, set internally to 9/0xa/0xb (movl at 0x89f5/0x8a12/0x8a2f);
              the 0x87be arm `mov ecx,0xa; sub ecx,[0x8490]; je/ja` loops the scale step call
              0xbc06 (ecx<0xa branch, scale-up) or 0xbed2 (ecx>0xa, scale-down), ecx = |0xa-[0x8490]| times.
  - position : [0x8648]/[0x864c] = blit x/y, centred by subtracting [0x5578]>>1 / [0x557c]>>1 (screen dims).
  - blit     : mov esi,[0x8644] (src frame ptr) ; mov edi,[0x85b8] (dest = frame surface) ; call 0x88af ;
              also sets [0xac60]=0xff, [0xac64]=[0x28a5] (palette/flags) ; then `xor eax,eax; ret` (status 0).
  - TCB      : mov esi,[0xc93]; add esi,0x6a at entry (the task's per-frame draw record base).
So op-0x78's faithful behaviour = set src [0x8644] from the posted command (task+0x3f2 frame id), dest
[0x85b8] = the frame surface (-> 0xA0000), position [0x8648/864c], scale [0x8490], then run 0x88af (the
core blit) with the 0xbc06/0xbed2 scale steps.  IMPLEMENT-SESSION MAP (all in re_out/fist_image.bin, i386):
  0x8799 gate entry ; 0x88af core blit ; 0xbc06 scale-up step ; 0xbed2 scale-down step ; state [0x8490];
  src [0x8644] ; dst [0x85b8] ; pos [0x8648/864c] ; screen [0x5578/557c] ; pal [0x28a5]->[0xac64], [0xac60].
Trace 0x88af/0xbc06/0xbed2 to byte-exact, wire them + the state setup into the extender shim's
fist_extender_gate (currently traps 0x8799->0), decode task+0x3f2, blit to 0xA0000; verify intro native==
wasm + AE vs a DOSBox intro frame; then land patch 411 across the intro; full 10x re-gate.

TRACTABILITY VERDICT (0x88af disassembled): op-0x78's blit is a SCALED ALPHA-BLEND RGB SPRITE RENDERER,
not a copy -- so board:0009 is genuinely a dedicated multi-function recovery, confirmed at the asm:
  - source byte -> blend-factor table [0x5599] (stride 3: `mov 0x5599(eax,eax,2),bl`); 0x1f = skip/term.
  - dest pixel  -> RGB palette [0x5260] (stride 3: `lea 0x5260(edx,edx,2)`).
  - TWO blend paths (jb at 0x890c): (A) 0x890e `shr bl,1; add 0x80; mul` per R/G/B; (B) 0x8935 weighted
    `shr bl,2; bh=~bl; mul dst*bh + bl` per R/G/B -> working RGB at [0xac68/69/6a], then nearest-index match.
  - wrapped by the scale loop 0xbc06(up)/0xbed2(down), shift by [0x8490]; addressing base [0x8498],
    mask [0x849c], src esi, dst-row [0x8838]=edi+[0x8498], screen [0x5578/557c].
Faithful reimplementation must recover: the source sprite format (+ its 0x1f run/skip), the [0x5599]
factor table, the [0x5260] RGB palette, BOTH blend formulas byte-exact, the RGB->8bpp match, and the scale
steps -- each verified native==wasm bit-exact.  This is a focused but real graphics-primitive recovery for
a dedicated session (with the symbol map above as its ready inputs), NOT a quick tail-of-session patch;
rushing the blend/match math would silently fail bit-identity.
