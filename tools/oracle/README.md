# Armored Fist — Runtime Oracle (QEMU + gdb stub)

A **debuggable runtime oracle**: the ORIGINAL Armored Fist running under
`qemu-system-i386` with the QEMU gdb stub and HMP monitor, so we can observe the
game's exact memory and execution. This is the instruction-level microscope that
complements the DOSBox fast-oracle (`tools/refcapture.sh`) and the foundation for
Stage-2 bit-exact reference capture.

Everything here is reproducible from the game files in `armoredfist/` (never
modified) plus a downloaded FreeDOS 1.3 boot floppy and two tiny DOS drivers.
Large artifacts (FreeDOS ISO, disk images, RAM dumps) live in the scratch dir
(`$OUT`, default `/tmp/wasm-fist-oracle`) and are gitignored.

---

## TL;DR

```bash
export OUT=/tmp/wasm-fist-oracle              # scratch for images (gitignored)

# 1. build the boot floppy (A:) + game HDD (C:)   [needs internet once]
tools/oracle/build_disk.sh himem              # or: jemmex | dpmi

# 2a. run headless, screenshot the framebuffer
tools/oracle/run_oracle.sh $OUT/boot_himem.img $OUT/game.img run &
tools/oracle/qmon.py 5512 'screendump "/tmp/f.ppm"' && convert /tmp/f.ppm /tmp/f.png

# 2b. dump the engine's DGROUP service vectors (incl. DGROUP:0x12)
tools/oracle/capture_dgroup.py 5512 $OUT/engram.bin

# 3. instruction-level debugging with the gdb stub
tools/oracle/run_oracle.sh $OUT/boot_himem.img $OUT/game.img gdb &
gdb -q -x tools/oracle/dump.gdb               # connects to :1234
```

---

## What boots, and how far

| Environment | Result |
|---|---|
| **DOSBox** (`tools/refcapture.sh`) | reaches the **main menu** (see `ref/main_menu.png`) |
| **QEMU + FreeDOS + HIMEMX/JEMMEX/CWSDPMI + CTMOUSE** | extender runs, loads FIST.DAT, prints the `Armored Fist / (C) Copyright 1994 by NovaLogic` banner, then the engine hits an **early software error reported as "Interrupt divide by zero"** and exits. **Does not yet reach the menu.** |

The QEMU crash is **environmental** (DOSBox runs the same bytes fine). It was
**not** fixed by: DPMI host (CWSDPMI `-p`), VCPI/EMS host (JEMMEX), CPU model
(`-cpu 486`), a loaded INT-33h mouse driver (CuteMouse, PS/2 detected), or CPU
throttling (`-icount shift=6/9/10`). It is **not a real CPU `#DE`** — `qemu -d int`
logs zero `v=00` exceptions; "Interrupt divide by zero" is the extender's
error-report *string*, printed on a software-detected fault inside the engine.
Root-causing it further is a reversing task the oracle now enables (see below).

Boot chain (verified live under gdb):
```
LOADGAME.EXE (real mode)
  -> FIST.RUN stub -> Doug-Huffman extender enters 32-bit PM, demand-pages its
     decrypted CRT/runtime image to LINEAR base 0x10000000
  -> CRT entry 0x10000d9d: alloc, install INT 0Dh handler, get PSP, build the
     "AF-FD:1U10" tracking tag, then DOS-EXEC the engine:
        int 0x21, AX=4B00, filename "FIST.DAT"   @ 0x10000e80
  -> FIST.DAT (16-bit engine) loads low, prints banner, errors, exits (int21/4C)
  -> CRT prints "Tracking Number: AF-FD:1U10" @ 0x10000da2, exits @ 0x10000b6e
```

---

## Runtime memory map (the numbers you need)

| Thing | Value |
|---|---|
| Extender/CRT (decrypted FIST.RUN app) | **linear base `0x10000000`** (CS sel 0x2b, DS/SS 0x33, paged) |
| CRT entry (post-decrypt) | `0x10000d9d` |
| CRT EXECs FIST.DAT | `int 0x21` (AX=4B00) at `0x10000e80`, filename at `[0x10000c2c]` |
| **Engine (FIST.DAT) load base** | **`0x14190`** (segment **`0x1419`**) — *varies ±0x100 segs per run*; always re-derive from strings |
| **Engine DGROUP** | **`base + 0x1c000` = `0x30190`** (runtime segment **`0x3019`**) |
| Engine service-code segment | `load_seg + 0xF69` (e.g. `0x2382`), base-0 offset `0xF690` |

The engine load base moves a little between runs (the extender's DOS picks the
child load segment), so **always re-derive it from the anchor strings** the way
`capture_dgroup.py` does (`MAINMENU`/`Armored`/`AF-FD` offsets in
`re_out/fist_dat_image.bin`). `pmemsave` gives **physical** RAM; for the engine's
low-memory (identity-mapped) region physical == linear.

---

## DGROUP service vectors (deliverable #3)

At runtime the engine's DGROUP holds a table of **far pointers** (offset,segment;
4-byte stride from `DGROUP:0x0e`) into the engine's own service-code segment
(`load_seg + 0xF69`). These are **null/unrelocated in the static image**
(`re_out/fist_dat_image.bin`) and get filled in at load — exactly the vectors the
engine reaches via `call far [DGROUP:0xNN]`.

