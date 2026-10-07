# Ground route and formation goals

WI 0084 implements the complete ac75 return. It dispatches saved command mode +43 through
the original eight-word DS:9800 table: mode 0 to ac7e, mode 2 to ac9e, mode 4 to ad03,
mode 6 to ad02 and modes 8/10/12/14 to ad04/ad05/ad06/ad07. The latter six entries are
actual single RET instructions. Odd or out-of-table saved modes are rejected, not masked.
No parent phase, route advancement or target resolution is claimed by this helper.

Mode zero copies the first signed DWORD XY at route +12/+16 and sets control bit 2 when
the canonical route is nonempty. Empty routes retain the complete old goal and valid bit.
No route/header/unused coordinate slot is mutated. Capacity 32 remains the original editor's
proved saved-data limit, owned by the route decoder rather than inferred from shipped counts.

Mode two returns without reading formation data when control bit 1 is clear. Automatic
followers use descriptor word +4, one of six formations, and member 0..3. DS:9830 points
to six consecutive sixteen-byte tables; every member contains a heading word and signed
distance word. Original UI mutation tails 62cf/62e2 independently prove six choices.
The shared roster's existing four-members-per-platoon contract supplies the member domain.

| Formation | Member 0 | Member 1 | Member 2 | Member 3 |
| --- | --- | --- | --- | --- |
| 0 | 0, 0 | 47320, 512 | 18200, 512 | 0, 512 |
| 1 | 0, 0 | 49140, 512 | 16380, 512 | 16380, 1024 |
| 2 | 0, 0 | 32760, 512 | 32760, 1024 | 0, 512 |
| 3 | 0, 0 | 43680, 512 | 43680, 1024 | 10920, 512 |
| 4 | 0, 0 | 21840, 512 | 21840, 1024 | 54600, 512 |
| 5 | 0, 0 | 43680, 848 | 21840, 848 | 32760, 912 |

The helper reads the canonical physical leader from roster `platoon * 4`. Absence or type 23
returns without changing the actor. A live ground leader may be a registry orphan or the actor
itself; registry binding is not substituted for physical roster identity. Inconsistent roster
references, wrong payload types/components and malformed used formation/member values fail
transactionally. Other mode entries and controlled followers preserve unused descriptor words.

Add the authored formation heading to the leader's retained +2e average modulo 65536, call
the existing fine planar rotation, multiply signed returned lanes by 32 and add them to the
leader's signed DWORD position modulo 2^32. The goal-valid bit is set; every other control bit
and actor/order/leader/RNG value is retained. Existing shared `fist_rotate` and `fist_position_add`
own numerical conversion; this callback adds no second trig table or coordinate wrapping rule.

Owned command state now retains signed goal XY and three heading-history words plus their
average across restoration and class initialization. Sampling still belongs to the pending
parent phase; +42 remains the existing counter owner. Common observations include these fields.

`original_ground_goal_oracle.py` executes unchanged ac75, nested active/return entries and actual
03a9 rotation. Declared caller setup supplies canonical platoon pointers, fine rotation and
input payloads. All 251 actor bytes, unchanged RNG and every unrelated DGROUP byte are checked;
only actual call-stack writes are excluded from the unrelated-state comparison. No instruction
or function return is replaced. Canonical tests load complete orders through the actual original
DOS loaders and preserve all physical payloads and registry orphans at every boundary.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_ground_goal.py \
  --target all --originals --oracle
python3 tools/rewrite/check_style.py
CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all
```

Required corpus coverage retains all 47 files / 960 ground snapshots, the ten currently
supported complete saved worlds and all 37 explicit unsupported-world rejections. Sanitizer
checks use the production fast-math flags and both bulk/canonical probes. Scope and final
results live in WI 0084. Full 0081 heading/callback scheduling, navigation/distance/targets,
living battle, PCM, outcomes and final complete ten-run WASM acceptance remain open.
