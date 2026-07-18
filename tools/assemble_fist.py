#!/usr/bin/env python3
"""Assemble the Armored Fist engine build unit (re_out/fist.c) from the pristine 16-bit-real-mode
Ghidra decompile (re_out/fist_decomp.c). Mechanical, reproducible: `make assemble`.

DOS analog of wasm-dd2's assemble_dd2h.py. The pristine decompile is NEVER edited; every transform
here is a documented RULE so the output regenerates byte-for-byte and drift stays visible. Each rule
resolves one Ghidra-idiom *error class* the DD2 way -- in the assembler, never by hand-editing the C.

RULES (each = one gcc error class; see the deliverable report):
  1  artifact sanitize      Ghidra name specials (" @ & `) -> '_'   (defensive; none in FIST today).
  2  FLAT MEMORY (DAT/_DAT) every DAT_SEG_OFF[_N] -> accessor into g_mem at linear (SEG<<4)+OFF[+N].
        called `(*N)(` -> undefined4 (far code ptr) + call-site cast ;  pointer `N[` or `*N` -> int** ;
        byte-suffix N -> undefined1 at +N ;  _DAT -> undefined4 (32-bit view) ;  else -> undefined2.
  3  STRINGS (s_)           s_..._SEG_OFF -> (char*)(g_mem + (SEG<<4)+OFF)   (string bytes in image).
  4  ABSOLUTE RAM           bRam/cRam/uRam/iRam/puRam/pcRam<hex> and ram0x<hex> -> g_mem accessor at
        <hex>, width/kind by prefix (b/c=byte, u=word, i=int, pu=ptr, pc=char*, ram=word).
  5  SUB-FIELD (._O_S_)     NAME._O_S_ -> typed S-byte access at byte offset O:
        g_mem symbol -> (*(uS*)(g_mem+L+O)) ;  local var -> (*(uS*)((char*)&NAME + O)).
  6  VOID-RETURN RETYPE     a `void` function whose result a caller consumes (asm returns in AX, Ghidra
        mis-modelled void) -> retype void->undefined4 at decl+def (return-mismatch left to -Wno-...).
  7  FUN/LAB AS DATA        code symbols used as VALUES, not calls (self-referential addr arithmetic,
        self-modifying-code stores, unresolved jump targets):
          _FUN_/_thunk_FUN_ , FUN_ store `FUN=`  -> g_mem accessor at the code's linear address (SMC);
          bare FUN value use                      -> ((uint)(uintptr_t)&FUN)  (host addr; round-trips
                                                     through uint32 on -m32, so call sites still work);
          &LAB_.._N (address-taken jump target)   -> g_mem accessor  (OPEN: unresolved indirect target).
  8  UNBOUND PSEUDO-VARS    register0x.. and stack0x.. Ghidra pseudo-vars Ghidra left undeclared ->
        shared globals (OPEN: register-file / per-frame stack not modelled; declared so it links).

Output: re_out/fist.c   (then `make patch` layers patches/NNN-*.diff onto it, as with DD2).
"""
import re, sys, os, bisect

ROOT   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DECOMP = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "re_out", "fist_decomp.c")
OUT    = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, "re_out", "fist.c")
MEM_SIZE = 0x100000   # keep in sync with FIST_MEM_SIZE in ghidra_compat.h

# ============================================================================
# DRIVER-OVERLAY (MULTI-MODULE) MODE  --  FIST_MODULE=<mod> FIST_MODULE_SIZE=<hex>
# ----------------------------------------------------------------------------
# The engine loads MGAVIDEO.DVR / SOUNDDVR.DVR at a RUNTIME segment (chosen by the
# engine, passed to INT 21h AH=4B AL=03). Their decompiles are base-0 like the
# engine, but at run time the module sits at load_base = load_seg<<4, so the
# module's OWN memory (linear < module_size: its code-segment data, near-call CS)
# must be accessed at g_mem + fist_<mod>_base + linear, where fist_<mod>_base is
# set at load (fist_dos.c -> fist_ovl_register). References at linear >= module_size
# are ABSOLUTE -- the driver runs with DS = the ENGINE's DGROUP, so its dominant
# refs are engine memory (DGROUP 0x1c000+, resources 0x2xxxx), plus the VGA
# aperture 0xA0000 and the INT reg-file 0xF0000 -- all correct at base-0 in the
# shared g_mem. The FUN map is keyed by base-0 MODULE OFFSETS; fist_icall subtracts
# the load base to look a runtime target up in it. Engine mode (MODULE=None) is
# unchanged and emits byte-identical output (every branch below is guarded).
MODULE      = os.environ.get('FIST_MODULE') or None          # e.g. 'mga' / 'snd' / 'ext'
MODULE_SIZE = int(os.environ.get('FIST_MODULE_SIZE', '0'), 0) # module load-module size (bytes)
FMAP_NAME   = ('fist_%s_fmap' % MODULE) if MODULE else 'fist_fmap'
BASE_SYM    = ('fist_%s_base' % MODULE) if MODULE else None

# ============================================================================
# 32-BIT-FLAT MODULE MODE  --  FIST_FLAT32=1 (with FIST_MODULE=ext)
# ----------------------------------------------------------------------------
# The Doug-Huffman EXTENDER kernel (FIST.RUN) that hosts the KDV intro-FMV player is 32-bit,
# FLAT, linked base 0 -- unlike the 16-bit-segmented engine + .DVR overlays.  Its Ghidra
# decompile (x86:LE:32) names every static symbol by its SINGLE flat linear address:
#   FUN_00006f3e / DAT_00006e80 / _DAT_00006e84 / LAB_0000bc98 / thunk_FUN_00000f64 /
#   PTR_DAT_0000748c / s_..._000084a0 / markers `@ 00006f3e =====`
# rather than the engine's SEG_OFF pair (DAT_1000_c246 / `@ 1000:c246`).  The whole extender
# image is < 64 KB (0xbf90), so every flat linear L fits an equivalent 0000:L SEG:OFF.  We
# therefore NORMALIZE the single-address form to the SEG_OFF form the rest of this assembler
# already understands (SEG=0000, OFF=L), and reuse 100% of the existing machinery (markers,
# DAT/FUN/LAB/thunk classification, the indirect-call dispatcher, the fmap).  The only genuine
# 32-bit deltas are (a) the natural word is 4 bytes -> scalar DATs default to undefined4, not
# undefined2; (b) new symbol families sRam (signed reg-file slot) and PTR_DAT/PTR_FUN/PTR_LAB/
# PTR_s_ (Ghidra pointer-typed data).  All of this is GATED behind FLAT32 so engine + 16-bit
# driver output stays byte-identical.  MEMORY MODEL (native_main seeds): the extender's flat
# POINTER slots ([0x90b]/[0x90f] heap base/top, [0x917] framebuffer, [0xc93] TCB) hold HOST
# pointers into g_mem, so the real bump-allocator / RLE decoder / present blit / DAC upload run
# with native pointer derefs (no per-deref rebase).  The extender image itself (static disp32
# refs < MODULE_SIZE) is rebased to g_mem + fist_ext_base + off exactly like the .DVR overlays.
FLAT32 = bool(os.environ.get('FIST_FLAT32'))

