#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference capture (Stage-2 bit-verify oracle).
#
# The plain tools/refcapture.sh grabs a SCALED X window (640x400 / 1024x768); downscaling that to
# 320x200 injects resampling artifacts and is NOT a valid bit reference (it yields a bogus AE ~55k
# against a pixel-exact render). This script instead renders DOSBox 1:1 UNSCALED and grabs the native
# VGA output verbatim:
#   [render] aspect=false scaler=none   -> no aspect stretch, no scaler => the SDL surface == mode 13h
#                                          320x200 exactly
#   [sdl]    output=surface  fullscreen=false
# The game window lands at a fixed offset on the (windowless) xvfb root; we auto-detect its bounding
# box with `-trim` (pure-black surround) and assert it is EXACTLY 320x200 => proof the grab pixels ARE
# the native framebuffer, with no scaling. 6->8-bit DAC expansion is DOSBox's VGA bit-replication
# (v<<2)|(v>>4), the same expansion fist_vga.c uses on the port side, so the two are directly comparable.
#
# Usage: tools/refcapture_native.sh [WAIT_SECONDS] [OUT.png]
#   default: WAIT=40 (menu is settled by then), OUT=ref/main_menu_native320.png
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WAIT="${1:-40}"; OUT="${2:-$ROOT/ref/main_menu_native320.png}"
CONF="$ROOT/scratch/refcap/dosbox_native.conf"
mkdir -p "$(dirname "$CONF")"
cat > "$CONF" <<EOF
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
[autoexec]
mount c $ROOT/armoredfist
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
EOF
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
ROOTPNG="$(mktemp /tmp/fist_refroot.XXXX.png)"
timeout $((WAIT+30)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_native.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
# The game rect is the only non-black region on the root; -trim finds it. Assert exactly 320x200.
GEOM="$(convert "$ROOTPNG" -fuzz 1% -format '%@' info:)"
echo "[refcapture-native] game rect on root: $GEOM"
case "$GEOM" in
  320x200+*+*) : ;;
  *) echo "[refcapture-native] FAIL: game rect is not a 1:1 320x200 grab ($GEOM) -> scaling present"; exit 1 ;;
esac
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[refcapture-native] captured GENUINE native 320x200 -> $OUT" || { echo "FAILED"; exit 1; }
