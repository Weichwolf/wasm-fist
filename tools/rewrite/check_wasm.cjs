'use strict';
const path = require('node:path');
const create = require(path.resolve(process.argv[2]));
let completed = false;
create({
    print: text => { console.log(text); },
    onExit: code => {
        if (code !== 0) throw new Error(`WASM probe exited ${code}`);
        completed = true;
    },
    onAbort: reason => { throw new Error(String(reason)); }
}).then(() => {
    if (!completed) throw new Error('WASM probe did not finish');
}).catch(error => { console.error(error); process.exitCode = 1; });
