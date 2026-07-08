#!/usr/bin/env bash
# Stage-2 REFERENCE ORACLE: boot the ORIGINAL Armored Fist under DOSBox headless and capture a frame.
# Validated: the game boots through the Doug-Huffman extender into VGA graphics mode under DOSBox's DPMI.
# This is the bit-verify reference source (analog to DD2's Wine refcapture). QEMU (qemu-system-i386 +
# gdb / -icount record-replay) is the instruction-level microscope for cycle-exact dumps (see CLAUDE.md).
#
# Usage: tools/refcapture.sh [WAIT_SECONDS] [OUT.png]   (needs xvfb; SDL1 DOSBox 0.74)
# TODO(refine): drive to a STABLE, known frame (menu) — wait for a specific screen / script keystrokes;
#   dump the 320x200x8 VGA framebuffer + palette (not the scaled X window) for exact fb compares. The
#   current version proves boot + grabs the X root; frame timing is approximate.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WAIT="${1:-24}"; OUT="${2:-$ROOT/ref/boot_frame.png}"
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
timeout $((WAIT+20)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$ROOT/tools/dosbox.conf' -exit >/tmp/fist_refcap.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  command -v import >/dev/null 2>&1 && import -window root '$OUT' 2>/dev/null
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
[ -f "$OUT" ] && echo "[refcapture] captured -> $OUT" || { echo "[refcapture] FAILED"; exit 1; }
