'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { chromium } = require(process.env.FIST_PLAYWRIGHT_MODULE || 'playwright');

async function main() {
    const [url, directory] = process.argv.slice(2);
    assert.ok(directory, 'A directory for complete visual evidence is required');
    fs.mkdirSync(directory, { recursive: true });
    const browser = await chromium.launch({
        executablePath: process.env.CHROMIUM || '/usr/bin/chromium',
        args: process.getuid?.() === 0 ? ['--no-sandbox'] : [], headless: true
    });
    const results = [];
    try {
        for (const mode of ['0', '4', 'contracts', 'cycles', 'failure-all', 'failure-partial']) {
            const page = await browser.newPage();
            try {
                const errors = [];
                page.on('pageerror', error => errors.push(String(error)));
                await page.goto(`${url}/render-profile.html?mode=${mode}`);
                await page.waitForFunction(() => window.fistProfileResult || window.fistProfileFailure,
                    null, { timeout: 20000 });
                const result = await page.evaluate(() => {
                    if (window.fistProfileFailure) throw new Error(window.fistProfileFailure);
                    const result = window.fistProfileResult;
                    result.isolated = crossOriginIsolated;
                    if (result.frame) {
                        const canvas = document.getElementById('preview');
                        result.size = [canvas.width, canvas.height];
                        const rgba = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height).data;
                        result.presentationEqual = rgba.length === result.frame.length;
                        const stride = canvas.width * 4;
                        for (let row = 0; row < canvas.height; ++row) {
                            for (let channel = 0; channel < stride; ++channel) {
                                result.presentationEqual &&= rgba[row * stride + channel] ===
                                    result.frame[(canvas.height - row - 1) * stride + channel];
                            }
                        }
                    }
                    return result;
                });
                assert.deepEqual(errors, [], 'No browser runtime errors');
                assert.equal(result.isolated, true);
                assert.equal(result.shared, true);
                assert.equal(result.heapBytes, 2147483648);
                assert.equal(result.exitCode, 0);
                assert.equal(result.returned, 0);
                if (mode === '0' || mode === '4') {
                    assert.deepEqual(result.size, [640, 360]);
                    assert.equal(result.presentationEqual, true, 'Every presented RGBA byte must match C');
                    assert.deepEqual(result.output, {
                        width: 640, height: 360, sample_buffers: mode === '4' ? 1 : 0,
                        samples: Number(mode), multisample_enabled: 1, configured_helpers: 3,
                        started_helpers: 3, total_threads: 4
                    });
                    assert.equal(result.frame.length, 640 * 360 * 4);
                    fs.writeFileSync(path.join(directory, `browser-${mode}.rgba`), Buffer.from(result.frame));
                    await page.locator('#preview').screenshot({path: path.join(directory, `browser-${mode}.png`)});
                    delete result.frame;
                } else if (mode === 'contracts') {
                    assert.equal(result.output, null);
                } else if (mode === 'cycles') {
                    assert.deepEqual(result.output, {cycles: 12});
                } else {
                    assert.ok(result.output.failed_starts > 0);
                    assert.equal(result.output.recovered_helpers, 3);
                    if (mode === 'failure-all') assert.equal(result.output.partial_starts, 0);
                    else assert.ok(result.output.partial_starts > 0);
                }
                results.push(result);
            } finally {
                await page.close();
            }
        }
    } finally {
        await browser.close();
    }
    fs.writeFileSync(path.join(directory, 'browser-results.json'), JSON.stringify(results, null, 2) + '\n');
    console.log(JSON.stringify(results));
}
main().catch(error => { console.error(error); process.exitCode = 1; });
