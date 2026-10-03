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
  `tools/oracle/callback_exchange_case.json`. This is source evidence only;
  setter/caller repair, live INT9 input consumption and port CPU/IRQ/time,
  complete keyboard/mouse behavior and full video/audio remain open.

## Next

1. Repair the reached46b6 full pointer exchange and3446 caller through the
   original cases above, reaching target regressions and complete production
   startup/full target matrix. Recover the actual INT9 callback consumer;
   registration alone does not prove that inputs traverse it.
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
