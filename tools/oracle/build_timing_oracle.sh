#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
VGA="$TREE/src/hardware/vga_memory.cpp"
FILES="$TREE/src/dos/dos_files.cpp"
DAC="$TREE/src/hardware/vga_dac.cpp"
INT10="$TREE/src/ints/int10.cpp"
TIMER="$TREE/src/hardware/timer.cpp"
PIC="$TREE/src/hardware/pic.cpp"
DRAW="$TREE/src/hardware/vga_draw.cpp"
bash "$ROOT/tools/oracle/build_sequence_oracle.sh"
vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
dac_hash="$(sha256sum "$DAC" | cut -d' ' -f1)"
int10_hash="$(sha256sum "$INT10" | cut -d' ' -f1)"
timer_hash="$(sha256sum "$TIMER" | cut -d' ' -f1)"
pic_hash="$(sha256sum "$PIC" | cut -d' ' -f1)"
draw_hash="$(sha256sum "$DRAW" | cut -d' ' -f1)"
if [ "$vga_hash" = cfcd97fcf3aa4ddd7370eef783afcc8600742ba22ecd38e304b258f197224f71 ] &&
   [ "$files_hash" = 8d96b701635021a913cb4e121df983bbbf7424116025a42decc8dde7add9889d ] &&
   [ "$dac_hash" = 3d9759ff435edccb38e984f8679c7b3c9a1022423fcf206bb91f885ff5e24837 ] &&
   [ "$int10_hash" = 7a406ff90636d8e00e405f6ed104d60c0ba28d99d2a0c5d099a7237bb5c97961 ] &&
   [ "$timer_hash" = 6adc0ea61440ff684275172385edc0cc4a1b43f1f32e75a702aaf1c86380793a ] &&
   [ "$pic_hash" = f1a6d6b330a24a72440c7f1581ac804530e534714c7f79109f47afce100ae12a ] &&
   [ "$draw_hash" = 1526c501a3d523a68677092b9d75f01eb53eb544d2c557d7150118860050df1e ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/timing_trace.patch"
  vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
  files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
  dac_hash="$(sha256sum "$DAC" | cut -d' ' -f1)"
  int10_hash="$(sha256sum "$INT10" | cut -d' ' -f1)"
  timer_hash="$(sha256sum "$TIMER" | cut -d' ' -f1)"
  pic_hash="$(sha256sum "$PIC" | cut -d' ' -f1)"
  draw_hash="$(sha256sum "$DRAW" | cut -d' ' -f1)"
fi
if [ "$vga_hash" != 0c48465ada7feab06c0e2cf7663d55a166bdcdc4c710818feff405218cedd8a9 ] ||
   [ "$files_hash" != ec7df6f3961655ecc614f25faa33bcd0bcfab5c9e6dc2f13e9e3e1ad39521099 ] ||
   [ "$dac_hash" != 387de9c171009fbf3388abbacabb77445c9739c6e9cac221be92f713caa48577 ] ||
   [ "$int10_hash" != 183b3ee0c17dc399b7d1c00260c7e2032fedd4013808563892077ea8813ad2e7 ] ||
   [ "$timer_hash" != 1bfe44e4bea365b7e40724ba5b0955904f9a6151cb0a73eee9ce33b877b83a72 ] ||
   [ "$pic_hash" != 9ce9d0b0de68f0253791d45e7c5365043bceb147485fe66839aa2f957f193b99 ] ||
   [ "$draw_hash" != a3c5c3bd0cb6d337cdd84653a158c00994941f226b207b7f42f9e984fffa82f0 ]; then
  echo 'DOSBox timing trace source differs from the expected base/patched revisions' >&2
  exit 1
fi
make -C "$TREE" -j4
printf 'DOSBox timing trace: %s\n' "$TREE/src/dosbox"
