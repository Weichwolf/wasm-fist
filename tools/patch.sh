#!/usr/bin/env bash
# Assemble build/ = the pristine re_out/ sources with the ordered patches/NNN-*.diff applied.
# Reproducible: exact-match application (-F0 --fuzz=0) so any drift from the committed decompile
# fails loudly instead of silently fuzzing. With no patches yet this just stages re_out/ -> build/.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"

rm -rf "$BUILD"
mkdir -p "$BUILD"
cp "$ROOT"/re_out/*.c "$ROOT"/re_out/*.h "$BUILD"/ 2>/dev/null || true
# OPL FM shim: the C++ DBOPL core (never patched -- vendored DOSBox 0.74-3) + its subdir.
cp "$ROOT"/re_out/*.cpp "$BUILD"/ 2>/dev/null || true
[ -d "$ROOT/re_out/opl" ] && cp -r "$ROOT"/re_out/opl "$BUILD"/ 2>/dev/null || true

shopt -s nullglob
patches=("$ROOT"/patches/*.diff)
units=("$BUILD"/*.c)
for p in "${patches[@]}"; do
  echo "[patch] $(basename "$p")"
  patch -p1 -s -F0 --fuzz=0 -d "$BUILD" < "$p"
done
echo "[patch] build/ staged (${#units[@]} .c units, ${#patches[@]} patches)"
