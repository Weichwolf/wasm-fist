#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export GATE_WHICH=wasm
exec bash "$ROOT/tools/consecutive.sh" "${1:-10}" "${2:-30}"
