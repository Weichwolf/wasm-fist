Type: feature
Title: Campaign progression persists and reloads byte-identically

## Contract

Persist the same player/campaign progress as the original and restore the same next playable state.
Do not invent a mid-mission save feature unless the original exposes one.

## Evidence

The recorded survey found 230-byte .FPL pilot profiles and no separate savegame subsystem.
Profile read/rewrite is already exercised; a completed-mission progression round-trip is not.
Offsets 0x24..0x53 are candidate progress records from cross-profile differences, not an
oracle-confirmed field layout. Filename carries the pilot name.

## Next

1. Use isolated copies of the same profile. Complete the same campaign mission on the original
   and port; retain before/after files and identify the exact writer and checksum semantics.
2. Document only fields confirmed by the write trace. Include promotions/stats, campaign position
   and any failure/abort behavior the original persists.
3. Restart, select that profile and continue the campaign on both targets. Compare file bytes,
   selected mission and resumed engine state to the original.

## Accept

Hermetic progression flows cover success, failure/abort and campaign transition where supported.
Saved bytes, reload state and subsequent deterministic behavior match. Originals stay read-only.
