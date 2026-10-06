'use strict';
const assert = require('node:assert/strict');
const { chromium } = require(process.env.FIST_PLAYWRIGHT_MODULE || 'playwright');

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
        await page.waitForFunction(() => document.getElementById('status').textContent
            .startsWith('softgl integration: 320x200 RGBA8 fnv1a='), null, { timeout: 15000 });
        const result = await page.evaluate(() => {
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
        assert.deepEqual(errors, [], 'Browser runtime errors');
        if (process.argv[3]) await page.screenshot({ path: process.argv[3], fullPage: true });
        console.log(JSON.stringify(result));
    } finally {
        await browser.close();
    }
}
main().catch(error => { console.error(error); process.exitCode = 1; });
