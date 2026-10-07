Type: Work item
Title: Untargeted M1 primary fire dispatch and live payload publication
Depends: 0065, 0067, 0074, 0075

## Contract

Consume the original M1 pending/automatic fire branch and complete untargeted station-zero
7e29 dispatch, including HUD/component refresh, reload blocking, handler carry and failed-attempt
cooldown. Reuse the launch owner and publish returned shell/muzzle payloads into the shared
mission world at their actual physical slots. Preserve unrelated world state and atomically
reject invalid input. Keep selected-player voice/device gates as explicit producer consumers;
requests are not PCM. Unsupported stations/targeted fire and full living-class orchestration
remain required, with no silent dispatch substitutes. No device fire binding until the live
flight/damage/update consumer is connected.

## Evidence

Original 7c65..7c7b decrements a nonzero pending byte before calling 7e29; otherwise secondary
bit 80 alone requests the dispatch. 7e29 calls complete f69:7a46 (physical 170d6), marking
all four weapon-panel globals
8e62/66/6a/6e with 3, then marks M1 component +db before reload blocking. Behavior +3e and
byte +63 are preserved. The first complete original comparison rejected the initially wrong
mode reset: that hypothesis came from reading physical 171d6 instead of 170d6. Original
segment arithmetic and the complete raw-record difference prove the correction.
Station zero is blocked while byte +a8 is nonzero. Actual handler f69:80b5 returns carry;
success requests DGROUP 8f06's voice byte 12 at bf3c, failure calls bf77. bf77 updates shared
word 9fce and requests cue 13 only when unsigned wrapped tick-minus-last reaches 240. Actual
bf3c selected-player/side/timing/device gates are separate from this request boundary.

### Verified implementation

The shared fire owner reuses the complete launch transaction. The mission wrapper publishes
full type-8/type-18 payloads at actual allocated physical slots before further world visits.
Source storage is released before the real 85-object TRAIN1 world selects station zero,
crosses declared reload/request stages and fires; unrelated complete world bytes remain intact.
These are selected stage calls, not 320 complete mission ticks or a complete living dispatcher.

The first WASM run exposed a verification-runner stack overflow, also corrupting the existing
launch mode. Main reserved 64,624 bytes and rejected_fire another 32,320 against the default
65,536-byte stack. Owning large verification worlds/captures on the heap reduces these frames
to 1,920/208 bytes, with all allocation failure paths freeing storage. No stack setting or
assertion was weakened. The initially stopped production build is not acceptance evidence.

Verified on 2026-10-07 against the final source hashes:

- Strict LLVM 19.1.7 format/tidy passes all 70 owned C units.
- Required native/WASM original fire gates pass all seven groups in 28.996/36.382 seconds,
  with zero skips. Each target has 2,828 fixtures and 4,480 complete stage observations,
  all 47 pinned occupancy contexts/all 179 M1 actors and the separate real TRAIN1 flow.
- Existing launch both-target original regression passes all eight groups in 29.761 seconds,
  including all 179 original M1 actors and all 47 complete occupancy contexts, without skips.
- Production-flags ASan/UBSan/LSan passes all seven required groups and complete fire totals
  in 11.332 seconds, without skips. The required WASM semantic gate also passes all seven
  groups in 14.639 seconds with the same complete totals and zero skips.
- Final production build: all 26 native CTest gates pass in 148.90 seconds. The required default
  WASM fire gate passes seven groups in 12.227 seconds with 2,648 fixtures/4,121 observations
  and two explicitly optional original groups. The complete sequential build is terminal with
  exit zero: all 24 required WASM Python gates and both actual renderer probes pass. Default
  gates declare their separate optional original/device groups; required fire/launch corpus
  and original comparisons above have zero skips.

See docs/primary-fire.md for complete reproduction, original instruction boundaries, the
corrected segment evidence, preservation assertions and truthful presentation/audio limits.

## Next

This bounded stage is accepted. Compact source/binary/log hashes and terminal results are
retained in /tmp/wasm-fist-0076-fire-review; obsolete owned logs and sanitizer binaries are
removed after verified commit/push. Keep 0065/0041 active. Consume canonical shell/muzzle
payloads through flight, damage-before-impact continuation and effect/retirement publication
using the existing current-entry iterator. Recover complete reached living methods and command
eligibility before binding device fire. Battle drawing, audible PCM and objectives/outcomes
remain required for a playable mission. No unknown class may receive a successful empty update.

## Accept

Both targets prove the declared complete untargeted M1 primary branch, handler and cooldown,
correct physical shell/muzzle publication and preservation of unrelated owners. Original
instructions support every implemented rule. Required build/style/original/memory gates pass;
reference/originals stay pristine, and the bounded success is committed/pushed/cleaned. This
does not claim complete living-class ticks, targeted/all-station firing or a playable mission.
