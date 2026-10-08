Type: Work item
Title: Prove original sound request queue returns and consuming support coupling
Depends: 0103

## Contract

Execute complete unchanged PM op-64 admission/queue returns for authored direct and queued
channels, real DSOUNDS/WVSOUNDS/EVSOUNDS records, silence, disabled sound and absent banks.
Predict complete queue, rate, count, mixer-immediate and return-register writes independently.
Keep original bank bytes read-only and verify every unrelated kernel byte and mapped buffer.
Prove the consuming b0be retreat/smoke/air/artillery return through actual DOS instructions,
real allocation/constructor/height and actual op-64 return, varying only legitimate bank
placement/device state. No fabricated audio return or gameplay replacement.

## Evidence

WI 0104 reaches a28c in retreat mode before comparing AL again. Complete type-20 smoke
handlers return AX=39 if c047 does not match the source, AX=0x3031 on failure, and pass a
real op-64 response otherwise. Original 786a -> 22ab/2377 returns an actual sample address.
This exposes a possible dependency of support choice on sample placement. Complete coupled
execution must prove it before remaining-callback consumption; full 0104 stays active.

### Verified preparation checkpoint

Static instruction review against the pinned images confirms that b108 calls a28c and b10b
compares the resulting AL. The M1 smoke path at physical 1784a calls the real type-20
constructor, loads AX=39 and calls c047. That producer calls e2c2 for admitted requests;
e2c2 selects PM operation 0x64. The kernel dispatch table selects 786a, which either returns
without sound admission or selects a bank record and calls 22ab/2377. Both admitted paths
return its sample pointer in EAX. The queued path also zero-extends DL into EDX at 239a,
including when its channel is already busy; an independent model must retain that write.

Reproduce from the repository root after provisioning the original files:

```sh
python3 tests/reference_images.py
objdump -D -b binary -m i386 -M intel,addr16,data16 --start-address=0xb0be --stop-address=0xb111 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 --start-address=0x1784a --stop-address=0x1787a /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 --start-address=0xc047 --stop-address=0xc06d /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 --start-address=0xe2c2 --stop-address=0xe2df /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel --start-address=0x786a --stop-address=0x78cd /tmp/wasm-fist-reference-images/fist_image.bin
objdump -D -b binary -m i386 -M intel --start-address=0x22ab --stop-address=0x23c4 /tmp/wasm-fist-reference-images/fist_image.bin
```

Engine SHA256: d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
Kernel SHA256: 102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1.
The frozen reference remains 349ad31a9fd21b350d435651bb2e90afda40cf60. At this static
preparation checkpoint, no independent queue model or complete coupled execution had passed.
The complete original proof is recorded in Delivery below; shared implementation and
functional repairs remain open. Runtime, existing tests, configuration and softgl are
unchanged from accepted 426ab57; its production evidence remains applicable.

## Next

Continue complete 0104 recovery with the actual consumed-response evidence. Preserve the
observed source/stock/capacity/device/heap dependency until an explicit evidenced shared
behavior repair is specified. Recover intended handling of the authored sample-15 sentinel
overread before accepting a shared bank/PCM consumer. Do not install an inferred gameplay rule.

## Accept

Required full-bank/request/domain and coupled complete original returns pass without skips
or missing output. Verify full kernel/DGROUP/code/text/mailbox, actual returns/registers,
read-only original banks and only declared queue/mixer writes. Source/reference/original
pins and existing consuming production evidence remain valid. Preserve compact reproduction
and fixtures under /tmp, clean obsolete owned artifacts, then commit/push this bounded proof.
PCM mixing/output/timing, complete device initialization, intended support repair, complete
0104/0081/class/battle/outcomes and independent complete-game WASM acceptance remain open.

## Delivery

Complete original primary gate passed six groups without skips in 369.779 seconds, terminal
exit zero. Coverage: 349012 kernel requests, 123260 complete DOS returns, 83291 actual paired
kernel returns, 89453 actual type-20 constructors/height returns and 256 complete effects
allocator returns. All 101 records, six channels, disabled packet word domain, sound byte,
rate word/attenuation domains, source/selected contexts, stock/capacity/admission and every
support queue index/full exit pass with whole-state guards. 38784 record placements and
3072 paired contexts are allocator-aligned; additional unaligned inputs are counted separately.
The actual post-smoke AL is observed before the original comparison and complete b0be return.

Identical complete DOS inputs at enabled aligned bank offsets 0/32/128 yield AL 8/40/136 and
air/artillery/no-support respectively. Disabled/absent sound yields AL 11 and air. Complete
request/register/queue/mixer and height/DGROUP/code/text/mailbox checks pass. No return or
gameplay call is fabricated. Emulator recycling bounds independent-fixture translator storage
without changing sequential state. Final output hash equals the preliminary complete pass:
7f04b24f3da9596c80c0cf838028e7c71129384b212f28672366f587c68f3be0.

The required authored-invalid gate also passed two groups without skips, terminal exit zero:
216 complete kernel requests, four complete c047/kernel pairs and four allocator returns.
Real selector 51 admits sample 15; its zero sentinel produces a one-past-DSOUNDS pointer and
reads an actually adjacent WV/EV bank as the effect header. This is explicit invalid-domain
evidence, not a valid extra record or accepted PCM repair. Result hash:
4b6ca963878f50afd891127ae42fe065ae2bfb7a193b14e992fd344442ee2666.

Reproduction and exact scope: docs/audio-request-boundary.md. Required commands run
tests/test_original_audio_request.py and tests/test_original_audio_tail.py, both with
--originals --review-dir /tmp/wasm-fist-0105-review using the pinned /tmp oracle environment.
Compact audio-request.json, audio-tail.json and receipt.json retain coverage, all bank-record
metadata, paired inputs, hashes and source/reference/original pins. All 419 original files
were reverified unchanged/read-only. Frozen ghidra/tag and pinned softgl are unchanged.
All 261 accepted tested sources and three production programs remain hash-identical to
0103; its native/WASM/build/style/memory/scene evidence is carried forward. This checkpoint
changes reference tests and documentation only. Full 0104/all-47/genuine-parent consumption,
shared smoke/support/PCM, intended repairs and complete-game acceptance remain open.
