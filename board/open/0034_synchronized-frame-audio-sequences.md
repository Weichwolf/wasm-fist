Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented frame with its presentation time and the continuous PCM stream over explicit
scenario start/end boundaries. Same initial state, timed input and devices; no host-stack equality.

## Evidence

Source audit at 22dfff2 (no sequence capture yet):

- Original: `third_party/dosbox-build/dosbox-0.74-3/src/gui/render.cpp`, `RENDER_EndUpdate`,
  supplies scanout pixels/palette to `CAPTURE_AddImage`; `hardware/mixer.cpp`, `MIXER_MixData`,
  supplies clipped stereo PCM to `CAPTURE_AddWave`. Check aborts, frameskip and the 1024-sample
  capture cap before treating these callbacks as complete. Capture mode also affects mixer timing.
- Port: `tools/native_main.c`, `fist_web_vblank`, posts a frame/audio on INT-8. Frame posting calls
  `fist_web_force_palette`, replacing DAC state from an engine buffer. Snapshot/native and browser
  presentation are therefore distinct paths; matching Node snapshots does not verify the browser.
- Browser audio drains only the OPL ring (`re_out/fist_opl.c`). OPL and SB each open the same
  `FIST_AUDIO_WAV` path independently with `wb`; the existing WAV is not evidence of a final mix.
  Both rings silently stop appending at capacity. Final output ownership belongs to board:0003.

## Next

1. Locate actual presentation and mixer-consumption boundaries on the original and both targets.
   Start with the intro→menu transition; a decoder frame-pin bypass is not a presentation trace.
2. Recommendation: share a passive port capture sink between native and browser presentation;
   hook original completed scanout/final mixing independently. Do not force palettes or add ticks
   to make captures agree. First record whether the suspected boundary differences occur.
3. Define one streaming capture format with monotonic event identity/time, frame dimensions and
   palette-resolved pixels (or indices plus DAC), PCM format/sample position, and completion record.
4. Record synchronously at those boundaries. Prove callback coverage and overflow/truncation failure.
   Frame duplication/removal matters; audio buffer chunk sizes may differ with the same PCM stream.
5. Add a three-way runner with isolated inputs, artifact retention and a first-difference report.
   Use memory/register traces only inside the first failing interval, then widen to mission runs.

## Accept

All three captures cover the same complete intro/menu scenario with independent oracle provenance.
The runner reports the first differing frame/time/sample; dropped, added or reordered frames,
changed samples and unfinished captures fail. Seed these faults to prove detection. A reproducible
original/port difference stays red and gets its producer WI; capture completion does not close
parity (0012), audio fidelity (0003) or presentation timing (0026).
