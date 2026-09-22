Type: feature
Title: Gameplay keyboard and mouse traces reproduce original control responses
Parent: 0005

## Contract

Drive, steer, aim, fire and navigate gameplay views through original keyboard/mouse semantics on
native, node WASM and the browser.

## Next

1. Record a short original input trace for each action; include press/hold/release, simultaneous
   keys/buttons and view transitions. Match seed and monotonic simulation step on the port.
2. Trace how BIOS keyboard reads, scan codes/INT 9 and mouse callbacks feed engine state. Implement
   the missing boundary behavior; do not bypass it by setting vehicle state directly.
3. Extend the existing injection harness with timed gameplay events, isolated data and explicit
   completion markers; reuse it for browser checks under board:0026.

## Accept

Input-visible state transitions and rendered/audio responses match the original and both targets.
No stuck controls after release, view switch or browser focus loss; emulate original behavior where
it differs from host behavior. Menu regression flows remain passing.
