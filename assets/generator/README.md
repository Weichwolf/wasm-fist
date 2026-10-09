# JSON terrain generator

`mapgen.py` is an offline owned-content tool. It reads only its JSON recipe and
does not need original game files, decoders, Blender or reference tools. NumPy
2.2.4 is pinned in `requirements.txt`; the game runtime remains C11.

```sh
OPENBLAS_NUM_THREADS=1 PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  python3 assets/generator/mapgen.py assets/maps/training-valley.json \
  --output-dir /tmp/wasm-fist-generated-maps
python3 tests/test_map_generator.py

# Generate every owned family sequentially after validating all recipes.
OPENBLAS_NUM_THREADS=1 PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  python3 assets/generator/generate_maps.py --output-dir /tmp/wasm-fist-generated-maps
```

Recipes are strict version-2 JSON; unknown, missing, duplicate and nonfinite
fields fail. Version 2 adds mandatory `domain_warp` controls; use empty X/Y octave
lists for unwarped geography. Historical version-1 recipes remain reproducible
with their recorded generator revision. All authoring controls are explicit:

| Field | Meaning |
| --- | --- |
| `name`, `seed`, `resolution` | Stable identity, PCG64 seed and square output size. |
| `height_range`, `base_height` | World-height encoding bounds and initial elevation. |
| `octaves` | Periodic gradient-noise layers: frequency, amplitude, seed offset, smooth/ridge kind and ridge power. |
| `domain_warp` | Periodic X/Y gradient-noise octave lists, with amplitudes in normalized map coordinates (0–0.25); deform analytic contours without source rasters. Empty lists preserve unwarped coordinates. |
| `features` | Analytic oriented hill/valley layout: normalized X/Y, both radii, angle and signed height. No source raster is embedded. |
| `hydraulic_erosion` | Iterations, time step, rainfall, evaporation, flow rate, sediment capacity, erosion/deposition rates and bedrock height. |
| `water` | Sea level, shoreline climate width and color depth scale. |
| `temperature` | Base Celsius, elevation lapse, periodic north/south variation and noise octaves. |
| `moisture` | Base moisture, shoreline gain and noise octaves. |
| `palette` | Explicit RGB lowland/upland/rock/sand/snow/shallow-water/deep-water colors. |
| `materials` | Height/slope, snow-temperature and dry-moisture transitions. |
| `lighting` | Azimuth/elevation, ambient/diffuse contribution and normal scale. |
| `color_noise` | Procedural surface color variation octaves. |

Coordinates wrap over the normalized map period. Raster rows increase in map Y;
export does not flip the authoring geometry. Hydraulic erosion uses four-neighbor
periodic finite-volume water/sediment transport in authoring grid units. Flux
cannot exceed available material; remaining sediment settles before output, and
total terrain material is checked. Tuning must account for output resolution.
An eroded height outside the configured export range fails instead of clipping
away material. These are artistic terrain controls, not vehicle physics constants.

Dryness selects sand in lowlands before elevation and slope select upland/rock.
A dry climate must not erase the configured elevation materials. Temperature
selects snow, while the configured rock-slope transition keeps steep rock faces
exposed. These deliberate visual rules are covered by separate behavior tests.

Outputs are a lossless unsigned 16-bit grayscale height PNG, an RGB8 colormap and
a manifest with recipe/generator hashes, NumPy version, dimensions, encoding,
statistics and output hashes. Decode heights as
`minimum + sample / 65535 * (maximum - minimum)`. PNGs contain complete unfiltered
rows with checked chunk CRCs. The manifest records generation, not visual approval.

All eight recipes are development baselines. Actual reference comparisons and
quality findings belong to active 0131; full family visual acceptance and owned runtime scene
integration remain open. Passing generator tests does not prove a complete game or
the requested visual similarity. Keep review captures under `/tmp`, version the
authoring recipe and validated generated baseline under `assets/`, and iterate
actual images before accepting quality.

`pack_maps.py` losslessly prepares generator outputs for shared C11 loading,
without a runtime PNG/deflate dependency. It checks the recipe/manifest and every
PNG hash, chunk, stream and row before writing a batch. See
`docs/owned-map-format.md` for FMAP1 layout and the package reproduction command.
