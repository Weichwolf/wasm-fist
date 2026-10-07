Type: Work item
Title: Own complete canonical physical roster promotion
Depends: 0098

## Contract

Consume complete b053/f69:b329 from verified 0098 in readable shared C11. Use the canonical
physical world and sole `combat.roster` owner. Retain null/wreck/ground/retiring/alias/orphan
branches, ordered predecessor and actor member changes and goal-valid bit clearing. Preserve
all unrelated actors, references, registry, orders, RNG, selected-player state and census.
Do not install a partial parent bank. Full remaining 0081 callbacks and living class consumption
remain required.

## Evidence

The original member-zero branch uses no platoon, roster or predecessor input. Otherwise the
preceding physical roster entry selects promotion. A null predecessor permits it regardless
of actor motion flags. A wreck permits it and increments its independent +24 member byte.
For another ground/retiring predecessor, actor +19 bits 0x10/0x06 prohibit promotion; otherwise
those predecessor bits permit it. Promotion swaps adjacent roster words, decrements actor
+1c, increments a used predecessor member with byte wrapping and clears control bit 2.
Actual registry-overwritten physical actors remain valid. ADD BL wrap and missing original
extent guards permit neighboring writes for malformed actor member values; safely reject
used values outside the authored eight-platoon/four-member domain before publication.

## Next

Continue active 0081 with remaining maneuver/fire/diagnostic callbacks and complete
heading/counter/RNG parent and living class consumption. Recover complete original boundaries
before installing the shared callback banks. First playable battle, device/PCM, outcomes and
complete-game acceptance remain open; this delivery installs no partial parent bank.

## Verified delivery

Readable `roster_promotion.c` uses the canonical physical world and sole combat roster. It
validates all used indices/identities/payload tags before publication. Null/wreck predecessors
retain their actual flag bypass; the member-zero branch ignores platoon/roster/order/RNG
inputs. Ground/retiring member and wreck member retain separate typed owners and byte wrap.
Registry-overwritten physical actors remain live; no new registry-derived identity is invented.

Both production targets pass all five required groups without skips in 203.029 seconds:
295405 complete cases, 11096 atomic rejections and 3840 actual prepared-world callbacks per
target. All 47 missions/eight height maps/four details are consumed through the real C loader,
preparation and sequential roster mutations. Complete source records are poisoned before
calls; full world preservation checks allow only the proved actor/predecessor fields and two
roster cells. The unchanged five-group original gate still produces 110451 complete returns,
including 11007 genuine parent returns and twenty actual allocated registry-overwritten actors.
Original output SHA256 remains the accepted 0098 checkpoint. C output SHA256:
`204bccefe06e0bc8cfb061b7c7661227ee34c29e3a718bd71a559e8eec745c63`.

Tests cover every control word over rotating classes, complete actor/predecessor flag-byte
domains, repeated promotion to leader, null/wreck/self/retiring branches and member wrap.
Used malformed indices and unavailable predecessors fail atomically. Actual C allocation,
duplicate registry import, in-place retype, release, reset, mistagged used payload and ignored
unused payload/order/RNG branches are checked for every ground class before each probe batch.
LLVM 19.1 formatting/tidy passes all 89 owned units/headers. Initial style diagnostics were
corrected by function decomposition and explicit arithmetic/I/O/loop handling; checks are
unchanged. An earlier default-only native pilot is supporting evidence, not required acceptance.
The complete sequential production build passes all 43 native CTests in 197.61 seconds,
all 41 WASM unittest suites and both Node renderer/vehicle-scene probes in 1753.690
seconds overall. Default optional original skips are covered by the required replay above.
ASan/UBSan passes the same five required groups without skips in 196.499 seconds:
295405 cases, 11096 atomic rejections, 3840 canonical callbacks and the same output SHA256.
The actual sanitized SDL scene also passes complete frame/input/pause/focus/shutdown checks
without memory diagnostics. Current SDL and Chromium TRAIN1 controls, selection/reload,
focus, shutdown and complete frames pass; both retained after PNGs were visually reviewed.
The final source/program/original/reference audit passes. All 419 original file hashes,
read-only permissions, immutable references and the softgl pin remain unchanged.

Compact results, exact commands/environments, source/program/original/reference hashes and
excluded-attempt summaries are retained in `/tmp/wasm-fist-0099-review/receipt.json` with the
two reviewed after PNGs. Obsolete owned logs/helpers, sanitizer builds and isolated scenes/
assets/captures are removed; production builds and reference tooling remain usable.

## Reproduction

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/check_style.py
    CTEST_PARALLEL_LEVEL=4 PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache bash tools/build.sh all
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_roster_promotion.py --target all --oracle --originals --review-dir /tmp/wasm-fist-0099-review

For the memory replay, configure a separate `/tmp` CMake build with clang, RelWithDebInfo,
`-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer` and sanitizer
linker flags. Build `fist_target_discovery_probe` and `fist_driving_preview` sequentially;
run the same required gate with `--target native --native-probe` pointing to that probe,
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`. Run
`tests/verify_driving_native.py --mission` against isolated TRAIN1 assets and the sanitized
preview. The receipt records every complete production/native/browser/memory command.

## Accept

Both production targets and sanitizer pass required full-domain/original/canonical/reaching
tests without skips. Complete state/output effects and lengths agree in the proved original
safe domain; malformed used member/index/payload inputs fail atomically. Existing controls,
visuals, compiler flags, strict LLVM 19.1 style/tidy and full builds pass. Commit/push the
bounded delivery, retain compact evidence and clean obsolete owned artifacts. Full parent,
playable battle/PCM/outcomes and independent complete-game WASM acceptance remain open.
