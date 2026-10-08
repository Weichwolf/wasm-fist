'use strict';
const path = require('node:path');
const assert = require('node:assert/strict');
const expectedHeapBytes = process.argv[3] === undefined ? undefined : Number(process.argv[3]);
if (expectedHeapBytes !== undefined) {
    assert.ok(Number.isSafeInteger(expectedHeapBytes) && expectedHeapBytes > 0,
        'Expected shared heap size must be a positive integer');
}
const create = require(path.resolve(process.argv[2]));
let completed = false;
create({
    print: text => { console.log(text); },
    onExit: code => {
        if (code !== 0) throw new Error(`WASM probe exited ${code}`);
        completed = true;
    },
    onAbort: reason => { throw new Error(String(reason)); }
}).then(module => {
    if (!completed) throw new Error('WASM probe did not finish');
    if (expectedHeapBytes !== undefined) {
        const buffer = module.HEAPU8.buffer;
        assert.ok(buffer instanceof SharedArrayBuffer, 'Renderer heap must be shared');
        assert.equal(buffer.byteLength, expectedHeapBytes, 'Renderer shared capacity changed');
        assert.equal(module.HEAP32.buffer, buffer, 'Renderer heap views must share one buffer');
    }
}).catch(error => { console.error(error); process.exitCode = 1; });
