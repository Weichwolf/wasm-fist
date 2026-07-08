# Armored Fist -> FIST.RUN 1:1 rebuild — make-driven pipeline; the target chain IS the documentation:
#
#   make image       FIST.RUN --[extract_image.py, static Huffman-bound parse]--> re_out/fist_image.bin
#   make decompile   fist_image.bin --[Ghidra headless, raw x86:LE:32 @ load base]--> re_out/fist_decomp.c
#   make assemble    re_out/fist_decomp.c --[assemble_fist.py, mechanical]--> re_out/fist.c (engine unit)
#   make patch       re_out/*.c|h + patches/NNN-*.diff (ordered, commented) --> build/
#   make native      build/ --[gcc -m32]--> $(NATIVE)         (primary debug target)
#   make wasm        build/ --[emcc]--> $(OUTJS)              (browser/node target)
#   make web         build/ --[emcc]--> web/fist              (interactive browser build)
#   make verify / verify-wasm   run the crash-free matrix on the target, print N/M
#
# Every engine correction is a patches/NNN-*.diff with its rationale in the file header.
# The decompile (re_out/fist_decomp.c, fist.c) is committed so `make patch` is reproducible without Ghidra.
# Doctrine: no guards, no approximations, no band-aids — see CLAUDE.md. Reference oracle = DOSBox + QEMU.
#
# NOTE (Phase 0): targets past `image` come online as their tools land; this file is the plan of record.

