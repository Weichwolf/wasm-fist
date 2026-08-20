# Armored Fist (1994) — native + WebAssembly

**THE FRAME IS BIT-IDENTICAL.** Given the same input and RNG state, this port produces the same 320×200
palette framebuffer and the same audio stream as NovaLogic's original *Armored Fist* running under
DOSBox/QEMU — every frame, every menu, every mission — and the native and WebAssembly builds are
byte-identical to each other. That is a falsifiable target: two byte streams either match or they do not,
and any deviation is by definition a bug we can find and close.

## What this file is

Vision, architecture, and approach — the shape of the work and the rules it runs by. It is the map, not
the territory: it says how the engine is built and how we know a change is right. It carries no status and
no findings. **The board carries those** (see *The board*): every open capability, every discovery, every
decision lives as one file under `board/`, and the git history carries what was done. Read this file to
orient; read the board to know what is next.

## What this is built on

The method is already proven end-to-end. `~/Git/wasm-dd2/` took *Destruction Derby 2* — a DOS-era
commercial game — all the way to a complete, bit-verified native+WebAssembly port through many patient
test/fix iterations. Armored Fist shares no code with it (DD2 is Win32/DirectDraw; AF is 32-bit DOS
protected-mode) but inherits the method and the discipline verbatim. AF is in several ways the easier
target: no COM/Win32/DirectDraw layer, one linear 320×200×8bpp framebuffer, deterministic under
DOSBox/QEMU. **It has already been done once; it is being done again.**

## Stance

- **Code is the truth.** Every claim is verified against `armoredfist/FIST.RUN` / `FIST.DAT` (static via
  `objdump -m i8086`, or a live DOSBox/QEMU run). No approximations, no band-aids, no stubs.
- **"Geht nicht gibt es nicht."** The whole chain is proven, so nothing here is unreachable — only
  bounded, decomposable work. When something looks hard, it is the next thing to break into a small,
  verifiable step, never a reason to declare a surface out of reach.
- **Land small, verify immediately.** One asm-verified patch that makes one flow bit-identical on both
  targets is a real win. Bank it, verify both targets, move on. Breadth grows one bit-verified surface at
  a time.
- **A finding is a discovery.** A failing test names a bug in the code, not the test. A disproven
  hypothesis is progress — it eliminates a dead end. Correcting course is the method working.

## The target

`FIST.RUN` is a loader plus a Doug-Huffman DOS-extender (mapped to the platform shim). **`FIST.DAT`
(216 KB) is THE ENGINE** and the decompile target: 16-bit segmented, base 0, almost entirely hand-written
assembly (Kyle Freeman), NovaLogic **Voxel-Space** terrain in the Comanche lineage. The port runs it under
a flat memory model:

- one `uint8_t g_mem[]` array is the entire address space;
- **DGROUP** lives at linear `0x1c000` (SS=DS=DGROUP=0x1c00);
- the VGA framebuffer is at `0xA0000`; the extender/overlay image loads at `0x100000`;
- game data lives under `armoredfist/FISTDATA/` (`.KLC` maps, `.MS3` missions, `.FSG` battles, `.M##`
  models, `.FSG/FSE/FSW` sounds & briefings, palette archives, …).

## The chain

Everything starts with `make` — the target chain *is* the documentation (`make help`).

```mermaid
flowchart LR
  image["make image<br/>Ghidra headless"] --> decompile["decompile<br/>re_out/*.c (pristine)"]
  decompile --> assemble["assemble<br/>flat g_mem model"]
  assemble --> patch["patch<br/>patches/NNN-*.diff"]
  patch --> native["native (gcc -m32)"]
  patch --> wasm["wasm (emcc)"]
  native --> verify["verify / verify-wasm<br/>tools/verify.sh"]
  wasm --> verify
```

- **image / decompile** — Ghidra headless (`x86:LE:16 Real Mode`, base 0) produces the pristine decompile,
  with register dataflow threaded via a custom `__allregs` model.
- **assemble** — a mechanical transform of the decompile into the flat `g_mem` model (forward decls,
  `ghidra_compat.h`).
- **patch** — every engine correction is one commented, asm-verified `patches/NNN-slug.diff`, applied
  `-F0 --fuzz=0` onto `re_out/` to produce `build/`. Each patch header carries its rationale; drift fails
  loudly.
- **native / wasm / verify** — both targets build from the same patched `build/`, and `tools/verify.sh`
  runs the bit-verify flow matrix on each.

## Architecture: three trees, one requirement

