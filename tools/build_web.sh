#!/usr/bin/env bash
# Armored Fist BROWSER build: the same C units as tools/build.sh (node/headless), re-linked for the
# web -- ENVIRONMENT=web, ASYNCIFY so the engine's non-returning main loop yields to requestAnimation-
# Frame (fist_timer_pump -> emscripten_sleep in web-mode), EXIT_RUNTIME=0 so the runtime + HEAP persist
# for live framebuffer/palette reads, and the game data (armoredfist/ + re_out/*.bin) preloaded into
# MEMFS at the same relative paths the engine opens under native's NODERAWFS.  Output: web/fist.js|.wasm|.data
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/web/fist.js}"; mkdir -p "$(dirname "$OUT")"
EMCC="${EMCC:-$(ls "$HOME"/Git/emsdk/upstream/emscripten/emcc 2>/dev/null || echo emcc)}"
EMXX="$(dirname "$EMCC")/em++"
if [ -f "$ROOT/build/fist.c" ]; then SRCDIR="$ROOT/build"; else SRCDIR="$ROOT/re_out"; fi
echo "[build_web] sources from $SRCDIR"

F="-O2 -std=gnu11 -w -fno-strict-aliasing -Wno-int-conversion -Wno-implicit-function-declaration \
   -Wno-builtin-declaration-mismatch -Wno-return-type -Wno-incompatible-pointer-types"
INCL="-I$SRCDIR"
OBJDIR="${OBJDIR:-/tmp/fistweb_obj}"; mkdir -p "$OBJDIR"

UNITS_C="$SRCDIR/fist.c $ROOT/tools/native_main.c $ROOT/tools/fist_base_rays.c $SRCDIR/fist_dos.c $SRCDIR/fist_vga.c \
         $SRCDIR/fist_icall.c $SRCDIR/fist_modules.c $SRCDIR/fist_sb.c $SRCDIR/fist_opl.c"
for m in mga snd ext; do [ -f "$SRCDIR/fist_$m.c" ] && UNITS_C="$UNITS_C $SRCDIR/fist_$m.c"; done

OBJS=""; err=0
for c in $UNITS_C; do
  o="$OBJDIR/$(basename "$c" .c).o"
  "$EMCC" -c $F $INCL "$c" -o "$o" 2>"$OBJDIR/cc.txt" || true
  grep -q 'error:' "$OBJDIR/cc.txt" && { echo "ERR $(basename "$c"):"; grep 'error:' "$OBJDIR/cc.txt"|head -6; err=1; }
  OBJS="$OBJS $o"
done
CXXF="-O2 -std=gnu++11 -w -fno-rtti -fno-exceptions -fno-strict-aliasing"
for pair in "$SRCDIR/fist_opl_dbopl.cpp:-I$SRCDIR -I$SRCDIR/opl" "$SRCDIR/opl/dbopl.cpp:-I$SRCDIR/opl"; do
  src="${pair%%:*}"; inc="${pair#*:}"; o="$OBJDIR/$(basename "$src" .cpp).o"
  "$EMXX" -c $CXXF $inc "$src" -o "$o" 2>"$OBJDIR/cc.txt" || true
  grep -q 'error:' "$OBJDIR/cc.txt" && { echo "ERR $(basename "$src"):"; grep 'error:' "$OBJDIR/cc.txt"|head -6; err=1; }
  OBJS="$OBJS $o"
done
[ "$err" = 1 ] && { echo "build aborted"; exit 1; }

# stage the runtime data (game assets + overlay images) for MEMFS preload at repo-relative paths
STAGE="$OBJDIR/data"; rm -rf "$STAGE"; mkdir -p "$STAGE/re_out"
cp -a "$ROOT/armoredfist" "$STAGE/armoredfist"
cp "$ROOT"/re_out/*.bin "$STAGE/re_out/" 2>/dev/null || true
echo "[build_web] staged $(du -sh "$STAGE"|cut -f1) of game data"

"$EMCC" $OBJS -o "$OUT" \
  -sSTACK_SIZE=16777216 -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=805306368 \
  -sEXIT_RUNTIME=0 -sERROR_ON_UNDEFINED_SYMBOLS=0 -sENVIRONMENT=web,worker \
  -sEMULATE_FUNCTION_POINTER_CASTS=1 -sBINARYEN_EXTRA_PASSES=pass-arg=max-func-params@64 \
  -sFORCE_FILESYSTEM=1 -sMODULARIZE=1 -sEXPORT_NAME=FistModule \
  -sEXPORTED_RUNTIME_METHODS='["callMain","ccall","cwrap","HEAPU8","FS","ENV"]' \
  -sEXPORTED_FUNCTIONS='["_main","_fist_web_start","_fist_web_fb","_fist_web_palette","_fist_web_in_mission","_fist_web_mouse","_fist_web_key","_malloc"]' \
  --preload-file "$STAGE"@/ \
  2>"$OBJDIR/link.txt" || { echo "LINK FAILED:"; tail -30 "$OBJDIR/link.txt"; exit 1; }
echo "[build_web] built $OUT (+ .wasm + .data) -- open web/index.html via a local http server"
