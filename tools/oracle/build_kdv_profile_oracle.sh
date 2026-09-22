#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
CORE="$TREE/src/cpu/core_normal.cpp"
BASE=9d40f513895b251f1e22a5bff1d47e5f0f2b08d6b8e35a4b8c504650c2154777
PROFILE=d6ad21e50b9d722b7d58cc3d54bf1f55d549b0423915a3da049f198ad9f00aff
STAGE=6f7aa3541d31c0d0381c489c5ec6704b0d0b3cdd837c7c6205125ca95211f940

bash "$ROOT/tools/oracle/build_timing_oracle.sh" > /dev/null
hash="$(sha256sum "$CORE" | cut -d' ' -f1)"
if [ "$hash" = "$STAGE" ]; then
    git -C "$TREE" apply -R "$ROOT/tools/oracle/kdv_stage.patch"
    hash="$(sha256sum "$CORE" | cut -d' ' -f1)"
fi
if [ "$hash" = "$BASE" ]; then
    patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/kdv_profile.patch"
    hash="$(sha256sum "$CORE" | cut -d' ' -f1)"
fi
[ "$hash" = "$PROFILE" ] || { echo 'DOSBox KDV profile source differs' >&2; exit 1; }
make -C "$TREE" -j4 > /dev/null
printf 'DOSBox KDV profiler: %s\n' "$TREE/src/dosbox"
