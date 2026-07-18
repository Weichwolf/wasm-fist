# Mission cockpit/HUD — native bit-verify + the wasm mission-parity blocker

Status 2026-07-18. Engine pristine (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`), 341 patches, `make
check` clean, `verify.sh native` = 26/26 (no regression). No engine patch landed this iteration.

## Result

The **native** AZER1-spawn cockpit-dashboard region is **bit-exact (AE=0)** vs a genuine, non-circular
DOSBox reference. The **wasm** mission path **cannot render the frame** (hard crash) — so a dual-target
`mission-cockpit` verify flow is **NOT** landed (native↔wasm 0-diff is impossible until the wasm blocker
below is fixed). Honest: no flow, not a false-green.

## The bit-verified region (native)

- Frame: op-0x24 post #1 (`FIST_MISSFB`), deterministic. Drive `BATTLES(160,100)→OK(205,128)→
  ACCEPT(40,186)` via `FIST_MOUSE`, `FIST_TICK_HZ=25000`, NO `FIST_COOP_TICK` (coop breaks mouse timing).
- Region: **cols 80–180, rows 96–188** (9200 px, 81 % non-black content, 54 colours — the central tank
  dashboard/gauge chrome). `port AE=0` vs BOTH committed refs AND a fresh 3rd capture; the three DOSBox
  refs agree byte-for-byte on the region (phase-invariant static chrome), md5s differ from the port
  (non-circular). Ref added: `ref/mission_azer1_cockpit_native320.png` (fresh, `refcapture_mission.sh
  160 100 205 128 40 186 42 9 9 28`).

Repro (native):
```
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
DD=$(mktemp -d)/af; cp -a armoredfist "$DD"
FIST_DATADIR=$DD FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" FIST_MISSFB=/tmp/s.ppm /tmp/fist_native
# region AE vs ref/mission_azer1_cockpit_native320.png over cols80-180 rows96-188 == 0
```

## What is EXCLUDED from the region, and why (the cockpit residual assessment)

Full cockpit rows 88–200 carry **AE ≈ 6086** vs the settled ref. Pixel-classified (phase-invariant =
both non-port refs agree; port-defect = refs agree but port differs):

| sub-region | cols | rows | port-defect px | verdict |
|---|---|---|---|---|
| overhead terrain MAP box | 0–80 | ~112–192 | ~1800 | **DEEP-BLOCKED terrain** — the 93c0 map-view of the colormap (mission-terrain-tile-pipeline frontier); off-limits, not closable here |
| RADAR + left rim | ~180–244 | ~112–164 | ~850 | animated sweep (refs disagree 681 px = phase) + a static-frame content defect; not phase-pinnable to the port's op-count frame |
| HUD status text | 244–320 | ~100–190 | ~230 | mission-sim-state (heading/ammo) at a different sim phase (refs disagree 76 px) |
| bottom-center stray | 152 | 189–192 | 4 | port draws stray idx-207 (terrain band) where ref has dashboard tan (158,121,93) — a small localized paint defect |
| **central chrome** | **80–180** | **96–188** | **0** | **the banked region** — static, refs agree perfectly, port AE=0 |

So the AE=0 surface is the static central dashboard; the map box (deep-blocked terrain), radar (animated),
and HUD (sim-phase) are legitimately excluded — none is a tractable single base-loss/width fix.

## The wasm blocker (root-caused this iteration, deeper than the prior "op-0x50 leak" note)

The wasm mission run **never produces the frame**: it spins the extender op **0x50** ~1241× then aborts
("heap corrupted, address zero" = stack overflow of the recursive depth-sort `FUN_0000_2486`). Native
posts op 0x50 **zero** times. Trace:

1. On the AZER1 spawn build, `FUN_0000_5cce` builds viewport A (`4823→4886`, sets view-mode word
   `DAT_2000_2d2c=0x3a3`) then viewport B (`5d43→44be→466c→7740→7751`, resets `2d2c=5`). The object-roster
   render phase-7 handler `FUN_0000_c584` (patch 321) enters its per-object op-0x50 projection loop iff
   `2d2c & 0x80` (0x3a3→loop, 5→skip). So building viewport B is what prevents the runaway.
2. Both targets reach `7740`; native then dispatches `7751` (→`2d2c=5`), **wasm does not** — inside
   `7740`'s `1cdb(0x2c7f,…)` display-list build, the `FUN_0000_20d6` child walk dispatches **wild method
   offsets** (`0xd200/0x0420/0x0d32/0xb382/0x07a3/0x0518` — landing in the 0x9000 extender region) instead
   of the clean native sequence (`0x3021/0x2112/0x491c/0x2043`). The walk reads a stale/host-pointer field
   as a 16-bit node offset.
3. **Not a single clean defect.** At the diverging build point, **~167 DGROUP bytes already differ
   native↔wasm** (event queue `0x18e0–0x1921`, per-object table `0xa0xx` stride ~0x37, `0x074d–0x077b`,
   `0x0000–0x000f`, …). The `faf0` display working array is byte-identical; all inputs at `7740` entry
   (`fafe/fe04/fe06/2d34/3ae0`) are identical — the contamination is **accumulated** across the mission
   object subsystem before the render.

**Verdict:** genuine, **diffuse device-subsystem debt** — accumulated host-pointer / tick-timing
divergence in the mission object/event subsystem (native SIGALRM vs wasm cooperative-tick deliver the
scripted mouse events at different sim ticks; some object/event node carries a host pointer read as a
16-bit offset). NOT a faithful single-patch (store-width / base-loss) fix. `DAT_2000_2ce7` (the 5cce view
select) was already correctly byte-typed by patch 299 — it is NOT the cause (identical on both targets).

## What's needed to land the dual-target flow

Fix the wasm mission divergence: (a) make the mission event delivery tick-deterministic (or pin the frame
by a `FIST_DUMPTICK`-style sim-phase like the `campaign-missions` flow, so native/wasm reach the same sim
state), and (b) find the object/event-node field that leaks a host pointer into the 16-bit display-node
walk (`20d6` reads a wild 0x9000-region offset). Then the central-chrome region AE=0 is bankable on both
targets and the `mission-cockpit` flow (region-limited compare over cols80-180 rows96-188) can land. The
map box stays blocked (terrain), the radar/HUD need sim-phase pinning.
