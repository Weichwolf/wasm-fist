# Original mission-ready boundary

The C saved-world loader ends at d84a/43c1 and participating c296 initialization. A later
original registry reset is required before a real mission start. WI 0088 proves that boundary;
it does not implement C readiness or extend complete gameplay acceptance.

## Actual dispatch and fields

Frozen DOS image SHA256:
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
The normal mission entry e78c calls d755 at e80d, after e006 and before 9d26/player setup.
The saved-game entry calls it at e8d9. DCBS serialization calls d740 before saving and d755
afterward at d73c. Actual caller bytes are checked; the decompilation omits a complete e78c body.

d755 uses c157/c164 to visit all non-null bindings in ascending registry order and dispatches
through DS:e668. It does not filter deleted flags, participants or generation words. Overwritten
bindings leave physical orphans which this pass does not visit. Registry release mutations
are visible to the iterator; its final pointer is dfbc + 182 * 4 and remaining count is zero.

| Types | Actual entries | Complete effect |
| --- | --- | --- |
| 0/1/2/3 | 7bcf/8791/8fde/9787 | Ground reset below. |
| 16 | b51a | Increment census word 9c89. |
| 21 | 9cd3 → 9bef | Increment 930a, consume two RNG values, restore heading/extent/scale/flags and sample height. |
| 23/25 | bc2c/9af6 | Sample height through actual f69:b73d, raw 1adcd. |
| 26 | bcbf → bcd5 | Restore variant extent/scale/model/flags, sample height, apply variant-bit-4 flags. |
| 27 | b433 | Sample height, restore variant extent/+1f, append side artillery list and apply opposing flags. |
| 4..15, 17..20, 22, 24 | c30f | Actual identity lookup and release, including pool/registry/deletion writes. |

**c30f is not a RET**: it calls f69:bcc4 and f69:bc5f before returning. The adjacent c30e is
the real single RET used by ground deletion methods. Readiness e668 differs from initializer
e4e0 and deletion-method e6a0; confusing these banks changes occupancy, ammunition and RNG.

Ground reset writes only operating bit 10h (set exactly when link mode is 2), operating bit 1,
camera +87 using existing class defaults, zero current target +97 and nearest candidate +9d,
automatic control bit 1, zero byte +36 and the complete existing component template. Other
operating/control bits, ammunition, selection/reload, motion, phases, orders, goals, range and
heading/position histories survive. Ground reset consumes no RNG.

All **14 nonzero saved ground target words** survive c296 and become zero here. These original
near references are not canonical slots. Do not guess an arena-offset translation or clear them
inside the earlier loader. Future runtime discovery still needs typed identities and lifetimes.

## Evidence and declared device transfer

`original_mission_ready_oracle.py` executes original constructors, saved-byte restoration with
the new pool word preserved, complete d84a returns and original PATH/PINF reads. All 47 FSGs
have ground-only participants. Additional non-ground c296 participant initialization is outside
this boundary and fails explicitly.

The complete d755/class instructions execute unchanged. At e1eb, the e339 DOS/protected-mode
call is a declared device boundary. The actual kernel op-54 handler 11a6 runs on the supplied
field; its result returns through the original e1ee continuation, using the existing flight
height adapter. Kernel image SHA256:
`102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1`.
No modified guest instructions, instruction hooks or address-zero/one device scaffolding are
used. DOS read/seek alone is supplied through the existing file interrupt adapter.

Caller census words 930a/9c89/9ccb/9ccd start at zero and the PM mailbox is separate. Two valid
constructed 2×2 height planes and link 0/2 cover every original mission; full e006 and original
mission terrain/device setup are not claimed. The independent expected state rejects artillery
counts beyond the actual four-word array per side. All corpus cases fit.

Every complete return compares **all 65536 DGROUP bytes**, excluding only actual stack writes
8ff0..9001. Every physical payload, registry release, metadata byte, order, census/list entry,
RNG word/result/cursor and PM operation word is accounted for. Additional cases cover all 28
entries, both release arena widths, empty worlds, reverse binding order, deleted nonparticipants
and overwritten-binding orphans. Ground methods cover every flag byte and link 0/2/255.

TRAIN1 retains 85 objects, samples 82 heights and consumes **98 extra RNG values** for its
49 trees. With seeds `(1, 2, 32768, 65535)`/cursor 0, the loader ends at
`(23040, 46080, 16384, 52223)`/cursor 2; readiness ends at `(5829, 11658, 45, 54666)`/cursor 0.
The complete first controlled ab03 call, after actual take-control, reaches bank entry 15's
genuine b111 RET with counter 254→255. Its consumed value changes from **8191** without
readiness to **48993** with readiness. Both complete DGROUP results agree independently.
This is a command-boundary proof, not a complete tick or changed C driving implementation.

## Reproduction and next work

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_mission_ready.py \
  --originals --review-dir /tmp/wasm-fist-0088-review
```

The required original-only gate is outside production C gates. Missing originals, incomplete
returns, missing groups and skips fail. Closed 0088 records exact coverage and both-target
regressions. Existing loader/prefix contracts retain their earlier boundaries.

Add a separate shared canonical readiness owner using existing templates, RNG, terrain sampling
and pool release. Own +36/+9d, static types 16/25, tree reset and bounded artillery lists;
preserve registry order, physical orphans and atomic failures. Then consume complete bearing,
range, target discovery and the remaining WI 0081 callbacks using actual runtime identities.
Do not fold readiness into c296 or silently clear targets in fixtures. Battle, PCM, outcomes,
all other surfaces and the independently verified ten-run WASM gate remain open; streak zero.
