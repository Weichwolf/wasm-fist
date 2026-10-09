# Mission terrain and environment inventory

This is optional original development evidence for owned-content planning.
It contains identities and associations, without original image, palette or
scenario payloads. The completed game must load owned mission/environment
definitions and FMAP assets; these original filenames are not runtime inputs.

All 47 pinned scenarios use eight height/color geography pairs and 19 normalized
height/color/palette/sky combinations. There are four sky identities: 2, 5, 7 and
8. DOS spelling differences are folded before counting. The eight owned map
recipes cover geography baselines; their current single RGB map per family does
not establish complete coverage of these 19 environment appearances.

| Owned geography family | Reference height/color pair | Palette / sky | Missions |
| --- | --- | --- | --- |
| rocky-highlands | D03.KLC / C03.KLC | 203.PAL / 2.SKY | UKRAINE5 |
| rocky-highlands | D03.KLC / C03.KLC | 503.PAL / 5.SKY | AZER7, INDIA3, TRAIN2, UKRAINE7 |
| rocky-highlands | D03.KLC / C03.KLC | 803.PAL / 8.SKY | CYPRUS4, UKRAINE3 |
| temperate-forest | D06.KLC / C06.KLC | 506.PAL / 5.SKY | CYPRUS1, CYPRUS5, INDIA1, INDIA4 |
| temperate-forest | D06.KLC / C06.KLC | 706.PAL / 7.SKY | UKRAINE6 |
| arid-ridges | D07.KLC / C07.KLC | 207.PAL / 2.SKY | AZER6, CYPRUS6, CYPRUS7 |
| arid-ridges | D07.KLC / C07.KLC | 507.PAL / 5.SKY | SYRIA7, TRAIN4 |
| arid-ridges | D07.KLC / C07.KLC | 707.PAL / 7.SKY | INDIA5 |
| arid-ridges | D07.KLC / C07.KLC | 807.PAL / 8.SKY | SYRIA5, SYRIA6 |
| sandy-desert | D08.KLC / C08.KLC | 208.PAL / 2.SKY | SAUDI3 |
| dry-mountains | D12.KLC / C12.KLC | 212.PAL / 2.SKY | SAUDI2, SAUDI6 |
| limestone-valleys | D30.KLC / C30.KLC | 530.PAL / 5.SKY | SAUDI1, SAUDI7, SYRIA1 |
| limestone-valleys | D30.KLC / C30.KLC | 730.PAL / 7.SKY | SAUDI4 |
| snowy-alpine | D31.KLC / C31.KLC | 531.PAL / 5.SKY | TRAIN3, UKRAINE4, UKRAINE8 |
| snowy-alpine | D31.KLC / C31.KLC | 831.PAL / 8.SKY | AZER2, AZER4, AZER5 |
| training-valley | D32.KLC / C32.KLC | 232.PAL / 2.SKY | CYPRUS2, INDIA2, SAUDI5, SYRIA2, TRAIN1, UKRAINE2 |
| training-valley | D32.KLC / C32.KLC | 532.PAL / 5.SKY | AZER1, INDIA6, SYRIA3 |
| training-valley | D32.KLC / C32.KLC | 732.PAL / 7.SKY | AZER3, CYPRUS3, UKRAINE1 |
| training-valley | D32.KLC / C32.KLC | 832.PAL / 8.SKY | INDIA7, SYRIA4 |

The first training mission, TRAIN1, uses training-valley geography with 232.PAL
and 2.SKY in the reference. Do not infer that all four training missions share
its map or environment: TRAIN2 uses rocky-highlands, TRAIN3 snowy-alpine and
TRAIN4 arid-ridges. The original sky numbers alone do not prove time of day,
weather or lighting semantics; inspect actual imagery/behavior before naming
owned environment presets.

## Evidence and reproduction

Inventory capture compared complete current native scenario-probe transcripts
with the independent interpretation in tests/test_scenario.py for every pinned
file in tests/scenario_originals.json: 47 inputs, 574215 bytes and 4213 unit
records. Each original size/SHA256 was checked before use and SHA256 afterward.
The native probe came from the original-file-free 0133 build at source base
075e9f5 with softgl pin 1d17a94. Native metadata observations are sufficient for
these file associations; they do not establish owned mission simulation or
environment rendering on either target. Capture commands, input/probe/interpreter
hashes and per-mission transcript hashes remain under
/tmp/wasm-fist-terrain-catalog-review. Original sources remain ignored/read-only.

The existing optional development gate reproduces complete native metadata
checks, using a separately built scenario probe:

```sh
python3 tests/test_scenario.py --originals --target native --native-probe /tmp/wasm-fist-owned-terrain-production/native/fist_scenario_probe
```

All eight groups passed without skips in this native reference check. The
inventory association capture also completed without missing inputs or failures.

## Remaining owned work

Own all 47 mission definitions, including spawns, routes, objectives, campaign
relationships and environment choices. Define explicit procedural sky, lighting
and terrain-material controls where these reference combinations differ; review
actual images before deciding which presets can share content. Palette files
must not be copied or needed. Preserve full RGB and 16-bit heights through the
shared owned loader and renderer, rather than reintroducing original palette
indices. Verify placements/traversability, editor round trips and native/browser
visuals for every mission/environment. Geography coverage alone does not close
0130, 0131, first playable 0041 or complete-game acceptance.
