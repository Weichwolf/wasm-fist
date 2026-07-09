#!/usr/bin/env bash
# Armored Fist WASM build: the SAME C units as tools/build_native.sh, compiled with emcc to a node
# (headless) target. The 32-bit-flat g_mem model, the DOS/hardware shims, the driver overlays, and the
# harness (tools/native_main.c, reused via #ifdef __EMSCRIPTEN__) are all portable C -- so the wasm
# build renders the SAME framebuffer as native, which is the project's hard invariant (native<->wasm
# bit-identical). Non-portable native diagnostics (SIGSEGV/backtrace, mprotect FIST_FBTRAP) and the
# SIGALRM/setitimer host timer are #ifdef'd out; the tick runs cooperatively (one INT-8 tick per pump).
#
# Usage: tools/build.sh [out.js]    (default /tmp/fisttest/fistrun.js)
# Sources come from build/ if `make patch` produced it, else straight from re_out/.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUTJS="${1:-/tmp/fisttest/fistrun.js}"
mkdir -p "$(dirname "$OUTJS")"

EMCC="${EMCC:-$(ls "$HOME"/Git/emsdk/upstream/emscripten/emcc 2>/dev/null || echo emcc)}"

if [ -f "$ROOT/build/fist.c" ]; then SRCDIR="$ROOT/build"; else SRCDIR="$ROOT/re_out"; fi
echo "[build.sh] wasm sources from $SRCDIR  (emcc=$EMCC)"

# Same compiler looseness as the native build: the decompile is faithful C with Ghidra type slack
# (int<->ptr, implicit decls, return-type slack) that is not a correctness signal at this stage.
F="-O0 -g -std=gnu11 -w \
  -fno-strict-aliasing -Wno-int-conversion -Wno-implicit-function-declaration \
  -Wno-builtin-declaration-mismatch -Wno-return-type -Wno-incompatible-pointer-types"
INCL="-I$SRCDIR"

# Engine unit + harness (native_main is reused as the wasm main via #ifdef __EMSCRIPTEN__).
UNITS_C="$SRCDIR/fist.c $ROOT/tools/native_main.c $SRCDIR/fist_dos.c $SRCDIR/fist_vga.c \
         $SRCDIR/fist_icall.c $SRCDIR/fist_modules.c"
# Driver overlay units, when present (weak fmap/base symbols in fist_modules.c resolve against them).
for m in mga snd ext; do
  [ -f "$SRCDIR/fist_$m.c" ] && UNITS_C="$UNITS_C $SRCDIR/fist_$m.c"
done

OBJS=""; err=0
for c in $UNITS_C; do
  o="/tmp/wasm_$(basename "$c" .c).o"
  "$EMCC" -c $F $INCL "$c" -o "$o" 2>/tmp/fist_wcc.txt || true
  if grep -q 'error:' /tmp/fist_wcc.txt; then
    echo "ERROR compiling $(basename "$c"):"; grep 'error:' /tmp/fist_wcc.txt | head -8; err=1
  fi
  OBJS="$OBJS $o"
done
[ "$err" = 1 ] && { echo "build aborted (compile errors)"; exit 1; }

# Link. g_mem[0x180000] is a 1.5 MB static array; INITIAL_MEMORY + growth cover it plus the C heap.
# EXIT_RUNTIME=1 so main()'s return (or the FIST_RUNMS _exit) terminates the node process, like native.
# NODERAWFS=1 gives node's real filesystem so the harness's relative paths (re_out/*.bin, armoredfist/*)
# resolve from the repo root exactly as native. wasm_pre.js mirrors process.env into ENV for getenv().
# EMULATE_FUNCTION_POINTER_CASTS: the engine's whole indirect-call surface is untyped K&R `code`
# (typedef int code();) resolved at runtime by fist_icall to any __allregs target; on x86 an untyped
# `(*fp)(args)` calls whatever the resolved function is regardless of its declared arity (cdecl caller-
# cleanup). wasm's call_indirect instead type-checks the table slot, so a call site's derived signature
# (from its args) vs the callee's declared signature must match exactly or it traps ("null function or
# function signature mismatch"). This flag makes emcc emit adapter thunks so an untyped-cast indirect
# call reaches any target regardless of signature -- i.e. it restores the NATIVE untyped-call ABI on
# wasm, uniformly and without touching a single engine call site or callee. It changes NO behavior or
# output (the same C runs), only the ABI mismatch wasm would otherwise impose -- so it is the mechanism
# that lets the wasm build render byte-for-byte what native renders. (DD2's alternative -- per-dispatch-
# family arity normalization -- is the doctrine-pure long game; this restores the invariant at once.)
"$EMCC" $OBJS -o "$OUTJS" \
  -sSTACK_SIZE=16777216 -sINITIAL_MEMORY=67108864 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912 \
  -sEXIT_RUNTIME=1 -sERROR_ON_UNDEFINED_SYMBOLS=0 -sNODERAWFS=1 -sENVIRONMENT=node \
  -sEMULATE_FUNCTION_POINTER_CASTS=1 \
  -sBINARYEN_EXTRA_PASSES=pass-arg=max-func-params@64 \
  -sEXPORTED_RUNTIME_METHODS='["ENV","callMain"]' \
  --pre-js "$ROOT/tools/wasm_pre.js" --emit-symbol-map 2>/tmp/fist_wlink.txt \
  || { echo "LINK FAILED:"; tail -25 /tmp/fist_wlink.txt; exit 1; }
echo "[build.sh] built $OUTJS (wasm)"
