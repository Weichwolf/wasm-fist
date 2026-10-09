Type: Work item
Title: Generate owned heightmaps and colormaps from explicit JSON parameters
Depends: 0130

## Contract

Prioritize an original-file-free offline generator for high-quality owned terrain
and colormaps. JSON must specify octaves, hydraulic erosion, palette, water level,
temperature, moisture, authored geography, material transitions and lighting.
Produce reproducible higher-resolution outputs with manifests. Inspect actual
generated/reference maps and iterate until terrain forms and visual character
are very close; do not accept numerical correlation alone as visual quality.

## Evidence

The draft assets/generator/mapgen.py produces periodic floating-point terrain,
16-bit height PNGs and RGB colormaps from validated JSON. It implements periodic
noise layers, oriented hill/valley features, conservative four-neighbor hydraulic
water/sediment transport, temperature/moisture/material/palette composition and
explicit lighting. Generation has no original-file or decoder dependency.

Eight meaningful generator tests pass, including in a separate tree containing
only the tool, owned recipe and tests, with no original directory: deterministic generation and input ownership,
actual erosion/material conservation/nonnegative water, effective JSON controls,
water/cold palette behavior, periodic rotated features, malformed JSON rejection,
complete PNG/manifest/hash/repeated CLI output, and explicit rejection when
aggressive erosion moves terrain outside the configured export range. An initial check caught tiny
negative water after floating-point cancellation; transport now computes outgoing
remainders before incoming flux and explicitly rejects overdraw. No test was weakened.

All eight original height/color families were decoded under /tmp for development
comparison only. The first 512-square training-valley recipe reconstructs broad
terrain as analytic authored hill/valley parameters rather than a stored source
raster. Actual first comparison shows broadly matching topography, but excessively
smooth detail and an overly green colormap. The original/generated height
correlation of0.954 does not establish visual acceptance. Subsequent actual iterations increase authored layout detail to1024 features and
output to1024-square/16-bit height/RGB color. Excessive ridge noise produced visible
raster/worm patterns; periodic gradient noise replaces interpolated random values,
and relief/material tuning continues. An overly aggressive erosion pilot produced
unwanted sediment contours and is rejected visually despite conserved mass.
Final visual acceptance, other-family recipes and runtime consumption remain open.

The bounded generator foundation is verified: eight groups pass both in this
checkout and in /tmp/wasm-fist-map-generator-isolated, which contains only the
generator, owned JSON and tests. Independent full1024-square generation there
matches both PNGs and the complete manifest exactly. The validated baseline is
versioned under assets/generated/maps. Generator SHA256 is
5b410ff6c2585a30c4cab6f430df989aba1b8afe1a84931cf4b1078e0f16c83e;
recipe SHA256 is61ff285415aa245acab3227377ad7decead627343c8bf5513926d0bb57673215.
Actual baseline height/color and reference comparisons were visually inspected.
Broad geography and warm brown character are close, but local relief/material
variation remain too smooth. The0.987 height correlation is supplemental evidence,
not full visual acceptance. Compact review and reproduction pins remain in
/tmp/wasm-fist-map-generator-review/baseline-review.json. Rejected aggressive
variants are excluded from the repository baseline. This WI stays active.


## Next

Run and inspect the revised training recipe, tune terrain/climate/color controls
against actual reference images and validate high-resolution output. Version the
generator/tool pin, recipe, final draft outputs and reproduction commands after
their bounded contract is verified; keep this WI active until the full visual
comparison requirement is met. Extend recipes to every original terrain family,
add near-ground/native/browser checks during shared owned-map runtime integration,
and preserve final first-playable/complete-game acceptance as separate work.

## Accept

Generation and meaningful tests work from owned sources/JSON without originals.
Every control is explicit and effective, outputs are complete/reproducible and
material/water/height/palette bounds are validated. Actual reference/generated
height, colormap and reaching-scene reviews establish very close terrain forms
and appearance with improved detail for all required families. Missing content,
partial coverage, smooth placeholders or image hashes alone fail acceptance.