# FLAT32 off-image scratch: the x86:LE:32 decompile emits a handful of DAT_/PTR_ "symbols" whose
# address is really a 32-bit IMMEDIATE the analyzer mis-typed as a global (e.g. DAT_826a1a82) -- far
# outside the 0xbf90 extender image and NOT on the executed KDV path.  Remap each such off-image
# linear into a compact scratch window appended to the module region so it links (and is harmless if
# an off-path read ever hit it).  ABS_WINDOW bytes reserved just above MODULE_SIZE (native_main sizes
# the ext module region to MODULE_SIZE + ABS_WINDOW).
ABS_WINDOW = 0x4000 if FLAT32 else 0
MODSPAN    = MODULE_SIZE + ABS_WINDOW
_absmap = {}
def _remap(lin):
    if FLAT32 and MODULE and lin >= MODULE_SIZE:
        # The shared INT reg-file (InstallIntFixup, linear 0xF0000..0xF00FF) must alias the shim's
        # reg-file at ABSOLUTE 0xF0000 -- NOT the per-module scratch window -- so the extender's
        # marshalled `int n` registers reach fist_int_dispatch() and its results flow back.  Same
        # layout as the 16-bit engine (0xF0000 AX .. 0xF0014 VEC); the 32-bit input shadows the
        # FLAT32 shadow-store emits (0xF0020..0xF0037) live just above and are also absolute.
        if 0xf0000 <= lin < 0xf0100:
            return lin
        if lin not in _absmap:
            slot = MODULE_SIZE + len(_absmap) * 4
            if slot >= MODSPAN:
                sys.exit("assemble_fist: FLAT32 off-image scratch overflow (grow ABS_WINDOW)")
            _absmap[lin] = slot
        return _absmap[lin]
    return lin

def G(lin):
    """g_mem base expression for a flat linear address. In driver mode a module-relative
    linear (< MODULE_SIZE) is rebased at runtime via the module's base global.  FLAT32 folds
    off-image absolute-immediate 'symbols' into the module scratch window (see _remap)."""
    lin = _remap(lin)
    if MODULE and lin < MODSPAN:
        return "g_mem+%s+0x%x" % (BASE_SYM, lin)
    return "g_mem+0x%x" % lin

src = open(DECOMP, encoding='utf-8', errors='surrogateescape').read()

# ---- FLAT32 NORMALIZATION: single flat-address symbols -> 0000:OFF SEG_OFF form -----------------
# Done BEFORE any other processing so every downstream regex (markers, DAT/FUN/LAB/thunk, indirect
# calls, fmap) sees the SEG_OFF shapes it already handles.  Every extender symbol linear < 0x10000,
# so the high SEG nibble word is always 0000 and OFF = the full linear (identity for G()/fist_icall).
if FLAT32:
    # function/label markers:  `@ HHHHHHHH =====`  ->  `@ HHHH:HHHH =====`
    src = re.sub(r'(@ )([0-9a-fA-F]{4})([0-9a-fA-F]{4})( =====)', r'\1\2:\3\4', src)
    # code symbols carrying an 8-hex flat address suffix (targeted families only; in_register_/
    # switchD_/code_r0x_ are Ghidra-declared locals/labels and are deliberately left untouched):
    for _pre in (r'_?(?:thunk_)?FUN', r'_?DAT', r'PTR_(?:DAT|FUN|LAB)'):
        src = re.sub(r'\b(%s)_([0-9a-fA-F]{4})([0-9a-fA-F]{4})\b' % _pre, r'\1_\2_\3', src)
    # LAB may carry a Ghidra duplicate/offset suffix `_N` (e.g. LAB_00006952_3) -> keep it.
    src = re.sub(r'\b(_?LAB)_([0-9a-fA-F]{4})([0-9a-fA-F]{4})(_[0-9]+)?\b', r'\1_\2_\3\4', src)
    # string symbols (s_...  /  PTR_s_...): the 8-hex address is the final _-separated field.
    src = re.sub(r'\b(PTR_s_[A-Za-z0-9_]+?|s_[A-Za-z0-9_]+?)_([0-9a-fA-F]{4})([0-9a-fA-F]{4})\b',
                 r'\1_\2_\3', src)
    #  - a jump-table switch on a POINTER var with pointer case labels (`switch(pbVar9){case
    #    (byte*)0x1:...}`) is illegal C.  Cast the switch quantity to int and strip the case casts.
    src = re.sub(r'\bswitch\(', 'switch((int)', src)
    src = re.sub(r'\bcase \([A-Za-z_][A-Za-z0-9_ ]*\*\)(0x[0-9a-fA-F]+|[0-9]+):', r'case \1:', src)
    #  - `code` used as a VALUE type (return slot `code FUN(`, cast `(code)x`) means "a code-pointer
    #    value"; C forbids a function type there.  Retype every `code` that is NOT a pointer (`code *`
    #    / `code*`) to undefined4.  The genuine `(*(code *)...)` indirect-call casts keep their '*'.
    src = re.sub(r'\bcode\b(?!\s*\*)', 'undefined4', src)
    #  - BADSPACEBASE (Ghidra "unresolved stack base" pointer type) -> a byte (off-path artifacts).
    src = src.replace('BADSPACEBASE', 'undefined1')
    #  - Ghidra sometimes emits a jump to a function ADDRESS as `goto FUN_x;` with a local label
    #    `FUN_x:` (a mid-function / overlapping-code entry).  Rename both to a distinct `Lbl_FUN_x`
    #    so it is a plain local label (not the value-use rewrite of the function symbol).  The Lbl_
    #    prefix also blocks RULE 7's bare-FUN-value rewrite (its lookbehind rejects a leading '_').
    src = re.sub(r'(?m)^(\s*)(FUN_[0-9a-fA-F_]+):(?!:)', r'\1Lbl_\2:', src)
    src = re.sub(r'\bgoto (FUN_[0-9a-fA-F_]+);', r'goto Lbl_\1;', src)
    # 32-bit INPUT CAPTURE for the extender's real flat `int n`.  The extender is 32-bit flat: its
    # `int 0x21` passes buffers/counts in full EDX/ECX/EBX (e.g. `mov edx,[0x6e80]` = a flat host
    # heap pointer, or `mov edx,0x5898` = a module offset).  Ghidra's 16-bit InstallIntFixup marshals
    # only the low word (`sRam000f0006 = (short)DAT_0000_6e80`), truncating host pointers.  Alongside
    # each reg-file store to a pointer/count/handle slot (BX=02/CX=04/DX=06/SI=08) emit a FULL-WIDTH
    # shadow at 0xF0020 + slot*2 with the truncating cast stripped, so an extender-mode INT-21
    # (fist_dos.c) can recover the exact 32-bit operand.  Engine builds are FLAT32-off => unaffected
    # (byte-identical); non-buffer INT calls set shadows the shim simply never consults.
    def _int32_shadow(m):
        ind, k, yy, rhs = m.groups()
        r = rhs
        for c in ('(short)', '(ushort)', '(undefined2)', '(uint)', '(int)'):
            if r.startswith(c):
                r = r[len(c):]; break
        sh = 0xf0020 + int(yy, 16) * 2
        return "%s%sRam000f00%s = %s; *(undefined4 *)(g_mem+0x%x) = (undefined4)(%s);" % (
            ind, k, yy, rhs, sh, r)
    src = re.sub(r'(?m)^([ \t]*)([su])Ram000f00(02|04|06|08) = ([^;\n]+);', _int32_shadow, src)

