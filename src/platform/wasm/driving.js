'use strict';
// Device events, monotonic clock and storage only; C owns keys, pause and simulation.
async function startDrivingPreview() {
    const status = document.getElementById('status');
    const {manifest, files} = await fistLoadPreviewAssets();
    if (!manifest.vehicle) throw new Error('Vehicle assets are required');
    const module = await createFistDrivingPreview({noInitialRun: true});
    module.FS.mkdir('/assets');
    for (const [name, bytes] of files) module.FS.writeFile(`/assets/${name}`, bytes);
    // Explicit inspection installation detail; original default TCB+59h is 11 bits.
    const sceneArguments = [`/assets/${manifest.scenario}`, '/assets', '2048'];
    if (manifest.mission === true) sceneArguments.push('mission');
    if (module.callMain(sceneArguments) !== 0) {
        module._fist_preview_destroy();
        module.PThread.terminateAllThreads();
        throw new Error('Could not load the driving scene');
    }
    let last = Math.floor(performance.now() * 1000);
    let animation;
    let stopped = false;
    const check = result => { if (result !== 0) throw new Error('Driving scene failed'); };
    function advance() {
        const now = Math.floor(performance.now() * 1000);
        let elapsed = now - last;
        last = now;
        while (elapsed > 0xffffffff) {
            check(module._fist_preview_advance(0xffffffff));
            elapsed -= 0xffffffff;
        }
        check(module._fist_preview_advance(elapsed));
    }
    function ascii(event) {
        if (/^Key[A-Z]$/.test(event.code)) return event.code.charCodeAt(3);
        if (/^Digit[1-5]$/.test(event.code)) return event.code.charCodeAt(5);
        if (event.code === 'Tab') return 9;
        return event.code === 'Space' ? 32 : 0;
    }
    function key(event) {
        const value = ascii(event);
        if (!value) return;
        event.preventDefault();
        try {
            advance();
            check(module._fist_preview_key(event.type === 'keydown' ? value : -value));
        } catch (error) { fail(error); }
    }
    function freeze() {
        if (stopped) return;
        try { advance(); check(module._fist_preview_freeze()); } catch (error) { fail(error); }
    }
    function hidden() { if (document.hidden) freeze(); }
    function stop() {
        if (stopped) return;
        stopped = true;
        cancelAnimationFrame(animation);
        window.removeEventListener('keydown', key);
        window.removeEventListener('keyup', key);
        window.removeEventListener('blur', freeze);
        document.removeEventListener('visibilitychange', hidden);
        window.removeEventListener('pagehide', stop);
        module._fist_preview_destroy();
        module.PThread.terminateAllThreads();
    }
    function fail(error) {
        stop();
        status.textContent = String(error);
        console.error(error);
    }
    function frame() {
        if (stopped) return;
        try {
            advance();
            check(module._fist_preview_frame());
            status.textContent = module._fist_preview_value(7) ? 'Paused — press P to continue' : 'Driving';
            animation = requestAnimationFrame(frame);
        } catch (error) { fail(error); }
    }
    window.addEventListener('keydown', key);
    window.addEventListener('keyup', key);
    window.addEventListener('blur', freeze);
    document.addEventListener('visibilitychange', hidden);
    window.addEventListener('pagehide', stop);
    window.fistDriving = {module, stop};
    frame();
}
startDrivingPreview().catch(error => {
    document.getElementById('status').textContent = String(error);
    console.error(error);
});
