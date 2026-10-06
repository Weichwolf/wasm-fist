Type: tooling
Title: Oracle traces preserve executed code variants, context and control-flow edges

## Contract

Recover executed indirect targets, self-modifying variants and overlay code with enough context to
map them back to original images. Execution coverage is bounded by the recorded scenarios.

## Evidence

`tools/oracle/dosbox_blktrace.patch` and `blktrace_mission.sh` exist. The current instrument records
deduplicated instructions in first-execution order; that is not yet the full proposed block/edge
archive. The high-address set repaired omission of the 32-bit app at linear 0x10000000, CS=0x002b.
Historical AZER1 trace: 25746 unique instructions, including 4459 app instructions.
Observed relocated selectors 1119/2082/2119 correspond to port clusters 0000/0f69/1000 in that run;
resolve bases from live context, not those sample selector numbers alone.

## Next

1. Inspect current trace output before designing another hook. Demonstrate which of bytes, effective
   CS base, CR3, instruction mode, branch target and SMC versions it preserves or loses.
2. Add the missing block boundaries and observed edges. Recommendation: key a block version by
   effective start/context plus instruction bytes; a byte-only hash conflates different locations.
3. Test one known indirect call, one self-modified instruction and the high-address app. Recover
   the expected target/version and match disassembly at a known entry.
4. Expand across unit/weapon types and win/lose/editor/link scenarios. Store trace provenance and
   reproducible collection commands; use traces to supply evidence to 0014/0015/0017.

## Accept

Executed blocks, variants and observed edges reconstruct the traced control flow with correct
context. A run yielding no new addresses does not prove all branches or the whole game covered.