# Ghidra jump-table case pseudo-functions are named `switchD_ADDR::caseD_N` (FLAT32) or, in the 16-bit
# segmented decompile, `switchD_SEG:OFF::caseD_N` (e.g. SOUNDDVR's switchD_0000:1e71::caseD_0) -- but
# MARKERED as just `caseD_N` -> drop the `switchD_ADDR::` / `switchD_SEG:OFF::` prefix so the def name ==
# marker name (== fmap key).  The trailing `::` disambiguates it from the seg:off colon.  Applies to
# FLAT32 (ext) and the 16-bit driver overlays (mga/snd); a no-op on engine + on any driver lacking a
# jump table -> byte-identical output where absent.
if FLAT32 or MODULE:
    src = re.sub(r'switchD_[0-9a-fA-F]+(?::[0-9a-fA-F]+)*::', '', src)
    src = src.replace('::', '_')                              # any residual C++ scope -> '_'

# De-duplicate function names: Ghidra names every far-jump-to-entry thunk (`jmp far 0:0`) `app_entry`,
# so a driver overlay carries several app_entry definitions (the real entry 0x0 + the 0x1cd/0x3d6 return
# thunks) -> a C redefinition.  Keep the FIRST marker's name; rename each later duplicate (marker + its
# def header + any in-block self-reference) to <name>_<seg>_<off>.  These duplicates are referenced only
# by numeric offset via the overlay fmap (never by name in a body), so a per-block rename is complete.
# Engine mode has no duplicate marker names -> no-op (fist.c byte-identical).
_MK = re.compile(r'/\* ===== (\S+) @ ([0-9a-fA-F]+):([0-9a-fA-F]+) ===== \*/')
_marks = [(m.start(), m.group(1), m.group(2), m.group(3)) for m in _MK.finditer(src)]
if _marks:
    _seen = {}
    _spans = []                                          # (block_start, block_end, oldname, newname)
    for _i, (_st, _nm, _sg, _of) in enumerate(_marks):
        _seen[_nm] = _seen.get(_nm, 0) + 1
        if _seen[_nm] > 1:
            _end = _marks[_i + 1][0] if _i + 1 < len(_marks) else len(src)
            _spans.append((_st, _end, _nm, "%s_%s_%s" % (_nm, _sg, _of)))
    if _spans:
        _out, _pos = [], 0
        for (_st, _en, _old, _new) in _spans:
            _out.append(src[_pos:_st])
            _out.append(re.sub(r'\b%s\b' % re.escape(_old), _new, src[_st:_en]))
            _pos = _en
        _out.append(src[_pos:])
        src = ''.join(_out)

# Capture defined function names from the PRISTINE markers before any rewrite mangles them
# (RULE 7's bare-FUN-value rewrite wraps `FUN @` in the marker line since it isn't followed by '(').
DEFINED_FNS = set(re.findall(r'/\* ===== (\S+) @', src))

# Capture the pristine {function name -> linear (SEG<<4)+OFF} from the markers, for the
# indirect-call dispatcher's FUN map (RULE 2 / fist_icall.c). Done on the untouched src so later
# rewrites (RULE 7 mangles FUN names inside marker comments) cannot corrupt the seg:off parse.
FN_LINEAR = {}
for _m in re.finditer(r'/\* ===== (\S+) @ ([0-9a-fA-F]+):([0-9a-fA-F]+) ===== \*/', src):
    FN_LINEAR[_m.group(1)] = (int(_m.group(2), 16) << 4) + int(_m.group(3), 16)

def w2type(s):   # byte-width -> Ghidra scalar type
    return {1: "undefined1", 2: "undefined2", 4: "undefined4"}.get(int(s), "undefined4")

# ---- RULE 1: identifier-artifact sanitization (defensive) ------------------------------------------
src = re.sub(r'([A-Za-z0-9_])["@&`]([A-Za-z0-9_(])', r'\1_\2', src)

# ---- RULE 0: INT-dispatcher hook ------------------------------------------------------------------
# The <callotherfixup targetop=swi> (tools/ghidra/InstallIntFixup.java) rewrites every `int n` to
# marshal registers through a g_mem reg-file and call the dispatcher at magic linear 0xF0100. Ghidra
# renders that as `(*(code *)0xf0100)(...)` (sometimes with spurious args the decompiler inferred).
# Map every such site to the shim entry point fist_int_dispatch(); args are ignored (the handler reads
# the reg-file). Balanced-paren scan so nested arg expressions are consumed correctly.
def _hook_dispatch(text):
    needle = "(*(code *)0xf0100)"
    out = []; i = 0
    while True:
        j = text.find(needle, i)
        if j < 0: out.append(text[i:]); break
        out.append(text[i:j]); k = j + len(needle)
        # k must point at '(' — consume the balanced argument list
        assert text[k] == '(', "unexpected dispatcher call form at %d" % k
        depth = 0; m = k
        while m < len(text):
            if text[m] == '(': depth += 1
            elif text[m] == ')':
                depth -= 1
                if depth == 0: m += 1; break
            m += 1
        out.append("fist_int_dispatch()"); i = m
    return "".join(out)
# normalize any leading-zero variants first, then hook
src = re.sub(r'\(\*\(code \*\)0x0*f0100\)', '(*(code *)0xf0100)', src)
_n_hook = src.count("(*(code *)0xf0100)")
src = _hook_dispatch(src)

# ================= collect flat-memory symbols (name -> linear) =================
symL = {}   # every g_mem-backed symbol name -> linear address (RULE 5 dispatch + accessor emission)

# --- DAT / _DAT (optional _N byte-suffix) ---
DAT_RE = r'\b(_?)DAT_([0-9a-fA-F]+)_([0-9a-fA-F]+)(?:_([0-9]+))?\b'
dat = {}   # name -> (linear, lead_underscore, byte_suffix_or_None)
for m in re.finditer(DAT_RE, src):
    name = m.group(0)
    lin  = (int(m.group(2), 16) << 4) + int(m.group(3), 16) + (int(m.group(4)) if m.group(4) else 0)
    dat[name] = (lin, m.group(1) == '_', m.group(4))
    symL[name] = lin

