#!/usr/bin/env bash
# Capture the ORIGINAL Armored Fist mixer output to a WAV under headless DOSBox.
# Boots to the menu, toggles DOSBox wave-record (Ctrl+F6), interacts, stops, copies the WAV out.
#   usage: tools/oracle/capture_audio.sh [OUT.wav] [MENU_WAIT] [REC_SECONDS]
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-$ROOT/ref/audio_menu_oracle.wav}"
MENU_WAIT="${2:-30}"; REC="${3:-8}"
CAP="$ROOT/tools/oracle/_audcap"; rm -rf "$CAP"; mkdir -p "$CAP"
CONF="$CAP/dosbox.conf"
cat > "$CONF" <<CFG
[sdl]
output=surface
[dosbox]
memsize=16
captures=$CAP
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
mount c $ROOT/armoredfist
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
timeout $((MENU_WAIT+REC+30)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_audcap.log 2>&1 &
  DPID=\$!
  sleep $MENU_WAIT
  /tmp/xkey F6 ctrl          # start wave recording
  sleep 1
  /tmp/xclick 160 100        # click a menu item (BATTLES row) -> blip if any
  sleep 2
  /tmp/xclick 205 140        # CANCEL area
  sleep 1
  /tmp/xclick 160 74         # SELECT PLAYER row
  sleep $REC
  /tmp/xkey F6 ctrl          # stop wave recording
  sleep 1
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
W=$(ls -t "$CAP"/*.wav 2>/dev/null | head -1)
if [ -n "$W" ]; then cp "$W" "$OUT"; echo "[audio] captured -> $OUT ($(stat -c%s "$OUT") bytes)"; else
  echo "[audio] NO WAV produced (no sound, or record key not bound)"; ls -la "$CAP"; tail -20 /tmp/fist_audcap.log; fi
