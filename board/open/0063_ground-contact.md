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

Prove original complete 7fa0/8480 and op 1c returns using the real setup instructions and owned
resampled fields. Add readable shared sampling and typed ground-contact members; exercise all
headings, periodic coordinate boundaries, steep signed-byte height differences, all detail
levels, all four classes, ownership and atomic failures on both targets. Keep the visual preview
and subsequent input loop as separately verified integration steps. Run required strict/build/
behavior/memory checks, commit/push, then connect installed motion to the interactive scene.

## Accept

Complete ground height, hull slope and independent turret slope fields agree with actual original
returns for the bounded contact pass on native/WASM. Coordinate/detail/heading/width boundaries,
all original height maps and complete preserved vehicle state are covered. Altitude transfer and
mission phase ordering are explicitly scoped; there is no collision, full suspension/tick,
controller, interactive scene or playable mission claim from sampler/contact-only tests.
