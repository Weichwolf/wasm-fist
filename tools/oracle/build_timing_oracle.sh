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
MISC="$TREE/src/hardware/vga_misc.cpp"
bash "$ROOT/tools/oracle/build_sequence_oracle.sh"
vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
dac_hash="$(sha256sum "$DAC" | cut -d' ' -f1)"
int10_hash="$(sha256sum "$INT10" | cut -d' ' -f1)"
timer_hash="$(sha256sum "$TIMER" | cut -d' ' -f1)"
pic_hash="$(sha256sum "$PIC" | cut -d' ' -f1)"
draw_hash="$(sha256sum "$DRAW" | cut -d' ' -f1)"
misc_hash="$(sha256sum "$MISC" | cut -d' ' -f1)"
if [ "$vga_hash" = cfcd97fcf3aa4ddd7370eef783afcc8600742ba22ecd38e304b258f197224f71 ] &&
   [ "$files_hash" = 8d96b701635021a913cb4e121df983bbbf7424116025a42decc8dde7add9889d ] &&
   [ "$dac_hash" = 3d9759ff435edccb38e984f8679c7b3c9a1022423fcf206bb91f885ff5e24837 ] &&
   [ "$int10_hash" = 7a406ff90636d8e00e405f6ed104d60c0ba28d99d2a0c5d099a7237bb5c97961 ] &&
   [ "$timer_hash" = 6adc0ea61440ff684275172385edc0cc4a1b43f1f32e75a702aaf1c86380793a ] &&
   [ "$pic_hash" = f1a6d6b330a24a72440c7f1581ac804530e534714c7f79109f47afce100ae12a ] &&
   [ "$draw_hash" = 1526c501a3d523a68677092b9d75f01eb53eb544d2c557d7150118860050df1e ] &&
   [ "$misc_hash" = c37209f9429620405e7cabfc92ff762d98badd967b779d0a63460a554e496389 ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/timing_trace.patch"
  vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
  files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
  dac_hash="$(sha256sum "$DAC" | cut -d' ' -f1)"
  int10_hash="$(sha256sum "$INT10" | cut -d' ' -f1)"
  timer_hash="$(sha256sum "$TIMER" | cut -d' ' -f1)"
  pic_hash="$(sha256sum "$PIC" | cut -d' ' -f1)"
  draw_hash="$(sha256sum "$DRAW" | cut -d' ' -f1)"
  misc_hash="$(sha256sum "$MISC" | cut -d' ' -f1)"
fi
if [ "$vga_hash" != a48fbbe096df86cb57321f63e765dfac99c01bbfbd0368a2984368340ff03cc4 ] ||
   [ "$files_hash" != ec7df6f3961655ecc614f25faa33bcd0bcfab5c9e6dc2f13e9e3e1ad39521099 ] ||
   [ "$dac_hash" != 73e1f9df13792e9a5aa7254477bc63d6a9f239f43925bee13193d4ad4cbd44b3 ] ||
   [ "$int10_hash" != 183b3ee0c17dc399b7d1c00260c7e2032fedd4013808563892077ea8813ad2e7 ] ||
   [ "$timer_hash" != 9fb24f05f4583726b2d836d4890abf03c51b711fb12736e512b54b709b9157dd ] ||
   [ "$pic_hash" != 9ce9d0b0de68f0253791d45e7c5365043bceb147485fe66839aa2f957f193b99 ] ||
   [ "$draw_hash" != a3c5c3bd0cb6d337cdd84653a158c00994941f226b207b7f42f9e984fffa82f0 ] ||
   [ "$misc_hash" != fc78e34f90e4fd6b24d33dd0daa4d8d6e2c1c0e0e34103a21b114fb7365656fa ]; then
  echo 'DOSBox timing trace source differs from the expected base/patched revisions' >&2
  exit 1
fi
make -C "$TREE" -j4
printf 'DOSBox timing trace: %s\n' "$TREE/src/dosbox"
