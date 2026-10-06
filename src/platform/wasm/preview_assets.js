// Platform storage: fetch and verify isolated preview inputs before giving them to C.
async function fistLoadPreviewAssets() {
    const manifestURL = new URL('assets/manifest.json', location.href);
    const response = await fetch(manifestURL);
    if (!response.ok) throw new Error(`Missing preview manifest (${response.status})`);
    const manifest = await response.json();
    const files = await Promise.all(manifest.files.map(async file => {
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
    return {manifest, files};
}
