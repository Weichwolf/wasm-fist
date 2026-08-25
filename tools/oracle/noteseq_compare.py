#!/usr/bin/env python3
"""OPL note-sequence comparator — strips ALL timing/phase to test musical content.

Extracts note-on rising edges (channel, fnum, block) from two OPL reglogs and
finds the longest contiguous matching run.  If the port plays a song faithfully,
a long run matches the oracle regardless of phase offset, loop position, or
cooperative-timeline drift; if notes genuinely diverge, no long run exists.

Auto-detects both formats:
  port   (FIST_OPL_REGLOG): "SEQ adv=N reg=XX val=XX"
  oracle (FISTOPLLOG):      "SEQ t=MS cs=.. port=XXX val=XX DATA reg=XX ..."

usage: noteseq_compare.py PORT.reglog ORACLE.log [--dump]
"""
import re, sys

RE_PORT   = re.compile(r'\breg=([0-9a-fA-F]{2})\s+val=([0-9a-fA-F]{2})')
RE_ORACLE = re.compile(r'\bDATA\s+reg=([0-9a-fA-F]{2})')
RE_OR_VAL = re.compile(r'\bval=([0-9a-fA-F]{2})\b')

def parse(path):
    """Return list of (reg,val) OPL register writes in order."""
    out = []
    with open(path) as f:
        for ln in f:
            if 'DATA' in ln:                       # oracle DATA line
                m = RE_ORACLE.search(ln); v = RE_OR_VAL.search(ln)
                if m and v: out.append((int(m.group(1),16), int(v.group(1),16)))
            elif 'adv=' in ln or (' reg=' in ln and 'cs=' not in ln):
                m = RE_PORT.search(ln)
                if m: out.append((int(m.group(1),16), int(m.group(2),16)))
    return out

def note_events(writes):
    """Rising-edge note-ons -> list of (channel, fnum, block)."""
    alo   = [0]*9          # last A0..A8 (fnum low)
    keyon = [False]*9      # last keyon state per channel
    ev = []
    for reg,val in writes:
        if 0xA0 <= reg <= 0xA8:
            alo[reg-0xA0] = val
        elif 0xB0 <= reg <= 0xB8:
            ch = reg-0xB0
            on = bool(val & 0x20)
            if on and not keyon[ch]:               # rising edge = note-on
                fnum  = alo[ch] | ((val & 0x03) << 8)
                block = (val >> 2) & 0x07
                ev.append((ch, fnum, block))
            keyon[ch] = on
    return ev

def longest_run(a, b):
    """Longest contiguous common subsequence (a[i..], b[j..]) via DP band."""
    if not a or not b: return (0,0,0)
    n, m = len(a), len(b)
    best = (0,0,0)                                  # (len, i, j)
    prev = [0]*(m+1)
    for i in range(1, n+1):
        cur = [0]*(m+1)
        ai = a[i-1]
        for j in range(1, m+1):
            if ai == b[j-1]:
                v = prev[j-1]+1; cur[j] = v
                if v > best[0]: best = (v, i-v, j-v)
        prev = cur
    return best

def load_ref(path):
    """Load a checked-in note-seq reference (channel fnum block per line; # comments)."""
    ev = []
    with open(path) as f:
        for ln in f:
            ln = ln.strip()
            if not ln or ln.startswith('#'): continue
            a = ln.split()
            ev.append((int(a[0]), int(a[1]), int(a[2])))
    return ev

def gate(port_reglog, ref_path):
    """board:0011 gate: the port's engine note stream must CONTAIN the full oracle
    reference as a contiguous run.  Exit 0 pass / 1 fail."""
    P = note_events(parse(port_reglog))
    R = load_ref(ref_path)
    L, i, j = longest_run(P, R)
    ok = (L == len(R))                              # the entire reference appears contiguously
    print("gate: port note-ons=%d  ref=%d  longest-contiguous-match=%d/%d  -> %s"
          % (len(P), len(R), L, len(R), "PASS" if ok else "FAIL"))
    if not ok and P:
        print("  ref[%d:%d] first mismatch context: port has %d contiguous from port[%d]"
              % (0, min(6, len(R)), L, i))
    return 0 if ok else 1

def main():
    if len(sys.argv) >= 2 and sys.argv[1] == '--gate':
        if len(sys.argv) < 4:
            print("usage: noteseq_compare.py --gate PORT.reglog REF_noteseq.txt"); sys.exit(2)
        sys.exit(gate(sys.argv[2], sys.argv[3]))
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(2)
    port_w   = parse(sys.argv[1])
    oracle_w = parse(sys.argv[2])
    P = note_events(port_w)
    O = note_events(oracle_w)
    print(f"port   : {len(port_w):6d} reg-writes -> {len(P):5d} note-ons")
    print(f"oracle : {len(oracle_w):6d} reg-writes -> {len(O):5d} note-ons")
    if not P or not O:
        print("!! one side has no note-ons — capture window missed the music"); return
    L,i,j = longest_run(P, O)
    frac = L / min(len(P), len(O)) if min(len(P),len(O)) else 0
    print(f"longest contiguous matching note-run: {L}  (port[{i}:{i+L}] == oracle[{j}:{j+L}])")
    print(f"  = {frac*100:.1f}% of the shorter sequence")
    if frac > 0.85:
        print("VERDICT: port plays the song FAITHFULLY — residual is timing/phase only.")
    elif frac > 0.30:
        print("VERDICT: PARTIAL match — a section agrees; investigate the break point.")
    else:
        print("VERDICT: note sequences DIVERGE — genuine content bug (or wrong section).")
    if '--dump' in sys.argv:
        print("\nport   note-ons [i-6:i+14]:", P[max(0,i-6):i+14])
        print(  "oracle note-ons [j-6:j+14]:", O[max(0,j-6):j+14])

if __name__ == '__main__':
    main()