# --- s_ string symbols (trailing _SEG_OFF) ---
strsym = {}
for name in set(re.findall(r'\bs_\w+\b', src)):
    m = re.match(r'^s_.*_([0-9a-fA-F]+)_([0-9a-fA-F]+)$', name)
    if not m:
        continue
    lin = (int(m.group(1), 16) << 4) + int(m.group(2), 16)
    strsym[name] = lin; symL[name] = lin

# --- absolute RAM accessors (xRam<hex>, no 0x) + ram0x<hex> (lowercase, with 0x) ---
# FLAT32 adds 's' (signed word, e.g. the extender's sRam000f0002 = the 16-bit BX reg-file slot) and
# an optional leading '_' (a 32-bit-wide view of a slot, e.g. _uRam00006add -> undefined4).
ram = {}   # name -> (kind, linear)   ; kind '_' => 4-byte view
if FLAT32:
    for m in re.finditer(r'\b(_?)(ppu|pu|pc|pb|pi|b|c|u|i|s)Ram([0-9a-fA-F]{4,8})\b', src):
        kind = '_' if m.group(1) else m.group(2)
        ram[m.group(0)] = (kind, int(m.group(3), 16)); symL[m.group(0)] = int(m.group(3), 16)
else:
    for m in re.finditer(r'\b(ppu|pu|pc|pb|pi|b|c|u|i)Ram([0-9a-fA-F]{4,8})\b', src):
        ram[m.group(0)] = (m.group(1), int(m.group(2), 16)); symL[m.group(0)] = int(m.group(2), 16)
for m in re.finditer(r'\bram0x([0-9a-fA-F]+)\b', src):
    ram[m.group(0)] = ('ram', int(m.group(1), 16)); symL[m.group(0)] = int(m.group(1), 16)

# --- _FUN_ / _thunk_FUN_ : code-address DATA locations (SMC / far-ptr build) -> g_mem ---
codeloc = {}   # name -> linear
_codepat = r'\b_((?:thunk_)?FUN|LAB)_([0-9a-fA-F]+)_([0-9a-fA-F]+)\b' if FLAT32 \
           else r'\b_((?:thunk_)?FUN)_([0-9a-fA-F]+)_([0-9a-fA-F]+)\b'
for m in re.finditer(_codepat, src):
    name = m.group(0)
    lin  = (int(m.group(2), 16) << 4) + int(m.group(3), 16)
    codeloc[name] = lin; symL[name] = lin

# --- FLAT32: PTR_DAT/PTR_FUN/PTR_LAB/PTR_s_ : Ghidra pointer-typed data locations -> g_mem ---
# A PTR_ symbol names a location holding a POINTER (4 bytes flat).  `&PTR_X` = the slot address (a
# pointer table base; `(&PTR_X)[i]` strides by 4); a bare `PTR_X` = the stored pointer VALUE.  Read
# the slot as undefined4 (matches -m32 pointer width).  The stored values are original-PM linears
# until the slot is (re)written at runtime -- correct for the address-of/table uses on the executed
# KDV path; a bare-value deref of an un-rewritten slot is an OPEN item (off the intro path).
ptrsym = {}   # name -> linear
if FLAT32:
    for m in re.finditer(r'\b(PTR_(?:DAT|FUN|LAB)|PTR_s_[A-Za-z0-9_]+?)_([0-9a-fA-F]+)_([0-9a-fA-F]+)\b', src):
        name = m.group(0)
        lin  = (int(m.group(2), 16) << 4) + int(m.group(3), 16)
        ptrsym[name] = lin; symL[name] = lin

# --- &LAB_.._N : address-taken jump targets that are NOT emitted as C goto-labels -> g_mem ---
label_decls = set(re.findall(r'(?m)^\s*(LAB_[0-9a-fA-F_]+):', src))
lab = {}   # name -> linear
for m in re.finditer(r'&(LAB_([0-9a-fA-F]+)_([0-9a-fA-F]+)(?:_([0-9]+))?)\b', src):
    name = m.group(1)
    if name in label_decls:
        continue                                   # a real C label; leave the goto/label alone
    lin = (int(m.group(2), 16) << 4) + int(m.group(3), 16) + (int(m.group(4)) if m.group(4) else 0)
    lab[name] = lin; symL[name] = lin
# The 32-bit-flat decompile AND the 16-bit driver overlays use a bare LAB_ (no '&') as an SMC byte
# value/lvalue -- e.g. `CONCAT11(LAB_x,LAB_x)` (read) or `LAB_x = 0x411` (self-modifying store of a
# jump/operand target into the code at that address; SOUNDDVR's FUN_0000_1ef2 patches LAB_0000_12d9).
# Any LAB_ NOT emitted as a real C label (not in label_decls) is such a code-address location -> a
# g_mem accessor (same class as _FUN_/_LAB SMC data).  Engine mode (MODULE=None, FLAT32=0) has no such
# bare-LAB use -> unchanged (byte-identical fist.c).
if FLAT32 or MODULE:
    for m in re.finditer(r'\b(LAB_([0-9a-fA-F]+)_([0-9a-fA-F]+)(?:_([0-9]+))?)\b', src):
        name = m.group(1)
        if name in label_decls or name in lab:
            continue
        lin = (int(m.group(2), 16) << 4) + int(m.group(3), 16) + (int(m.group(4)) if m.group(4) else 0)
        lab[name] = lin; symL[name] = lin

# ---- classify DAT usage: called (func ptr) / pointer-var / scalar ----------------------------------
called_dat = set(re.findall(r'\(\*(_?DAT_[0-9a-fA-F]+_[0-9a-fA-F]+(?:_[0-9]+)?)\)\(', src))
indexed    = set(re.findall(r'(_?DAT_[0-9a-fA-F]+_[0-9a-fA-F]+(?:_[0-9]+)?)\[', src))
deref      = set(re.findall(r'(?<![\w])\*(_?DAT_[0-9a-fA-F]+_[0-9a-fA-F]+(?:_[0-9]+)?)\b', src))
ptrvar     = (indexed | deref) - called_dat

# ================= text rewrites =================
# --- RULE 8b: default the FLAG pseudo-vars to 0 (Stage-1 flag scaffolding) ---
# x86-16 has no Ghidra flag-register calling-convention slot, so a carry/zero/etc. flag produced by a
# callee (or a prior op) never threads to the caller: the decompiler emits it as a read-before-write
# local `in_CF`/`in_ZF`/... which gcc leaves as stack garbage -> random branch at the first flag gate
# (e.g. app_entry's `if (in_CF) INT21/4C-terminate`). Default every such flag local to 0 = "entered
# with flags clear / no error", the common-case-correct value that lets execution flow. OPEN/Stage-1:
# a real per-site carry is the documented flag patch class -> replaced by true flag threading later.
src = re.sub(r'\b(undefined[124])\s+(in_(?:CF|ZF|SF|OF|PF|AF|DF|IF|TF|NT))\s*;',
             r'\1 \2 = 0;', src)

