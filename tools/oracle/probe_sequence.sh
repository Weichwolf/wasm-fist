#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DOSBOX="${DOSBOX:-$ROOT/third_party/dosbox-build/dosbox-0.74-3/src/dosbox}"
DURATION_SEC="${1:-50}"
OUT="${2:-$ROOT/scratch/sequence-probe/run}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [ -e "$OUT/game" ]; then
  echo "capture directory already used: $OUT" >&2
  exit 2
fi
cp -a "$ROOT/armoredfist" "$OUT/game"
cat > "$OUT/dosbox.conf" <<CFG
[sdl]
fullscreen=false
output=surface
autolock=false
[render]
frameskip=0
aspect=false
scaler=none
[dosbox]
memsize=16
[cpu]
core=auto
cputype=auto
cycles=max
[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
[autoexec]
mount c $OUT/game
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11 FIST_SEQPROBE=1
set +e
timeout --signal=TERM "$DURATION_SEC" xvfb-run -a --server-args="-screen 0 1024x768x24" "$DOSBOX" -conf "$OUT/dosbox.conf" -exit > "$OUT/dosbox.log" 2>&1
status=$?
set -e
printf 'process status: %s (124 means scenario time limit)\n' "$status" > "$OUT/status.txt"
if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
  cat "$OUT/status.txt" >&2
  exit "$status"
fi
python3 - "$OUT/dosbox.log" <<'PY'
import collections
from pathlib import Path
import sys
lines = Path(sys.argv[1]).read_text(errors='replace').splitlines()
frames = [s.split() for s in lines if s.startswith('FIST_FRAME ')]
mixes = [s.split() for s in lines if s.startswith('FIST_MIX ')]
assert frames and mixes, 'missing presentation or mixer observations'
print(f'frame callbacks={len(frames)} first={frames[0][1]}ms last={frames[-1][1]}ms')
print('frame modes=' + str(dict(collections.Counter((f[2], f[3], f[4]) for f in frames))))
print('abort=' + str(dict(collections.Counter(f[6] for f in frames))))
print('changed=' + str(dict(collections.Counter(f[7] for f in frames))))
print('capture states=' + str(dict(collections.Counter(f[8] for f in frames))))
print(f'mix callbacks={len(mixes)} first={mixes[0][1]}ms last={mixes[-1][1]}ms')
print('max added=' + str(max(int(m[4]) for m in mixes)))
print('mix capture states=' + str(dict(collections.Counter(m[5] for m in mixes))))
PY
