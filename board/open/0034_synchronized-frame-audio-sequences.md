Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented frame with its presentation time and the continuous PCM stream over explicit
scenario start/end boundaries. Same initial state, timed input and devices; no host-stack equality.

## Next

1. Locate actual presentation and mixer-consumption boundaries on the original and both targets.
   Start with the intro→menu transition; a decoder frame-pin bypass is not a presentation trace.
2. Define one streaming capture format with monotonic event identity/time, frame dimensions and
   palette-resolved pixels (or indices plus DAC), PCM format/sample position, and completion record.
3. Record synchronously at those boundaries. Prove callback coverage and overflow/truncation failure.
   Frame duplication/removal matters; audio buffer chunk sizes may differ with the same PCM stream.
4. Add a three-way runner with isolated inputs, artifact retention and a first-difference report.
   Use memory/register traces only inside the first failing interval, then widen to mission runs.

## Accept

A complete intro/menu scenario matches original/native/WASM output and timing. Dropped, added or
reordered frames, changed samples and unfinished captures fail. Record independent oracle provenance.