# --- RULE 6: retype void functions whose result is consumed ---
void_funcs = set(re.findall(r'\bvoid\s+(?:__allregs\s+)?(FUN_[0-9a-fA-F]+_[0-9a-fA-F]+|thunk_[A-Za-z0-9_]+|app_entry)\s*\(', src))
valctx = set()
for pat in (r'=\s*(?:\([^;()]*\)\s*)*([A-Za-z_]\w*)\s*\(',
            r'return\s+(?:\([^;()]*\)\s*)*([A-Za-z_]\w*)\s*\(',
            r'[-+*/%&|^~<>!]=?\s*([A-Za-z_]\w*)\s*\(',
            r'[(,]\s*(?:\([^;()]*\)\s*)*([A-Za-z_]\w*)\s*\('):
    valctx.update(re.findall(pat, src))
retype = void_funcs & valctx
for fn in retype:
    src = re.sub(r'\bvoid(\s+(?:__allregs\s+)?%s\s*\()' % re.escape(fn), r'undefined4\1', src)

# --- RULE 7 (SMC store): bare `FUN_x = ` (writing into code) -> g_mem lvalue at the code's linear ---
def fun_store(m):
    seg, off = m.group(2), m.group(3)
    lin = (int(seg, 16) << 4) + int(off, 16)
    return "(*(undefined2 *)(%s))%s" % (G(lin), m.group(4))
src = re.sub(r'\b(FUN)_([0-9a-fA-F]+)_([0-9a-fA-F]+)\b(\s*=(?!=))', fun_store, src)

# --- RULE 7 (value use): bare FUN not-called, not-address-of -> host address (round-trips on -m32) ---
src = re.sub(r'(?<![\w&])(FUN_[0-9a-fA-F]+_[0-9a-fA-F]+)\b(?!\s*\()', r'((uint)(uintptr_t)&\1)', src)

# --- RULE 2 (INDIRECT-CALL DISPATCHER): route every `call far/near [mem]` through fist_icall ------
# A g_mem-stored code pointer is a 16-bit real-mode SEG:OFF (far) or bare OFFSET (near, implied
# CS = the enclosing function's code segment) -- NEVER a host address. Calling through the raw
# stored value jumps into a small integer and segfaults (the `call far [DGROUP:0x12] -> NULL`
# crash). Resolve each site's LINEAR target through the dispatcher (fist_icall.c), which returns a
# real host `code *` (known FUN_ -> that C fn; saved-INT-vector magic -> fist_int_dispatch chain;
# else honest trap). Two shapes, from Ghidra's rendering (near = single 16-bit read; far = 32-bit
# far-pointer read via a code-typed DAT/Ram accessor):
#
# 2a NEAR: (*(code *)EXPR)(   EXPR is a single-word read through a pointer/local. Implied segment =
#    the enclosing function's CS (0x0000 for FUN_0000_*, 0x1000 for FUN_1000_*). Balanced-paren scan
#    (EXPR may itself contain parens); an address-taken jump target (&LAB_seg_off) resolves directly.
_marks = [(mm.start(), int(mm.group(1), 16))
          for mm in re.finditer(r'/\* ===== \S+ @ ([0-9a-fA-F]+):[0-9a-fA-F]+ =====', src)]
_mpos = [p for p, _ in _marks]; _mseg = [s for _, s in _marks]
def _seg_at(pos):
    k = bisect.bisect_right(_mpos, pos) - 1
    return _mseg[k] if k >= 0 else 0
_LABX = re.compile(r'^\s*&LAB_([0-9a-fA-F]+)_([0-9a-fA-F]+)(?:_[0-9]+)?\s*$')
# A `call [DGROUP:disp]` through a FIXED memory location `*(undefined2 *)&DAT_seg_off` is, in this
# engine, a FAR indirect call (verified FF/3 `call DWORD PTR` in every case -- the engine's 11-entry
# far-ptr service-dispatch table in DGROUP: off:seg pairs, e.g. DGROUP:0x12 -> FUN_0000_f842). Ghidra
# under-renders the far pointer as a single undefined2 read; read the full off:seg dword and dispatch
# FAR. (Register/local-indirect `call [bx]` forms stay NEAR -- implied CS = the caller's segment.)
_FARDAT = re.compile(r'^\s*\*\(undefined2 \*\)&DAT_([0-9a-fA-F]+)_([0-9a-fA-F]+)(?:_[0-9]+)?\s*$')
_needle = "(*(code *)"
_near_hits = []                       # (start, outer_close_idx, expr)  top-level, non-overlapping
i = 0
while True:
    j = src.find(_needle, i)
    if j < 0: break
    depth = 0; p = j
    while p < len(src):
        c = src[p]
        if c == '(': depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0: break
        p += 1
    if p + 1 < len(src) and src[p+1] == '(':          # ')(' => it's a call through the code ptr
        _near_hits.append((j, p, src[j+len(_needle):p]))
    i = p + 1                                          # resume AFTER this site (skip nested inners)
n_near = n_fardat = 0
for (j, p, expr) in reversed(_near_hits):             # descending => indices stay valid
    ml = _LABX.match(expr)
    mf = _FARDAT.match(expr)
    if ml:                                            # &LAB_seg_off: address-taken jump target
        lin = (int(ml.group(1), 16) << 4) + int(ml.group(2), 16)
        if MODULE and lin < MODULE_SIZE:
            repl = "(*(code *)fist_icall(%s+0x%xu))" % (BASE_SYM, lin)
        else:
            repl = "(*(code *)fist_icall(0x%xu))" % lin
    elif mf:                                          # call [DGROUP:disp]: FAR service-table entry
        lin = (int(mf.group(1), 16) << 4) + int(mf.group(2), 16)
        repl = "(*(code *)fist_icall_far(*(uint32_t *)(%su)))" % G(lin)  # 'u' preserves engine byte-identity
        n_fardat += 1
    else:                                             # register/local-indirect: NEAR, implied CS
        seg = _seg_at(j)
        if MODULE:                                     # runtime CS = load_seg + module_cs -> add base
            repl = "(*(code *)fist_icall(%s+0x%xu+(uint16_t)(%s)))" % (BASE_SYM, seg << 4, expr)
        else:
            repl = "(*(code *)fist_icall_near(0x%xu,(uint16_t)(%s)))" % (seg, expr)
        n_near += 1
    src = src[:j] + repl + src[p+1:]

