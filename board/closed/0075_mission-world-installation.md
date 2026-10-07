Type: Work item
Title: Typed mission object installation and retained initialization randomness
Depends: 0054, 0060, 0066, 0074

## Contract

Install the complete normal-side TRAIN1 DCBS sequence into owned typed runtime payloads using
the existing physical arena/registry owner. Restore every record before actual participation
initialization, preserve overwritten physical orphans and roster references, initialize every
participating ground actor in input order and retain the resulting shared RNG. Reuse existing
ground/other/wreck/smoke owners. Own type-21 saved tree fields and complete conditional 9c4f update.
Ground nonparticipants retain saved state without consuming initialization randomness.

Support complete payload installation for delivered classes; unsupported classes fail the whole
transaction explicitly instead of skipping records. This is a world installation stage, not
the full terrain/contact/take-control sequence, living-class dispatcher, battle rendering, audio
or playable mission. Keep every 0065/0041 and later all-mission requirement open.

## Evidence

Original d81e allocates the saved type/registry/word, d82e..d847 restores all saved bytes while
preserving the new physical pool index, and d84a calls 43c1. Type 23 sets flags 40/secondary 24
and optionally writes its alternate platoon/member roster slot. Other nonparticipants return;
flag-20 participants call complete c296/class initialization and assign the normal roster slot.
All 960 saved ground actors participate; all saved short actors do not. TRAIN1 has 85 records:
0:1, 2:2, 21:49, 23:7, 26:22, 27:4. Complete normal-side roster assignment must include orphans.
9c4f copies global byte 930c to tree byte +19 only when byte 930d equals 1; no general tree no-op.

Shared snapshot restoration/initialization, staged typed world installation and the complete
conditional tree method are implemented. The installation API publishes only a complete staged
world, supports initial RNG aliasing the previous output for reload, and preserves the previous
world on every error. Source snapshots and definitions are released before observations. The
existing immutable roster decoder supplies definition ordinals; the runtime owner resolves them
to physical slots, including overwritten registry orphans. No duplicate runtime roster is added.

Verified on 2026-10-07:

- `bash tools/rewrite/build.sh all`: all 25 native CTest gates pass in 196.84 seconds, followed
  by the complete WASM build/test script. The mission-world gate is required on both targets.
  Standard gates retain their explicitly optional corpus/device groups; required gates below
  have zero skips. The default WASM world gate passes six groups in 94.605 seconds, covering
  853 fixtures, 4,308 installed states, 269 explicit rejections and 65,569 tree requests.
- `python3 tools/rewrite/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 69 owned C
  units. The verification caller's reset checks were split after a real complexity error;
  the earlier partial build was explicitly stopped, not accepted. No rules were weakened.
- Required `test_mission_world.py --target native --originals --oracle`: all six groups pass
  in 13.697 seconds, without skips. Required WASM original verification passes the same six
  groups in 114.424 seconds. Per target: 900 fixtures, 4,979 installed object states including
  reload, 306 explicit transaction rejections and 65,569 tree requests. Complete ground flags,
  all link bytes, initialization order, seeds/cursors, duplicate bindings/roster, physical
  orphans, reset/reload, both capacity boundaries and whole-transaction errors are covered.
- All 47 complete pinned files are hash-checked: ten complete supported installations/671
  objects, including all 85 TRAIN1 records, and 37 explicit unsupported rejections. None is a
  complete living-class/mission execution claim. No records are skipped to claim success.
- The existing ground initializer's required original regression passes all seven groups
  with all 960 ground records in all 47 missions. The combined run takes 176.045 seconds and
  proves the final native initializer; the final rebuilt WASM binary independently passes
  all seven groups in 99.840 seconds. Both required runs have zero skips.
- Production-flags ASan/UBSan/LSan world probe: all six groups, the complete required corpus
  and the same 900/4,979/306/65,569 totals pass in 21.725 seconds, without skips. This includes
  source-release ownership, repeated output/RNG-alias reload, whole-world error preservation
  and proving no unrelated world bytes change during tree updates. The memory compile starts
  only after the complete production build is terminal.
- Original/reference/dependency paths remain pristine and the pinned load-module digest
  matches. No presentation/device/PCM/persistence code changes; the reviewed driving baseline
  remains player-only. Final complete-mission WASM acceptance streak remains zero.

The original oracle executes complete actual pool/roster resets and constructors, supplies only
the declared saved-byte DOS-read boundary with the fresh physical word, then runs actual d84a,
43c1 and complete c296/class returns. Complete original payloads, unrelated prior live/orphan
records, metadata, roster and RNG are compared. Tree updates execute complete 9c4f for every
pair of flag/variant bytes. Reload comparisons use two complete independent original
installations from the corresponding declared RNG start states, not a claimed full original
session reload. Unsupported/invalid/repaired-exhaustion inputs are explicit C-only error
assertions; no unsafe original overflow is executed as a successful counterpart. No original
instruction, class initializer or return is replaced by a hook or stub.

See docs/mission-world.md for exact boundaries and reproduction commands. Compact publication
and cleanup evidence belongs under /tmp/wasm-fist-0075-world-review; obsolete owned logs and the
memory executable are removed after verified commit/push.

## Next

Continue 0065/0041 by consuming this saved world through terrain/contact and take-control
installation, recovering complete living ground methods and command eligibility, and installing
dynamic shell/muzzle/explosion/retirement payloads. Reuse the shared current-entry iterator,
post-initialization RNG and one physical roster owner; preserve damage-before-impact continuation
and same-pass birth order. TRAIN1 still requires the shared 9c5d tree-change producer and live
type-26 modes 0..3, plus battle rendering, audible events and objectives/outcomes. Unsupported
living methods cannot be replaced by empty dispatch. Prioritize the complete first mission
before broadening later all-mission AI coverage; this installation alone is not playable combat.

## Accept

Native/WASM install every TRAIN1 record and complete supported mission context correctly,
including source-release ownership, initialization order, all ground classes, nonparticipants,
duplicate registry/roster writes, orphans, reset/reload and exact capacity/error boundaries.
Required original instructions and complete payload observations agree; unknown classes fail
atomically. Tree updates match original conditional byte semantics. Build/style/memory pass,
reference/originals remain pristine, and the bounded success is committed/pushed/cleaned.
