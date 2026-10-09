Type: Work item
Title: Prove complete original selected-ground manual composition
Depends: 0121, 0122, 0123, 0124

## Contract

Recover and independently predict complete original a57a with both complete
six-entry drive and five-entry turret/elevation banks. Prove selected/unselected
and inhibited admission, saved selector/view retention, complete class/display
refresh, shared selectors and adaptive elevation/device-clock ordering. Use real
allocated actors, unchanged instructions and independent complete memory/ABI
predictions. Identify producer bounds and unsafe used selectors explicitly.
This checkpoint accepts original evidence only.

## Evidence

The pinned original image is
d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
The immutable reconstruction, ghidra and pinned softgl remain unchanged.
tests/manual_control_contract.py composes the existing independently proved
axis, turret and elevation models. Its SHA256 is
a0cdfaa3372e2724d479209e1f6aa68015b4480d0dddf0fba8518d5fcfaaf838.
Versioned fixtures live in tests/fixtures/ground_class_callbacks/.

All six required fixture commands terminate with exit 0; none skips coverage:

| Fixture | Required cases | Complete scope |
| --- | ---: | --- |
| manual_parent_original.py | 591872 | Both banks, all view bytes, axis/profile/elevation boundaries, every ignored/inhibited word and wrapped bank aliases. |
| manual_parent_shared.py | 12288 | Three retained 1024-tick sequences across four actual allocated actors and one shared selector/control state. |
| manual_retention_original.py | 4096 | All saved +a0/+a4 byte values, four classes, start/readiness and both link boundaries. |
| manual_selector_producers.py | 265512 | Complete key tail, actual UI/config store prefixes and full manual-parent couplings. |
| manual_parent_corpus.py | 86400 | All 47 pinned read-only saved sources / 960 actors through both banks, selected/inhibited/unselected. |
| manual_invalid_selectors.py | 263200 | Proposed used-domain guard inventory, genuine erroneous returns/double turns and four required original fetch failures. |

The first five groups include 960168 complete original observations. The last
group separately distinguishes Python guard-domain checks from actual DOS:
262144 drive-word and 1024 action-byte guard cases; four next-used rejections;
16 accidental returns, four inhibited turns, four erroneous held double turns
and four intentional fetch failures. Those failures are required negative
evidence, never accepted as completed game behavior.

The parent, shared, corpus and producer fixtures compare all 0x60000 mapped
bytes and eleven registers, including independently predicted stack scratch.
There are no patched instructions, masked differences or replacement providers.
Retention compares the complete actor, RNG and other retained actors through
the existing independently proved initialization/readiness owners and return ABI.
Source corpus restoration preserves the constructor's actual physical index;
explicit actions/admission/clock/control inputs are recorded in the fixture.
It does not claim complete prepared-world execution.

First-bank word indexing wraps after multiplication by two. Modes 0..5 and
32768..32773 name the same complete bank. Only modes 1..4 apply decoded axes;
mode 2 then selects view 1/2/6 from unsigned +a4 bands below 80/below 160/otherwise,
refreshing all three class components and all three display bytes before the
weapon callback. Second-bank byte offsets are exactly 0/2/4/6/8. Turn callbacks
retain selector 88/232; elevation retains shared adaptive controls and borrowed
0452 clock. Inhibition only suppresses the first bank; a manual turn/elevation
can clear it, making the same held mode used on the next call.

The complete a521 key tail is tested for every word on all four classes. Its
ordered bits 16/32/128/256/64 write action 0/2/4/6/8 respectively; later set bits
win. No set action bit retains the previous byte, including invalid values.
All 256 retained action bytes are covered. Genuine UI 6b27/6b55 calls execute
through the completed 6c38 store at stop 6c3b. Exactly five UI table entries
produce modes 0..4; the next callback at 6b60 is a different setting. Config
6ef5 through store-stop 6f05 maps ASCII 0..4 to those modes and other bytes to 0.
The sixth bank entry 5 is a complete no-op, but no UI producer for it is proved.
These named prefixes prove stores and domains; remaining widget callbacks,
configuration parsing and full hardware/UI returns remain open. No meaning
for unresolved +a3 or physical device identity for decoded +a4 is asserted.

Unsafe used words do not universally trap: modes 6/32774 resolve to a59b RET,
4096/65535 to code 0 RET. Mode 7 resolves to the left turret wrapper and produces
two turns after inhibition clears; whole memory and ABI match that independently
predicted erroneous composition. Action 255 resolves to a804 and causes an
actual unmapped fetch on each class. Shared C must reject used indices outside
the proved complete bank before mutations, while ignoring truly unused values.
It must preserve the complete actor, controls and output events on failure.

Three development hypotheses failed and were corrected from unchanged bytes:
the near target-clear return overwrites the earlier pushed CS in turret scratch;
the UI has five setting callbacks, not six; and mode 4096 accidentally returns
instead of trapping. Compact rejected-run explanations are retained. No failed
run supplies an accepted fixture result or weakened assertion.

Retained child regressions also terminate with exit 0 and preserve published
digests: elevation_shared.py 9, analog_drive_shared.py 384 and
manual_turret_shared.py 2048 complete original calls. This checkpoint changes
Python evidence and work-item documentation only. Owned C, build configuration,
dependencies and production binaries retain closed 0124 acceptance; no new
native/WASM C acceptance is claimed.

Output SHA256 pins, in the table's order:

    623507754554d7f6234f6172bbb52f998c3bc21bdf155cc13f23bc8cbe957a32
    890bd6a90b2f24d0b324e6edc6b9b6fa2df9db2b20f1958d988e0f5795e407ab
    47612981d838d0d5c893a98cabd70950962f330a33ed71c8c0ffbc62a02807b4
    f419d978d804e1587ec24ade5d0c40b4683743b3a644e49b4873302262f67971
    6cf0da850b62afbd48a979ef059a0a03c105f6baa21b1e7b49b785a1960093e7
    979c6ac1b6aebf37d0f2435c1f119844f7b2fb0f94d63908c44cb7d024e42833

Exact fixture/model/image pins, terminal receipts and rejected hypotheses are
under /tmp/wasm-fist-0125-review. Run from the repository root, with ignored
read-only originals provisioned and tests/oracle_requirements.txt installed in
an external environment. Fixtures write compact receipts to that /tmp directory:

    mkdir -p /tmp/wasm-fist-0125-review
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_parent_original.py
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_parent_shared.py
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_retention_original.py
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_selector_producers.py
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_parent_corpus.py
    PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/manual_invalid_selectors.py

## Next

Implement complete shared manual composition under 0126 using the existing
driver/weapon owners, saved selector/view retention, full refresh and one
transaction for both banks. Complete class contacts/maintenance/engine, device
production, world scheduling, first playable battle/PCM/outcomes remain under
0121/0041/0065 and the later game-surface items.

## Accept

Met: all six complete required original/evidence groups and retained child
regressions pass with exact coverage, genuine allocated actors, unchanged
instructions and explicit producer/repair boundaries. Publish only versioned
fixtures/model and compact work-item evidence; clean obsolete owned raw logs.
Shared C, full class/device/world/game acceptance and the final ten-run WASM
gate remain open. Final complete-game streak is 0.
