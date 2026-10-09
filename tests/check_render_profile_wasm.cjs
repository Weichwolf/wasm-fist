'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

async function main() {
    const [program, mode, output, destination = '/frame.rgba'] = process.argv.slice(2);
    const create = require(path.resolve(program));
    let exitCode;
    const module = await create({
        noInitialRun: true,
        print: text => console.log(text),
        onExit: code => { exitCode = code; },
        onAbort: reason => { throw new Error(String(reason)); }
    });
    assert.ok(module.HEAPU8.buffer instanceof SharedArrayBuffer);
    assert.equal(module.HEAPU8.buffer.byteLength, 2147483648);
    const args = [mode];
    if (output !== undefined) args.push(destination);
    const returned = module.callMain(args);
    assert.equal(exitCode, returned, 'The C run must complete with an observed exit status');
    if (exitCode !== 0) {
        process.exitCode = exitCode;
        return;
    }
    if (output !== undefined) {
        const frame = module.FS.readFile(destination);
        assert.equal(frame.length, 640 * 360 * 4, 'A complete RGBA8 frame is required');
        fs.writeFileSync(output, frame);
    }
}
main().catch(error => { console.error(error); process.exitCode = 1; });
