#!/usr/bin/env bash
# =============================================================================
# build_disk.sh  --  Build the QEMU boot floppy + game HDD for the runtime oracle
# =============================================================================
# Produces, under $OUT (default: scratch), two raw images used to boot the
# ORIGINAL Armored Fist under qemu-system-i386:
#
#   game.img       64 MB FAT16 partitioned HDD (C:) with a *copy* of armoredfist/
#   boot_<mm>.img  FreeDOS 1.3 boot floppy (A:) that loads a memory manager +
#                  (optionally) a DPMI host + CTMOUSE, then launches the game.
#
# The original game files are NEVER modified: mcopy reads them into a fresh
# image. Large inputs (FreeDOS LiveCD, driver zips) live under $OUT (gitignored).
#
# Requirements (auto-resolved without root):
#   * qemu-img, qemu-system-i386
#   * mtools (mformat/mcopy/mpartition), mkfs.fat   -- extracted from .deb if absent
#   * FreeDOS 1.3 boot floppy (FD13BOOT.img, from FD13-LiveCD.zip)  -- downloaded
#   * CTMOUSE.EXE (INT 33h mouse driver)                           -- downloaded
#   * CWSDPMI.EXE (optional DPMI host)                             -- downloaded
#
# Usage:
#   tools/oracle/build_disk.sh [mm]
#     mm = memory-manager variant for the floppy autoexec:
#          himem   HIMEMX only (raw/XMS)                      [default]
#          jemmex  JEMMEX (VCPI + EMS off)
#          dpmi    HIMEMX + CWSDPMI -p (resident DPMI host)
#     (all variants also load CTMOUSE; the game REQUIRES a mouse.)
#
# Env:
#   OUT   output/scratch dir (default: $SCRATCH or /tmp/wasm-fist-oracle)
#   GAME  path to the game dir (default: <repo>/armoredfist)
# =============================================================================
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
MM="${1:-himem}"
GAME="${GAME:-$REPO/armoredfist}"
OUT="${OUT:-${SCRATCH:-/tmp/wasm-fist-oracle}}"
mkdir -p "$OUT"
LOCAL="$OUT/local"                 # locally-extracted host tools (mtools etc.)
export MTOOLS_SKIP_CHECK=1

log(){ printf '\033[36m[oracle]\033[0m %s\n' "$*"; }

