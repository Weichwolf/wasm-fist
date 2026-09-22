#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
CORE="$TREE/src/cpu/core_normal.cpp"
STAGE=a427a92012fd1a5af04cc9af77ac317b9dd251fa07e33b0d3d23cbaf0c9fa279
PROFILE=d6ad21e50b9d722b7d58cc3d54bf1f55d549b0423915a3da049f198ad9f00aff

hash="$(sha256sum "$CORE" | cut -d' ' -f1)"
if [ "$hash" != "$STAGE" ]; then
    bash "$ROOT/tools/oracle/build_kdv_profile_oracle.sh" > /dev/null
    [ "$(sha256sum "$CORE" | cut -d' ' -f1)" = "$PROFILE" ] || exit 1
    git -C "$TREE" apply "$ROOT/tools/oracle/kdv_stage.patch"
fi
[ "$(sha256sum "$CORE" | cut -d' ' -f1)" = "$STAGE" ] || {
    echo 'DOSBox KDV stage source differs' >&2
    exit 1
}
make -C "$TREE" -j4 > /dev/null
printf 'DOSBox KDV stage oracle: %s\n' "$TREE/src/dosbox"
