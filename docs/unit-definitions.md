# Mission unit definitions and roster

`src/assets/units.c` decodes immutable DCBS snapshots into owned host definitions. The set copies
all DCBS bytes, preserves every definition in file order, and exposes registry and platoon-roster
indices into that array. No guest pointer or saved allocation slot becomes a runtime identity.
The borrowed scenario/input may be destroyed immediately after successful decoding. Destroy is
idempotent; failed decoding leaves the caller's output unchanged.

## Original evidence

The instruction image is `/tmp/wasm-fist-reference-images/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`, frozen at reference `349ad31`.
These are raw load-module addresses. DGROUP is paragraph `0x1c00` (linear `0x1c000`); the far
service code uses CS `0x0f69`. Raw `0x1b1a2` is therefore original `0f69:bb12`.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xd7e1 --stop-address=0xd84e /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x1b176 --stop-address=0x1b294 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x43c1 --stop-address=0x4430 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x1a84c --stop-address=0x1a892 /tmp/wasm-fist-reference-images/fist_dat_image.bin
```

`d7e1/d81e` reads count, six-byte record header, then the first state word as constructor type.
Header words are **state length, registry index, saved registry word**. The historical typed
field `generation` preserves that last word. Constructor `1b1a2` receives the
type in AX, registry index in BX and saved word in CX. It stores DI and that word in the
four-byte DGROUP registry at `dfbc + index*4`; reset `1b197` clears 182 slots.
Runtime release decrements the saved word; a remaining nonzero value reserves an empty entry.
It is not a monotonic generation ID. See [runtime allocation](object-pool.md) for the recovered
identity/reuse contract and explicit exhaustion repairs.

Allocator `1b21d` uses bit 0 of the 28 type flags at DGROUP `e614`. Types 0, 1, 2, 3 and 19 allocate
251-byte objects; all other types 0..27 allocate 55-byte objects. Pool bases are `a022` and `c05c`.
The constructor writes type and its new pool index, then clears the remaining 51/247 bytes.
`d82e..d847` preserves that new pool index around the DOS read of snapshot bytes starting at +2.
Thus the saved second word is an allocation artifact, independent of registry identity/generation.

The new decoder rejects unknown types, mismatched type/size, out-of-range registry indices and
out-of-range participating platoon/member fields. It does not emulate allocation pools or impose
pool capacity as an asset-definition limit.

## Typed snapshot fields

| State offset | Wire field | Host interpretation |
| --- | --- | --- |
| `00` | uint16 | Original type ID, 0..27. Model/catalog semantics are subsequent work. |
| `02` | uint16 | Saved pool index; preserved for evidence only. |
| `04`, `08`, `0c` | int32 each | Original map X, Y and altitude; full two's-complement width. |
| `10` | uint16 | Heading, one unsigned 16-bit turn. |
| `16`, `17` | uint8 each | Original flag bytes; preserved before runtime initialization. |
| `1b`, `1c` | uint8 each | Regular platoon and member. |
| `23`, `24` | uint8 each | Type-23 placeholder platoon and member instead. |

Actual service instructions `1a866..1a876` copy X/Y, and `1a884..1a892` copy altitude/heading to
camera state. Original altitude's integer height byte is at +0d, giving 256 original altitude
units per decoded height unit; vehicle camera code also adds a separate offset at +87.
No runtime camera offset or ground placement is fabricated by the definition decoder.
Other fields are retained in the complete immutable snapshot, without assigning common meanings
to type-dependent bytes. Runtime vehicle initialization, AI, combat, paths and model resolution
remain open contracts.

## Registry and platoon roster

Registry and roster maps contain definition ordinals, or `UINT16_MAX` for absence. Both retain
the last assignment to a slot; duplicate identities do not silently discard earlier definitions.
The roster has 8 platoons × 4 members, cleared by `4413`. Original `43c1` implements:

- For regular units, participate only when flag byte +16 has bit `0x20`. The roster slot is
  `platoon*4 + member`, from bytes +1b/+1c; slot zero is valid.
- For type 23, use bytes +23/+24 regardless of that flag. Placeholder slot zero is skipped.
- Later assignments overwrite earlier entries. Header order and SHDR coordinates do not select
  the player. Runtime initialization sets additional flags; snapshots remain unmodified here.

Original startup `46f1` runs player selection and then looks up the selected roster byte offset
at DGROUP `6d36` in table `6d3c`. Its initial value is zero. Slot zero is an explicit sensible
inspection default, not proof that every original setting/user choice always controls that slot.
Side swap at `6db4` changes vehicle type/side during initialization; the decoder exposes the
normal-side definition roster and does not pretend to implement that setting.

Every one of the 47 published scenarios has an ordinary vehicle at roster slot zero. INDIA3
uses type 1; all others type 0. TRAIN1's slot-zero vehicle has registry index 32, X `-1061797`,
Y `1816527`, altitude `1280`, heading `51700`; it is not the first DCBS record. AZER2 uses index 29.
PINF remains opaque: its eight 22-byte records do not replace the roster assignment contract.

## Verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/build.sh all
python3 tools/check_style.py
python3 tests/test_units.py --originals
# Optional independent instruction oracle; fail if its pinned requirements are missing:
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_units.py --originals --oracle
```

Install the optional oracle requirements into an isolated `/tmp` virtualenv with
`tests/oracle_requirements.txt`. All probes and tests use temporary files under `/tmp`.
The normal build runs synthetic contracts on both targets; explicit original coverage requires
all manifest-pinned missions, exact complete output and unchanged original hashes without skips.

The C probe overwrites/frees input before inspecting every typed field and copied snapshot byte,
checks all registry/roster slots and getters, null API errors, unchanged outputs on semantic
failure, and repeated destruction. Tests cover all 28 allocation types, full signed pose widths,
all 32 roster slots, flags, alternate placeholder fields, missing player, duplicate assignments,
type/size/identity/roster errors and incomplete scenario framing.

`original_unit_oracle.py` executes pinned real 16-bit instructions with Unicorn 2.1.4, without
instruction replacement hooks. It runs original pool/registry/roster resets and constructor,
checks complete constructor initialization and snapshot restoration, executes the actual pose
copies and participation branches, then reads every original registry generation/roster entry.
For ordinary participants, execution resumes at `4402` after the excluded `c296` vehicle
initialization and side-swap branch; it proves assignment, not those runtime behaviors.
Type-23 assignment executes `43c1..43e9` including its real flag updates; comparison uses the
pre-initialization snapshot flags. Saved allocation indices are intentionally preserved by C,
while the oracle confirms original replacement with the fresh runtime pool index.

Verified on 2026-10-06: both production build gates, strict LLVM 19.1.7 format/tidy and all seven
unit test groups passed on native and WASM with the original-instruction oracle and no skips.
The corpus contains 47 missions and 4213 records (960 extended, 3253 short). The moved shared
signed-word reader also passed the eight complete original scenario groups on both targets.
ASan/UBSan with leak detection passed the same seven unit groups and complete original corpus:

```sh
mkdir -p /tmp/wasm-fist-unit-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itests src/assets/scenario.c src/assets/units.c \
  tests/probe_io.c tests/unit_probe.c \
  -o /tmp/wasm-fist-unit-sanitizer/unit_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  python3 tests/test_units.py --originals --target native \
  --native-probe /tmp/wasm-fist-unit-sanitizer/unit_probe
```

This proves snapshot/identity/pose/assignment ownership, not vehicle initialization, selected
player presentation, gameplay, audio or full rewrite completion. Presentation is unchanged.
