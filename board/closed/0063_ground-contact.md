Type: Work item
Title: Shared ground height/slope queries and vehicle contact state
Depends: 0062

## Contract

Query the owned installed numerical height field and publish ground contact for the four ground
classes in typed shared C. Recover complete height and heading-dependent slope behavior before
integrating the moving scene. Keep installation/contact separate from class-tick altitude transfer
and mission clock ownership. Preserve all other state; invalid inputs publish nothing.

## Evidence

Closed 0062 supplies the original height expansion/reduction prerequisite. Kernel op 1c at
1109..11a4 processes its 32-entry unit list, calls 7fa0 separately for hull and absolute turret
heading, and writes the high words into hull +32h/+34h and turret +22h/+24h. It calls 8480 and
writes height byte +1dh. This pass does not publish altitude: ground class ticks separately
copy +1dh into altitude byte +0dh before motion. Verify when these passes are reached.

7fa0 uses the original Q31 turn table at 9450 and fixed-offset four-corner samples; it forms
signed byte differences before shifting, not an invented geometric angle. 8480 uses installed
side/detail-dependent SHLD indices. Original map setup writes the actual detail into these
sampler instruction immediates. The pristine image's default dimension is not the mission's
installed height-field size. Recover the complete register/width contracts at actual returns.

## Next

Complete. Continue open 0064 by connecting initialized installed state and shared motion to the
first controlled moving native/browser scene. Recover altitude-byte transfer, controller cadence
and live model/camera rules at that boundary. Collision, complete class ticks, mission clock,
targeting, combat/HUD/audio and full mission completion remain within parent 0041.

## Accept

Complete ground height, hull slope and independent turret slope fields agree with actual original
returns for the bounded contact pass on native/WASM. Coordinate/detail/heading/width boundaries,
all original height maps and complete preserved vehicle state are covered. Altitude transfer and
mission phase ordering are explicitly scoped; there is no collision, full suspension/tick,
controller, interactive scene or playable mission claim from sampler/contact-only tests.


## Verification

2026-10-06: `bash tools/build.sh all` passes all thirteen native CTests and all WASM
asset/scene/pixel gates. `python3 tools/check_style.py` passes all 35 owned C units with
unchanged LLVM 19.1 rules and requested production compiler flags. A final forward/backward
local-name correction preserves the entire native probe .text (7546 bytes) and .rodata
(2624 bytes); the final production build/style gates pass again. Shared `sim/world.h` owns
map/turn constants used directly by simulation and rendering; no platform math is introduced.

`test_ground.py --originals --oracle` passes all seven groups on both production targets, no
skips (86.011 s, final shared-coordinate build). The optional oracle executes actual installed-
detail stores at 8aa7..8b0b, complete 7fa0/8480 returns and the complete 32-entry op 1c pass.
Return addresses and stacks are checked. No instruction hooks, replacement calls or externally
patched instructions are used. Original self-modifying setup performs its normal sampler
immediate updates; the frozen input image stays pristine.

Every heading value, all 65536 height-byte pairs, signed-byte wrapping, complete cardinal/grid
footprints, periodic seams, extreme 32-bit coordinates, tiny/constant fields and independent
hull/turret headings are covered. All four classes have 512 varied complete initialized/contact
states, preserving altitude 12345678h, controls, weapons, component payloads, phases and selectors.
The full suite compares 168124 direct samples and 5888 complete vehicle states per target against
independent fixed-data expectations and actual original returns. Every byte of each original
251-byte result is compared, including the fields that must remain unchanged.

The original corpus gate includes all 960 ground snapshots in 47 hash-pinned FSGs and all eight
Dxx height maps. Complete original KLC/resampling routines produce fields at 512/1024/2048/4096.
Every snapshot is tested at all four sizes (3840 contacts); each field has all 512 heading bins
(16384 direct samples). Source and decoded hashes match pins and original files remain unchanged.
Missing/truncated/trailing inputs, null/invalid fields, unsupported classes and wrong component
sizes fail without publication. Input/request snapshots are overwritten/freed before queries;
owned height storage is released before all typed observations.

ASan/UBSan with leak checking passes all seven native groups including the complete original
corpus, no skips (47.616 s). The shared-state initialization regression passes all seven groups
on both targets against complete original routines/RNG (87.341 s); the motion regression passes
all eight groups, including all rotations, slope profiles, sustained drives and original corpus
against complete original returns (55.244 s). The new ground fields are preserved during
initialization/motion and are included in their single shared complete-state serializer.

Actual original Q31 data confirms asymmetric cardinal offsets +33554431/-33554432 after SAR6.
On the reached 512-square fixture at (0,0), original height/roll/pitch are 0/14848/512. A symmetric
footprint would give roll 12288 and pitch -4096, reaching different speed-profile bins. The
512-entry retained offsets and pinned independent raw-Q31 fixture preserve the original widths,
floors and turn-bin selection; no guessed geometric angle or clamp hides the difference.

Original class entry 7c1d copies only byte +1dh into +0dh before motion. Original 4354 queues op
04/10/1c/28 then flushes via 1664; the surrounding class dispatch/mission pipeline remains to be
integrated. This contact pass updates only its five typed fields and advances no phase/clock.
The per-vehicle helper accepts supported actors; the future roster owner selects participation
rather than treating all 960 corpus actors as one normal-side roster.

Contracts, field mapping, exact math and reproduction/sanitizer commands are in
`docs/ground-contact.md`. No displayed frame changes in this simulation step: default native/
WASM scene pixel checks remain green and reviewed WI 0059 is the static visual baseline.
No installed live rendering, full suspension/tick, collision, controller, combat or audio claim
is made. Commit/push the verified step and clean its owned logs/sanitizer binary under /tmp.
