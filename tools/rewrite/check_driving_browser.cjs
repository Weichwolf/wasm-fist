'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {chromium} = require(process.env.FIST_PLAYWRIGHT_MODULE || 'playwright');

async function state(page) {
    return page.evaluate(() => Array.from({length: 10}, (_, i) => window.fistDriving.module._fist_preview_value(i)));
}
async function frame(page, output) {
    const result = await page.evaluate(async () => {
        const module = window.fistDriving.module;
        const canvas = document.getElementById('preview');
        const rgba = canvas.getContext('2d').getImageData(0, 0, 640, 400).data;
        const start = module._fist_preview_pixels();
        if (!start) throw new Error('Missing shared C frame');
        let complete = true;
        for (let row = 0; row < 400; ++row) {
            for (let offset = 0; offset < 640 * 4; ++offset) {
                complete &&= rgba[row * 640 * 4 + offset] === module.HEAPU8[start + (399 - row) * 640 * 4 + offset];
            }
        }
        const colors = new Set();
        for (let i = 0; i < rgba.length; i += 4) {
            complete &&= rgba[i + 3] === 255;
            colors.add(`${rgba[i]},${rgba[i + 1]},${rgba[i + 2]}`);
        }
        const digest = await crypto.subtle.digest('SHA-256', rgba);
        return {complete, colors: colors.size, isolated: crossOriginIsolated,
            hash: Array.from(new Uint8Array(digest), b => b.toString(16).padStart(2, '0')).join('')};
    });
    assert.equal(result.complete, true, 'Every canvas pixel must equal the current complete C frame');
    assert.equal(result.isolated, true);
    assert.ok(result.colors > 256, 'Actual textured terrain and vehicle output is required');
    if (output) await page.locator('#preview').screenshot({path: output});
    return result;
}
async function main() {
    const [url, directory] = process.argv.slice(2);
    if (directory) fs.mkdirSync(directory, {recursive: true});
    const browser = await chromium.launch({executablePath: process.env.CHROMIUM || '/usr/bin/chromium', headless: true});
    try {
        const page = await browser.newPage();
        const errors = [];
        page.on('pageerror', e => errors.push(String(e)));
        await page.goto(url);
        await page.waitForFunction(() => window.fistDriving, null, {timeout: 20000});
        await page.keyboard.press('p');
        await page.waitForTimeout(100);
        const before = await state(page);
        assert.equal(before[7], 1);
        const first = await frame(page, directory && path.join(directory, 'browser-before.png'));
        await page.keyboard.down('w');
        await page.waitForTimeout(150);
        const held = await state(page);
        assert.equal(held[0], before[0], 'Pause excludes elapsed time even with held controls');
        assert.equal(held[8] & 1, 1);
        await page.keyboard.up('w');
        await page.keyboard.down('p');
        await page.keyboard.down('p');
        assert.equal((await state(page))[7], 0, 'Repeated keydown must not produce a second pause edge');
        await page.keyboard.up('p');
        await page.keyboard.down('w');
        await page.waitForTimeout(800);
        await page.keyboard.down('d');
        await page.keyboard.down('e');
        await page.waitForTimeout(600);
        await page.keyboard.up('w');
        await page.keyboard.up('d');
        await page.keyboard.up('e');
        await page.keyboard.press('p');
        await page.waitForTimeout(100);
        const after = await state(page);
        assert.ok(after[0] > before[0]);
        assert.notDeepEqual(after.slice(1, 3), before.slice(1, 3), 'Held driving inputs must move the actor');
        assert.notEqual(after[3], before[3], 'Hull steering must reach shared state');
        assert.notEqual(after[3], after[4], 'Turret must rotate independently of the hull');
        const last = await frame(page, directory && path.join(directory, 'browser-after.png'));
        assert.notEqual(first.hash, last.hash, 'Movement must reach the displayed complete frame');
        await page.waitForTimeout(200);
        assert.deepEqual((await state(page)).slice(0, 8), after.slice(0, 8), 'Paused state must remain stable');
        assert.equal((await frame(page)).hash, last.hash);
        await page.keyboard.press('p');
        await page.keyboard.down('w');
        await page.evaluate(() => window.dispatchEvent(new Event('blur')));
        const frozen = await state(page);
        assert.equal(frozen[7], 1, 'Focus loss must pause');
        assert.equal(frozen[8], 0, 'Focus loss must release every held control');
        await page.keyboard.up('w');
        await page.waitForTimeout(100);
        assert.deepEqual(await state(page), frozen);
        await page.evaluate(() => window.fistDriving.stop());
        assert.deepEqual(await page.evaluate(() => {
            const m = window.fistDriving.module;
            return [m._fist_preview_advance(1000), m._fist_preview_key(87), m._fist_preview_frame(), m._fist_preview_pixels()];
        }), [-1, -1, -1, 0], 'Shutdown must release scene/frame ownership and reject later work');
        assert.deepEqual(errors, [], 'Browser runtime must be clean');
        // Reach a C startup failure after valid manifest/download verification.
        // Its scene and prestarted render workers must both be released.
        const manifestURL = new URL('assets/manifest.json', url);
        const manifest = await (await fetch(manifestURL)).json();
        const damaged = Buffer.from(await (await fetch(new URL(manifest.scenario, manifestURL))).arrayBuffer());
        damaged[0] ^= 255;
        manifest.files.find(file => file.name === manifest.scenario).sha256 = createHash('sha256').update(damaged).digest('hex');
        const failed = await browser.newPage();
        await failed.route('**/assets/manifest.json', route => route.fulfill({json: manifest}));
        await failed.route(`**/assets/${manifest.scenario}`, route => route.fulfill({body: damaged}));
        await failed.goto(url);
        await failed.waitForFunction(() => document.getElementById('status').textContent.includes('Could not load the driving scene'));
        await failed.waitForTimeout(100);
        assert.equal(await failed.evaluate(() => window.fistDriving === undefined), true, 'Failed loading must not publish a running scene');
        assert.deepEqual(failed.workers(), [], 'Failed C startup must release the prestarted worker pool');
        await failed.close();
        console.log(JSON.stringify({before, after, first, last, focusLoss: true, shutdown: true, startupFailure: true}));
    } finally { await browser.close(); }
}
main().catch(e => { console.error(e); process.exitCode = 1; });
