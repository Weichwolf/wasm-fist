#!/usr/bin/env python3
# =============================================================================
# sim_voxel6980_framematched.py -- the COLOUR-GATE reproduction over the
# FRAME-MATCHED full 6980 state captured by FIST_R6980CAP (see
# tools/oracle/capture_6980_framematched.sh + docs/oracle_colour_gate.md).
#
# Unlike sim_voxel6980.py (which used the r92 9200-globals, frame-skewed for
# 6980 because 8120 overwrites 90d4/90d8 between 6980 and 9200), this loads
# 6980's OWN world camera + depth ramps + render-time colormap, all snapshotted
# DURING 6980.  Bundle: tools/oracle/samples/voxel6980_framematched_pass08.bin.gz
#   'C69G' + passno + ext[0x9000..0x9200] + 16 globals + ramps(4*256) +
#   proj(0xfa00) + heightmap(1MB) + LIGHT colormap(1MB) + dark C32(1MB) +
#   tile(64K) + 6980-shadow(64K, its exact stores).
#
# GATE A (decisive, data-driven, geometry-independent): the value-set of 6980's
# ACTUAL stores (== the bytes it read at 6b1a) is contained in the LIGHT reduce
# colormap and NOT in the dark C32 -> the render-time colormap is LIGHT.
# =============================================================================
import struct, sys, gzip
ROOT = sys.argv[1] if len(sys.argv) > 1 else '.'
b = gzip.decompress(open(ROOT+'/tools/oracle/samples/voxel6980_framematched_pass08.bin.gz','rb').read())
assert b[:4] == b'C69G', b[:4]
o = 4
passno, = struct.unpack('<I', b[o:o+4]); o += 4
ext = b[o:o+0x200]; o += 0x200
g   = struct.unpack('<16I', b[o:o+64]); o += 64
ramp= struct.unpack('<1024I', b[o:o+4096]); o += 4096
proj= b[o:o+0xfa00]; o += 0xfa00
HM  = b[o:o+0x100000]; o += 0x100000     # render-time heightmap (1024x1024)
RED = b[o:o+0x100000]; o += 0x100000     # render-time LIGHT colormap 6980 reads (the reduce)
CM  = b[o:o+0x100000]; o += 0x100000     # dark C32 the PORT currently has at [0x85bc]+0x100000
tile= b[o:o+0x10000]; o += 0x10000
shadow = b[o:o+0x10000]; o += 0x10000
names = ['85bc','85b8','3909','3918','38ed','38f1','38fd','3901','3905','3a20','90c0','90dc','90e0','90fc','9100','90c4']
G = dict(zip(names, g))
ge = lambda off: struct.unpack('<I', ext[off-0x9000:off-0x9000+4])[0]

print("=== FRAME-MATCHED 6980 state (pass %02d) ===" % passno)
print("world camera: 90d4=%08x 90d8=%08x 90dc=%08x 90e0=%08x 90fc=%08x 9100=%08x"
      % (ge(0x90d4), ge(0x90d8), ge(0x90dc), ge(0x90e0), ge(0x90fc), ge(0x9100)))
print("[0x85bc]=%08x [0x85b8]=%08x [0x3909]=%08x 90c0=%08x  cols[%d..%d) 3a20=%d"
      % (G['85bc'], G['85b8'], G['3909'], G['90c0'], G['3901'], G['3905'], G['3a20']))

def stats(buf, lbl):
    print("  %-30s dist=%3d mean=%6.1f min=%3d max=%3d" % (lbl, len(set(buf)), sum(buf)/len(buf), min(buf), max(buf)))
print("\n--- buffers ---")
stats(HM,  "heightmap [0x85bc]")
stats(CM,  "dark C32  [0x85bc]+0x100000 (PORT)")
stats(RED, "LIGHT reduce [0x85bc]+0x400000")
stats(tile[:0x10000], "settled tile (whole)")

# ---- GATE A: the DECISIVE value-set containment (geometry-independent) ----
sv = set(x for x in shadow if x)
print("\n=== GATE A -- what colormap did 6980 ACTUALLY read? (its stores == its 6b1a reads) ===")
print("6980 render-time stores: n=%d distinct=%d mean=%.1f min=%d max=%d"
      % (sum(1 for x in shadow if x), len(sv), sum(x for x in shadow if x)/max(1,sum(1 for x in shadow if x)),
         min(sv), max(sv)))
for lbl, buf in [("dark C32 [+0x100000]", CM), ("LIGHT reduce [+0x400000]", RED)]:
    bs = set(buf)
    print("  contained in %-26s : %2d/%2d %s" % (lbl, len(sv & bs), len(sv),
          "<-- 6980 CANNOT read this (missing %d values, max=%d)" % (len(sv-bs), max(buf)) if (sv - bs) else "<-- ALL present (max=%d)" % max(buf)))
print("VERDICT gate-A: the render-time colormap 6980 dereferences is %s." %
      ("LIGHT (== the reduce); the port's dark C32 at [0x85bc]+0x100000 is the defect"
       if sv <= set(RED) and not (sv <= set(CM)) else "AMBIGUOUS"))
