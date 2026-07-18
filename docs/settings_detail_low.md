# SETTINGS — DETAIL LOW radio (verify flow `settings-detail-low`)

Fills the DETAIL group's third radio, uncovered by `settings-detail-med` (patch 319 already
handles all three DETAIL radios — **no new engine patch**).

## Layout (from the base vs med diff, radio column x≈181)
DETAIL radios stack `LOW / MED / HIGH` at y `44 / 52 / 60`; default HIGH lit.

## Path
click (181,44) → `6af0` → `6b27` → `6bcc` (bx=0x12) → `6eb8(value 0)`:
- DETAIL state word `0x8b47 = 0`
- status "DETAIL SET TO LOW" via `7018`, per-value status id from `DGROUP:0x8b3d`
  (the LOW slot; MED used `0x8b3f`, HIGH `0x8b41`)
- marks the 3 DETAIL widgets dirty (BYTE) → dirty-walk `209e`/`6cf4` moves the lit
  indicator HIGH → LOW.

Exercises the value-0 / `0x8b3d` status slot the MED flow does not.

## Reference (genuine, non-circular)
`tools/refcapture_click2.sh 160 126 181 44 40 8 8 ref/settings_detail_low_native320.png`.
Two independent DOSBox captures agree AE=0 (deterministic). Port native AND wasm both AE=0
vs the ref; native↔wasm 0-diff. Full suite `bash tools/verify.sh both` = 26/26.
