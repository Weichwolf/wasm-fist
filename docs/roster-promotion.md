# Original physical roster promotion

Closed WI 0098 proves the complete unchanged b053 -> f69:b329 (physical 1a9b9) return and
genuine ab03 entry thirteen in both control banks. Closed 0099 delivers shared C consumption;
complete 0081 parent/class work, living battle and PCM remain required.

The actor's member byte +1c gates the callback before any platoon/roster input. A nonzero
member selects the preceding physical roster pointer. Null predecessors permit promotion
even when the actor's motion flags would prohibit the ordinary ground branch. Type-23
predecessors also permit it, incrementing their independent +24 member byte. For another
ground/retiring predecessor, actor motion bits 0x10/0x06 prohibit promotion; otherwise those
bits on the predecessor permit it. Neither deletion/side flags nor registry occupancy gates
this callback. Actual allocated registry-overwritten actors retain usable physical roster
references.

## Shared C transaction

Closed WI 0099 implements `fist_mission_world_promote_member` in `src/sim/roster_promotion.c`.
The existing physical world/payload and combat roster remain the only owners. Admission
validates used actor/member/index/predecessor identities and tags before any write; a leader
does not inspect platoon, roster, orders or RNG. Null/wreck predecessors bypass actor motion
flags. Other ground/retiring predecessors retain their actual flag admission, including
ignoring unused predecessor payload data after actor rejection. A malformed used member/index
or unavailable/mistagged predecessor fails with the entire world unchanged. Orders, RNG,
registry, selected-player state, census, references and all unrelated payload fields survive.

The extended existing probe retains its earlier discovery/acquisition protocols and adds
typed roster transport. Actual allocation/retype/release/reset/registry-overwrite checks run
before promotion batches. Raw source records are poisoned before simulation. Every stage
checks whole-world preservation and publishes the complete roster and every retained typed
member/control field, rather than a selected scalar subset.

The required native/WASM replay passes five groups without skips: 295405 complete cases,
11096 atomic rejections and 3840 actual canonical callbacks per target through all 47 missions
at four details. The consuming C loader/preparation verifies actual occupancy, bindings,
poses, flags, member/control words, complete roster and RNG before each sequential callback.
The unchanged five-group original replay retains its accepted 0098 output SHA256 and full
110451-return scope, including genuine parent returns. No partial C parent bank is installed.
C output SHA256: `204bccefe06e0bc8cfb061b7c7661227ee34c29e3a718bd71a559e8eec745c63`.
Strict LLVM 19.1 formatting/tidy passes all 89 owned units/headers. The complete production
build passes 43 native CTests, 41 WASM unittest suites and both Node probes in 1753.690
seconds. ASan/UBSan passes the same five required groups without skips in 196.499
seconds, with identical complete case/canonical counts and output SHA256. Actual SDL,
Chromium and sanitized SDL scenes pass complete control/frame checks; both retained after
images were visually reviewed. Final source/program/original/reference hashes agree.
Compact results and reproduction commands are retained in `/tmp/wasm-fist-0099-review/receipt.json`
with two reviewed PNGs; obsolete owned builds/scenes/assets/helpers/logs are removed.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_roster_promotion.py --target all --oracle --originals --review-dir /tmp/wasm-fist-0099-review

## Original verification

Promotion writes the actor into the preceding roster word and the old predecessor into the
next word, decrements actor +1c and clears goal-valid control bit 2. A used predecessor member
increments with byte wrapping. The wreck branch bypasses both motion-flag checks. The callback
does not recalculate the census, choose a player, advance a route, consume RNG or produce
presentation/audio. Keep one canonical roster owner and mutate typed payloads rather than
saved pointer words in the C successor.

The original index calculation shifts platoon by two, adds member to BL without carrying into
BH and shifts the word once more. The authored domain has eight platoons and four members.
All 2016 constructed member values 4..255 across eight platoons complete unchanged original
returns; 1799 write outside the roster and 112 wrap the low-byte lane. These are malformed
input observations, not a claim about untouched authored missions. Shared C must reject used
out-of-domain values before mutation and preserve the unused member-zero early return.

The required five-group gate passes without skips in 100.935 seconds:

- 110451 complete child/parent returns, including 11007 genuine parent returns.
- 81920 flag-domain cases cover every actor/predecessor flag byte, all four actor classes and
  ground/retiring predecessor classes with all relevant joint flag combinations.
- 11668 null/wreck/alias/member-wrap/unused-leader cases, including all 256 unneeded leader
  platoon bytes and 20 actual allocated registry-overwritten ground/wreck continuations.
- Every parent admission byte, both banks, all classes/cursors, counter high nibbles and RNG
  boundaries preserve the unconditional draw, conditional dispatch and exact global state.
- All 47 pinned missions, eight original height maps and four actual preparation details:
  188 worlds, 3840 ground actors and 11520 complete sequential child/parent returns.
  SAUDI1 has one actual child roster change and SAUDI5 has two per detail; total twelve.

Output SHA256: `bfcc7a60796d740c9279def6e9b79110e4a3bfc664c66cc23b2416ab6ad37473`.
Each return compares full DGROUP outside bounded call-stack scratch, unchanged instruction
bytes, GS text/tables, mailbox and complete actor/segment/stack return state. Parent inputs
explicitly select entry thirteen and clear diagnostic selection; other parent callbacks and
the diagnostic producer are not replaced or accepted by this gate.

The first complete run passed before adding actual allocated-orphan and unused-leader-byte
coverage; it is superseded. An additional fixture initially expected a wreck at roster index
zero, which actual 43c1 explicitly skips. The fixture now uses platoon one with the unchanged
model and complete-state assertions. Supporting/failed pilots are excluded from acceptance.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_roster_promotion.py --originals --review-dir /tmp/wasm-fist-0098-review

Compact results, source/original/reference hashes, commands and exclusions are retained in
`/tmp/wasm-fist-0098-review/receipt.json`; obsolete owned logs are removed. At the original-only
0098 checkpoint, production C/build configuration and existing scene artifacts were unchanged
from 224cec0; that research added no native/WASM/style/presentation/PCM acceptance. Closed
0099 shared C acceptance is described above; full parent/class/battle/PCM remain open. The independent
full-game WASM streak is zero.
