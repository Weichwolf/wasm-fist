#!/usr/bin/env python3
"""Run the real-browser integration gate, including temporary server lifecycle."""
import argparse
import os
import pathlib
import subprocess
import time
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-rewrite/wasm"))
    parser.add_argument("--browser-tools", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-browser-tools"))
    parser.add_argument("--port", type=int, default=8127)
    parser.add_argument("--screenshot", type=pathlib.Path)
    args = parser.parse_args()
    module = args.browser_tools / "node_modules/playwright"
    if not module.is_dir():
        parser.error("Install the pinned browser tooling first; see README.md")
    env = dict(os.environ, FIST_PLAYWRIGHT_MODULE=str(module.resolve()))
    server = subprocess.Popen(["python3", str(ROOT / "tools/rewrite/serve.py"),
                               "--directory", str(args.build_dir), "--port", str(args.port)],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    url = f"http://localhost:{args.port}"
    try:
        for _ in range(50):
            if server.poll() is not None:
                parser.error("Preview server failed to start")
            try:
                with urllib.request.urlopen(url, timeout=1):
                    break
            except OSError:
                time.sleep(0.1)
        else:
            parser.error("Preview server did not become ready")
        command = ["node", str(ROOT / "tools/rewrite/check_browser.cjs"), url]
        if args.screenshot:
            command.append(str(args.screenshot))
        subprocess.run(command, check=True, timeout=30, env=env)
    finally:
        server.terminate()
        server.wait(timeout=5)


if __name__ == "__main__":
    main()
