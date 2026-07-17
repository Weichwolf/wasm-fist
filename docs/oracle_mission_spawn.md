# Oracle frame-matched AZER1 mission SPAWN capture — the port's black terrain is a RENDER bug, not a flight-model gap

> ## VERDICT (2026-07-17) — **THE ORACLE RENDERS FULL VOXEL TERRAIN + SKY AT FRAME-1 / SPAWN.** The
> port's black windshield is a **render-wire + camera-Z bug fixable NOW** — it does NOT need the
> extender flight-model to first drive/settle the vehicle. Proven by a genuine frame-matched DOSBox
> capture of the ORIGINAL at the earliest mission frame, compared against the port's deterministic
> frame `6bc113e6` at the SAME sim state.

## Method (genuine, non-circular, 1:1 320×200)
`tools/oracle/capture_mission_spawn.sh` drives the ORIGINAL (`LOADGAME FIST.RUN`) under the instrumented
DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`, `/tmp/debs/dosbox-fist`) through
BATTLES→OK→ACCEPT, then polls the SDL surface every 0.3 s. The `LOADING MISSION WITH SUPER DETAIL…`
photo grabs are large PNGs (~30–40 KB); the **first windshield frame** drops to ~14–22 KB. That
transition **is** frame-1 / spawn. The grab is the `output=surface scaler=none aspect=false` mode-13h
surface (`-trim` rect asserted **320×200**), 6→8-bit DAC = `(v<<2)|(v>>4)` — the same expander
`fist_vga.c`/`FIST_MISSFB` use, so it is byte-comparable to the port dump.
**Non-circular:** the reference is DOSBox rendering the ORIGINAL, never a copy of the port; its md5
(`75d40a0d…`) ≠ the port frame-1 md5 (`6bc113e6…`).

At the exact frame-1 grab, `SIGUSR2` dumps guest RAM; the live camera is read by scanning for the
spawn XY.

## What the oracle draws at frame-1 / spawn
- **Full voxel TERRAIN is rendered** — brown/tan hills, 91 distinct colours, only 1.2 % black in the
  windshield (rows 8–88). It is NOT a fade-in / black / cockpit-only frame.
- **SKY is present** — blue cloud band (e.g. RGB `93,97,117`) across the top of the windshield.
- The LOADING photo → windshield transition is a hard cut; the very first windshield frame already has
  the terrain drawn (camera barely moved: odometer ≈ spawn, XY within ~500 units of spawn).

## The camera at frame-1 / spawn (guest RAM, live TCB mirror @ phys `0x1002c`, robust across 2 runs)
| | X | Y | alt | terrain h = alt/≈292 |
|---|---|---|---|---|
| **oracle spawn (frame-1)** | 584312 | 1142051 | **12800** | **43.8** |
| mission-file spawn record (`0x034d14`) | 583982 | 1142557 | 0 | — |
| **port live spawn** (per task) | 583982 | 1142557 | **3328** | 11.4 |
| oracle settled (task) | — | — | 26880 | 92 (h≈97) |

- The oracle camera Z **tracks the terrain height under the camera**: at spawn `h=43 → alt≈12800`
  (`43×292≈12556`), settled `h=97 → alt≈26880`. This "climb" is the **tank driving from low ground
  (h=43) onto higher ground (h=97)** — it is applied per-frame (instant terrain-follow at spawn), NOT a
  multi-second altitude ramp.
- **The port's spawn alt (3328) is itself wrong** — far below the oracle's 12800; 3328 corresponds to
  h≈11.4, i.e. the port camera is set well BELOW the terrain surface it sits on. The port likely sets
  camera Z to a fixed eye offset WITHOUT adding `terrain_height × scale`.

## The CORRECT comparison (port frame-1 `6bc113e6` vs oracle frame-1, both at spawn)
Cockpit chrome is already registered (proof the frames are aligned), so the whole delta is the missing
terrain+sky render — not a sim-position mismatch:

| region | metric |
|---|---|
| terrain/windshield rows 8–88 (25600 px) | **AE = 23438** |
| port terrain black | **87.2 %** (vs oracle **1.2 %**) — the window is empty, not garbage-terrain |
| port / oracle distinct terrain colours | 74 / 91 |
| sky band rows 8–20 | port 90 % black; oracle blue `93,97,117` |
| cockpit gearstick/gauge chrome (rows 108–150, cols 88–160) | **AE = 0** (frames registered) |

The prior frame-1-vs-**settled** terrain AE was 24618; this frame-1-vs-**frame-1** AE (23438) confirms
the magnitude was not driven by the sim-position mismatch — it is the missing render.

## Consequence for the frontier
The port's black terrain is **fixable now**, independent of the extender flight/momentum model:
1. **Render-wire:** the windshield voxel writer (extender `9200`, dispatched via op-0x24) must actually
   draw into `0xA0000` — the oracle proves terrain renders at the exact spawn camera state.
2. **Camera-Z init:** set the spawn camera alt to follow terrain height (`h×scale + eye`) so it matches
   the oracle's ~12800 (not the current 3328); the port camera is presently below the surface. This is
   an instant per-frame terrain-follow, NOT the gradual flight-model settle.

Whether (1) alone suffices, or (2) is also required for 9200 to produce non-black pixels, is the
port-side next step (set the port camera alt to 12800 and re-check 9200's output) — outside this
capture task.

## Artifacts committed
- `ref/mission_azer1_spawn_native320.png` — genuine 1:1 320×200 oracle frame-1 (md5 `75d40a0d…`).
- `tools/oracle/samples/oracle_mission_loading.png` — the LOADING photo (context; the frame BEFORE spawn).
- `tools/oracle/samples/oracle_mission_settled_ref.png` — a settled windshield (context; terrain evolved).
- `tools/oracle/capture_mission_spawn.sh` — reproducible capture + live-camera read.

## Repro
```bash
# needs the instrumented dosbox (tools/oracle/README_terrain.md) at /tmp/debs/dosbox-fist
DOSBOX=/tmp/debs/dosbox-fist tools/oracle/capture_mission_spawn.sh
# port frame-1 for the AE comparison (md5 6bc113e6):
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 \
  FIST_MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0" \
  FIST_MISSFB=/tmp/port_f1.ppm FIST_MISSFB_N=30 /tmp/fist_native
```