```mermaid
flowchart TB
  subgraph pristine["re_out/ — pristine, never hand-edited"]
    dc["fist.c / fist_ext.c / fist_mga.c<br/>(Ghidra decompile)"]
  end
  subgraph shim["platform shim — hand-written, freely edited"]
    sh["fist_*.c, tools/native_main.c<br/>VGA · DOS/FILEMGR · input · audio · extender · icall"]
  end
  patches["patches/NNN-*.diff<br/>asm-verified corrections"]
  dc --> build["build/ (make patch)"]
  patches --> build
  shim --> link["native / wasm binary"]
  build --> link
  link --> oracle{{"bit-verify vs original"}}
```

Two things are authored by hand and nothing else: the **patches** (each an asm-verified engine correction)
and the **platform shim** — VGA (linear `0xA0000` + DAC → framebuffer), DOS/FILEMGR (INT 21h), mouse /
keyboard / joystick (INT 33h / INT 9 / port 0x201), SB/GUS + OPL audio, the PIT/INT 8 timer, the
extender/overlay loader, and indirect-call dispatch. The decompile stays pristine; the shim may grow.

## Code is the truth: the oracle

```mermaid
flowchart LR
  orig["armoredfist/FIST.RUN + FIST.DAT"] --> dosbox["DOSBox oracle<br/>fast, instrumentable, reaches missions"]
  orig --> qemu["QEMU oracle<br/>-s -S gdb-stub, icount replay"]
  dosbox --> cap["capture: framebuffer / RAM / ports"]
  qemu --> cap
  cap --> cmp{{"compare to port g_mem / fb"}}
  port["port build"] --> cmp
```

Two oracles verify every claim. **DOSBox** is fast and hackable — instrument it to dump the framebuffer,
guest RAM, or I/O ports at a chosen tick; it reaches missions. **QEMU** gives a gdb-stub and record/replay
for true protected-mode inspection. A rebuilt instrumented DOSBox relocates the engine's DGROUP to a known
guest physical address, so any engine field is directly comparable byte-for-byte to `g_mem`. For the
windshield, `tools/oracle/capture_battle_burst.sh` grabs the original spawn under stock DOSBox and selects
the frame whose dashboard matches the port, giving a provenance-verified framebuffer reference without
needing guest RAM.

## What decides done

Completeness is the DD2 standard — every menu, screen, and dialog; every campaign mission and map; every
setting (detail level, sound devices, input modes, serial link); and the built-in level/mission editor as
a first-class deliverable with a bit-verified round-trip (create → save → reload → simulate,
byte-identical). The gate: a subagent confirms the WASM build error-free and fully functional **ten
consecutive times** on the complete exhaustive test matrix — one failure resets the count to zero. As the
matrix grows to drive every surface named above, re-passing the gate is what turns "the matrix passes"
into "the port is done." `tools/wasm_gate.sh` runs the endurance.

## The board

`board/` is the canonical work state, and this section is its only statement — a convention written twice
is the defect the board removes. Three directories and the path is the state: `board/open/` ·
`board/active/` · `board/closed/`. There is no fourth: *blocked* is a line in a body naming what blocks,
and the task stays open. One task
is one file — an RFC 822 header, a blank line, a markdown body — named `NNNN_description.md`. An item says
what **will be true**: a title is a capability the tree gains, a closed item is a sentence that begins
*this engine can*. The board may be extended and may not be shortened. Code cites a requirement with a
`board:NNNN` marker in a comment; the board never names the code. Findings become items the round they are
found — a discovery that lives only in a report is lost at the next context boundary.

```
ls board/active/                                     # what is in flight
cat board/*/0042_*.md                                # one item, wherever it lives
grep -l '^Type: bug'      board/active/*.md          # by kind
grep -l '^Parent: 0007'   board/*/*.md               # a feature's children
git grep -n 'board:0042'  -- src/ test/              # every site that implements or proves it
git mv board/active/X.md  board/closed/              # the transition IS the diff
```

## Conventions

- **English throughout** the repository — files, headers, commit messages, board items, code comments.
- `re_out/` stays pristine; corrections are `patches/NNN-*.diff`; `make check` must apply every patch.
- **Verify both targets after every change** — native ↔ wasm byte-identity is a hard invariant.
- Original files under `armoredfist/` are present at run time and read-only; `third_party/` and extracted
  images are gitignored.
- Git commits and PRs carry no "Co-Authored-By: Claude" or "Generated with Claude Code" attribution.
- Diagrams are welcome — mermaid, because it renders and ASCII rots at the first edit.
