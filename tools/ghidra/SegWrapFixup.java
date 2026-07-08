// wasm-fist postScript: fix real-mode near-call/near-jump SEGMENT-WRAP mis-resolution at the
// decompile stage (the "code-overlay / per-region CS" class).
//
// THE DEFECT (oracle + disasm verified). The engine image is imported FLAT (x86:LE:16 Real Mode,
// base 0), so Ghidra addresses every byte in a fixed grid of 64 KB "page segments" (0x0000 for
// linear 0..0xffff, 0x1000 for 0x10000..0x1ffff, ...). But a large CRT/service cluster runs with a
// NON-ZERO CS -- segment 0xf69 (linear window 0xf690..0x1f68f, straddling 0x10000), entered via
// `call far 0xf69:xxx`. A near `call rel16` / `jmp rel16` there wraps its target within the 0xf69
// 64 KB window; Ghidra wraps it within the WRONG page window (at 0xffff / 0x10000) instead, so the
// target resolves in the 0-page rather than the 0xf69 window. Proven: fd1c's `push cs; call 0x2380`
// must reach linear 0x12380 (FUN_1000_2380 = DOS getdrive) but Ghidra resolved FUN_0000_2380.
// (Far call/jmp carry their segment explicitly -> already correct; only NEAR rel flows are victims.)
//
// THE FIX (surgical, reproducible, NO re-home / NO rename -> zero blast radius outside the cluster):
//   1. Discover the non-zero CS segments from every direct far call/jmp whose segment operand names
//      a window Ghidra's page grid can't form (default: the seg is used by >= FIST_CS_MINSITES sites,
//      which isolates 0xf69 from byte-pattern noise). Each such seg has a 64 KB window [seg<<4, +64K)
//      that OVERLAPS the page grid -- e.g. 0xf69's window 0xf690..0x1f68f overlaps page 0x1000, so
//      cluster (CS=0xf69) code and normal (CS=0x1000) engine code physically coexist at 0x10000+.
//   2. Per-instruction validity-guided scan over each CS window (reachability-free, so it also fixes
//      cluster functions reached only by INDIRECT dispatch, which a call-graph BFS misses). For each
//      NEAR rel call/jmp at site S in the window, recompute the correct in-window target
//         ft = base + ((S + len + rel - base) & 0xffff),  base = CS<<4
//      and compare to Ghidra's current (page-wrapped) flow reference `cur`. If they differ, decide
//      whether S truly executes with this CS:
//        - S below the seg's next 64 KB page boundary (the sub-window the page grid CANNOT represent,
//          e.g. 0xf690..0xffff) => S is CERTAINLY this CS -> fix to ft;
//        - S in the overlap (page-seg is a real seg too, e.g. 0x1000) => fix only if ft is decoded
//          code and `cur` is NOT (a backward cross-window call resolves under the page grid to a
//          linear beyond the code region == garbage; under this CS it hits a real CRT helper). This
//          validity test distinguishes a mislabelled FUN_1000_* cluster function from genuine CS=0x1000
//          engine code without needing reachability.
//      The correct target is ALREADY a decoded function/instruction in the flat image (both 0x2380 and
//      0x12380 are real code); re-pointing the flow reference makes the decompiler emit the right
//      callee (calls) / re-route the CFG (jumps). Function linear addresses, names, the FUN map and
//      the indirect-call dispatcher are all untouched.
//
// Generalised over ALL non-zero CS segments discovered from far targets (env FIST_CS_SEGS can pin a
// list; default = auto-discover any far-call/jmp target segment that is not its own page segment and
// carries >= FIST_CS_MINSITES sites, which isolates 0xf69 from byte-pattern noise).
//
// Runs AFTER RecoverAll (functions/instructions exist) and BEFORE ApplyConv (so the corrected call
// graph threads registers) / ExportDecomp. Context+refs live in the program DB, so a fresh
// `make decompile FIST_FRESH=1` and a reuse re-export emit the same corrected C.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.util.*;

public class SegWrapFixup extends GhidraScript {
    Address MIN;
    long lin(Address a){ return a.subtract(MIN); }          // flat linear (MIN = toAddr(0))
    Address at(long l){ return toAddr(l & 0xfffffL); }

