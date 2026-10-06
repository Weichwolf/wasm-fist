'use strict';
const assert = require('node:assert/strict');
const { chromium } = require(process.env.FIST_PLAYWRIGHT_MODULE || 'playwright');

async function terrainResult(page) {
    await page.waitForFunction(() => document.getElementById('status').textContent
        .startsWith('terrain inspection: 640x400 RGBA8 fnv1a='), null, { timeout: 15000 });
    const result = await page.evaluate(async () => {
        const canvas = document.getElementById('preview');
        const rgba = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height).data;
        const ppm = window.fistTerrainFrame;
        const header = new TextEncoder().encode('P6\n640 400\n255\n');
        let equal = ppm.length === header.length + canvas.width * canvas.height * 3;
        for (let index = 0; index < header.length; ++index) equal &&= ppm[index] === header[index];
        const colors = new Set();
        for (let pixel = 0; pixel < canvas.width * canvas.height; ++pixel) {
            for (let channel = 0; channel < 3; ++channel) {
                equal &&= rgba[pixel * 4 + channel] === ppm[header.length + pixel * 3 + channel];
            }
            equal &&= rgba[pixel * 4 + 3] === 255;
            colors.add(`${rgba[pixel * 4]},${rgba[pixel * 4 + 1]},${rgba[pixel * 4 + 2]}`);
        }
        const digest = await crypto.subtle.digest('SHA-256', ppm);
        return {
            isolated: crossOriginIsolated,
            size: [canvas.width, canvas.height],
            completePresentation: equal,
            distinctColors: colors.size,
            ppmSHA256: Array.from(new Uint8Array(digest), value => value.toString(16).padStart(2, '0')).join(''),
            status: document.getElementById('status').textContent
        };
    });
    assert.equal(result.isolated, true, 'Worker isolation headers are missing');
    assert.deepEqual(result.size, [640, 400]);
    assert.equal(result.completePresentation, true, 'Every canvas RGBA pixel must match the completed C frame');
    assert.ok(result.distinctColors > 256, 'A complete textured terrain scene must be visible');
    return result;
}

async function main() {
    const browser = await chromium.launch({
        executablePath: process.env.CHROMIUM || '/usr/bin/chromium',
        args: process.getuid?.() === 0 ? ['--no-sandbox'] : [],
        headless: true
    });
    try {
        const page = await browser.newPage();
        const errors = [];
        page.on('pageerror', error => errors.push(String(error)));
        await page.goto(process.argv[2] || 'http://localhost:8000', { waitUntil: 'networkidle' });
        let result;
        if (process.argv[4] === 'terrain') {
            result = await terrainResult(page);
        } else {
            await page.waitForFunction(() => document.getElementById('status').textContent
                .startsWith('softgl integration: 320x200 RGBA8 fnv1a='), null, { timeout: 15000 });
            result = await page.evaluate(() => {
                const canvas = document.getElementById('preview');
                const context = canvas.getContext('2d');
                const pixel = (x, y) => Array.from(context.getImageData(x, y, 1, 1).data);
                return {
                    isolated: crossOriginIsolated,
                    size: [canvas.width, canvas.height],
                    top: pixel(160, 40),
                    bottomLeft: pixel(65, 165),
                    corner: pixel(0, 0),
                    status: document.getElementById('status').textContent
                };
            });
            assert.equal(result.isolated, true, 'Worker isolation headers are missing');
            assert.deepEqual(result.size, [320, 200]);
            assert.deepEqual(result.corner, [0, 0, 0, 255]);
            assert.ok(result.top[2] > result.top[0] && result.top[2] > result.top[1],
                'The blue vertex must appear near the top');
            assert.ok(result.bottomLeft[0] > result.bottomLeft[1] &&
                result.bottomLeft[0] > result.bottomLeft[2], 'The red vertex must appear bottom left');
        }
        assert.deepEqual(errors, [], 'Browser runtime errors');
        if (process.argv[3]) await page.screenshot({ path: process.argv[3], fullPage: true });
        console.log(JSON.stringify(result));
    } finally {
        await browser.close();
    }
}
main().catch(error => { console.error(error); process.exitCode = 1; });
