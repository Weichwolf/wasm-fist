Type: bug
Title: INDIA3 cockpit spawn is covered on both targets

This engine can load INDIA3 and produce matching native/WASM spawn cockpit chrome without the
historical allocation crash in the exercised flow.

## Evidence

Rechecked during consolidation (2026-09-22) against engine revision
`7fb23e5f26716b0dd1d90c30b3fb7a8807ac9bee`, with freshly built native and WASM binaries:

```sh
FIST_FLOWS='^(intro|mainmenu|mission-cockpit-india3)$' bash tools/verify.sh both
```

Result: intro, mainmenu and mission-cockpit-india3 each PASS; three passed, zero failed.
Log: `/tmp/wasm-fist-docs-smoke-both.log` (session-local; command above reproduces the check).
The INDIA3 comparison covers only `MC_REGION=100x92+80+96`, uses the default op-0x24 capture,
and has no DOSBox reference. The old notes described an op-0x2c experiment; these are distinct checks.

## Limits and follow-up

This closes the named spawn-flow regression, not full-frame oracle fidelity or a whole mission.
Board:0001/0027 own complete render checks; board:0012 owns post-spawn parity. The historical
near-heap OOM's fixing patch was never established by bisect; do not invent that attribution.
