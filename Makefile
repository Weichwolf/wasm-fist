export PYTHONPYCACHEPREFIX := /tmp/wasm-fist-python-cache

.PHONY: all check native wasm style provision reference-images help

all: check

check: ## Build and test native/WASM, then enforce strict C style and analysis
	bash tools/build.sh all
	python3 tools/check_style.py --build-dir "$${FIST_REWRITE_BUILD_ROOT:-/tmp/wasm-fist-rewrite}/native"

native: ## Build and test the native target under /tmp
	bash tools/build.sh native

wasm: ## Build and test the WASM target under /tmp
	bash tools/build.sh wasm

style: ## Check owned C with LLVM 19.1.x clang-format and clang-tidy
	python3 tools/check_style.py --build-dir "$${FIST_REWRITE_BUILD_ROOT:-/tmp/wasm-fist-rewrite}/native"

provision: ## Provision the ignored original game without replacing existing files
	python3 tools/provision_game.py

reference-images: ## Provision checksum-pinned original instruction images under /tmp
	python3 tests/reference_images.py

help: ## List development targets
	@awk 'BEGIN { FS = ":.*## " } /^[a-z-]+:.*## / { printf "  %-18s %s\n", $$1, $$2 }' $(MAKEFILE_LIST)
