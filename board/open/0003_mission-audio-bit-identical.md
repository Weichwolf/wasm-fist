Type: feature
Title: Menu and mission audio match the original sample-for-sample

## Contract

Produce the original OPL, digital effects and mixed output over menu, intro, mission, cockpit switch
and debrief. Match both targets and the original at a defined sample rate and capture boundary.

## Evidence

- Menu note content is gated by board:0011; earlier silence came from sequencer re-entry through
  port I/O → timer pumping, corrected in d7bd0aa. The missing-program-change/vector-base theories
  were retracted. Do not reimplement already-working instrument dispatch.
- Menu/intro native↔WASM WAV comparisons passed historically. Note order is not sample equality.
- Old mission-audio divergence investigations predate the PIT clock and patch 610. Sustained
  missions now run (board:0012); the old “mission load is unreachable” blocker is stale.
- Mission mixer work remains: op 0x64 → 786a → 22ab assigns channels; mixer 2630 advances cursor
  0x15ef by pitch 0x15cb and frees a channel when cursor>>16 reaches length 0x15e3.
- Mixer completion alone cannot repair simulation reads of relocated segment bytes (board:0017).
- Source audit at 22dfff2: OPL and SB have independent `wav_open()` writers targeting the same
  `FIST_AUDIO_WAV` file with `wb`. Browser `fist_web_post_audio()` drains only OPL. A simultaneous
  OPL+SB run must first prove final mixer ownership; two equal WAVs can share missing digital effects.

## Next

1. Capture fresh native/WASM/original menu and AZER1 streams with matched devices, rate, RNG and
   boundaries. Establish the first differing event or sample before choosing a fix.
2. Compare timed OPL register writes and SB channel/DMA events separately, then mixed PCM.
   Recommendation: one final mixer owns WAV/browser output; preserve device streams for diagnosis.
   Reproduce simultaneous OPL+SB emission and buffer saturation before changing either output path.
   Recommendation: drive sample production with an integer remainder accumulator over the shared
   PIT clock; derive the conversion from captured rate/timing, not a fitted MUSIC_DIV constant.
3. Recover missing channel allocation, pitch, cursor, completion and output semantics from the
   extender's own handlers. Test overlapping effects, channel exhaustion and release.
4. Extend through mission music, player loss and debrief; hand reusable content fixtures to 0011.

## Accept

Equal sample counts and PCM bytes on both targets and against the matching original mixer output.
No resampling, time-warping or correlation threshold may replace the exact final comparison.
Record any deterministic start-offset alignment explicitly; test device configurations separately.

## Evidence entry points

`FIST_AUDIO_WAV`, `FIST_OPL_REGLOG`; `tools/oracle/capture_audio.sh`, `trace_opl.sh`,
`noteseq_compare.py`; `ref/audio_menu_noteseq.txt`. Never compare OPL-only mono against an
intro-contaminated full-mixer stereo reference and diagnose the resulting difference as a bug.
