#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TREE="$ROOT/third_party/dosbox-build/dosbox-0.74-3"
RENDER="$TREE/src/gui/render.cpp"
MIXER="$TREE/src/hardware/mixer.cpp"
EXECUTE="$TREE/src/dos/dos_execute.cpp"
source_hashes() {
  render_hash="$(sha256sum "$RENDER" | cut -d' ' -f1)"
  mixer_hash="$(sha256sum "$MIXER" | cut -d' ' -f1)"
  execute_hash="$(sha256sum "$EXECUTE" | cut -d' ' -f1)"
}
source_hashes
if [ "$render_hash" = 3bce9f5aded634ec34497a0ee8875e89e0b90cdb7de6e72bbbb81482c09dc7fc ] &&
   [ "$mixer_hash" = 5de7a0105be8f7655992b22cc61a0511ae370a5407ca972d73a4488284c820a0 ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/sequence_probe.patch"
  source_hashes
fi
if [ "$render_hash" = 421fadc54312b5a5fdf999a25329a69542b0c9b1a491805e02a9f76e5cfd6bff ] &&
   [ "$mixer_hash" = d0e30cf048ac6cd8206ba580e838e6d0c8c535d990c63e4135cd2baace58a52f ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/sequence_capture.patch"
  source_hashes
fi
if [ "$execute_hash" = ba8fe41677ca655f034fb6ae435dc2641f5b3a44cc7d40acfbb7a4aabdc24807 ]; then
  patch -p1 --fuzz=0 -d "$TREE" < "$ROOT/tools/oracle/sequence_start_state.patch"
  source_hashes
fi
if { [ "$render_hash" != c61b728e565fe3c3b86d94728e45d6649348113b4145c73aacb0b20019421ad2 ] &&
     [ "$render_hash" != 3a7c2007435883faea4e291d89296e0d697a29236525337b628b70ad717a701e ]; } ||
   [ "$mixer_hash" != 38ce5a7c87f00a725429fbb66537d07abc3a31dfc52d1eb88c1789ff2d50fe4c ] ||
   [ "$execute_hash" != 89f88e209349354346c0de76c4e312cbb27152b0eb613e42578c314c0b8a9211 ]; then
  echo 'DOSBox source differs from the expected base/probe/capture revisions' >&2
  exit 1
fi
cp "$ROOT/tools/oracle/fist_sequence_capture.h" "$TREE/src/gui/fist_sequence_capture.h"
make -C "$TREE" -j4
printf 'DOSBox sequence capture: %s\n' "$TREE/src/dosbox"