Captured live (engine base 0x14190, DGROUP 0x30190):

```
DGROUP:0x0a = 2382:015f     DGROUP:0x1e = 2382:01ff     DGROUP:0x2e = 2382:02a2
DGROUP:0x0e = 2382:018d     DGROUP:0x22 = 2382:0224     DGROUP:0x32 = 2382:02d0
DGROUP:0x12 = 2382:01b2 <== DGROUP:0x26 = 2382:0242     DGROUP:0x36 = 2382:02fb
DGROUP:0x16 = 2382:01d0     DGROUP:0x2a = 2382:0279
DGROUP:0x1a = 2382:01d9
```

**`DGROUP:0x12 = 2382:01b2` (linear `0x239d2`) = engine routine at base-0 offset
`0xf842`.** Its low word `0x39d2` is exactly the deterministic value seen in every
crash stack dump, confirming the engine faults on/through this `call far
[DGROUP:0x12]`. The target is **not null at runtime** — the loader installed it.
The routine (byte-identical in the live dump and the static image at 0xf842) is a
guarded word-copy / relocation service:

```asm
0f842: or bx,bx ; je ...      ; bail if segment param BX==0
       pushf ; cli ; push ds
       push ss ; pop es        ; es = ss
       mov ds,bx               ; ds = caller-supplied segment
   .l: lods ax ; ...           ; copy words [ds:si]->[es:di] until a 0 terminator
       pop ds ; popf ; lret     ; far return
```

So the DGROUP table is the engine's **internal service dispatch** (11 entries,
0x0a..0x36), relocated into segment `0x2382` at load — not an external extender
ABI. The crash is the engine invoking one of these services in a state that the
QEMU environment produces but DOSBox does not; single-stepping the
`call far [DGROUP:0x12]` site is the next reversing step, and the oracle can do it.

---

## The gdb-stub + memory-dump workflow (deliverable #2)

Launch frozen with the stub, then drive gdb:

```bash
tools/oracle/run_oracle.sh $OUT/boot_himem.img $OUT/game.img gdb &   # qemu -s -S
gdb -q -ex 'target remote :1234'
```

Proven commands (gdb addresses are LINEAR = CS.base + EIP):

```gdb
hbreak *0x10000e80            # HARDWARE bp at the FIST.DAT EXEC (works pre-paging)
continue
x/s 0x10000c2c               # -> "FIST.DAT"
info registers eax ebx ecx edx esi edi eip
x/40i 0x10000b00             # disassemble loader code
find /1 0x1000, 0x2000000, 0xb8,0x00,0x1c,0x8e,0xd8   # scan for engine entry sig
dump binary memory out.bin 0x30190 0x30290            # DUMP a linear range
```

Or via the QEMU HMP monitor (`tools/oracle/qmon.py <port> "cmd"`):

```
screendump "/tmp/frame.ppm"                 # framebuffer  (convert -> PNG)
pmemsave 0 0x2000000 "ram.bin"              # 32 MB physical RAM  (MUST quote the path!)
memsave  0x10000000 0x10000 "loader.bin"    # 64 KB virtual (current CPU context)
info registers
stop | cont | quit
```

> **HMP gotcha:** the monitor parses a `pmemsave`/`memsave` filename as an
> *expression*, so a bare `/tmp/x` is read as division (`invalid char 't'`).
> **Always quote the filename** (`"…"`) — `qmon.py`/`capture_dgroup.py` do this.

---

## Determinism / Stage-2 capture

- `-icount shift=N` makes execution deterministic (virtual clock tied to
  instruction count). `mode slow` uses `shift=9`; combine with
  `-icount shift=N,rr=record,rrfile=…` for full record/replay to re-run a capture
  cycle-for-cycle.
- For a bit-exact framebuffer at a known point: breakpoint the frame/present site,
  then `pmemsave 0xa0000 0x10000 "fb.bin"` (Mode-13h linear FB) plus the DAC
  palette; or `screendump` for a quick PNG. This mirrors DD2's reference capture.
- The engine load base moves per run — re-derive it from strings every time
  (`capture_dgroup.py`) before dereferencing engine addresses.

---

## Files

| File | Purpose |
|---|---|
| `build_disk.sh` | build `game.img` (FAT16 HDD) + `boot_<mm>.img` (FreeDOS floppy) with mtools; fetches FreeDOS/CTMOUSE/CWSDPMI (no root — extracts mtools from .deb if needed) |
| `run_oracle.sh` | launch QEMU headless: `run` / `gdb` (`-s -S`) / `slow` (`-icount`) |
| `qmon.py` | HMP monitor TCP client (screendump / pmemsave / stop / info) |
| `dump.gdb` | gdb driver: connects, `atexec`/`findsig`/`dumplin` helpers, wired constants |
| `capture_dgroup.py` | dump RAM, locate the engine by strings, print DGROUP service vectors + DGROUP:0x12 |

## Requirements

`qemu-system-i386` (10.x), `gdb`, `mtools`+`mkfs.fat` (auto-extracted from .deb if
absent), `unzip`, `curl`, ImageMagick (`convert`) for PPM→PNG, and internet on
first `build_disk.sh` run (FreeDOS 1.3 LiveCD ≈ 390 MB, CTMOUSE, CWSDPMI).
