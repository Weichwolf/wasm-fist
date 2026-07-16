#!/usr/bin/env bash
# GENUINE 1:1 UNSCALED 320x200 DOSBox reference of the INTRO FMV's settled "ARMORED FIST" title card.
#
# The intro (TITLE.KDV) animates, then HOLDS ~2.5 s on the fully-formed "ARMORED FIST" title (the KDV
# stream literally repeats ~25 identical frames) before fading to black and showing the main menu.  Unlike
# the menu -- a PERMANENT terminal state a fixed-WAIT grab can reliably catch (refcapture_native.sh) -- the
# title is TRANSIENT, and DOSBox's cycles=max wall-clock timing drifts a few seconds run-to-run, so a single
# fixed-WAIT grab is unreliable (it can land on the animation-in, the black fade, or the menu).
#
# This script instead captures a DENSE BURST spanning the hold, renders DOSBox 1:1 UNSCALED (surface,
# scaler=none, aspect=false => the SDL surface == mode 13h 320x200 exactly), locates the game rect on the
# xvfb root with -trim and asserts it is EXACTLY 320x200 (proof the grab pixels ARE the native VGA
# framebuffer, no scaling; same method + 6->8-bit DAC bit-replication as refcapture_native.sh), and then
# SELECTS the reference as the LONGEST run of consecutive PIXEL-identical 320x200 crops that is NOT the
# terminal menu state (the last captured frame).  That run is the intro's stable title hold.  The selection
# uses ImageMagick's metadata-independent pixel signature (convert %#) and is PURELY DOSBox-INTERNAL -- it
# never compares against the port render, so the reference is non-circular by construction, and robust to
# the wall-clock drift.
#
# Usage: tools/refcapture_intro.sh [START_S] [COUNT] [INTERVAL_S] [OUT.png]
#   default: START=22, COUNT=70, INTERVAL=0.3, OUT=ref/intro_title_native320.png
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
START="${1:-22}"; COUNT="${2:-70}"; IVAL="${3:-0.3}"; OUT="${4:-$ROOT/ref/intro_title_native320.png}"
CONF="$ROOT/scratch/refcap/dosbox_intro.conf"; mkdir -p "$(dirname "$CONF")"
BURST="$(mktemp -d /tmp/fist_introburst.XXXX)"
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
# total wall budget: START + COUNT*(IVAL+import overhead ~0.15s) + slack
BUDGET=$(awk -v s="$START" -v c="$COUNT" -v i="$IVAL" 'BEGIN{printf "%d", s + c*(i+0.2) + 40}')
timeout "$BUDGET" xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_intro.log 2>&1 &
  DPID=\$!
  sleep $START
  for i in \$(seq -w 1 $COUNT); do
    import -window root '$BURST'/g\$i.png 2>/dev/null
    sleep $IVAL
  done
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
# Crop every 1:1 320x200 grab and pixel-signature it (metadata-independent).  Build an ordered sig list.
SIGS="$BURST/sigs.txt"; : > "$SIGS"
for f in "$BURST"/g*.png; do
  g="$(convert "$f" -fuzz 1% -format '%@' info: 2>/dev/null || true)"
  case "$g" in
    320x200+*+*) convert "$f" -crop "$g" +repage "${f%.png}.c.png" 2>/dev/null
                 sig="$(convert "${f%.png}.c.png" -format '%#' info: 2>/dev/null)"
                 echo "$sig ${f%.png}.c.png" >> "$SIGS" ;;
    *) : ;;  # non-320 (black fade / text mode) -- skip, breaks any run
  esac
done
N=$(wc -l < "$SIGS")
[ "$N" -ge 6 ] || { echo "[intro] FAIL: only $N usable 320x200 grabs"; exit 1; }
MENUSIG="$(tail -1 "$SIGS" | awk '{print $1}')"   # terminal state = the menu (permanent)
# Longest run of consecutive equal sigs whose sig != menu sig.  Emit the middle frame of that run.
read BEST_LEN BEST_FILE <<<"$(awk -v menu="$MENUSIG" '
  { sig[NR]=$1; file[NR]=$2 }
  END{
    best=0; bs=1; be=0; i=1;
    while(i<=NR){
      j=i; while(j<NR && sig[j+1]==sig[i]) j++;
      if(sig[i]!=menu && (j-i+1)>best){ best=j-i+1; bs=i; be=j; }
      i=j+1;
    }
    mid=int((bs+be)/2);
    print best, file[mid];
  }' "$SIGS")"
echo "[intro] longest non-terminal hold = $BEST_LEN consecutive identical 320x200 grabs ; menu sig ${MENUSIG:0:12}"
[ "${BEST_LEN:-0}" -ge 3 ] || { echo "[intro] FAIL: no stable pre-menu hold >=3 grabs (drift/timing) -- widen the burst"; exit 1; }
cp "$BEST_FILE" "$OUT"
rm -rf "$BURST"
[ -f "$OUT" ] && echo "[intro] captured GENUINE native 320x200 intro title -> $OUT (hold len $BEST_LEN)" || { echo FAILED; exit 1; }
