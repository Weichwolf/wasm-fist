// Armored Fist WASM (node) pre-js: mirror the process environment into emscripten's ENV so the engine
// harness's getenv() (FIST_TICK_HZ / FIST_RUNMS / FIST_FBDUMP / FIST_KDV / ...) reads the values passed
// on the node command line exactly as the native build reads them from the OS environment. Runs at
// preRun, when ENV is initialized. Native parity: same env knobs -> same deterministic behavior.
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  try {
    if (typeof process !== 'undefined' && process.env) {
      for (var k in process.env) ENV[k] = process.env[k];
    }
  } catch (e) {}
});
