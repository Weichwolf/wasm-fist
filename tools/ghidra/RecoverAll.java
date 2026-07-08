// wasm-fist postScript: maximize code coverage of the 16-bit engine, iterating to a fixpoint.
// Runs AFTER auto-analysis, BEFORE ExportDecomp. Three levers, looped until nothing new appears:
//
//   (T) JUMP/CALL TABLE RECOVERY  <-- top priority (each resolved table = one patch we don't write later)
//       Find every computed jmp/call with an index register + a static table base (DS/CS/SS relative),
//       enumerate ALL entries (near word / far dword), promote every target to code+function, and add
//       COMPUTED_JUMP/COMPUTED_CALL references so the decompiler renders the dispatch instead of an
//       opaque indirect call. NovaLogic engines use big message/opcode dispatch tables.
//   (P) PROLOGUE SCAN  - disassemble undefined bytes in the code region that begin with a 16-bit C
//       prologue (55 8B EC / 55 89 E5 / C8 enter) and create functions. Recovers indirect-only code
//       Ghidra never flowed to.
//   (C) CALL-TARGET PROMOTION - any call reference into the code region whose target is not yet a
//       function becomes one.
//
// After each pass, re-run analysis so new code exposes new calls/tables. Env:
//   FIST_CODE_END (default 0x1c000)  code/data boundary; DGROUP data begins here.
//   FIST_DS (0x1c00) FIST_SS (0x2ba9) segment paragraph values (must match PrepAnalysis).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.lang.Register;
import ghidra.program.model.symbol.*;
import ghidra.app.cmd.function.CreateFunctionCmd;
import java.util.*;

public class RecoverAll extends GhidraScript {
    long CODE_END, DS_BASE, SS_BASE;
    Memory mem;
    ReferenceManager refm;
    FunctionManager fm;
    Set<String> SEGREGS = new HashSet<>(Arrays.asList("CS","DS","ES","SS","FS","GS"));
    // inventory rows for the report
    List<String> tableLog = new ArrayList<>();
    int tblTargets = 0;

    long envHex(String k, long d){ String s=System.getenv(k); try{ return (s==null||s.isEmpty())?d:Long.decode(s);}catch(Exception e){return d;} }
    Address at(long lin){ return toAddr(lin); }
    boolean inCode(long lin){ return lin>=0x4 && lin<CODE_END; }
    int rd16(long lin){ try{ return mem.getShort(at(lin))&0xffff; }catch(Exception e){ return -1; } }

    public void run() throws Exception {
        CODE_END = envHex("FIST_CODE_END", 0x1c000);
        DS_BASE  = envHex("FIST_DS", 0x1c00)*16;
        SS_BASE  = envHex("FIST_SS", 0x2ba9)*16;
        mem = currentProgram.getMemory();
        refm = currentProgram.getReferenceManager();
        fm = currentProgram.getFunctionManager();

        int pass=0;
        while (pass < 10) {
            int before = fm.getFunctionCount();
            int t = recoverTables();
            int p = prologueScan();
            int c = promoteCallTargets();
            analyzeChanges(currentProgram);
            int after = fm.getFunctionCount();
            println(String.format("RECOVER pass %d: tables+%d prologue+%d calls+%d  fns %d->%d",
                    pass, t, p, c, before, after));
            if (t==0 && p==0 && c==0 && after==before) break;
            pass++;
        }
        // final lever: disassemble frameless-code gaps the prologue scan can't see (functions that
        // start with mov/xor/push-ax rather than 55 8B EC). Conservative: skips zero-fill/BSS and
        // word-string data tables embedded in the code segment. Then one more fixpoint sweep.
        int g = gapScan();
        pass=0;
        while (pass < 6) {
            int before = fm.getFunctionCount();
            int t = recoverTables();
            int p = prologueScan();
            int c = promoteCallTargets();
            analyzeChanges(currentProgram);
            int after = fm.getFunctionCount();
            println(String.format("RECOVER post-gap pass %d: gap+%d tables+%d prologue+%d calls+%d  fns %d->%d",
                    pass, (pass==0?g:0), t, p, c, before, after));
            if (t==0 && p==0 && c==0 && after==before) break;
            pass++;
        }
        println("TABLE_INVENTORY count=" + tableLog.size() + " promoted_targets=" + tblTargets);
        for (String s : tableLog) println("TABLE| " + s);
    }

    // ---- (T) jump/call table recovery ----
    // Contiguous dispatch-table arrays (NovaLogic packs per-type handler tables back-to-back) are bounded
    // by knowing every table base up-front: a near table stops enumerating at the next table's base.
    TreeSet<Long> allBases = new TreeSet<>();
    int recoverTables() throws Exception {
        int made=0;
        List<Instruction> work = new ArrayList<>();
        InstructionIterator it = currentProgram.getListing().getInstructions(true);
        while (it.hasNext()) {
            Instruction i = it.next();
            FlowType ft = i.getFlowType();
            if (ft.isComputed() && (ft.isJump()||ft.isCall())) work.add(i);
        }
        // phase 1: collect all statically-resolvable table bases (for precise contiguous bounds)
        for (Instruction i : work) { Long b = tableBase(i); if (b!=null) allBases.add(b); }
        // phase 2: enumerate + promote
        for (Instruction i : work) made += recoverOne(i);
        return made;
    }

