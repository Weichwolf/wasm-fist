#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
CORE="$TREE/src/cpu/core_normal.cpp"
STAGE=6f7aa3541d31c0d0381c489c5ec6704b0d0b3cdd837c7c6205125ca95211f940
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
