# wasm-fist — Armored Fist (1994) → native + WebAssembly

A faithful, **bit-identical** port of NovaLogic's 1994 *Armored Fist* (`FIST.DAT` — a 16-bit segmented
DOS protected-mode engine with **Voxel-Space** terrain, hand-written 386 assembly by Kyle Freeman) to
reproducible C compiling to native and WebAssembly. Given the same mission, RNG seed, and input, the port
produces the same 320×200 palette framebuffer and audio stream as the original running under DOSBox/QEMU —
every frame, every menu, every mission — and the native and WebAssembly builds are byte-identical to each
other. That is a falsifiable target: two byte streams either match or they do not.

The engine C is **mechanically derived from the binary via Ghidra** (`make image` → decompile → assemble
into a flat `g_mem[]` model → patch → native/wasm). `re_out/` stays pristine; every engine correction is a
single commented, asm-verified `patches/NNN-*.diff`; only the platform **shim** is hand-written — VGA
(linear `0xA0000` + DAC → framebuffer), DOS/FILEMGR (INT 21h), mouse/keyboard/joystick, SB/GUS + OPL audio,
the PIT/INT-8 cooperative timer, and the Doug-Huffman DOS-extender.

The method is inherited verbatim from [`wasm-dd2`](../wasm-dd2) (Destruction Derby 2), which took a DOS-era
commercial game all the way to a complete, bit-verified native+WebAssembly port.

```
FIST.DAT → tools/decompile.sh → make patch → make native / make wasm → tools/verify.sh
```

- **CLAUDE.md** — vision, architecture, approach, and conventions (start here; `make help` is the map).
- **board/** — the live work state, one file per capability: `board/open` · `board/active` · `board/closed`.
- **patches/** — every asm-verified engine correction, applied `-F0 --fuzz=0` onto `re_out/`.

Original game files live under `armoredfist/` at run time (read-only); `third_party/` and extracted images
are gitignored.
