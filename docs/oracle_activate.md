# Oracle recon — the mission-entry view state, captured on the ORIGINAL

**Date:** 2026-07-13 · **Method:** instrumented-DOSBox targeted memory WATCH (writer flat-IP + near-call
stack backtrace) on a live AZER1 mission, driven BATTLES→OK→ACCEPT via XTest, armed before ACCEPT.
**Scope:** recon + oracle tooling only; **no engine/ext/patch change**, pristine unchanged
(`re_out/fist.c`=`61453e42`, `fist_ext.c`=`0051cb56`, `fist_mga.c`=`75c6d726`).

This **DISPROVES the "cockpit-view activation → `DAT_2000_4d0e = 2`" premise for the reference frame.**

## HEADLINE — the reference cockpit terrain frame runs with `4d0e = 0`; `72d2`/`4d0e=2` is NEVER reached

### Engine load base in guest phys = `0x11190` (deterministic)
Located by finding `re_out/fist_dat_image.bin` content in a prior 16 MB RAM dump: engine offsets
`0x1000/0x5000/0x6f1f/0x72d2/0x8000` all sit at `phys = 0x11190 + off` (5/5 consistent, reproducible run
to run under `LOADGAME -K400,0,1000 -X5000`). So:
- engine code `FUN_0000_X` → phys `0x11190 + X`
- `DAT_2000_` (segment 0x2000) → phys `0x11190 + 0x20000 + off`  ⇒ **`4d0e` → `0x35e9e`**
- Cross-checked: `DAT_2000_7b18` reads `0x0200` at `0x34ca8` — matches the port's `FIST_R3D2` probe exactly.

### The WATCH (decisive)
Extended the instrumented DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`) with a phys-range watch
(`FIST_WATCHPHYS`/`FIST_WATCHSPAN`) that logs, for every guest write into the range, the writer's flat IP +
20-word near-call stack backtrace. `tools/oracle/trace_activate.sh` arms it before ACCEPT and runs through
the settled terrain frame.

- **Watch `[0x35e8a .. 0x35eb2)` (covers `4d0e`=`0x35e9e`): 36254 writes captured — `4d0e` itself: `0` writes.**
  The adjacent per-frame clear (`0x4cf0..0x4d06` → phys `0x35e80..0x35e96`, writer at engine offset
  `0x20a9`) fires **7251×** — proving the arm/base are correct — yet the byte at `0x35e9e` is **never
  touched** through the entire mission entry + settle.
- **⇒ `4d0e` stays `0` the whole time. `FUN_0000_72d2` (the only writer of `4d0e=2`) is NEVER executed.**

### Corroboration (independent)
- The settled frame IS the cockpit (`/tmp/oracle_azer1.png` = same dashboard, MPH gauge, radar, windshield
  terrain as `ref/mission_azer1_native320.png`; the residual AE≈23k is animated gauges / heading `1740` vs
  `1776` / LOADING-vs-READY / a slightly different camera angle — **not** a different view).
- The prior settled-frame RAM dump (`scratch/oracle/terrain.ram.bin`, a confirmed-cockpit frame) at base
  `0x11190`: **`4d0e = 0`**, **`2ce4 = 0xc30e`**, `4d52 = 0`, camera `4e1c/4e20/4e24 = 0`, player
  `2d34 = 0xc05c` (installed). `FUN_0000_c30e` disassembles as a bare **`ret`** — so the per-frame view
  vector `2ce4` is a **no-op** in this state, and `773e` (the cockpit render `72d2` would install) is NOT
  installed ⇒ `72d2` never ran. Fully consistent with the watch.

## Where the terrain is ACTUALLY drawn (from the writer trace `scratch/oracle/terrain.writers.txt`)
| writer | count | fb off | identity |
|---|---|---|---|
| `cs=002b eip=9260` / `9275` | 3.8M each | `[0656..6b66]` (rows 5-86 = windshield) | **EXTENDER `FUN_0000_9260`** (the 9200-family, op 0x09), flat `0x10009260` — 32-bit PM |
| `cs=4ec3 eip=2422/1a8d/1a42/…` | 231k↓ | `[931a..e8c8]`, `[0c94..8814]` (lower fb) | dashboard / HUD / gauges (driver/overlay 16-bit code) |

So the **windshield terrain = the EXTENDER's `9200`-family (op 0x09)** and the **dashboard = `cs=4ec3`
driver/overlay code** — and **all of it renders with `4d0e = 0`**.

## Consequence for the port (the corrected frontier)
1. **Do NOT reconstruct the cockpit trampolines (`0x7e22/0x8a1a/0x9255/0x9a20`) / `773e` and force `4d0e=2`.**
   The original reference frame does NOT do that — forcing it would DIVERGE from the reference (violates
   doctrine: *no blind force of 4d0e=2*). The port's current `4d0e=0` already MATCHES the original.
2. The reference cockpit **terrain** is the **extender op-0x09 (`9200`) render**, posted per frame with the
   **mission camera sourced from the extender TCB — NOT engine `4e1c` (which is `0` in both original and
   port)**. `FUN_1000_6f1f`'s `4d0e==2`-gated camera write is therefore NOT the reference camera path.
3. The engine's main render dispatch (`FUN_0000_22dd` → render-pass) posts the op to the extender; the port
   posts op `0x0c` instead of `0x09` (per `docs/stage1.md`). **The frontier is: (a) the colormap-tile fill
   (patch 288 got `85b8` 36→112; the `bc06/bc5a/bc9c` upsample into tile `3918` at map-load, per
   `docs/oracle_tile_writer_traced.md`), (b) the mission camera in the extender TCB, (c) the engine posting
   op-0x09 with that camera** — all at `4d0e=0`. The `FIST_R3D2` seam already renders `9200`; it just needs
   the real TCB camera (it currently uses the degenerate intro TCB) + the filled tile.

## Tooling added (reproducible)
- `tools/oracle/dosbox_vga_terrain_trace.patch` — now also adds a targeted **phys WATCH**: `FIST_WATCHPHYS`
  (center) + `FIST_WATCHSPAN` (± bytes) → `<prefix>.watch.txt` logs `ph val flatip cs eip ss sp stack:[20w]`
  for each write, gated on the same SIGUSR1 arm as the tile recorder.
- `tools/oracle/trace_activate.sh` — BATTLES→OK→ACCEPT with the watch armed before ACCEPT; default watch =
  `4d0e` region (`FIST_WATCHPHYS=0x35f50 FIST_WATCHSPAN=0x100`), overridable.

## Reproduce
```bash
# rebuild instrumented dosbox (adds the watch): patch + make in /tmp/debs/dosbox-0.74-3, cp src/dosbox /tmp/debs/dosbox-fist
FIST_WATCHPHYS=0x35e9e FIST_WATCHSPAN=0x14 DOSBOX=/tmp/debs/dosbox-fist \
   bash tools/oracle/trace_activate.sh 160 100 205 128 40 186 60 12 12 25 $PWD/scratch/oracle/act4d0e
grep -c "ph=00035e9e" scratch/oracle/act4d0e.watch.txt    # -> 0  (4d0e never written)
```
