#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
VGA="$TREE/src/hardware/vga_memory.cpp"
FILES="$TREE/src/dos/dos_files.cpp"
bash "$ROOT/tools/oracle/build_sequence_oracle.sh"
vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
if [ "$vga_hash" = cfcd97fcf3aa4ddd7370eef783afcc8600742ba22ecd38e304b258f197224f71 ] &&
   [ "$files_hash" = 8d96b701635021a913cb4e121df983bbbf7424116025a42decc8dde7add9889d ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/timing_trace.patch"
  vga_hash="$(sha256sum "$VGA" | cut -d' ' -f1)"
  files_hash="$(sha256sum "$FILES" | cut -d' ' -f1)"
fi
if [ "$vga_hash" != 20f989617311f0c20ab69d70ddd916d19cf164cba7417e6d99b6e9f501559a34 ] ||
   [ "$files_hash" != ec7df6f3961655ecc614f25faa33bcd0bcfab5c9e6dc2f13e9e3e1ad39521099 ]; then
  echo 'DOSBox timing trace source differs from the expected base/patched revisions' >&2
  exit 1
fi
make -C "$TREE" -j4
printf 'DOSBox timing trace: %s\n' "$TREE/src/dosbox"
