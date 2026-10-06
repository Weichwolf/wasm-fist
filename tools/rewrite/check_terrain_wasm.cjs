'use strict';
// Feed isolated input files to the browser-compatible MEMFS preview and require
// a completed C run before publishing its full PPM output to the host.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

async function main() {
    const [program, scenario, directory, output, heading] = process.argv.slice(2);
    const create = require(path.resolve(program));
    let exitCode;
    const module = await create({
        noInitialRun: true,
        print: text => console.log(text),
        onExit: code => { exitCode = code; },
        onAbort: reason => { throw new Error(String(reason)); }
    });
    module.FS.mkdir('/assets');
    for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
        if (entry.isFile()) {
            module.FS.writeFile(`/assets/${entry.name}`, fs.readFileSync(path.join(directory, entry.name)));
        }
    }
    module.FS.writeFile('/scenario.fsg', fs.readFileSync(scenario));
    const args = ['/scenario.fsg', '/assets', '/frame.ppm'];
    if (heading !== undefined) args.push(heading);
    const returned = module.callMain(args);
    assert.equal(exitCode, returned, 'C run must report its completed exit status');
    if (exitCode !== 0) {
        process.exitCode = exitCode;
        return;
    }
    const frame = module.FS.readFile('/frame.ppm');
    assert.ok(frame.length > 0, 'Complete frame is required');
    fs.writeFileSync(output, frame);
}
main().catch(error => { console.error(error); process.exitCode = 1; });