ROOT    := $(CURDIR)
GAMEDIR := $(ROOT)/armoredfist
NODE    := $(firstword $(wildcard $(HOME)/Git/emsdk/node/*/bin/node) node)
EMCC    := $(firstword $(wildcard $(HOME)/Git/emsdk/upstream/emscripten/emcc) emcc)
NATIVE  ?= /tmp/fist_native
OUTJS   ?= /tmp/fisttest/fistrun.js

.PHONY: all pipeline image image-drivers kernel-image decompile decompile-kernel decompile-drivers assemble symbols patch check native wasm web verify verify-wasm refcapture clean help

all: wasm             ## default: patch + WASM build

pipeline: image decompile assemble patch native verify wasm verify-wasm ## FULL from-binary chain: FIST.RUN -> image -> Ghidra -> assemble -> patch -> native+WASM -> crash test (both targets)
	@echo "pipeline OK: FIST.RUN -> image -> decompile -> assemble -> patch -> compile (native + WASM) -> run"

help:                 ## list targets
	@grep -hE '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-14s %s\n",$$1,$$2}'

image: ## extract the flat ENGINE image (FIST.DAT, 16-bit, relocs applied) -> re_out/fist_dat_image.bin
	python3 $(ROOT)/tools/extract_dat_image.py $(GAMEDIR)/FIST.DAT $(ROOT)/re_out/fist_dat_image.bin

kernel-image: ## extract the FIST.RUN runtime-kernel image (32-bit, self-decrypted) -> re_out/fist_image.bin [shim reference]
	python3 $(ROOT)/tools/extract_image.py $(GAMEDIR)/FIST.RUN $(ROOT)/re_out/fist_image.bin

image-drivers: ## extract the driver OVERLAY images (MGAVIDEO/SOUNDDVR .DVR, 16-bit MZ, base 0 + .relocs) for decompile
	python3 $(ROOT)/tools/extract_dat_image.py $(GAMEDIR)/MGAVIDEO.DVR $(ROOT)/re_out/fist_mga_image.bin --relocs
	python3 $(ROOT)/tools/extract_dat_image.py $(GAMEDIR)/SOUNDDVR.DVR $(ROOT)/re_out/fist_snd_image.bin --relocs

decompile-drivers: ## Ghidra headless on each driver overlay image -> re_out/fist_{mga,snd}_decomp.c (engine seeders gated off)
	FIST_IMAGE=$(ROOT)/re_out/fist_mga_image.bin FIST_PROJ_NAME=fist_mga FIST_ENTRY=0x0 FIST_ENGINE_SEEDERS=0 FIST_FRESH=1 \
	  FIST_DRIVER_SEED_OFFS="0x2,0x5,0x9,0x93,0xa3,0xb3,0xe8,0x166,0x18e,0x197,0x24f,0x26e,0x284,0x2c1,0x310,0x328,0x340,0x388,0x3bd,0x3e0,0x3f2,0x3fe,0x43e,0x467,0x471,0x478,0x483,0x4a3,0x4cd,0x4d8,0x4e6,0x4f1,0x63c,0x647,0x655,0x660,0x84e,0x859,0x869,0x874,0x9c6,0x9cd,0xa64,0xabc,0xb1f,0xdb4,0xe37,0xedc,0xefa,0xf1a,0xf3a,0xfbc,0x103e,0x1091,0x10d1,0x1114,0x1157,0x119f,0x11ba,0x11db,0x1290,0x1335,0x1350,0x1376,0x14b1,0x14fb,0x161c,0x1952,0x1963,0x2004,0x22bf,0x23d8,0x2441,0x2547,0x2552,0x2595,0x25d1,0x2604,0x2660,0x26a1,0x2758,0x279d,0x283a,0x290c,0x294d,0x29f4,0x2a39,0x2ae9,0x2baa,0x2c54,0x3068,0x3fca,0x400b,0x401a,0x4030,0x405e,0x40b6,0x410a" \
	  bash $(ROOT)/tools/decompile.sh /tmp/fist_mga_decomp && \
	  cp /tmp/fist_mga_decomp/fist_decomp.c $(ROOT)/re_out/fist_mga_decomp.c && \
	  cp /tmp/fist_mga_decomp/functions.txt $(ROOT)/re_out/fist_mga_functions.txt
	FIST_IMAGE=$(ROOT)/re_out/fist_snd_image.bin FIST_PROJ_NAME=fist_snd FIST_ENTRY=0x0 FIST_ENGINE_SEEDERS=0 FIST_FRESH=1 \
	  bash $(ROOT)/tools/decompile.sh /tmp/fist_snd_decomp && \
	  cp /tmp/fist_snd_decomp/fist_decomp.c $(ROOT)/re_out/fist_snd_decomp.c && \
	  cp /tmp/fist_snd_decomp/functions.txt $(ROOT)/re_out/fist_snd_functions.txt

decompile: ## Ghidra headless on the ENGINE image (x86:LE:16 Real Mode @ base 0) -> re_out/fist_decomp.c
	bash $(ROOT)/tools/decompile.sh
	@echo "decompile: $$(grep -cE '/\* ===== .* @ ' $(ROOT)/re_out/fist_decomp.c 2>/dev/null || echo 0) functions exported"

decompile-kernel: ## Ghidra headless on the FIST.RUN extender kernel (x86:LE:32 @ base 0) -> re_out/fist_kernel_decomp.c ; seeds the KDV video-player + op-service cluster (reached only via the real->PM gate at runtime)
	FIST_IMAGE=$(ROOT)/re_out/fist_image.bin FIST_PROJ_NAME=fist_kernel FIST_LANG="x86:LE:32:default" FIST_ENTRY=0xd9d FIST_ENGINE_SEEDERS=0 FIST_FRESH=1 \
	  FIST_DRIVER_SEED_OFFS="0x11a6,0x11cb,0x11dd,0x11f4,0x6e95,0x6f17,0x6f3e,0x701e,0x706b,0x708b,0x7120,0x7135,0x746b,0x3322,0x3439,0x7fa0,0x8480" \
	  bash $(ROOT)/tools/decompile.sh /tmp/fist_kernel_decomp && \
	  cp /tmp/fist_kernel_decomp/fist_decomp.c $(ROOT)/re_out/fist_kernel_decomp.c && \
	  cp /tmp/fist_kernel_decomp/functions.txt $(ROOT)/re_out/fist_kernel_functions.txt
	@echo "decompile-kernel: $$(grep -cE '/\* ===== .* @ ' $(ROOT)/re_out/fist_kernel_decomp.c 2>/dev/null || echo 0) functions exported"

assemble: ## mechanical: re_out/*_decomp.c -> re_out/fist{,_mga,_snd}.c (engine + loaded driver overlays)
	python3 $(ROOT)/tools/assemble_fist.py
	@[ -f $(ROOT)/re_out/fist_mga_decomp.c ] && FIST_MODULE=mga FIST_MODULE_SIZE=0x7a9c \
	  python3 $(ROOT)/tools/assemble_fist.py $(ROOT)/re_out/fist_mga_decomp.c $(ROOT)/re_out/fist_mga.c || true
	@[ -f $(ROOT)/re_out/fist_snd_decomp.c ] && FIST_MODULE=snd FIST_MODULE_SIZE=0x413c \
	  python3 $(ROOT)/tools/assemble_fist.py $(ROOT)/re_out/fist_snd_decomp.c $(ROOT)/re_out/fist_snd.c || true
	@[ -f $(ROOT)/re_out/fist_kernel_decomp.c ] && FIST_MODULE=ext FIST_FLAT32=1 FIST_MODULE_SIZE=0xbf90 \
	  python3 $(ROOT)/tools/assemble_fist.py $(ROOT)/re_out/fist_kernel_decomp.c $(ROOT)/re_out/fist_ext.c || true

symbols: ## regenerate re_out/fist_symbols.h from the Ghidra symbol export
	python3 $(ROOT)/tools/gen_symbols.py $(ROOT)/re_out/data_symbols.txt $(ROOT)/re_out/fist_symbols.h

patch: ## apply patches/NNN-*.diff onto re_out/ -> build/ (exact match; drift fails loudly)
	bash $(ROOT)/tools/patch.sh

check: ## dry-run the patch series against the pristine decompile (anchor check)
	@rm -rf /tmp/fist_patchcheck && mkdir -p /tmp/fist_patchcheck && cp $(ROOT)/re_out/*.c $(ROOT)/re_out/*.h /tmp/fist_patchcheck/ 2>/dev/null; \
	for p in $(ROOT)/patches/*.diff; do [ -e "$$p" ] || continue; patch -p1 -s -F0 --fuzz=0 -d /tmp/fist_patchcheck < $$p || { echo "CHECK FAILED: $$p"; exit 1; }; done && \
	echo "check: all patches apply cleanly"

native: patch ## native 32-bit build (no ASan by default = real crash semantics) -> $(NATIVE)
	ASAN=' ' bash $(ROOT)/tools/build_native.sh $(NATIVE)

wasm: patch ## WASM build -> $(OUTJS)
	EMCC=$(EMCC) bash $(ROOT)/tools/build.sh $(OUTJS)

web: patch ## interactive browser build -> web/fist
	EMCC=$(EMCC) bash $(ROOT)/tools/build_web.sh $(ROOT)/web/fist

verify: native ## crash-free check on the native build
	bash $(ROOT)/tools/verify.sh native $(NATIVE)

verify-wasm: wasm ## crash-free check on the WASM build (node)
	bash $(ROOT)/tools/verify.sh wasm "$(NODE) $(OUTJS)"

refcapture: ## Stage-2 reference capture: run the original under DOSBox/QEMU, dump framebuffer + memory
	bash $(ROOT)/tools/refcapture.sh

clean: ## remove generated build/ and outputs
	rm -rf $(ROOT)/build $(OUTJS) $(OUTJS:.js=.wasm) $(ROOT)/web/fist
