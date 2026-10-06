Type: feature
Title: Gameplay keyboard and mouse traces reproduce original control responses
Parent: 0005

## Contract

Drive, steer, aim, fire and navigate gameplay views through original keyboard/mouse semantics on
native, node WASM and the browser.

## Evidence

- Actual scheduler3446's PUSH-CS/near-CALL reaches46b6 before startup's first6c.
  The original helper exchanges both WORD operands of the keyboard ISR's inline
  FAR-CALL at image14627: callback offset/segment live at image14628/1462a.
  Natural input2082:3e0b replaces1119:0001; AX/ES return1119 and BX returns1,
  preserving upper register bits. The actual module CS rebases to0f69; the
  generated DAT4f98/4f9a instead alias instruction bytes14f98=740e8bcb.
  Static original14517 installs INT9 at0f69:4ea7 (image14537), whose port60/61
  input path contains that inline call. The generated engine has no4537 ISR
  body/fmap entry: source registration is not production keyboard dispatch.
- Reproduction: `python3 -B tools/oracle/capture_callback_exchange.py --output
  /tmp/wasm-fist-callback-exchange`. Fresh natural, independent high-word/
  segment/offset, IF0/IF1 and null-install cases account for10 fetches each,
  all GP/segments/control/raw+lazyflags and every16-MiB stack/operand write.
  PUSHF materializes the natural WORD-ADD flags; CLI clears IF temporarily;
  POPF restores all saved flags. XCHG returns both old WORDs, including when
  installing a null pointer. Explicit control restoration changes no clock or
  budget. All39 frames/27518 mixed samples/end600ms retain unobserved-original
  bytes. Compact states, code/image/script/binary hashes and source command:
  `tools/oracle/callback_exchange_case.json`. This source evidence accepts no port behavior by itself; the production
  WORD exchange is separately proved below. Live INT9 input consumption,
  port CPU/IRQ/time, complete keyboard/mouse behavior and full video/audio
  remain open.

- Patch636 accepts the source-proven WORD callback owners at14628/1462a,
  explicit0f69 caller ES, packed old offset/segment and actual13483 CLC in
  the existing carry lane. The actual parent Native3446/46b6 startup left
  the real callback unchanged, overwrote instruction bytes14f98 and returned
  AXcb8b instead of0: eight whole16-MiB differences at the reaching writer.
  Five actual parent leaf/caller cases per target fail all ten whole-memory
  comparisons; five new behavior tests pass both targets, including null
  install, independent old/new segments, actual caller list writes and CLC.
- Base89ee773's frozen1015-input unfiltered gate passes117 tests, exact
  patches, sequential Native/WASM builds, six complete startup cases and all
  178 existing flows, durable gate/runner exit0. All419 original file hashes
  and inventory remain unchanged. Actual Native3446/46b6 observation proves
  precisely four callback-byte writes, packed old pointer1, unchanged leaf
  clock and preserved stale instruction bytes through first6c. Reproduce with
  `python3 -B tools/capture_callback_exchange_native.py --native NATIVE
  --output /tmp/wasm-fist-callback-native`; its fresh portable observation
  also passes. Compact source/parent/target/binary/output proofs and commands:
  `tools/oracle/callback_exchange_production_case.json`.
- Fresh complete30000-ms captures retain all2100 parent frame/end bytes,
  43 sound-register rows and47 full packets per target. Native/WASM frame
  records match; all original time/layout/palette fields match. The same28
  pixel differences begin at817/11704156us/byte8754. Strict comparison fails
  absent final mixed port PCM; the full op68 Native/WASM inequality persists.
  The first attempt failed Native linking because the temporary output parent
  was missing; it is excluded. Source upper GP/raw+lazyflags/farstack and
  IRQ/IF/time remain separate port architecture work. The actual INT9 ISR
  input consumer, keyboard/mouse behavior and complete output remain open.

## Next

1. Recover the actual INT9 callback consumer using the registered pointer proved
   above. The generated engine has no4537 ISR body/fmap entry. Registration
   does not prove that input traverses it; require original timed scan-code
   evidence, reaching two-target regressions and complete required coverage.
2. Record a short original input trace for each action; include press/hold/release, simultaneous
   keys/buttons and view transitions. Match seed and monotonic simulation step on the port.
3. Trace how BIOS keyboard reads, scan codes/INT 9 and mouse callbacks feed engine state. Implement
   the missing boundary behavior; do not bypass it by setting vehicle state directly.
4. Extend the existing injection harness with timed gameplay events, isolated data and explicit
   completion markers; reuse it for browser checks under board:0026.

## Accept

Input-visible state transitions and rendered/audio responses match the original and both targets.
No stuck controls after release, view switch or browser focus loss; emulate original behavior where
it differs from host behavior. Menu regression flows remain passing.
