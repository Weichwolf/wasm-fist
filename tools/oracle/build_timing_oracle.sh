#!/usr/bin/env bash
# All probes share the complete source/build contract in build_oracle.sh.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
exec bash "$ROOT/tools/oracle/build_oracle.sh"
