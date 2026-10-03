// Observe the live linear heap at exit without changing the supplied JS/WASM bytes.
const fs = require('fs');
const path = require('path');
const program = path.resolve(process.argv[2]);
process.argv = [process.argv[0], program, ...process.argv.slice(3)];
const mod = { exports: {} };
const options = {
  onExit(code) {
    if (!options.HEAPU8 || code !== 0) throw Error('missing live WASM heap / failed producer');
    fs.writeFileSync(process.env.FIST_LINEAR_HEAP, Buffer.from(options.HEAPU8));
  }
};
new Function('Module', '__dirname', '__filename', 'require', 'process', 'module', 'exports',
             fs.readFileSync(program, 'utf8'))(options, path.dirname(program), program,
                                            require, process, mod, mod.exports);