    public void run() throws Exception {
        Program p = currentProgram;
        MIN = p.getMinAddress();
        ReferenceManager refm = p.getReferenceManager();
        Listing lst = p.getListing();
        FunctionManager fm = p.getFunctionManager();

        int minSites = (int) envInt("FIST_CS_MINSITES", 8);
        long codeEnd = envInt("FIST_CODE_END", 0x1c000);

        // ---- (1) discover non-zero CS segments from direct far call/jmp target operands ----
        // Map seg -> {entry linear targets} and seg -> site count. A "real" CS segment is one whose
        // seg operand != the page segment of its own target (so it names a window Ghidra can't form)
        // and which is used by many sites (isolates the true cluster from random 9a/ea data bytes).
        Map<Integer,Set<Long>> segEntries = new TreeMap<>();
        Map<Integer,Integer> segSites = new HashMap<>();
        String pin = System.getenv("FIST_CS_SEGS");
        Set<Integer> pinned = new HashSet<>();
        if (pin != null && !pin.isEmpty())
            for (String s : pin.split("[ ,]+")) if (!s.isEmpty()) pinned.add((int) Long.decode(s.trim()).longValue());

        InstructionIterator it = lst.getInstructions(true);
        while (it.hasNext()) {
            Instruction ins = it.next();
            byte[] b = ins.getBytes();
            int op = b[0] & 0xff;
            if (op != 0x9a && op != 0xea) continue;          // direct far call / far jmp
            if (b.length < 5) continue;
            int off = (b[1] & 0xff) | ((b[2] & 0xff) << 8);
            int seg = (b[3] & 0xff) | ((b[4] & 0xff) << 8);
            long tgt = ((long) seg << 4) + off;
            if (seg == 0 || tgt < 4 || tgt >= codeEnd) continue;
            int pageSeg = (int) ((tgt >> 16) << 12);
            if (seg == pageSeg) continue;                    // representable page seg -> not a victim
            segEntries.computeIfAbsent(seg, k -> new TreeSet<>()).add(tgt);
            segSites.merge(seg, 1, Integer::sum);
        }
        // choose the CS segments to process
        List<Integer> csList = new ArrayList<>();
        for (Map.Entry<Integer,Integer> e : segSites.entrySet())
            if (pinned.contains(e.getKey()) || e.getValue() >= minSites) csList.add(e.getKey());
        if (!pinned.isEmpty()) { csList.clear(); csList.addAll(pinned); }
        Collections.sort(csList);
        println("SEGWRAP discovered non-zero CS segments (site>=" + minSites + "): " + hexList(csList));
        for (int seg : segSites.keySet())
            if (segSites.get(seg) >= 2)
                println("SEGWRAP   seg 0x" + Integer.toHexString(seg) + " : " + segSites.get(seg)
                        + " far sites, " + segEntries.get(seg).size() + " entries"
                        + (csList.contains(seg) ? "  <== PROCESS" : ""));

        int tx = p.startTransaction("seg-wrap near-flow fixup");
        long fixCalls = 0, fixJumps = 0, skipAmbig = 0, unresolvable = 0;
        Set<Long> fnHits = new HashSet<>();
        try {
            for (int seg : csList) {
                long base = (long) seg << 4;
                long winLo = base, winHi = base + 0x10000;        // [base, base+64K)
                long certainHi = ((base >> 16) + 1) << 16;        // next 64 KB page boundary (0x10000)
                long segFix = 0, segJmp = 0;
                InstructionIterator si = lst.getInstructions(true);
                while (si.hasNext()) {
                    Instruction ins = si.next();
                    long l = lin(ins.getMinAddress());
                    if (l < winLo || l >= winHi) continue;        // outside this CS window
                    byte[] b = ins.getBytes();
                    int i = 0;
                    while (i < b.length && isPrefix(b[i] & 0xff)) i++;
                    int op = b[i] & 0xff;
                    Integer rel = null; boolean isCall = false, uncond = false;
                    if (op == 0xe8 || op == 0xe9) { rel = s16(b, i+1); isCall = (op==0xe8); uncond = (op==0xe9); }
                    else if (op == 0xeb) { rel = s8(b, i+1); uncond = true; }
                    else if ((op >= 0x70 && op <= 0x7f) || op == 0xe3) { rel = s8(b, i+1); }
                    else if (op == 0x0f && i+1 < b.length && (b[i+1]&0xff) >= 0x80 && (b[i+1]&0xff) <= 0x8f) { rel = s16(b, i+2); }
                    if (rel == null) continue;                    // far / indirect / non-flow: leave alone

                    long len = ins.getLength();
                    long ft = base + ((l + len + rel - base) & 0xffff);     // correct in-CS-window target
                    long pageBase = (l >> 16) << 16;
                    long pt = pageBase + ((l + len + rel - pageBase) & 0xffff);  // page-grid target Ghidra uses
                    if (ft == pt) continue;                       // no window cross -> Ghidra already correct
                    Address curTgt = primaryFlowTarget(ins);
                    long cur = (curTgt != null) ? lin(curTgt) : -1;
                    if (cur == ft) continue;                      // already fixed / correct

                    if (ft < 4 || ft >= codeEnd) { unresolvable++; continue; }  // ft must be cluster code
                    // The correct in-window target may not have been decoded (Ghidra followed the wrapped
                    // WRONG flow), so force-disassemble it before judging validity.
                    Address fta = at(ft);
                    if (lst.getInstructionAt(fta) == null) { try { disassemble(fta); } catch (Exception ex) {} }
                    boolean ftValid = lst.getInstructionAt(fta) != null;
                    if (!ftValid) { unresolvable++; continue; }   // correct target isn't code -> leave alone
                    // certain sub-window (page grid can't form it, e.g. 0xf690..0xffff): this CS for sure.
                    // overlap (page-seg is a real seg too): this CS iff the PAGE-grid target lands OUTSIDE
                    // the engine code region [4, codeEnd). A genuine CS=0x1000 call keeps its target inside
                    // the code region under the page grid; a wrap-victim's page target lands in the
                    // dispatch-table / resource-data gap beyond codeEnd (where RecoverAll may even have
                    // promoted a SPURIOUS function from the mis-wrapped references -- so a function-level
                    // test is NOT enough; the code-region boundary is the reliable discriminator).
                    boolean certain = l < certainHi;
                    boolean ptInCode = pt >= 4 && pt < codeEnd;
                    boolean doFix = certain || !ptInCode;
                    if (!doFix) { skipAmbig++; continue; }

                    repointFlow(refm, ins, at(ft), isCall, uncond);
                    if (isCall) { fixCalls++; segFix++; } else { fixJumps++; segJmp++; }
                    Function f = fm.getFunctionContaining(ins.getMinAddress());
                    if (f != null) fnHits.add(lin(f.getEntryPoint()));
                }
                println("SEGWRAP seg 0x" + Integer.toHexString(seg) + ": window 0x"
                        + Long.toHexString(winLo) + "..0x" + Long.toHexString(winHi)
                        + " fixedCalls=" + segFix + " fixedJumps=" + segJmp);
            }
        } finally {
            p.endTransaction(tx, true);
        }
        println("SEGWRAP DONE fixedCalls=" + fixCalls + " fixedJumps=" + fixJumps
                + " funcsTouched=" + fnHits.size() + " skippedAmbiguous=" + skipAmbig
                + " unresolvable=" + unresolvable);
    }

