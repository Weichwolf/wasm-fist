Type: Work item
Title: Consuming untargeted M1 shell flight, pending impact and effect lifecycle
Depends: 0067, 0068, 0063

## Contract

Consume the owned launched type-8 shell through original age/integration, current-position ground
contact, collision grace and ordered unit collision. Expiry releases its actual pool binding.
Ground and unit impacts suspend the shell at an explicit pending boundary; unit damage must run
before the post-damage continuation allocates the correct original explosion and retires the
shell. Return original notice/sound/hit-voice requests to their future consumers. Own complete
original type-4 explosion and type-18 muzzle animation/removal through the shared pool.
Do not introduce a fake damage method, audible sound or visible scene claim. Preserve 0065's
complete player firing, feedback and consuming-world acceptance requirements.

## Evidence

Untouched DOS image methods b5e7..b6c8, ba33..bab3, bab4..bb1a and 9bc6..9be7; protected-mode
op 54 at 11a6..11ca. Actual e454 method table resolves types 8/4/18 to b5e7/bab4/9bc6.
M1 ground template 9c1d and unit-hit template 9c4d are pinned by the new original oracle.
Explosion ba38 calls service bb4f (physical 1b1df), normal priority, before projectile release;
this differs from muzzle allocation at low-priority service bb46 (physical 1b1d6).

The original terrain service consumes DI: DOS e1a7 supplies shell+4 and protected-mode 11a8
zero-extends DI before adding its installed DGROUP base. The stale EBX inbox does not identify
this handler's coordinates. The new staged oracle checks the actual DI and executes the complete
original 11a6 handler, preserving its DI return; no inferred inbox repair supplies the evidence.

Verified on 2026-10-06: both production builds, all 19 native CTest and complete WASM gates pass;
expanded native flight coverage separately passes after fixture additions. Strict LLVM 19.1.7
format/tidy passes all 52 owned units. The seven-group original gate passes without skips
(37,691 fixtures/41,158 updates, 210.790 seconds). Subsequent full effect-group comparison proves
all added period bytes on both targets (3,075 fixtures/5,895 updates, 10.195 seconds), including
256 new fixtures/767 updates. Distinct current coverage is 37,947 fixtures/41,925 updates per
target. Production-flags ASan/UBSan/leak detection passes all seven current groups, originals and
those distinct totals without skips in 7.181 seconds. See docs/projectile-flight.md for commands,
full state/ownership assertions, evidence boundaries and exact remaining consumers.

## Next

Continue 0065 with the reached bbb7/c31e damage/profile/parameter/component and destruction
contract, then typed live-world scheduling, actual fire eligibility/input and effect/render/audio
consumers. Keep the first playable mission and complete 0065/0041 acceptance intact.

## Accept

Both targets match the original methods within the declared service/damage boundaries, complete
pool state, relevant payload fields and random state for every required case; unchanged original
source fields and unrelated bodies remain unchanged. Cover age/coordinate/counter wrapping,
all ground height/subtraction bytes, ground-before-grace, post-integration collision, origin-first
ordering, probability consumption, saved bindings, normal-priority explosion capacity before
release, complete animation-to-removal sequences and atomic invalid-input rejection. Require all
179 M1 launch positions in all 47 complete original mission collision worlds and no original-gate
skips. Builds/style and sanitizers pass; original files stay read-only; docs state limitations.
