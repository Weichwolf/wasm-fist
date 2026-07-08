#!/usr/bin/env bash
# =============================================================================
# run_oracle.sh  --  Boot the ORIGINAL Armored Fist under qemu-system-i386
# =============================================================================
# Launches QEMU headless with:
#   * a FreeDOS boot floppy (A:) + game HDD (C:)  -- built by build_disk.sh
#   * the HMP monitor on 127.0.0.1:$MON_PORT      -- screendump / pmemsave / stop
#   * (optional) the gdb stub on :1234 with -S    -- instruction-level debugging
#   * (optional) -icount for deterministic / slowed execution
#
# The original game files are read-only inputs; nothing here writes to them.
#
# Usage:
#   tools/oracle/run_oracle.sh [FLOPPY.img] [GAME.img] [mode]
#     mode: run      boot and run free (default)
#           gdb      freeze at reset (-s -S); attach with dump.gdb / gdb :1234
#           slow     -icount shift=9 (stretch the engine's startup window)
#
# Env:
#   MON_PORT   HMP monitor TCP port (default 5512)
#   MEM        guest RAM in MB (default 32)
#   ICOUNT     override icount, e.g. "shift=6" (empty = off unless mode=slow)
#   EXTRA      extra qemu args
#
# Notes / findings (see README.md):
#   * The loader (FIST.RUN's decrypted 32-bit CRT) runs at LINEAR base 0x10000000.
#   * It DOS-EXECs FIST.DAT (the 16-bit engine) via INT21/4B00 at 0x10000e80.
#   * -vga std + screendump gives PNG-able PPMs of any frame.
# =============================================================================
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="${OUT:-${SCRATCH:-/tmp/wasm-fist-oracle}}"
FLOPPY="${1:-$OUT/boot_himem.img}"
GAME="${2:-$OUT/game.img}"
MODE="${3:-run}"
MON_PORT="${MON_PORT:-5512}"
MEM="${MEM:-32}"
ICOUNT="${ICOUNT:-}"
EXTRA="${EXTRA:-}"

[ -f "$FLOPPY" ] || { echo "missing floppy $FLOPPY (run build_disk.sh)"; exit 1; }
[ -f "$GAME" ]   || { echo "missing game hdd $GAME (run build_disk.sh)"; exit 1; }

args=( -m "$MEM"
  -drive "file=$FLOPPY,format=raw,if=floppy"
  -drive "file=$GAME,format=raw,if=ide"
  -boot a -vga std -display none
  -monitor "tcp:127.0.0.1:$MON_PORT,server,nowait"
  -name armoredfist-oracle )

case "$MODE" in
  gdb)  args+=( -s -S ) ;;
  slow) ICOUNT="${ICOUNT:-shift=9}" ;;
  run)  ;;
  *) echo "unknown mode $MODE"; exit 2 ;;
esac
[ -n "$ICOUNT" ] && args+=( -icount "$ICOUNT" )
[ -n "$EXTRA" ]  && args+=( $EXTRA )

echo "[oracle] qemu-system-i386 ${args[*]}"
echo "[oracle] monitor: 127.0.0.1:$MON_PORT   (screendump/pmemsave via tools/oracle/qmon.py)"
[ "$MODE" = gdb ] && echo "[oracle] gdb stub: :1234  (gdb -ex 'target remote :1234')"
exec qemu-system-i386 "${args[@]}"
