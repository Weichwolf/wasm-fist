#!/usr/bin/env python3
# Static server for web/ with the cross-origin-isolation headers that SharedArrayBuffer (the input
# channel) requires.  Usage: python3 tools/serve_web.py [port]   then open http://localhost:PORT/
import http.server, socketserver, sys, os
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8099
ROOT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'web')
class H(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *a, **k): super().__init__(*a, directory=ROOT, **k)
    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        # COEP require-corp blocks any subresource (fist.data / fist.wasm) that lacks CORP -> without this
        # the MEMFS .data preload intermittently stalls ("still waiting on run dependencies").
        self.send_header('Cross-Origin-Resource-Policy', 'same-origin')
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()
    def log_message(self, *a): pass
socketserver.TCPServer.allow_reuse_address = True
print(f'serving {ROOT} on http://localhost:{PORT}/  (COOP/COEP for SharedArrayBuffer)')
socketserver.TCPServer(('', PORT), H).serve_forever()
