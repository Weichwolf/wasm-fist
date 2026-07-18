#!/usr/bin/env bash
# Native 32-bit build of the Armored Fist engine (re_out/fist.c) + the runtime harness
# (tools/native_main.c). gcc -m32, matching the 16-bit engine's <=32-bit pointer model and the
# wasm32 target. For the `assemble` stage the bar is COMPILE + LINK; running crash-free is Stage 1.
#
# Sources come from build/ if `make patch` produced it, else straight from re_out/ (no patches yet).
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-/tmp/fist_native}"
ASAN="${ASAN:-}"

if [ -f "$ROOT/build/fist.c" ]; then SRCDIR="$ROOT/build"; else SRCDIR="$ROOT/re_out"; fi
echo "[build_native] sources from $SRCDIR"

# -m32: 16-bit engine composes 20-bit linear / far pointers into 32-bit ints -> 32-bit host.
# -w plus the permissive flags: the decompile is faithful C with Ghidra type looseness (int<->ptr,
#   implicit decls, return-type slack) that is NOT a correctness signal at this stage.
F="-m32 -no-pie -g -O0 $ASAN -std=gnu11 -w ${FIST_XCFLAGS:-} \
  -fno-strict-aliasing -Wno-int-conversion -Wno-implicit-function-declaration \
  -Wno-builtin-declaration-mismatch -Wno-return-type -Wno-return-mismatch -Wno-incompatible-pointer-types"

# -I so #include "ghidra_compat.h" resolves next to fist.c
INCL="-I$SRCDIR"

set -x
gcc $F $INCL -c "$SRCDIR/fist.c"          -o /tmp/fist_engine.o
gcc $F $INCL -c "$ROOT/tools/native_main.c" -o /tmp/fist_main.o
gcc $F $INCL -c "$SRCDIR/fist_dos.c"      -o /tmp/fist_dos.o
gcc $F $INCL -c "$SRCDIR/fist_vga.c"      -o /tmp/fist_vga.o
gcc $F $INCL -c "$SRCDIR/fist_icall.c"    -o /tmp/fist_icall.o
gcc $F $INCL -c "$SRCDIR/fist_sb.c"       -o /tmp/fist_sb.o
gcc $F $INCL -c "$SRCDIR/fist_opl.c"      -o /tmp/fist_opl.o
gcc $F $INCL -c "$SRCDIR/fist_modules.c"  -o /tmp/fist_modules.o
# OPL FM: the DOSBox 0.74-3 DBOPL core (C++) + extern "C" bridge.  -nostdinc++ -fno-rtti -fno-exceptions
# -> no 32-bit libstdc++ dependency (none is installed) -> links straight into the C program with gcc.
CXXF="-m32 -no-pie -g -O2 -std=gnu++11 -w -nostdinc++ -fno-rtti -fno-exceptions -fno-strict-aliasing"
g++ $CXXF -I"$SRCDIR" -I"$SRCDIR/opl" -c "$SRCDIR/fist_opl_dbopl.cpp" -o /tmp/fist_opl_dbopl.o
g++ $CXXF -I"$SRCDIR/opl" -c "$SRCDIR/opl/dbopl.cpp" -o /tmp/fist_dbopl.o
# Driver overlay units (re_out/fist_mga.c / fist_snd.c): compiled + linked when present, so their
# weak fmap/base symbols in fist_modules.c resolve and the overlay dispatch wires them. Absent -> the
# weak refs stay 0 and loaded overlays trap honestly (engine-only build still links).
DRVOBJ=""
for m in mga snd ext; do
  if [ -f "$SRCDIR/fist_$m.c" ]; then
    gcc $F $INCL -c "$SRCDIR/fist_$m.c" -o "/tmp/fist_$m.o" && DRVOBJ="$DRVOBJ /tmp/fist_$m.o"
  fi
done
gcc $F $INCL /tmp/fist_engine.o /tmp/fist_main.o /tmp/fist_dos.o /tmp/fist_vga.o /tmp/fist_icall.o \
    /tmp/fist_modules.o /tmp/fist_sb.o /tmp/fist_opl.o /tmp/fist_opl_dbopl.o /tmp/fist_dbopl.o $DRVOBJ -o "$OUT" -lm
set +x
echo "[build_native] built $OUT (32-bit)"
