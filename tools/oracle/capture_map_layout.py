#!/usr/bin/env python3
# =============================================================================
# capture_map_layout.py -- DECISIVE oracle: the ORIGINAL's voxel map-buffer
# LAYOUT (heightmap 85bc, colormap 85b8, the tile 3918), measured from the
# instrumented-DOSBox RAM dump of a live AZER1 mission.
#
# Resolves the step-1 questions:
#   * DAT_0000_85bc value + allocation size
#   * DAT_0000_85b8 - DAT_0000_85bc  (the plane separation)
#   * whether 85bc+0x100000 holds the COLORMAP (what FUN_0000_6980 reads)
#   * the voxel map dimension (1024^2 vs 2048^2)
#
# KEY to reading the dump: the Doug-Huffman extender's FLAT 32-bit linear space
# maps  flat_linear == guest_physical - 0x131000  in this DOSBox capture (proven
# by DAT_0000_3918's value 0x00044200 reading back byte-exact at 0x131000+0x3918,
# and the 256x256 tile at guest-phys 0x131000+0x44200 being a 253-distinct
# terrain texture). So a flat pointer P (e.g. 85bc=0x074e60) points at guest-phys
# 0x131000 + P.  This is the CR3/paging resolution the raw pmemsave lacked.
#
# Input : scratch/oracle/terrain.ram.bin  (from tools/oracle/trace_terrain.sh)
# Output: measurements to stdout.
# =============================================================================
import struct, sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RAM  = os.path.join(ROOT, "scratch/oracle/terrain.ram.bin")
EXT  = 0x131000                      # extender image base in guest-physical RAM
f = open(RAM, "rb").read()

def rd32(flat): return struct.unpack('<I', f[EXT+flat:EXT+flat+4])[0]
def rd16(flat): return struct.unpack('<H', f[EXT+flat:EXT+flat+2])[0]
def rd8 (flat): return f[EXT+flat]

def region(flat_ptr, n):
    """Read n bytes of the buffer that flat pointer 'flat_ptr' points at."""
    phys = EXT + flat_ptr
    if phys+n > len(f): return None
    b = f[phys:phys+n]
    h = [0]*256
    for x in b: h[x] += 1
    distinct = sum(1 for c in h if c)
    top = sorted(range(256), key=lambda i:-h[i])[:6]
    return distinct, [(i, h[i]) for i in top if h[i]]

def main():
    print("=== ORACLE: voxel map-buffer layout (live AZER1, detail=%d) ===" % rd8(0x8490))
    b85bc = rd32(0x85bc); b85b8 = rd32(0x85b8); b3918 = rd32(0x3918)
    dim   = rd32(0x8494); size = rd32(0x8498)
    print("DAT_0000_85bc (heightmap base) = 0x%08x" % b85bc)
    print("DAT_0000_85b8 (colormap  base) = 0x%08x" % b85b8)
    print("DAT_0000_3918 (256x256 tile  ) = 0x%08x" % b3918)
    print("DAT_0000_8494 (map dimension ) = 0x%x  (%d)  -> %dx%d" % (dim, dim, dim, dim))
    print("DAT_0000_8498 (plane size    ) = 0x%x  (%d MB)" % (size, size>>20))
    print()
    print(">>> 85b8 - 85bc = 0x%x  (%d MB)" % (b85b8-b85bc, (b85b8-b85bc)>>20))
    print(">>> FUN_0000_6980 reads color at 85bc+0x100000; delta above tells whether that hits the colormap")
    print()
    print("--- plane contents (distinct / dominant idx) ---")
    print("heightmap  85bc         [+1MB]:", region(b85bc, 0x100000))
    print("6980 color 85bc+0x100000[+1MB]:", region(b85bc+0x100000, 0x100000))
    print("colormap   85b8         [+4MB]:", region(b85b8, size))
    print("tile       3918         [64KB]:", region(b3918, 0x10000))
    print()
    print("VERDICT: colormap-at-85bc+0x100000 =",
          "YES" if abs((b85b8-b85bc)-0x100000) < 0x1000 else "NO (real colormap is at 85bc+0x%x)" % (b85b8-b85bc))

if __name__ == "__main__":
    if not os.path.exists(RAM):
        sys.exit("missing %s -- run tools/oracle/trace_terrain.sh first" % RAM)
    main()
