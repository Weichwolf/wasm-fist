#!/usr/bin/env bash
# board:0024: restore the complete diagnostic/capture oracle from pinned upstream source.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="${DOSBOX_TREE:-$ROOT/third_party/dosbox-build/dosbox-0.74-3}"
OUTPUT="${DOSBOX_OUTPUT:-$ROOT/third_party/dosbox-fist}"
ARCHIVE="$(dirname "$TREE")/dosbox-0.74-3.tar.gz"
if [ ! -d "$TREE" ]; then
  mkdir -p "$(dirname "$TREE")"
  if [ ! -f "$ARCHIVE" ]; then
    curl -fL --retry 2 --connect-timeout 15 \
      https://deb.debian.org/debian/pool/main/d/dosbox/dosbox_0.74-3.orig.tar.gz \
      -o "$ARCHIVE.download"
    mv "$ARCHIVE.download" "$ARCHIVE"
  fi
  printf '%s  %s\n' c0d13dd7ed2ed363b68de615475781e891cd582e8162b5c3669137502222260a "$ARCHIVE" | sha256sum -c -
  mkdir "$TREE"
  tar xzf "$ARCHIVE" --strip-components=1 -C "$TREE"
  for spec in \
    0:dosbox_vga_terrain_trace.patch \
    0:dosbox_opl_trace.patch \
    0:dosbox_watchmax_sigusr2.patch \
    1:dosbox_0a28_counter.patch \
    1:dosbox_blktrace.patch \
    1:dosbox_exclog.patch \
    1:dosbox_time_stamps.patch \
    1:timing_trace.patch \
    1:sequence_probe.patch \
    1:sequence_capture.patch \
    1:sequence_start_state.patch \
    1:speaker_trace.patch \
    1:sequence_endpoint.patch \
    1:kdv_profile.patch \
    1:kdv_stage.patch \
    1:kdv_stage_limit.patch \
    1:cpu_trace.patch; do
    patch --batch --fuzz=0 -p"${spec%%:*}" -d "$TREE" < "$ROOT/tools/oracle/${spec#*:}"
  done
fi
# Refuse changed, partial or historical trees; never erase an existing source tree.
(cd "$TREE" && sha256sum -c "$ROOT/tools/oracle/oracle_sources.sha256")
cp "$ROOT/tools/oracle/fist_sequence_capture.h" "$ROOT/tools/oracle/fist_sequence_endpoint.h" "$TREE/src/gui/"
cp "$ROOT/tools/oracle/fist_cpu_trace.h" "$TREE/src/cpu/"
if [ ! -f "$TREE/Makefile" ]; then
  (cd "$TREE" && CXXFLAGS='-O2 -g -w' CFLAGS='-O2 -g -w' ./configure --disable-opengl)
fi
make -C "$TREE" -j4
mkdir -p "$(dirname "$OUTPUT")"
cp "$TREE/src/dosbox" "$OUTPUT.new"
mv "$OUTPUT.new" "$OUTPUT"
printf 'DOSBox oracle: %s\n' "$OUTPUT"
sha256sum "$OUTPUT"
