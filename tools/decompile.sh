#!/usr/bin/env bash
# wasm-fist pipeline STAGE 1: reproduce the named decompile from an extracted image.
#   re_out/fist_dat_image.bin --[Ghidra headless, x86:LE:16 Real Mode @ base 0]--> re_out/fist_decomp.c
#
# DEFAULT TARGET = the ENGINE (FIST.DAT): a 16-bit segmented DOS image, flat seg<<4 layout, entry
# linear 0x0004 (see docs/recon.md). MarkEntry seeds analysis at the entry; auto-analysis flows out.
# Reruns reuse the saved project DB (curated names survive) unless FIST_FRESH=1 forces a clean import.
#
# Overridable via env (kernel example in comments):
#   FIST_IMAGE     re_out/fist_dat_image.bin     # kernel: re_out/fist_image.bin
#   FIST_LANG      "x86:LE:16:Real Mode"         # kernel: "x86:LE:32:default"
#   FIST_ENTRY     0x4                           # kernel: 0xd9d   (read by MarkEntry.java)
#   FIST_PROJ_NAME fist_engine                   # kernel: fist_kernel
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

GHIDRA="${GHIDRA_HOME:-$ROOT/third_party/ghidra_12.1.2_PUBLIC}"
export JAVA_HOME="${JAVA_HOME:-$ROOT/third_party/jdk-21.0.11+10}"
PROJ_DIR="${FIST_GHIDRA_PROJ_DIR:-$ROOT/third_party/fist_ghidra_proj}"
PROJ_NAME="${FIST_PROJ_NAME:-fist_engine}"
IMG="${FIST_IMAGE:-$ROOT/re_out/fist_dat_image.bin}"
LANG="${FIST_LANG:-x86:LE:16:Real Mode}"
export FIST_ENTRY="${FIST_ENTRY:-0x4}"
export FIST_ES_CTX="${FIST_ES_CTX:-1}"  # board:0010: ES=DGROUP baseline (eliminates unaff_ES); FIST_ES_CTX=0 to disable
OUT="${1:-$ROOT/re_out}"

[ -s "$IMG" ] || { echo "[decompile] missing $IMG — run 'make image' first"; exit 1; }
mkdir -p "$PROJ_DIR"

HEADLESS="$GHIDRA/support/analyzeHeadless"
# preScripts run BEFORE auto-analysis: PrepAnalysis enables aggressive analyzers + sets DS/SS segment
#   context (kills unaff_DS garbage, resolves DGROUP DAT_ refs); MarkEntry seeds the entry point.
# postScripts run AFTER: RecoverAll iterates jump-table/prologue/call discovery to a fixpoint (raises
#   coverage); ApplyConv installs the custom "__allregs" prototype model (SpecExtension, all GP regs
#   in/out) and threads each function's used-register subset into explicit C params/returns (kills the
#   ~43k in_*/unaff_* register pseudo-vars from the hand-written asm); ExportDecomp writes the pristine C.
#   See tools/ghidra/*.java headers.  Tunable: FIST_CONV_VARIANT (A default / B), FIST_CONV_ITERS (5).
PRE=(-preScript PrepAnalysis.java -preScript MarkEntry.java)
# InstallIntFixup runs BETWEEN RecoverAll and ApplyConv: it installs a <callotherfixup targetop=swi>
# spec-extension that rewrites every `int n` into register-threaded C against a g_mem reg-file (so the
# DOS/BIOS register operands + return values become visible), and ApplyConv then detects those INT
# register reads as function params. See tools/ghidra/InstallIntFixup.java.
# SegmentFixup runs AFTER ApplyConv, BEFORE ExportDecomp: it statically tracks the real-mode DS segment
# per function and overrides the DS context register wherever DS is provably CS (push cs;pop ds /
# mov ax,cs;mov ds,ax prologues), so near data accesses into the code segment fold to the correct
# DAT_<cs>_off / g_mem linear address instead of losing their segment base -> SIGSEGV. See its header.
# SegWrapFixup runs AFTER RecoverAll, BEFORE ApplyConv: it repairs real-mode near-call/near-jump
# SEGMENT-WRAP mis-resolution in the engine's non-zero-CS clusters (segment 0xf69 CRT/service code,
# linear window 0xf690..0x1f68f). Ghidra's flat page-segment load wraps a near rel target at the wrong
# 64 KB boundary; SegWrapFixup BFS-discovers each CS cluster from its far-call entries and re-points
# every wrap-victim near flow reference to the correct in-window linear target (which is already
# decoded code). Fixing calls BEFORE ApplyConv lets the corrected call graph thread registers. See hdr.
# SeedServiceVecs runs FIRST (before RecoverAll): it creates functions at the 12 DGROUP
# service-dispatch table targets (seg 0xf69 service routines reached only via `call far [DGROUP:0xNN]`
# at runtime, so static discovery never seeds them). Seeding them before RecoverAll lets its fixpoint
# analysis + ApplyConv register threading cover the new functions, so they land in the FUN_ map and the
# indirect-call dispatcher resolves the service dispatch instead of trapping. See its header.
# The two engine-specific seeders (SeedServiceVecs = the DGROUP:0xNN service table at seg 0xf69;
# SeedRuntimeVecs = the INT-8 ISR entry at linear 0x130f8) are hard-wired to the ENGINE's runtime
# layout and are meaningless for the driver overlays (MGAVIDEO/SOUNDDVR are self-contained 16-bit MZ
# modules with their own segments, no DGROUP service table, no installed IRQ ISR at those linears).
# Gate them behind FIST_ENGINE_SEEDERS (default 1 = engine); driver decompiles set it to 0.
if [ "${FIST_ENGINE_SEEDERS:-1}" = "1" ]; then
  SEED=(-postScript SeedServiceVecs.java -postScript SeedRuntimeVecs.java)
