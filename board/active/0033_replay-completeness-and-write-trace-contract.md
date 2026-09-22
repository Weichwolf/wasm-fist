Type: bug
Title: Frame and PCM acceptance rejects missing, malformed and truncated output
Parent: 0012

## Contract

Existing snapshot/audio flows compare complete captured frames and PCM samples. A missing output,
failed producer, invalid format, unequal length or missing required reference fails acceptance.
Internal memory equality is diagnostic only. Complete temporal sequences belong to board:0034.

## Evidence

Source review: verify.sh counts lines from `cmp -l` while discarding its status, so an equal prefix
with an extra suffix can pass. Frame output and configured reference absence can also pass. Several
capture helpers accept a file even after producer failure. An empty flow selection passes.
`tools/wavcompare.py` is a correlation diagnostic and can print BIT-EXACT after resampling/truncating
with nonzero tolerance; it must not claim exact acceptance. Diagnostic replay gaps moved to 0035.

## Next

1. Add strict frame/PCM comparisons and negative tests for missing/truncated/extra/corrupt outputs,
   failed producers, missing references and empty selections; demonstrate failure on old behavior.
2. Integrate them into the existing matrix without weakening any original-reference assertion.
3. Run the affected real flows on both targets, then complete the regression matrix. Retain commands,
   exact scope and logs; commit/push only after the bounded acceptance below passes.

## Accept

All negative cases fail. Valid frame and PCM captures compare exactly on both targets. PCM payload
comparison ignores harmless container metadata, never rate/channel/width or missing/extra samples.
Snapshot success is labeled as snapshot success; no claim of full temporal fidelity.
