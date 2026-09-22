Type: feature
Title: The PIT clock and browser presentation preserve original timing

## Contract

Keep one deterministic PIT timeline and transparent interrupts on both targets; pace browser
presentation without changing simulation/audio state or hiding timing differences from the original.

## Evidence

The shared clock, ISR register/flag save-restore and BIOS chaining landed (576/582 and shim work).
Mode-13h period is modeled as 17025 PIT counts; calibration measures 0x427f. The historical PLL reload
settled near 0x411c versus oracle 0x426e because status-poll skipping changed poll count. Equal average
frame rate does not by itself settle that difference. Scripted mouse times now use menu-relative
vblanks; old FIST_COOP_TICK/FIST_TICK_HZ/SIGALRM instructions are obsolete.

## Next

1. Capture retrace, channel-0 reload and INT-8 events around calibration and re-arm. Determine whether
   poll skipping changes observable event/sample order; preserve it exactly if it does.
2. Audit port-driving oracle scripts for old mouse-time units, especially r92/9200 and debrief tools.
   Keep DOSBox wall-clock XTEST timing distinct from port vblank scripts.
3. Run the actual browser worker build (`make web`); measure vblank/presentation and input latency,
   background/resume and audio continuity. Verify Atomics.wait/shared-memory prerequisites.
4. Recommendation: pace at the presentation boundary using the existing deterministic clock;
   do not inject wall-clock-dependent simulation ticks. Retain an event trace to compare with node.

## Accept

Matched PIT/retrace/ISR effects and sample ordering, correct fades, responsive live browser play,
and native↔WASM parity. No claim of browser functionality based solely on node matrix passes.
