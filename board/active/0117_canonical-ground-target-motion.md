Type: Work item
Title: Consume target-aware ground motion in the canonical world
Depends: 0114, 0115, 0116

## Contract

Deliver readable shared C movement followed by the actual per-class target/turret
stage. Preserve stored M1/M3 feedback and refresh T80/BMP feedback after translation,
including self targets, both angle modes and signed coordinate wrap. Use the existing
movement, geometry, allocation identity and canonical actor owners. A released or
reused target cannot address its successor. Invalid used projections/state must
preserve the complete world and output. Unused orders, RNG and target fields remain
unused; this step does not accept full command/class/mission scheduling or PCM.

## Evidence

Closed 0114/0115 prove complete original ordered motion/turret returns. The isolated
integration source is /tmp/wasm-fist-ground-motion-integration-source at published
base cf2a7ab88bc75dfd82e6c7cafbd8aa47e45c1ca7, with pinned softgl
f93dbe9e744b48fa01d7f8eb8a44d2ff510b2d18. Builds are under
/tmp/wasm-fist-ground-motion-integration-build. All 508 source files were hashed and
rechecked. Ground-phase command prototypes and their extra dynamic-class text I/O
are excluded; exact target-motion acceptance is separate from parent acceptance.
Disposable submodule object storage also resides under /tmp.

Both fresh production compiler phases complete in 158 steps each without warnings.
The WASM phase is terminal0, session83319; it ran while the full sequential script
was already in native CTest, with one actual compiler writer. Requested production
C11/fast-math/warning flags and owned -Werror are checked in compile commands.
Strict LLVM 19.1 format/tidy passes all 95 owned translation units, session37146,
terminal0. The complete bash tools/build.sh all regression remains live under
session81969; partial native CTest results do not accept this integration.

Default native/WASM comparisons pass 10,032 complete motion cases per target,
session27440, terminal0, with digest
720a7a8c014b8da78a5325c05f35e6267e11d8072cc1401fb7b73edb1d945ed6.
Both targets also pass all 240 lifecycle/atomicity/projection/stage cases. Fully
instrumented ASan/UBSan with production fast-math passes 10,032 default, 10,752
retained/all-28-target-type, 92,160 all-47/four-detail canonical and 240 lifetime
cases: 113,184 cases total, matching production digests and no sanitizer stderr.
The fresh sanitizer source and libraries are from this exact candidate.

The retained original/native/WASM gate is terminal0 under session67145: 10,752
complete returns per target, 10,416 retained updates and all 28 target classes,
with digest 36a0b11854e27a9939aee9d2280ba634533e0b929bbf50f19d4c2ed776874f65.
The original/native/WASM canonical corpus is terminal0 under session80057: all
47 missions, eight heights, four details, 188 worlds, 3,840 actors and 92,160
returns per target pass without skips. The complete digest is
c2331281c9229fdb60e4bc9cfb5b54c1b4abd284b95fadd5ff817644d57e47f5, matching
the exact fresh sanitizer corpus. The receipt is original/corpus.json; earlier
development results are not substituted for this exact-candidate gate.

Valgrind 3.24.0 passes the actual production lifetime program: 91 allocations and
91 frees, 5,331,376 allocated bytes, zero remaining blocks/errors/suppressions,
terminal0 under session7984. This proves the 240-case lifetime program only,
not the complete graphical application or game.

Actual browser TRAIN1 input/weapon/pause/focus/shutdown/failed-start checks pass,
session17412, terminal0. Paired actual WASM presentation against the published
renderer integration passes all 47 missions/four details, 188 worlds and 376 full
frames per module, session81913, terminal0. Its state/frame digest remains
849e71ca6422b3f18ae61b4e77e8f709d0d3b5059028041be4d754bdec6e6175.
Actual native complete scene/input checks pass, session53044; native-after and
browser-after images were visually inspected. An initial intermittent native
startup failure was reproduced as SDL x11 unavailability; closed 0118 repairs the
private Xvfb test fixture without changing the game or scene assertions. Its
repaired actual native gate is terminal0 under session75368, with 17 captures.

Compact commands, source/program/capture pins and results are under
/tmp/wasm-fist-0117-review: candidate.json, original/retained.json,
sanitized/retained.json, sanitized/corpus.json, presentation-corpus.json and
native-display-lifecycle.json. Sixteen completed passed logs were retired after preserving compact results and
hashes in completed-cleanup.json; the live production and unresolved diagnostic
logs remain retained. All sources, reference images and disposable artifacts remain
under /tmp; original assets remain ignored/read-only.

## Next

Poll session81969 to terminal completion without changing candidate inputs.
All 48 native CTest contracts now pass; the WASM regression remains live.
Require its complete remaining contracts and no warnings. The exact original
corpus is complete. Separately, diagnostic session97569 is terminal1: its
first screenshot precedes complete textured-frame publication; the client exits
cleanly on externally requested SDL_QUIT. Full graphical Memcheck/input acceptance
remains open and needs explicit first-frame acknowledgement in the profiling
fixture. Normal real-client scene assertions remain unchanged and pass under0118.
Keep frozen refs/original assets and unaccepted root parent prototypes pinned.
Publish only the accepted target-motion implementation, API and registered tests;
continue complete command/class scheduling under 0081 afterward.

## Accept

The exact integration passes full production native/WASM build/regression, strict
LLVM 19.1 style, complete original/domain/retained/canonical motion comparisons,
all lifetime/atomicity cases, fresh instrumented memory checks and actual scene
controls/presentation. Retain compact reproduction evidence and retire obsolete
owned logs. Commit/push the bounded success separately from parent-command and
class/mission work. Full battle/PCM and ten complete-game WASM runs remain open.