else
  SEED=()
  # Driver overlays: seed the runtime entry-vector (0x2/0x5 thunks + their jmp targets) and the
  # init-registered method offsets (FIST_DRIVER_SEED_OFFS, comma-hex, per-driver from the asm) so the
  # overlay's INIT + methods become C functions the engine's overlay dispatcher can resolve. See
  # tools/ghidra/SeedDriverVecs.java. Runs FIRST (before RecoverAll) like the engine seeders.
  if [ -n "${FIST_DRIVER_SEED_OFFS:-}" ]; then
    SEED=(-postScript SeedDriverVecs.java)
    export JAVA_TOOL_OPTIONS="${JAVA_TOOL_OPTIONS:-} -Dfist.seed.offs=${FIST_DRIVER_SEED_OFFS}"
  fi
fi
# SetCSContext runs AFTER SegWrapFixup (which discovers the CS clusters) and BEFORE ApplyConv: it sets the
# CS segment-register context per code segment (page-seg baseline + far-call cluster certain-window
# override, e.g. 0xf690..0xffff => CS=0xf69) and, with FIST_ES_CTX=1, an ES=DGROUP baseline.  This resolves
# `mov [mem],cs` / ES refs to real segment CONSTANTS so ApplyConv does NOT thread them as unaff_CS/unaff_ES
# params -- eliminating the ~184-patch segment-dataflow-loss class at the decompile root (board:0010).
POST=("${SEED[@]}" -postScript RecoverAll.java -postScript SegWrapFixup.java -postScript SetCSContext.java -postScript InstallIntFixup.java -postScript ApplyConv.java -postScript SegmentFixup.java -postScript ExportDecomp.java)
# ExportDecomp writes <GHIDRA_FIST_OUT>/fist_decomp.c + functions.txt; point it at $OUT so a driver
# decompile lands beside the engine's without clobbering re_out/fist_decomp.c.
export JAVA_TOOL_OPTIONS="${JAVA_TOOL_OPTIONS:-} -DGHIDRA_FIST_OUT=$OUT"
LOGP=(-scriptPath "$ROOT/tools/ghidra" -scriptlog /tmp/fist_decompile.log)

echo "[decompile] img=$IMG lang='$LANG' entry=$FIST_ENTRY proj=$PROJ_NAME"
if [ -n "$FIST_FRESH" ] || [ ! -d "$PROJ_DIR/$PROJ_NAME.rep" ]; then
  echo "[decompile] fresh import (base 0) + aggressive discovery"
  "$HEADLESS" "$PROJ_DIR" "$PROJ_NAME" \
    -import "$IMG" \
    -processor "$LANG" \
    -loader BinaryLoader -loader-baseAddr 0x0 \
    -overwrite \
    "${LOGP[@]}" "${PRE[@]}" "${POST[@]}" \
    2>&1 | grep -iE 'MARK_ENTRY|PREP_|SEEDVEC|RECOVER|TABLE_INVENTORY|APPLYCONV|SEGFIX|SEGWRAP|EXPORT_DONE|ERROR REPORT|Exception|Analysis (succeeded|failed)' || true
else
  echo "[decompile] reuse project (FIST_FRESH=1 to re-import)"
  "$HEADLESS" "$PROJ_DIR" "$PROJ_NAME" \
    -process "$(basename "$IMG")" -noanalysis \
    "${LOGP[@]}" -postScript ExportDecomp.java \
    2>&1 | grep -iE 'EXPORT_DONE|ERROR REPORT|Exception' || true
fi

[ -s "$OUT/fist_decomp.c" ] && echo "[decompile] OK: $(grep -cE '/\* ===== .* @ ' "$OUT/fist_decomp.c") functions -> $OUT/fist_decomp.c" \
  || { echo "[decompile] FAILED (no output)"; exit 1; }
