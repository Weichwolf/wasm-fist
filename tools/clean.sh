#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/tools/work_dir.sh"
rm -rf -- "$FIST_BUILDDIR" "$FIST_WORKDIR/verify" "$FIST_WORKDIR/sequence-capture"
