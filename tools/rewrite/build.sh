#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
output=${FIST_REWRITE_BUILD_ROOT:-/tmp/wasm-fist-rewrite}
case "${1:-all}" in
    native|wasm|all) target=${1:-all} ;;
    *) echo "Usage: $0 [native|wasm|all]" >&2; exit 2 ;;
esac
if [[ $target == native || $target == all ]]; then
    cmake -S "$root" -B "$output/native" -G Ninja \
        -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "$output/native"
    ctest --test-dir "$output/native" --output-on-failure --no-tests=error
fi
if [[ $target == wasm || $target == all ]]; then
    emcmake cmake -S "$root" -B "$output/wasm" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "$output/wasm"
    timeout 30s node "$root/tools/rewrite/check_wasm.cjs" "$output/wasm/fist_renderer_probe.js"
    python3 "$root/tools/rewrite/test_scenario.py" --target wasm --build-root "$output"
fi