# 2b FAR: (*SYM)(  where SYM is a code-typed DAT_/xRam accessor reading the stored SEG:OFF dword
#    (CONCAT22(seg,off)). fist_icall_far computes linear = seg*16 + off and dispatches.
src, n_far = re.subn(
    r'\(\*((?:_?DAT_[0-9a-fA-F]+_[0-9a-fA-F]+(?:_[0-9]+)?)|(?:(?:ppu|pu|pc|pb|pi|b|c|u|i)Ram[0-9a-fA-F]+))\)\(',
    r'(*(code *)fist_icall_far((uint32_t)(\1)))(', src)

# --- RULE 5: sub-field NAME._O_S_ ---
def sub_field(m):
    name, off, size = m.group(1), int(m.group(2)), m.group(3)
    ty = w2type(size)
    if name in symL:
        return "(*(%s *)(%s))" % (ty, G(symL[name] + off))
    return "(*(%s *)((char *)&%s + %d))" % (ty, name, off)
src = re.sub(r'\b([A-Za-z_]\w*)\._(\d+)_(\d+)_', sub_field, src)

# ================= emit accessor #defines =================
defs_dat = []
n_call = n_ptr = n_byte = n_s32 = n_s16 = 0
for name in sorted(dat):
    lin, lead, sub = dat[name]
    if name in called_dat:
        acc = "(*(undefined4 *)(%s))" % G(lin); n_call += 1
    elif name in ptrvar:
        # A pointer-valued DAT (dereferenced / indexed).  In FLAT32 the extender is byte-flat: every
        # `DAT + off` is a BYTE offset (e.g. the current-TCB [0xc93] read at +0xBA/+0x496, the KDV chunk
        # buffer [0x6e80] read at +4), and Ghidra's `int **` typing scales those by 4 -> wrong address.
        # Emit a BYTE pointer (undefined1 **) so `DAT + off` is a byte offset and `*(T*)(DAT+off)` reads
        # the intended field (same byte-addressed-pointer doctrine as the engine's task-struct macro,
        # patch 068).  16-bit engine/driver units keep `int **` (byte-identical; FLAT32 off).
        acc = "(*(undefined1 **)(%s))" % G(lin) if FLAT32 else "(*(int **)(%s))" % G(lin)
        n_ptr += 1
    elif sub is not None:
        acc = "(*(undefined1 *)(%s))" % G(lin); n_byte += 1
    elif lead:
        acc = "(*(undefined4 *)(%s))" % G(lin); n_s32 += 1
    elif FLAT32:
        # 32-bit flat: the natural scalar/pointer word is 4 bytes.  Defaulting scalars to undefined4
        # makes flat pointer slots ([0x90b]/[0x917]/[0xc93]) + pointer-table strides + dword fields
        # read the full value.  (Byte-sized flags like the KDV palette-dirty [0x6e94] are written and
        # read only as whole words on the intro path -> self-consistent; documented as a residual.)
        acc = "(*(undefined4 *)(%s))" % G(lin); n_s32 += 1
    else:
        acc = "(*(undefined2 *)(%s))" % G(lin); n_s16 += 1
    defs_dat.append("#define %s %s" % (name, acc))

defs_str = ["#define %s ((char *)(%s))" % (n, G(strsym[n])) for n in sorted(strsym)]

RAMTY = {'b':"undefined1 *", 'c':"undefined1 *", 'u':"undefined2 *", 'i':"int *", 's':"short *",
         'pu':"undefined2 **", 'pc':"char **", 'pb':"undefined1 **", 'pi':"int **",
         'ppu':"undefined2 ***", 'ram':"undefined2 *", '_':"undefined4 *"}
defs_ram = ["#define %s (*(%s)(%s))" % (n, RAMTY[ram[n][0]], G(ram[n][1])) for n in sorted(ram)]

_codew = "undefined4" if FLAT32 else "undefined2"   # FLAT32 _FUN/_LAB/_thunk are 32-bit code-addr views
defs_code = ["#define %s (*(%s *)(%s))" % (n, _codew, G(codeloc[n])) for n in sorted(codeloc)]
defs_lab  = ["#define %s (*(undefined1 *)(%s))" % (n, G(lab[n])) for n in sorted(lab)]
# FLAT32 PTR_ pointer-typed data slots.  The slot holds a POINTER: a bare `PTR_X` is the stored
# pointer (byte*, so `PTR_X[i]` derefs and any `(T*)PTR_X` recasts), and `&PTR_X` is the slot address
# (a pointer-table base -- `(&PTR_X)[j]` strides one pointer = 4 bytes on -m32).  undefined1** gives
# both correctly.
defs_ptr  = ["#define %s (*(undefined1 **)(%s))" % (n, G(ptrsym[n])) for n in sorted(ptrsym)] if FLAT32 else []

# RULE 7 (unresolved indirect targets): func_0xADDR = a call to engine code Ghidra never promoted to
# a function (mid-function / register-only-indirect target). Stubbed K&R (any args) so the unit links;
# OPEN -- a live call here returns 0 instead of running the real code (Stage 1 must recover the target).
funcaddr = sorted(set(re.findall(r'\bfunc_0x[0-9a-fA-F]+\b', src)))
defs_func = ["int %s(){ return 0; }  /* OPEN: unresolved indirect/mid-function call target */" % f for f in funcaddr]

# RULE 8: unbound pseudo-vars declared as shared globals (only surface for compilation)
regs  = sorted(set(re.findall(r'\bregister0x[0-9a-fA-F]+\b', src)))
stks  = sorted(set(re.findall(r'\bstack0x[0-9a-fA-F]+\b', src)))
# leading-underscore param artifacts (e.g. `_param_7 = ...`) Ghidra emits undeclared, a dead-store
# scratch in mangled x87/BCD blocks -- same class as register0x/stack0x. Engine has none (byte-identical);
# they only appear in driver decompiles. Declared as shared globals so the unit links (OPEN).
uparm = sorted(set(re.findall(r'\b_param_[0-9]+\b', src)))
defs_reg = ["undefined2 %s;  /* OPEN: unbound register pseudo-var (register-file not modelled) */" % r for r in regs]
defs_stk = ["undefined1 %s;  /* OPEN: SP-relative stack slot modelled as a shared global */" % s for s in stks]
defs_reg += ["undefined4 %s;  /* OPEN: undeclared _param_ pseudo-var (dead scratch in mangled block) */" % u for u in uparm]

max_lin = max([0] + [_remap(v) for v in symL.values()])
if not (MODULE and FLAT32) and max_lin >= MEM_SIZE:
    sys.exit("assemble_fist: max flat linear 0x%x >= FIST_MEM_SIZE 0x%x -- enlarge g_mem" % (max_lin, MEM_SIZE))