# --- resolve mtools / mkfs.fat (no root; extract from .deb if missing) -------
need_mtools(){ command -v mformat >/dev/null 2>&1 && command -v mcopy >/dev/null 2>&1; }
if need_mtools; then MBIN=""; else
  if [ ! -x "$LOCAL/usr/bin/mformat" ]; then
    log "mtools not installed; fetching .deb locally (no root)"
    mkdir -p "$OUT/debs"; ( cd "$OUT/debs" && apt-get download mtools dosfstools >/dev/null 2>&1 )
    for d in "$OUT"/debs/*.deb; do dpkg-deb -x "$d" "$LOCAL"; done
  fi
  MBIN="$LOCAL/usr/bin"; export PATH="$LOCAL/usr/bin:$LOCAL/usr/sbin:$PATH"
fi

# --- FreeDOS boot floppy ------------------------------------------------------
FDBOOT="$OUT/FD13BOOT.img"
if [ ! -f "$FDBOOT" ]; then
  log "fetching FreeDOS 1.3 LiveCD (for FD13BOOT.img) -> $OUT"
  curl -fsSL -o "$OUT/FD13-LiveCD.zip" \
    "https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.3/official/FD13-LiveCD.zip"
  ( cd "$OUT" && unzip -o FD13-LiveCD.zip FD13BOOT.img >/dev/null )
fi

# --- CTMOUSE (INT 33h) --------------------------------------------------------
CTM="$OUT/CTMOUSE.EXE"
if [ ! -f "$CTM" ]; then
  log "fetching CTMOUSE"
  curl -fsSL -o "$OUT/ctmouse.zip" \
    "http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/1.3/base/ctmouse.zip"
  ( cd "$OUT" && unzip -oj ctmouse.zip BIN/CTMOUSE.EXE >/dev/null )
fi

# --- CWSDPMI (only needed for the 'dpmi' variant) -----------------------------
CWS="$OUT/CWSDPMI.EXE"
if [ "$MM" = dpmi ] && [ ! -f "$CWS" ]; then
  log "fetching CWSDPMI"
  curl -fsSL -o "$OUT/csdpmi.zip" \
    "http://na.mirror.garr.it/mirrors/djgpp/current/v2misc/csdpmi7b.zip"
  ( cd "$OUT" && unzip -oj csdpmi.zip bin/CWSDPMI.EXE >/dev/null )
fi

# =============================================================================
# 1. Game HDD  (64 MB, FAT16, one active primary partition)
# =============================================================================
GIMG="$OUT/game.img"
log "building game HDD -> $GIMG (from $GAME)"
rm -f "$GIMG"
qemu-img create -f raw "$GIMG" 64M >/dev/null
cat > "$OUT/mtools.conf" <<EOF
drive c: file="$GIMG" partition=1 mformat_only
EOF
export MTOOLSRC="$OUT/mtools.conf"
mpartition -I c:
mpartition -c -t 130 -h 16 -s 63 c:
mpartition -a c:                          # mark bootable/active
mformat -F c:                             # FAT16
mcopy -s -Q "$GAME/"* c:/
log "game HDD: $(mdir c:/ | grep -c '20[0-9][0-9]-') root entries"

# =============================================================================
# 2. Boot floppy  (FreeDOS + memory manager + CTMOUSE + auto-launch)
# =============================================================================
BIMG="$OUT/boot_${MM}.img"
log "building boot floppy -> $BIMG (mm=$MM)"
cp "$FDBOOT" "$BIMG"

# --- FDCONFIG.SYS -------------------------------------------------------------
CFG="$OUT/FDCONFIG.SYS.tmp"
{
  echo '!LASTDRIVE=Z'
  echo '!BUFFERS=20'
  echo '!FILES=40'
  echo 'DOS=HIGH,UMB'
  echo 'DOSDATA=UMB'
  case "$MM" in
    himem|dpmi) echo 'DEVICE=\FREEDOS\BIN\HIMEMX.EXE' ;;
    jemmex)     echo 'DEVICE=\FREEDOS\BIN\JEMMEX.EXE NOEMS' ;;
    *) echo "unknown mm=$MM" >&2; exit 2 ;;
  esac
  echo 'SHELLHIGH=\FREEDOS\BIN\COMMAND.COM \FREEDOS\BIN /E:2048 /P=\FDAUTO.BAT'
} > "$CFG"

# --- FDAUTO.BAT ---------------------------------------------------------------
AUT="$OUT/FDAUTO.BAT.tmp"
{
  echo '@echo off'
  echo 'SET DOSDIR=\FREEDOS'
  echo 'SET PATH=%DOSDIR%\BIN'
  [ "$MM" = dpmi ] && echo 'A:\CWSDPMI.EXE -p'
  echo 'A:\CTMOUSE.EXE'
  echo 'C:'
  echo 'cd \'
  echo 'echo === ORACLE: launching Armored Fist ==='
  echo 'LOADGAME -K400,0,1000 -X5000 FIST.RUN'
  echo 'echo === GAME EXITED errorlevel %errorlevel% ==='
} > "$AUT"

mcopy -o -i "$BIMG" "$CFG" ::/FDCONFIG.SYS
mcopy -o -i "$BIMG" "$AUT" ::/FDAUTO.BAT
mcopy -o -i "$BIMG" "$CTM" ::/CTMOUSE.EXE
[ "$MM" = dpmi ] && mcopy -o -i "$BIMG" "$CWS" ::/CWSDPMI.EXE
mdel -i "$BIMG" ::/SETUP.BAT 2>/dev/null || true    # kill the installer autorun

log "DONE"
log "  floppy : $BIMG"
log "  hdd    : $GIMG"
log "  run    : tools/oracle/run_oracle.sh $BIMG $GIMG"
