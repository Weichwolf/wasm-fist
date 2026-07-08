#!/usr/bin/env python3
"""qmon.py -- minimal QEMU HMP (human monitor) TCP client for the oracle.

The oracle launches QEMU with `-monitor tcp:127.0.0.1:<port>,server,nowait`.
This talks to it line-by-line (no socat/nc needed).

Usage:
    qmon.py <port> "cmd1" ["cmd2" ...]

Handy commands:
    screendump /path/out.ppm            # capture the framebuffer (convert to PNG with ImageMagick)
    pmemsave 0 0x2000000 /path/ram.bin  # dump 32 MB of physical RAM
    memsave  0x10000000 0x10000 f.bin   # dump 64 KB of *virtual* (current-CPU) memory
    xp /16xw 0x10000000                 # examine physical memory
    stop | cont                         # pause / resume the VM
    info registers                      # CPU state
    quit                                # terminate QEMU

Examples:
    tools/oracle/qmon.py 5512 "screendump /tmp/f.ppm"
    tools/oracle/qmon.py 5512 "pmemsave 0 0x2000000 /tmp/ram.bin"
"""
import socket, sys, time

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(1)
    port = int(sys.argv[1]); cmds = sys.argv[2:]
    s = socket.create_connection(("127.0.0.1", port), timeout=15)
    s.settimeout(2.0)
    def drain():
        out = b""
        while True:
            try: d = s.recv(65536)
            except socket.timeout: break
            if not d: break
            out += d
        return out.decode("latin1", "replace")
    print(drain(), end="")                     # banner
    for c in cmds:
        s.sendall((c + "\n").encode())
        # pmemsave of 32 MB needs a moment to flush before the reply/prompt
        time.sleep(1.5 if c.strip().startswith(("pmemsave", "memsave", "dump")) else 0.4)
        print(drain(), end="")
    s.close()

if __name__ == "__main__":
    main()