# ---- forward declarations from the decompile markers ----
MARKER = re.compile(r'/\* ===== (\S+) @ ([0-9a-fA-F]+:[0-9a-fA-F]+) ===== \*/')
lines = src.split("\n")
sigs, seen = [], set()
for i, l in enumerate(lines):
    m = MARKER.match(l.strip())
    if not m or m.group(1) in seen:
        continue
    sig, j = [], i + 1
    # generous lookahead in driver mode: WARNING-comment blocks can precede a multi-line signature
    # (driver decompiles have many). Engine keeps the original 14 so its output stays byte-identical.
    _cap = 40 if MODULE else 14
    while j < len(lines) and j < i + _cap:
        s = lines[j].strip()
        if '{' in s:
            s = s[:s.find('{')].strip()
            if s and not s.startswith(('/*', '*')):
                sig.append(s)
            break
        if s and not s.startswith(('/*', '*')):
            sig.append(s)
        j += 1
    joined = ' '.join(sig)
    if '(' not in joined:
        continue
    seen.add(m.group(1)); sigs.append(joined + ';')

# ---- RULE 9: pad register-threaded calls that pass FEWER args than the callee declares -----------
# The __allregs / INT-threading model gives some hand-asm functions register PARAMS that a given call
# site leaves in registers (the decompiler judged them not-passed there), so `f()` calls a 4-param f.
# gcc rejects too-few-args to a prototyped function. Pad such calls to the declared arity with 0 (the
# value the decompiler deemed dead at that site). Fully general + reproducible; residual faithfulness
# gap (an asm-threaded register not modelled) is documented, same class as the other __allregs gaps.
def _argc(sig):
    if '(' not in sig or ')' not in sig: return 0
    p = sig[sig.index('(')+1:sig.rindex(')')].strip()
    if p == '' or p == 'void': return 0
    depth = 0; n = 1
    for ch in p:
        if ch in '([': depth += 1
        elif ch in ')]': depth -= 1
        elif ch == ',' and depth == 0: n += 1
    return n
argc = {}
for s in sigs:
    mm = re.search(r'\b([A-Za-z_]\w*)\s*\(', s)
    if mm: argc[mm.group(1)] = _argc(s)
CALLRE = re.compile(r'\b(FUN_[0-9a-fA-F]+_[0-9a-fA-F]+|thunk_[A-Za-z0-9_]+|app_entry)\s*\(')
inserts = []   # (pos_of_close_paren, text_to_insert)
n_pad = 0
for m in CALLRE.finditer(src):
    name = m.group(1)
    need = argc.get(name)
    if not need:      # unknown or zero-arg callee: nothing to pad
        continue
    i2 = m.end()      # first char inside the '(' (m.end() is just after '(')
    depth = 0; got_any = False; commas = 0
    while i2 < len(src):
        c = src[i2]
        if c in '([': depth += 1
        elif c in ')]':
            if depth == 0: break     # matching close of this call
            depth -= 1
        elif c == ',' and depth == 0: commas += 1
        if not c.isspace(): got_any = True
        i2 += 1
    if i2 >= len(src): continue      # i2 now indexes the matching ')'
    have = (commas + 1) if got_any else 0
    if have < need:
        txt = ",".join(["0"]*need) if have == 0 else ("," + ",".join(["0"]*(need-have)))
        inserts.append((i2, txt)); n_pad += 1
for pos, txt in sorted(inserts, reverse=True):
    src = src[:pos] + txt + src[pos:]

# ---- RULE 10 (driver mode): faithful DUPLICATE-LABEL dedup ---------------------------------------
# Ghidra sometimes splits a single real-mode string instruction (rep/repne scasb/movsb) into a loop
# and labels TWO blocks with the same code_r0xADDR name -> a C "duplicate label" error. The names are
# address-derived (unique per function), so a duplicated name's definitions are all in one function.
# Reconstruct the control flow faithfully: rename the 2nd+ definitions uniquely, and re-point each
# `goto NAME` to the nearest FOLLOWING definition (forward exit/continue), or -- if none follows (a
# backward branch) -- to the FIRST definition (the loop top). This is exactly the repne-scasb shape
# (check-at-top, body-below): the forward goto enters the body, the backward goto loops to the check.
# LOUD (prints each fix) + OPEN-documented; engine mode is untouched (no duplicated labels there).
if MODULE:
    _lines = src.split('\n')
    _labre = re.compile(r'^([ \t]*)(code_r0x[0-9a-fA-F]+):\s*$')
    _defidx = {}
    for _ix, _l in enumerate(_lines):
        _m = _labre.match(_l)
        if _m: _defidx.setdefault(_m.group(2), []).append(_ix)
    for _name, _idxs in _defidx.items():
        if len(_idxs) < 2:
            continue
        _idxs = sorted(_idxs)
        _fam = {_idxs[0]: _name}                       # lineno -> label name (D1 keeps the name)
        for _k, _ix in enumerate(_idxs[1:], start=2):
            _nm = "%s__%d" % (_name, _k)
            _fam[_ix] = _nm
            _ind = _labre.match(_lines[_ix]).group(1)
            _lines[_ix] = "%s%s:" % (_ind, _nm)
        _defpos = sorted(_fam.items())                 # [(lineno, name)] ascending
        _first = _defpos[0][1]
        _gotore = re.compile(r'goto %s;' % re.escape(_name))
        _ng = 0
        for _ix, _l in enumerate(_lines):
            if _gotore.search(_l):
                _following = [nm for (ln, nm) in _defpos if ln > _ix]
                _tgt = _following[0] if _following else _first     # forward: nearest below; backward: loop top
                _lines[_ix] = _gotore.sub("goto %s;" % _tgt, _l); _ng += 1
        print("assemble_fist: RULE10 deduped label %s (%d defs, %d gotos repointed) [OPEN: split string-op]"
              % (_name, len(_idxs), _ng))
    src = '\n'.join(_lines)

# ---- RULE 7b: referenced-but-undefined FUN_/thunk_ -> stub (OPEN: unpromoted mid-function target) --
# The decompile can call a code address it never promoted to a top-level function body (e.g. a
# mid-function entry the analyzer split differently). Emit a stub so the unit links; a live call is an
# OPEN item (returns without running the real code) -- same class as func_0x.
called_fns  = set(re.findall(r'\b(FUN_[0-9a-fA-F]+_[0-9a-fA-F]+|thunk_[A-Za-z0-9_]+)\s*\(', src))
undef_fns   = sorted(n for n in called_fns if n not in DEFINED_FNS)
defs_undef  = ["undefined4 %s(){ return 0; }  /* OPEN: referenced but not promoted to a function body */" % n
               for n in undef_fns]
# Ghidra FID library-signature FALSE POSITIVES: a misdisassembled data/SMC region (e.g. SOUNDDVR's
# runtime-patched dispatch stub at 0x12ab/0x12f5, flagged halt_baddata) sometimes emits a phantom call
# to a named library routine (TaskRegister) with no body.  These occur only in unreachable
# misdisassembled-data gap functions (never fmap-dispatched); stub to a no-op so the unit links.
GHIDRA_PHANTOMS = ('TaskRegister',)
phantom_fns = sorted(p for p in GHIDRA_PHANTOMS if re.search(r'\b%s\s*\(' % p, src) and p not in DEFINED_FNS)
defs_undef += ["undefined4 %s(){ return 0; }  /* OPEN: Ghidra FID phantom in misdisassembled data */" % p
               for p in phantom_fns]

