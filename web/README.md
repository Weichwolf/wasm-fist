# Armored Fist — browser build

Runs the WebAssembly engine interactively in a browser (the `make wasm` build is headless/node, for
frame verification only).

## Build + run

```sh
bash tools/build_web.sh          # -> web/fist.js + fist.wasm + fist.data (game data preloaded, ~10 MB)
python3 tools/serve_web.py 8100  # serves web/ with the COOP/COEP headers SharedArrayBuffer needs
# open http://localhost:8100/  in a browser
```

Click the menu to navigate (SELECT PLAYER / CAMPAIGNS / BATTLES / REVIEW / SETTINGS / …).
BATTLES → pick a battle → OK → ACCEPT loads a mission.

## How it works

- The engine's main loop never returns, so it runs **blocking in a Web Worker** (`worker.js`); ASYNCIFY
  is not used because it inflates a giant decompiled function past the browser's wasm local-count limit.
- Each ~frame the pump (`native_main.c` `fist_web_post_frame`, web-mode only) posts a copy of the
  320×200×8bpp framebuffer + 6-bit palette to the main thread, which renders it to the canvas.
- Live input travels the other way through a **SharedArrayBuffer** (`index.html` writes mouse/keyboard;
  the pump polls it and drives the engine's INT-33h mouse handler / BIOS key path). One queued mouse
  event is delivered per frame so a press and its release land on different frames (a real click); mouse
  X uses the mode-13h virtual 0..639 space. SharedArrayBuffer requires cross-origin isolation — hence the
  COOP/COEP headers in `serve_web.py`.

All of this is `__EMSCRIPTEN__` / `g_web_mode`-gated, so the native + node-wasm verification builds
(and their native↔wasm bit-identity invariant) are unaffected.
