#!/usr/bin/env python3
"""Serve external WASM builds with the isolation headers required by softgl workers."""
import argparse
import functools
import http.server
import pathlib


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--directory", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-rewrite/wasm"))
    parser.add_argument("--port", type=int, default=8000)
    args = parser.parse_args()
    if not (args.directory / "index.html").is_file():
        parser.error("Build WASM first; index.html is missing")
    handler = functools.partial(Handler, directory=str(args.directory.resolve()))
    with http.server.ThreadingHTTPServer(("127.0.0.1", args.port), handler) as server:
        print(f"Serving http://localhost:{args.port}", flush=True)
        server.serve_forever()


if __name__ == "__main__":
    main()
