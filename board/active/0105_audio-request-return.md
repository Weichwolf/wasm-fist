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
The frozen reference remains 349ad31a9fd21b350d435651bb2e90afda40cf60. This checkpoint
records the prerequisite only: no independent queue model, complete coupled execution,
shared implementation or functional repair has passed. Runtime, tests, configuration and
softgl are unchanged from accepted 426ab57; its production evidence remains applicable.

## Next

Recover and independently model all authored op-64 queue branches and complete bank record
selection. Compare unchanged instructions with original file records, missing/disabled/silence
contexts and complete state guards. Pair actual b0be returns at sample placements that select
air, artillery or no request; account for stock, allocation, height, clock, target/caller
identity, notices and real queues. Record the observed dependency and the still-unproved
functional repair; do not install an inferred gameplay rule.

## Accept

Required full-bank/request/domain and coupled complete original returns pass without skips
or missing output. Verify full kernel/DGROUP/code/text/mailbox, actual returns/registers,
read-only original banks and only declared queue/mixer writes. Source/reference/original
pins and existing consuming production evidence remain valid. Preserve compact reproduction
and fixtures under /tmp, clean obsolete owned artifacts, then commit/push this bounded proof.
PCM mixing/output/timing, complete device initialization, intended support repair, complete
0104/0081/class/battle/outcomes and independent complete-game WASM acceptance remain open.
