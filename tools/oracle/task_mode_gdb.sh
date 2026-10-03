#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
exec gdb -q -batch -x "$ROOT/tools/oracle/task_mode.gdb" --args \
  "$ROOT/third_party/dosbox-build/dosbox-0.74-3/src/dosbox" "$@"
