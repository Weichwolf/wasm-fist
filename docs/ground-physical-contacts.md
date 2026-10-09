# Shared ground physical contacts

`fist_mission_world_ground_contacts()` in `src/sim/ground_contacts.c` owns the
complete selected and unselected physical-contact transition. It operates on
the canonical mission world's physical registry and existing typed payloads.
Terrain height/slope contact remains the separate owner in `sim/ground.c`.
Full living-class/world scheduling and final PCM playback remain open.

The request identifies the actor, selection wrapper, coarse prediction mode,
independent logical sound-source slot and borrowed contact sound records. Actual
selection belongs to `world.combat.selected_slot`. A mismatched wrapper returns
an unadmitted empty result. Admitted calls clear motion-blocked bit 8, then
consume a nonzero saved cooldown without scanning other payloads.

The scan visits the complete physical registry in order. It skips null/self
bindings and candidates without collision flag 64 or with exclusion flag 16.
Existing directional proximity measures candidate to actor. A nonzero distance
high word skips the candidate. The margin is the wrapped word sum of both
projection scales and 256, divided by two. Outside that margin, a gap of at most
7680 consumes the existing obstacle-prediction owner and continues the scan.

The first actual hit ends the scan. A repeated contact sets cooldown 4 and
consumes no tree damage, random draw or collision sound. A new non-tree hit
rewinds each position by its signed velocity times 32, negates the signed speed
word, clears throttle and sets cooldown 4. Negating -32768 retains its word.
No hit clears contact flag 2 while retaining all other contact flag bits.

A new tree hit uses signed arithmetic halves for speed and throttle, including
negative odd values. Existing damage arithmetic consumes the real random draw
for original record `(100, 0)` and the canonical combat source-scale factor.
Damage adds in a byte before the threshold comparison. A resulting value of at
least 20 releases the exact tree allocation, sets its deleted flag and decrements
the tree census word. A first tree hit leaves cooldown unchanged and emits a
damage-display refresh for both retained and released trees.

Saved ground contact flags are hexadecimal offset +62 (decimal 98); cooldown is
+93 (decimal 147). They are distinct from decimal +62 behavior and +93 movement
gate. Saved tree damage is +1a (decimal 26). Restore, initialization, readiness
and tree variant updates preserve these values. Vehicle restore initializes its
complete representation before decoding members, so whole-byte transaction
checks also have defined padding under the production compiler.

The canonical damage caller captures the source's side bit as a value in
`world.combat.damage_source_enemy`. Tree scaling consumes that value after source
retirement and slot reuse. This intentionally repairs the original cached near
pointer's stale dereference. The initial null source has a clear side, proved on
all 188 prepared original mission/detail worlds. The original oracle retains
the original defect; a separate C flight/impact/retirement/real neutral-smoke
reuse sequence proves the repaired behavior through all four ground classes.

Sound source ownership is independent of selection. Only a matching source on
an admitted first hit requires the borrowed audio configuration. Obstacle and
tree records select histories 45 and 48; authored packets are 13 and 270. The
logical request preserves the packet and shifts attenuation right by one. A
sample byte outside 0..15 suppresses the request while preserving selector
history. The result contains logical sound and display events; it does not
substitute a kernel, PCM renderer or complete engine/configuration producer.

Invalid used state preserves the complete world and output. The owner validates
only reached payloads; unused audio, random state, components and candidate
allocation details do not prevent early ignored/cooldown/repeated returns.
Independent tests check whole typed write footprints, late rollback after a
predictive write, saved-field retention and held multi-call contact transitions.
Original comparison scopes and reference reproduction are documented in
`ground-class-callbacks.md` and closed WI 0127. Required C acceptance is WI 0129.

## Contact and memory verification

The required contact gate passes nine groups without skips on production native,
WASM and production-fast-math ASan/UBSan programs: 705001 observations per
program, including 427968 unchanged original contacts and 15360 canonical
prepared-world contacts. Independent saved-field checks additionally cover 6144
ground and 6144 tree restore/start/readiness/variant observations. Genuine C
source capture covers eight contexts, 24 flight updates, eight impacts and
retirements, eight real neutral-smoke reuses and eight consuming tree contacts.

The 520 guard cases include 56 atomic used-input/late failures and 464 genuinely
unused malformed inputs. Another 128 sequences compare all 4096 consecutive
contact results and write footprints. The receipt's branch histogram describes
the first call of each encoded input; subsequent held outputs remain part of the
complete comparison and output digest. The accepted contact digest is
`da5aeec5e6ba77603163ed53f8069ffb1f0134abc5e24923e983e79589834ace`.

Reproduce after building both production targets, with the pinned Unicorn 2.1.4
environment and ignored, read-only originals provisioned:

```sh
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_contacts.py \
  --target all --originals \
  --build-root /tmp/wasm-fist-contact-production-build-v3 \
  --sanitized-probe /tmp/wasm-fist-contact-sanitized-v3/fist_ground_contact_probe \
  --review-dir /tmp/wasm-fist-contact-reproduction
```

The sanitizer program is a separate CMake `RelWithDebInfo` build with C flags
`-fsanitize=address,undefined -fno-omit-frame-pointer` and linker flags
`-fsanitize=address,undefined`; target production flags retain `-O2` and
`-ffast-math`. Build target `fist_ground_contact_probe` in that directory. The
versioned probe's `--retention` and `--source-lifetime` paths, plus the guard and
held groups, pass five actual Memcheck processes with no errors, suppressions,
lost blocks or remaining heap bytes. Full production regressions and actual
presentation gates are separately required before WI 0129 closes.
