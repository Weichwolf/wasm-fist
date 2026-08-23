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
