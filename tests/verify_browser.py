#!/usr/bin/env python3
"""Run the real-browser integration gate, including temporary server lifecycle."""
import argparse
import os
import pathlib
import subprocess
import time
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-rewrite/wasm"))
    parser.add_argument("--browser-tools", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-browser-tools"))
    parser.add_argument("--port", type=int, default=8127)
    parser.add_argument("--screenshot", type=pathlib.Path)
    parser.add_argument("--terrain", action="store_true", help="Check prepared terrain.html instead of the triangle")
    parser.add_argument("--driving", action="store_true", help="Check timed input and complete controlled scene presentation")
    parser.add_argument("--output-dir", type=pathlib.Path, help="Driving frame captures under /tmp")
    args = parser.parse_args()
    if args.terrain and args.driving:
        parser.error("Choose one browser scene")
    if args.output_dir and not args.output_dir.resolve().is_relative_to(pathlib.Path("/tmp")):
        parser.error("Driving captures belong under /tmp")
    module = args.browser_tools / "node_modules/playwright"
    if not module.is_dir():
        parser.error("Install the pinned browser tooling first; see docs/development.md")
    env = dict(os.environ, FIST_PLAYWRIGHT_MODULE=str(module.resolve()))
    server = subprocess.Popen(["python3", str(ROOT / "tools/serve.py"),
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
        command = ["node", str(ROOT / "tests/check_browser.cjs"),
                   url + ("/terrain.html" if args.terrain else "/"),
                   str(args.screenshot) if args.screenshot else "",
                   "terrain" if args.terrain else "triangle"]
        if args.driving:
            command = ["node", str(ROOT / "tests/check_driving_browser.cjs"),
                       url + "/driving.html", str(args.output_dir) if args.output_dir else ""]
        subprocess.run(command, check=True, timeout=30, env=env)
    finally:
        server.terminate()
        server.wait(timeout=5)


if __name__ == "__main__":
    main()
