#!/usr/bin/env bash
# One workspace-specific owner for disposable builds, captures and verification.
# ROOT is set by the calling tool; FIST_WORKDIR permits an explicit temporary root.
if [ -z "${FIST_WORKDIR:-}" ]; then
  FIST_WORKDIR="/tmp/wasm-fist-$(id -u)-$(printf '%s' "$ROOT" | sha256sum | cut -c1-12)"
fi
FIST_BUILDDIR="$FIST_WORKDIR/build"
export FIST_WORKDIR FIST_BUILDDIR