# ---- DRIVER MODE: namespace every function-like identifier so the driver unit's symbols do not
# collide with the engine unit's identically-named ones (both have app_entry / FUN_0000_* / func_0x*
# / register0x* at low seg:off). A preprocessor #define per name rewrites its DEFINITION and every
# reference (incl. the same-TU fmap's &FUN) uniformly to m_<mod>_<name> -- external-linkage collisions
# gone, with zero surgery on the pristine bodies. The driver's exported globals (fist_<mod>_fmap,
# _fmap_n, _base) are NOT namespaced (fist_modules.c wires them). Engine mode emits none of this.
defs_ns = []
if MODULE:
    ns_names = set(DEFINED_FNS) | set(undef_fns) | set(phantom_fns) | set(funcaddr) | set(regs) | set(stks) | set(uparm)
    defs_ns = ['/* ---- driver-unit symbol namespacing (avoid engine-unit link collisions) ---- */']
    defs_ns += ["#define %s m_%s_%s" % (n, MODULE, n) for n in sorted(ns_names)]

if MODULE:
    P = ['/* re_out/fist_%s.c -- Armored Fist %s DRIVER-OVERLAY translation unit.' % (MODULE, MODULE.upper()),
         ' * GENERATED by tools/assemble_fist.py from the pristine driver decompile. DO NOT EDIT.',
         ' * Corrections go in patches/NNN-*.diff (applied by `make patch`); regenerate with `make assemble`.',
         ' */',
         '#include "ghidra_compat.h"', '',
         '/* runtime load base (module-relative accessors add it); defined at end, declared here */',
         'extern uint32_t %s;' % BASE_SYM, '']
    P += defs_ns
else:
    P = ['/* re_out/fist.c -- Armored Fist ENGINE translation unit.',
         ' * GENERATED by tools/assemble_fist.py from the pristine re_out/fist_decomp.c. DO NOT EDIT.',
         ' * Corrections go in patches/NNN-*.diff (applied by `make patch`); regenerate with `make assemble`.',
         ' */',
         '#include "ghidra_compat.h"', '']
P += ['/* ---- RULE 7: unresolved indirect/mid-function call targets, stubbed (OPEN) ---- */']
P += defs_func
P += ['/* ---- RULE 7b: referenced-but-undefined FUN_/thunk_ targets, stubbed (OPEN) ---- */']
P += defs_undef
P += ['/* ---- RULE 8: unbound register/stack pseudo-vars (OPEN: shared globals, not the real files) ---- */']
P += defs_reg + defs_stk
P += ['', '/* ---- forward declarations (every engine function; from the decompile markers) ---- */']
P += sigs
P += ['', '/* ---- RULE 2: DAT_SEG_OFF -> g_mem accessor at linear (SEG<<4)+OFF ---- */']
P += defs_dat
P += ['/* ---- RULE 3: s_ string symbols -> pointer to string bytes in g_mem ---- */']
P += defs_str
P += ['/* ---- RULE 4: absolute RAM accessors -> g_mem ---- */']
P += defs_ram
if defs_code:
    P += ['/* ---- RULE 7: _FUN/_thunk_FUN code-address data locations (SMC / far-ptr) -> g_mem ---- */']
    P += defs_code
if defs_lab:
    P += ['/* ---- RULE 7: address-taken jump targets (OPEN: unresolved indirect dispatch) -> g_mem ---- */']
    P += defs_lab
if defs_ptr:
    P += ['/* ---- FLAT32: PTR_ pointer-typed data slots -> g_mem (4-byte) ---- */']
    P += defs_ptr
P += ['', '/* ===================== engine function bodies (verbatim from the decompile) ===================== */']

# ---- RULE 2: the indirect-call dispatcher's {linear (SEG<<4)+OFF -> &FUN} map (fist_icall.c) ----
# Sorted, deduped (first name per linear). Emitted AFTER the bodies so every &FUN is defined; the
# forward decls at the top already declare them, so taking &FUN is well-formed anywhere below.
fmap = {}
for _nm, _lin in FN_LINEAR.items():
    fmap.setdefault(_lin, _nm)
FMAP = ['']
if MODULE:
    # Driver-overlay unit: the FUN map is keyed by BASE-0 MODULE OFFSETS (fist_icall subtracts the
    # runtime load base to look a call target up here); the base global is set at load by
    # fist_ovl_register and added by every module-relative g_mem accessor above.
    FMAP += ['/* ---- driver-overlay runtime base (set at INT 21h AH=4B load; accessors add it) ---- */',
             'uint32_t %s = 0;' % BASE_SYM,
             '/* ---- module {offset -> &FUN} map, sorted for fist_icall overlay-range dispatch ---- */',
             'const struct fist_fent %s[] = {' % FMAP_NAME]
    FMAP += ["  {0x%xu,(void*)&%s}," % (lin, fmap[lin]) for lin in sorted(fmap)]
    FMAP += ['};', 'const unsigned %s_n = sizeof(%s)/sizeof(%s[0]);' % (FMAP_NAME, FMAP_NAME, FMAP_NAME)]
else:
    FMAP += ['/* ---- RULE 2: indirect-call FUN map (linear -> &FUN), sorted for fist_icall binary search ---- */',
             'const struct fist_fent fist_fmap[] = {']
    FMAP += ["  {0x%xu,(void*)&%s}," % (lin, fmap[lin]) for lin in sorted(fmap)]
    FMAP += ['};', 'const unsigned fist_fmap_n = sizeof(fist_fmap)/sizeof(fist_fmap[0]);']

open(OUT, 'w', encoding='utf-8', errors='surrogateescape').write(
    "\n".join(P) + "\n" + src + "\n" + "\n".join(FMAP) + "\n")

print("assemble_fist: icall near=%d far=%d (far-fixed-loc=%d) fmap=%d | INT-dispatch hooks=%d | arg-pad calls=%d"
      % (n_near, n_far + n_fardat, n_fardat, len(fmap), _n_hook, n_pad))
print("assemble_fist: %d fns, %d fwd decls | DAT %d(call=%d ptr=%d byte=%d _32=%d 16=%d) | s_=%d xRam=%d "
      "code=%d lab=%d | reg=%d stk=%d func_0x=%d | retyped-void=%d | max 0x%x/0x%x -> %s"
      % (src.count('/* ===== '), len(sigs), len(defs_dat), n_call, n_ptr, n_byte, n_s32, n_s16,
         len(defs_str), len(defs_ram), len(defs_code), len(defs_lab), len(defs_reg), len(defs_stk),
         len(defs_func), len(retype), max_lin, MEM_SIZE, os.path.relpath(OUT, ROOT)))
