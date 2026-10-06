'use strict';
// Platform I/O and presentation only. Shared C decodes assets and renders.
async function startTerrainPreview() {
    const status = document.getElementById('status');
    const manifestURL = new URL('assets/manifest.json', location.href);
    const response = await fetch(manifestURL);
    if (!response.ok) throw new Error(`Missing preview manifest (${response.status})`);
    const manifest = await response.json();
    const inputs = await Promise.all(manifest.files.map(async file => {
        if (!/^[A-Z0-9_.-]+$/.test(file.name)) throw new Error('Invalid preview filename');
        const response = await fetch(new URL(file.name, manifestURL));
        if (!response.ok) throw new Error(`Missing ${file.name} (${response.status})`);
        const bytes = new Uint8Array(await response.arrayBuffer());
        if (bytes.length !== file.size) throw new Error(`Incomplete ${file.name}`);
        const digest = await crypto.subtle.digest('SHA-256', bytes);
        const hash = Array.from(new Uint8Array(digest), byte => byte.toString(16).padStart(2, '0')).join('');
        if (hash !== file.sha256) throw new Error(`Changed ${file.name}`);
        return [file.name, bytes];
    }));
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
