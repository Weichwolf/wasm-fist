Type: feature
Title: Two-player serial sessions reproduce the original link protocol
Parent: 0005

## Contract

Reproduce original-compatible serial setup, active two-player gameplay and failure handling
through the recovered engine protocol.

## Evidence

The link transport is not reliably located. Previous 0x3f8/0x62f string-helper hits were DGROUP data
offsets. Absence from one battle trace does not establish that the original has no link code.

## Next

1. Drive the original link setup and record file loads, interrupt/port accesses and control flow.
   Locate the actual serial boundary and identify framing, handshake and timeout behavior before
   selecting a host transport. Use board:0013 tracing where static dispatch recovery is incomplete.
2. Implement a deterministic byte-stream adapter around the recovered engine boundary. Recommendation:
   keep protocol state in the engine, host transport outside it; do not invent a replacement protocol.
3. Exercise two isolated peers with connection, gameplay, delay, dropped connection and reconnect
   scenarios the original supports. Keep seed/input/scheduling logs for both peers.

## Accept

A two-player session completes with original-compatible traffic and state on native and WASM.
Link settings alone do not count; handshake, active play and original failure handling are verified.
