#!/usr/bin/env bash
# Build the CLEAN menu-music OPL reference (ref/audio_menu_oracle_clean.wav).
#
# WHY (docs/audio.md iteration 19): the prior reference was built from a capture
# that started at BOOT and MIXED the intro audio (a ch3/5/6-heavy "spread"
# distribution) with the menu music -- which made iters 16/17/18 chase a
# non-existent voice "redistribution".  The port's menu-music voicing is
# actually FAITHFUL (voice=channel).  This script captures a fresh window,
# TRIMS OFF the intro (keep only the post-instrument-setup menu loop), and
# replays it through the port's DBOPL -> a pure menu-music reference whose
# note-on channel distribution is byte-for-byte the MAINMENU.MS3 song.
#
#   usage: tools/oracle/make_menu_ref.sh   (needs the patched dosbox-fist, README_opl.md)
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
PFX="$TMP/oplcap"

# 1) capture ~117 s from boot (intro ~0-26 s, then continuous menu music)
bash "$ROOT/tools/oracle/trace_opl.sh" "$PFX" 48 70

# 2) trim to the post-intro menu window: start at the ~26 s menu instrument-setup
#    burst, keep one 63 s loop, rebase the t= timestamps to 0.
python3 - "$PFX" "$TMP/menu_trim" <<'PY'
import re,sys
src,dst=sys.argv[1],sys.argv[2]
lines=open(src).read().splitlines()
def gt(l):
    m=re.search(r't=(\d+)',l); return int(m.group(1)) if m else None
t0=gt(next(l for l in lines if gt(l) is not None))
START=t0+26000; SPAN=63000; out=[]
for l in lines:
    t=gt(l)
    if t is None or not (START<=t<=START+SPAN): continue
    out.append(re.sub(r't=\d+','t=%d'%(t-START),l,count=1))
open(dst,'w').write('\n'.join(out)+'\n')
PY

# 3) build oplreplay (port DBOPL) and synthesize the WAV
g++ -O2 -c "$ROOT/re_out/fist_opl_dbopl.cpp" -o "$TMP/dbopl.o"     -I"$ROOT/re_out"
g++ -O2 -c "$ROOT/re_out/opl/dbopl.cpp"       -o "$TMP/dboplcore.o" -I"$ROOT/re_out/opl"
gcc -O2 "$ROOT/tools/oracle/oplreplay.c" "$TMP/dbopl.o" "$TMP/dboplcore.o" -o "$TMP/oplreplay" -lstdc++ -lm
"$TMP/oplreplay" "$TMP/menu_trim" "$ROOT/ref/audio_menu_oracle_clean.wav"
echo "[menu-ref] wrote ref/audio_menu_oracle_clean.wav"
