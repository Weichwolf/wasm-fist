#!/usr/bin/env bash
# =============================================================================
# capture_mission_spawn.sh -- GENUINE frame-matched AZER1 mission SPAWN capture.
#
# Captures the ORIGINAL Armored Fist's windshield framebuffer at the EARLIEST
# mission frame (spawn / frame-1) -- the sim state the port's deterministic
# op-0x24 frame is at -- NOT the fully-settled frame. Drives BATTLES->OK->ACCEPT
# under the instrumented DOSBox (tools/oracle/dosbox_vga_terrain_trace.patch),
# then polls the SDL surface every 0.3s; the LOADING photo grabs are large PNGs
# (~30-40 KB), the first windshield frame drops to ~14-22 KB -> that transition
# marks frame-1. At that instant it saves the 1:1 320x200 game rect and fires
# SIGUSR2 to dump guest RAM (for the live camera XY/alt read).
#
# 6->8-bit DAC expansion is DOSBox's VGA bit-replication (v<<2)|(v>>4), the same
# expander fist_vga.c / FIST_MISSFB use, so the grab is directly byte-comparable
# to the port's frame dump. output=surface + scaler=none + aspect=false => the
# SDL surface IS the mode-13h 320x200 framebuffer (asserted via -trim rect).
#
# Non-circular: the reference comes from DOSBox running the ORIGINAL FIST.RUN
# (never a copy of the port output); its md5 differs from the port frame.
#
# Findings: docs/oracle_mission_spawn.md.  Env: DOSBOX (default /tmp/debs/dosbox-fist).
# Usage: capture_mission_spawn.sh [OUT_PNG]   (default ref/mission_azer1_spawn_native320.png)
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-$ROOT/ref/mission_azer1_spawn_native320.png}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
PREFIX="${FISTLOG:-$ROOT/scratch/oracle/mspawn}"
[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX (see tools/oracle/README_terrain.md)"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")" "$(dirname "$OUT")"
SCRATCH="$(mktemp -d /tmp/fist_mspawn.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/db.conf"
cat > "$CONF" <<CFG
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
core=normal
cputype=auto
cycles=max
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG="$SCRATCH/db.log"
rm -f "$PREFIX".ram.bin "$PREFIX".pal.bin "$PREFIX".vram.bin
timeout 160 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_TILEPHYS=0x141000
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep 60
  import -window root '$SCRATCH/root.png' 2>/dev/null
  GEOM=\$(convert '$SCRATCH/root.png' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[mspawn] menu rect \$GEOM\"
  echo '[mspawn] BATTLES'; /tmp/xclick \$((OX+160)) \$((OY+100)); sleep 12
  echo '[mspawn] OK';      /tmp/xclick \$((OX+205)) \$((OY+128)); sleep 12
  echo '[mspawn] ACCEPT';  /tmp/xclick \$((OX+40))  \$((OY+186))
  DONE=0
  for i in \$(seq 1 40); do
     import -window root '$SCRATCH/g.png' 2>/dev/null
     G=\$(convert '$SCRATCH/g.png' -fuzz 1% -format '%@' info:)
     convert '$SCRATCH/g.png' -crop \$G +repage '$SCRATCH/frame.png' 2>/dev/null || true
     SZ=\$(stat -c%s '$SCRATCH/frame.png' 2>/dev/null || echo 0)
     echo \"[mspawn] poll \$i sz=\$SZ rect=\$G\"
     # first windshield frame (~14-22 KB) after the LOADING photo (~30-40 KB)
     if [ \$DONE -eq 0 ] && [ \$SZ -gt 10000 ] && [ \$SZ -lt 26000 ]; then
        case \"\$G\" in 320x200+*+*) : ;; *) echo '[mspawn] rect not 320x200 -> scaling'; exit 1 ;; esac
        cp '$SCRATCH/frame.png' '$OUT'
        echo '[mspawn] FIRST windshield frame captured -> '$OUT'; SIGUSR2 RAM dump'
        kill -USR2 \$DPID; DONE=1; sleep 1
     fi
     sleep 0.3
  done
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo '=== reference (X11 grab, may be frame-skewed vs the RAM/VRAM/DAC) ==='; identify "$OUT" 2>/dev/null
echo '=== ram dump (for the camera read) ==='; ls -la "$PREFIX".ram.bin 2>/dev/null

# =============================================================================
# FRAME-MATCHED derivation.  The instrumented DOSBox writes .ram.bin + .vram.bin
# + .pal.bin ATOMICALLY in the ONE SIGUSR2 handler (fist_dump()), so they are all
# from the SAME guest instruction -- self-consistent by construction.  The X11
# PNG above is grabbed a few frames earlier (moving camera) and is NOT frame-
# matched.  Here we DERIVE the reference from the VRAM+DAC captured at the SIGUSR2
# tick: idx[n]=vram[((n>>2)<<4)|(n&3)] (mode-13h chain-4), then 6->8 DAC-expand.
# The result (idx buffer + PNG) is trivially self-consistent (raw->DAC == PNG).
# Outputs:  <OUT:.png->.fm.png>  <PREFIX>.idx.bin (64000 raw indices)
# and reports the oracle spawn camera TCB+0x3a/+0x3c and V-base 90d8 (tile row).
# =============================================================================
FMPNG="${OUT%.png}_framematched.png"
if [ -f "$PREFIX.vram.bin" ] && [ -f "$PREFIX.pal.bin" ]; then
echo '=== FRAME-MATCHED derivation (VRAM+DAC @ the single SIGUSR2 tick) ==='
python3 - "$PREFIX.vram.bin" "$PREFIX.pal.bin" "$PREFIX.ram.bin" "$PREFIX.idx.bin" "$FMPNG" <<'PY'
import struct,sys
vram=open(sys.argv[1],'rb').read()
pal =open(sys.argv[2],'rb').read()
ram =open(sys.argv[3],'rb').read() if len(sys.argv)>3 else b''
idxout,pngout=sys.argv[4],sys.argv[5]
# 1. de-interleave chain-4 -> linear mode-13h index buffer
idx=bytearray(64000)
for n in range(64000):
    idx[n]=vram[((n>>2)<<4)|(n&3)]
open(idxout,'wb').write(idx)
# 2. 6->8 DAC expand, write PPM->PNG
def x(v): return (v<<2)|(v>>4)
rgb=bytearray(64000*3)
for n in range(64000):
    p=idx[n]; rgb[3*n]=x(pal[3*p]&0x3f); rgb[3*n+1]=x(pal[3*p+1]&0x3f); rgb[3*n+2]=x(pal[3*p+2]&0x3f)
ppm=("P6\n320 200\n255\n").encode()+bytes(rgb)
open('/tmp/_fm.ppm','wb').write(ppm)
import subprocess; subprocess.run(['convert','/tmp/_fm.ppm',pngout],check=False)
# 3. stats of the derived (self-consistent) frame
seen=set(idx); tb=[idx[r*320+c] for r in range(8,88) for c in range(320)]
tban=[v for v in tb if v>=80]
print("[fm] derived idx.bin=%s png=%s"%(idxout,pngout))
print("[fm] whole-frame distinct=%d nonzero=%d"%(len(seen),sum(1 for v in idx if v)))
print("[fm] terrain rows8-88: mean=%.1f  band(idx>=80) count=%d mean=%.1f distinct=%d"%(
      (sum(tb)/len(tb)), len(tban), (sum(tban)/len(tban) if tban else 0), len(set(tb))))
# 4. camera + V-base from the SAME-tick RAM dump.  TCB @ phys 0x10000 (X@+0x2c);
#    extender base @ phys 0x131000 (proj globals 9104/9108/90d4/90d8 there).
if ram:
    def s16(p): return struct.unpack_from('<h',ram,p)[0]
    def u16(p): return struct.unpack_from('<H',ram,p)[0]
    def u32(p): return struct.unpack_from('<I',ram,p)[0]
    def i32(p): return struct.unpack_from('<i',ram,p)[0]
    T=0x10000; E=0x131000
    print("[fm] TCB@0x10000  X=%d Y=%d alt=%d  head=%u +0x3a(pitch)=%d +0x3c(roll)=%d +0x3e(foc)=%u"%(
        i32(T+0x2c),i32(T+0x30),i32(T+0x34),u16(T+0x38),s16(T+0x3a),s16(T+0x3c),u16(T+0x3e)))
    d8=u32(E+0x90d8)
    print("[fm] proj 90d4=%08x 90d8=%08x (V-base hi=0x%02x=tile row %d)  9104=%08x 9108=%08x 90c0=%08x"%(
        u32(E+0x90d4),d8,(d8>>24)&0xff,(d8>>24)&0xff,u32(E+0x9104),u32(E+0x9108),u32(E+0x90c0)))
    print("[fm] proj 90b8=%08x 90bc=%08x 90f8(width)=%u 90f0(colh)=%u 3918(tilebase)=%08x"%(
        u32(E+0x90b8),u32(E+0x90bc),u32(E+0x90f8),u32(E+0x90f0),u32(E+0x3918)))
PY
fi

# live camera at spawn: X~583982 Y~1142557, alt @ +8 (=terrain height h*~292):
[ -f "$PREFIX.ram.bin" ] && python3 - "$PREFIX.ram.bin" <<'PY'
import struct,sys
d=open(sys.argv[1],'rb').read(); seen=set()
for i in range(0,len(d)-12,2):
    x=struct.unpack_from('<i',d,i)[0]
    if 570000<x<620000:
        y=struct.unpack_from('<i',d,i+4)[0]
        if 1100000<y<1180000:
            a=struct.unpack_from('<i',d,i+8)[0]
            if 300<a<45000 and (x,y,a) not in seen:
                seen.add((x,y,a)); print("live-cam off=0x%06x X=%d Y=%d alt=%d (h~%.1f)"%(i,x,y,a,a/292.0))
PY