    // re-point the (single) flow reference of a rel call/jmp to the correct target.
    private void repointFlow(ReferenceManager refm, Instruction ins, Address tgt,
                             boolean isCall, boolean uncond) {
        int opIndex = 0;
        for (Reference r : ins.getReferencesFrom()) {
            RefType rt = r.getReferenceType();
            if (rt.isFlow() && !rt.isIndirect()) { opIndex = r.getOperandIndex(); refm.delete(r); }
        }
        RefType nt = isCall ? RefType.UNCONDITIONAL_CALL
                            : (uncond ? RefType.UNCONDITIONAL_JUMP : RefType.CONDITIONAL_JUMP);
        Reference nr = refm.addMemoryReference(ins.getMinAddress(), tgt, nt,
                                               SourceType.USER_DEFINED, opIndex);
        refm.setPrimary(nr, true);
    }

    private Address primaryFlowTarget(Instruction ins) {
        Reference pr = null;
        for (Reference r : ins.getReferencesFrom()) {
            RefType rt = r.getReferenceType();
            if (rt.isFlow() && !rt.isIndirect()) { if (r.isPrimary()) return r.getToAddress(); pr = r; }
        }
        return pr != null ? pr.getToAddress() : null;
    }

    private static boolean isPrefix(int x){
        switch (x){ case 0x26: case 0x2e: case 0x36: case 0x3e: case 0x64: case 0x65:
                    case 0x66: case 0x67: case 0xf0: case 0xf2: case 0xf3: return true; }
        return false;
    }
    private static boolean isRet(int op){
        return op==0xc3||op==0xc2||op==0xcb||op==0xca||op==0xcf;   // ret/retf/iret
    }
    private static int s8(byte[] b, int i){ int v = b[i]&0xff; return (v>=0x80)? v-0x100 : v; }
    private static int s16(byte[] b, int i){ int v = (b[i]&0xff)|((b[i+1]&0xff)<<8); return (v>=0x8000)? v-0x10000 : v; }
    private long envInt(String k, long def){ String s=System.getenv(k); try { return (s==null||s.isEmpty())?def:Long.decode(s); } catch(Exception e){ return def; } }
    private static String hexList(List<Integer> l){ StringBuilder sb=new StringBuilder(); for(int x:l) sb.append("0x").append(Integer.toHexString(x)).append(" "); return sb.toString().trim(); }
}