    // compute the linear table base for a computed jmp/call, or null if not a static-base indexed table
    Long tableBase(Instruction i) {
        boolean hasIndex=false; Scalar disp=null; String seg=null;
        for (int op=0; op<i.getNumOperands(); op++)
            for (Object o : i.getOpObjects(op)) {
                if (o instanceof Register) {
                    String rn=((Register)o).getName();
                    if (SEGREGS.contains(rn)) seg=rn;
                    else if (rn.equals("BX")||rn.equals("SI")||rn.equals("DI")||rn.equals("BP")) hasIndex=true;
                } else if (o instanceof Scalar) disp=(Scalar)o;
            }
        if (!hasIndex || disp==null) return null;
        int instrSeg=((SegmentedAddress)i.getAddress()).getSegment();
        long tableSeg;
        if (seg==null||seg.equals("DS")) tableSeg=DS_BASE>>4;
        else if (seg.equals("CS")) tableSeg=instrSeg;
        else if (seg.equals("SS")) tableSeg=SS_BASE>>4;
        else return null;
        return (tableSeg<<4) + disp.getUnsignedValue();
    }

    // dedupe tables already processed (by base+kind)
    Set<String> doneTables = new HashSet<>();

    int recoverOne(Instruction i) throws Exception {
        FlowType ft = i.getFlowType();
        boolean isCall = ft.isCall();
        boolean far = i.getMnemonicString().toUpperCase().contains("F"); // CALLF / JMPF
        // find index register + scalar disp + segment override among op objects
        boolean hasIndex=false; Scalar disp=null; String seg=null;
        for (int op=0; op<i.getNumOperands(); op++) {
            for (Object o : i.getOpObjects(op)) {
                if (o instanceof Register) {
                    String rn = ((Register)o).getName();
                    if (SEGREGS.contains(rn)) seg = rn;
                    else if (rn.equals("BX")||rn.equals("SI")||rn.equals("DI")||rn.equals("BP")) hasIndex=true;
                } else if (o instanceof Scalar) {
                    disp = (Scalar)o;
                }
            }
        }
        if (!hasIndex || disp==null) return 0;          // not a static-base indexed table
        int instrSeg = ((SegmentedAddress)i.getAddress()).getSegment();
        Long baseObj = tableBase(i);
        if (baseObj==null) return 0;
        long tableLin = baseObj;
        int elem = far?4:2;
        String key = tableLin+":"+elem+":"+isCall;
        if (!doneTables.add(key)) return 0;
        // precise upper bound: the next distinct table base above this one (contiguous dispatch arrays)
        Long nb = allBases.higher(tableLin);
        long nextBase = (nb==null) ? Long.MAX_VALUE : nb;
        // enumerate
        int cap = 512, n=0, promoted=0;
        List<Long> targets = new ArrayList<>();
        for (int k=0; k<cap; k++) {
            long e = tableLin + (long)k*elem;
            if (e >= nextBase) break;                    // reached the next table in a contiguous array
            long tgt;
            if (far) {
                int off=rd16(e), sg=rd16(e+2);
                if (off<0||sg<0) break;
                if (off==0 && sg==0) break;              // far terminator
                tgt = ((long)sg<<4) + off;
            } else {
                int w=rd16(e);
                if (w<0) break;
                if (w==0 || w==0xffff) break;            // near terminator
                tgt = ((long)instrSeg<<4) + w;
            }
            if (!inCode(tgt)) break;
            // reject mid-instruction targets (guards against running off the table end into data)
            Address ta = at(tgt);
            Instruction cont = getInstructionContaining(ta);
            if (cont!=null && !cont.getAddress().equals(ta)) break;
            Data dcont = getDataContaining(ta);
            if (dcont!=null && dcont.isDefined()) break;
            targets.add(tgt);
            n++;
        }
        if (n==0) return 0;
        int opIdx = i.getNumOperands()>0 ? 0 : 0;
        RefType rt = isCall ? RefType.COMPUTED_CALL : RefType.COMPUTED_JUMP;
        for (long tgt : targets) {
            Address ta = at(tgt);
            try { disassemble(ta); } catch (Exception ex) {}
            refm.addMemoryReference(i.getAddress(), ta, rt, SourceType.USER_DEFINED, Reference.MNEMONIC);
            if (isCall && fm.getFunctionAt(ta)==null) {
                CreateFunctionCmd cmd = new CreateFunctionCmd(ta);
                if (cmd.applyTo(currentProgram, monitor)) { promoted++; }
            } else if (!isCall && fm.getFunctionAt(ta)==null) {
                promoted++; // case block, will be folded into its function
            }
        }
        tblTargets += n;
        tableLog.add(String.format("@%s %s %s base=lin:0x%05x elem=%d entries=%d seg=%s",
                i.getAddressString(false,true), isCall?"CALL":"JMP", far?"far":"near",
                tableLin, elem, n, (seg==null?"DS":seg)));
        return 1;
    }

