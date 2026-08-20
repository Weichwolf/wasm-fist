#!/usr/bin/env python3
# =============================================================================
# sim_bc9c_diag.py -- reference simulation of the extender's bc9c terrain blend
# LUT + ac70 nearest-palette match, used to LOCALIZE the windshield voxel defect.
#
# Pipeline (verified against the extender asm, objdump re_out/fist_image.bin):
#   9f70: 9f10 luma-sorts [5598][80..255] ascending by (r+2g+b), copies [5598]
#         -> [5260], a033 builds [4f60]=[5260]>>1, then calls bc9c.
#   bc9c: M[ch][cl] = ac70( per-channel rounded avg of [5598][ch],[5598][cl] ).
#   ac70 (patch 238, SELF-MODIFYING CODE in the original): cost over palette
#         [4f60][80..255] = a060[(pal.r - target_r)&0xff] + a460[...g] + a860[...b],
#         target = avg>>1, a060[k]=(31k)^2 / a460=(43k)^2 / a860=(26k)^2.
#
# DECISIVE RESULT (this run's inputs = AZER1 map-load dump /tmp/pal.bin):
#   bc9c fed the SORTED palette ([5260]) reproduces the ORACLE diagonal (80..86),
#   NOT the port's bc9c diagonal (102..129).  => the port's bc9c reads a WRONG
#   (unsorted) [5598]; the correct input is the sorted palette that [5260] holds.
#   The fix must ensure [5598]@bc9c == the 9f10-sorted palette.
#
# Inputs: /tmp/pal.bin (FIST_PALDUMP: [5598]|[4f60]|[5260]|a060|a460|a860),
#         /tmp/port_bc9c.bin (FIST_BC90DUMP live matrix),
#         tools/oracle/samples/oracle_bc9c_matrix_blockB_0x175200.bin.
# =============================================================================
import sys, numpy as np

def load(pal_bin):
    d = open(pal_bin, "rb").read()
    return (np.frombuffer(d[0:768],   np.uint8).reshape(256,3).astype(int),   # 5598
            np.frombuffer(d[768:1536], np.uint8).reshape(256,3).astype(int),   # 4f60
            np.frombuffer(d[1536:2304],np.uint8).reshape(256,3).astype(int))   # 5260

A060 = (31*np.arange(256))**2
A460 = (43*np.arange(256))**2
A860 = (26*np.arange(256))**2

def ac70(tr, tg, tb, pal, lo=80, hi=255):
    Tr, Tg, Tb = tr >> 1, tg >> 1, tb >> 1          # ac70 prologue: target = avg>>1
    best, bc = -1, 1 << 62
    for i in range(lo, hi+1):
        c = (A060[(pal[i,0]-Tr) & 0xff] +
             A460[(pal[i,1]-Tg) & 0xff] +
             A860[(pal[i,2]-Tb) & 0xff])
        if c < bc:
            bc, best = c, i
            if c == 0:
                break
    return best

def bc9c_diag(pal, lo=80, hi=96):
    # diagonal M[ch][ch] = ac70(avg(pal[ch],pal[ch])) = ac70(pal[ch])
    return [ac70(pal[c,0], pal[c,1], pal[c,2], pal) for c in range(lo, hi)]

if __name__ == "__main__":
    p5598, p4f60, p5260 = load(sys.argv[1] if len(sys.argv) > 1 else "/tmp/pal.bin")
    print("diag (sorted palette [5260])  :", bc9c_diag(p5260))
    print("diag (raw [5598]@dump)        :", bc9c_diag(p5598))
    try:
        portM = np.frombuffer(open("/tmp/port_bc9c.bin","rb").read(), np.uint8).reshape(256,256)
        print("port_bc9c live diag           :", np.diag(portM)[80:96].tolist())
    except FileNotFoundError:
        pass
    try:
        orcM = np.frombuffer(open("tools/oracle/samples/oracle_bc9c_matrix_blockB_0x175200.bin","rb").read(),
                             np.uint8).reshape(256,256)
        print("oracle_bc9c diag              :", np.diag(orcM)[80:96].tolist())
    except FileNotFoundError:
        pass
