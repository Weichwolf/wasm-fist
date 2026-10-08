# Shared ground station and support consumption

WI 0107 is accepted. Complete station/support children and canonical lifetime ownership
pass their bounded acceptance gates.
Original source and complete reference evidence are recorded in
[remaining-ground-command.md](remaining-ground-command.md) and
[support-state-boundary.md](support-state-boundary.md).

| Boundary | Ownership and behavior |
| --- | --- |
| `fist_mission_world_configure_support` | Copies explicitly decoded catalog or manual-mission inputs. Resets side clocks and queue admission fields; retains global request history, queue tails and canonical artillery resources. Configuration may alias the world's stored configuration. |
| `fist_mission_world_select_station` | Uses authored target/class preferences, canonical target lifetimes, existing mechanical weapon selection and shared voice admission. Publishes a logical station/HUD event and selected advisory. |
| `fist_mission_world_request_support` | Applies side, target, range and wrapped-age admission; consumes conditional RNG; stages smoke construction and real air/artillery request queues transactionally. |

`preparation.artillery` remains the sole ordered resource list. Preparation captures each
physical allocation's lifetime; consumers never recapture a possibly reused slot. Type-27
`rounds` is the existing original ammunition word at +1f, initialized to five by preparation,
cleared by destruction and spent before artillery busy/queue-capacity checks. The former
`animation_counter` name did not describe that producer/consumer role. Orphaned live resources
remain usable; mode/deleted flags do not introduce additional eligibility rules. A live
reference with an inconsistent physical type or payload is an atomic input error.

Smoke spends the existing M1/M3 byte stock or T80 word stock before allocation. BMP does not
spend its retained trailing byte. Successful construction publishes a typed type-20 object
with wrapped position, hull rotation, signed velocity and sampled altitude. Its ground-height
member and other constructor fields remain zero. T80 dirties its existing component and sets
its eight-tick countdown only on successful creation. Later type-20 dispatch remains explicitly
unsupported; publishing a constructor does not complete its lifetime or rendering behavior.

The deliberate gameplay repairs are explicit:

- Used type-26 variants outside 0..3 fail atomically. Expired targets clear their runtime
  reference without matching a reused successor; saved near words remain opaque.
- Every chosen artillery gun writes the typed side cooldown, including indices two and
  three, which wrote an unaligned original word. Ammunition still precedes busy/full exits.
- Every smoke attempt selects artillery, preserving the proved source-unmatched branch
  as a stable design choice. Logical sound is emitted separately for the matching source.
  Sound banks, device responses and PCM return values have no simulation API input and
  cannot select support. Without a smoke attempt, the original RNG decision is retained.

The consuming probe uses production builds and existing independent reference models.
It compares the complete owned actor observation, RNG, both queues, configuration/history,
resource ammunition and constructor. It also checks unchanged unrelated world bytes,
unchanged error outputs, actual target/resource release and same-type reuse, registry
orphans, source storage destruction and malformed/missing probe input. The separate
`tests/test_remaining_ground_canonical.py` gate loads and actually prepares all 47 missions
at four details with eight original height maps. It checks complete preparations and child
observations, captured prepared resource lifetimes and admitted queue requester release/reuse.
Its independent canonical inputs and predictions were verified against complete original
returns by closed WI0109. Artificial declared fixtures alone do not establish this gate.

Production native/WASM and ASan/UBSan domain/canonical replay pass; actual native/browser
scene and input checks pass too. Strict LLVM19.1.7 style/tidy, both complete production builds,
all46 native tests, all44 WASM suites and both Node renderer probes pass. Required original
ownership and both-target original preparation/driving regressions also pass without skips.
Exact source/program pins, commands, compact results and completed-log evidence are retained
in `/tmp/wasm-fist-0107-review/acceptance-final-receipt.json` and its linked receipts.

Full parent dispatch, queued strike dispatch, living battle, subsequent smoke methods,
PCM/device presentation and complete-game acceptance remain separate unfinished work.
