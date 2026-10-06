'use strict';
// Platform I/O and presentation only. Shared C decodes assets and renders.
async function startTerrainPreview() {
    const status = document.getElementById('status');
    const {manifest, files: inputs} = await fistLoadPreviewAssets();
    let exitCode;
    let completion;
    const module = await createFistTerrainPreview({
        noInitialRun: true,
        print: text => { completion = text; },
        onExit: code => { exitCode = code; },
        onAbort: reason => { throw new Error(String(reason)); }
    });
    module.FS.mkdir('/assets');
    for (const [name, bytes] of inputs) module.FS.writeFile(`/assets/${name}`, bytes);
    const args = [`/assets/${manifest.scenario}`, '/assets', '/frame.ppm'];
    if (manifest.vehicle) {
        args.push(manifest.heading === undefined ? 'default' : String(manifest.heading), 'vehicle');
    } else if (manifest.heading !== undefined) {
        args.push(String(manifest.heading));
    }
    const returned = module.callMain(args);
    if (exitCode !== 0 || returned !== 0 || !completion?.startsWith('terrain inspection:')) {
        throw new Error(`Terrain preview did not complete (${exitCode})`);
    }
    // Retain the emitted C frame for the presentation gate; gameplay state stays in C.
    window.fistTerrainFrame = module.FS.readFile('/frame.ppm');
    status.textContent = completion;
}
startTerrainPreview().catch(error => {
    document.getElementById('status').textContent = String(error);
    console.error(error);
});