    // ---- (P) prologue scan ----
    int prologueScan() throws Exception {
        int made=0;
        long lin = 0x4;
        while (lin < CODE_END) {
            Address a = at(lin);
            Instruction ins = getInstructionAt(a);
            if (ins!=null) { lin += ins.getLength(); continue; }
            Data d = getDataAt(a);
            if (d!=null && d.isDefined()) { lin += Math.max(1,d.getLength()); continue; }
            // undefined byte: check prologue signature
            int b0=rd16(lin)&0xff, b1=(rd16(lin)>>8)&0xff, b2=rd16(lin+1)&0xff;
            boolean prologue =
                (b0==0x55 && b1==0x8b && b2==0xec) ||    // push bp; mov bp,sp
                (b0==0x55 && b1==0x89 && b2==0xe5) ||    // push bp; mov bp,sp (att-enc)
                (b0==0xc8);                              // enter
            if (prologue) {
                try {
                    disassemble(a);
                    if (getInstructionAt(a)!=null && fm.getFunctionAt(a)==null) {
                        CreateFunctionCmd cmd = new CreateFunctionCmd(a);
                        if (cmd.applyTo(currentProgram, monitor)) made++;
                    }
                } catch (Exception ex) {}
            }
            lin += 1;
        }
        return made;
    }

    // ---- (G) frameless-code gap disassembly ----
    // For each undefined run in the code region, decide code-vs-data, and if code, disassemble it and
    // make a function at its start. Data discriminators (skip): >40% zero bytes (BSS/padding), or the
    // XX 00 XX 00 wide-string/word-table pattern, or a non-code lead byte.
    int gapScan() throws Exception {
        int made=0;
        long lin = 0x4;
        while (lin < CODE_END) {
            Address a = at(lin);
            Instruction ins = getInstructionAt(a);
            if (ins!=null) { lin += ins.getLength(); continue; }
            Data d = getDataAt(a);
            if (d!=null && d.isDefined()) { lin += Math.max(1,d.getLength()); continue; }
            // start of an undefined run; measure it
            long s = lin, e = lin;
            while (e < CODE_END) {
                Address ae = at(e);
                if (getInstructionAt(ae)!=null) break;
                Data de = getDataAt(ae);
                if (de!=null && de.isDefined()) break;
                e++;
            }
            long len = e - s;
            if (len >= 6 && looksLikeCode(s, len)) {
                try {
                    disassemble(a);
                    Instruction got = getInstructionAt(a);
                    if (got!=null) {
                        // require at least 2 instructions decoded before creating a function
                        Instruction nxt = getInstructionAfter(got);
                        if (nxt!=null && nxt.getAddress().getOffset() < e + 32 && fm.getFunctionAt(a)==null) {
                            CreateFunctionCmd cmd = new CreateFunctionCmd(a);
                            if (cmd.applyTo(currentProgram, monitor)) made++;
                        }
                    }
                } catch (Exception ex) {}
            }
            lin = Math.max(e, lin + 1);
        }
        return made;
    }
    boolean looksLikeCode(long s, long len) {
        try {
            long span = Math.min(len, 64);
            int zeros=0, oddZeros=0, oddTot=0;
            for (long k=0;k<span;k++) {
                int b = mem.getByte(at(s+k))&0xff;
                if (b==0) zeros++;
                if ((k&1)==1) { oddTot++; if (b==0) oddZeros++; }
            }
            if (zeros*100 > span*40) return false;                 // zero-fill / BSS
            if (oddTot>0 && oddZeros*100 >= oddTot*90) return false; // XX 00 XX 00 word-string table
            int b0 = mem.getByte(at(s))&0xff;
            if (b0==0x00 || b0==0xff) return false;                 // padding lead
            return true;
        } catch (Exception e) { return false; }
    }

    // ---- (C) call-target promotion ----
    int promoteCallTargets() throws Exception {
        int made=0;
        List<Address> tgts = new ArrayList<>();
        InstructionIterator it = currentProgram.getListing().getInstructions(true);
        while (it.hasNext()) {
            Instruction i = it.next();
            FlowType ft = i.getFlowType();
            if (!ft.isCall()) continue;
            for (Reference r : i.getReferencesFrom()) {
                if (!r.getReferenceType().isCall()) continue;
                Address ta = r.getToAddress();
                if (ta==null || !ta.isMemoryAddress()) continue;
                if (!inCode(ta.getOffset())) continue;
                if (fm.getFunctionAt(ta)==null) tgts.add(ta);
            }
        }
        for (Address ta : tgts) {
            if (fm.getFunctionAt(ta)!=null) continue;
            try { disassemble(ta); } catch (Exception e) {}
            CreateFunctionCmd cmd = new CreateFunctionCmd(ta);
            if (cmd.applyTo(currentProgram, monitor)) made++;
        }
        return made;
    }
}
