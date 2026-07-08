// wasm-fist postScript: thread hand-written-asm register dataflow into explicit C params/returns.
// Runs AFTER RecoverAll (all functions discovered), BEFORE ExportDecomp.
//
// WHY: the engine is hand-written asm (2242/2245 frameless; Kyle Freeman) with NO uniform C calling
// convention. Register dataflow (AX/BX/CX/DX/SI/DI/BP + their 32-bit forms) is the inter-function
// interface, so the stock x86-16 default_proto (__stdcall16near: only stack inputs, SI/DI/BP unaffected)
// makes the decompiler emit ~43k free-floating in_*/unaff_* register pseudo-vars.
//
// FIX (fully in the program DB, no edit to the shared Ghidra install):
//   1) Install a custom "__allregs" prototype model via SpecExtension: all GP regs (AX/BX/CX/DX/SI/DI/BP
//      + 32-bit aliases) are candidate INPUTS; AX/EAX is the OUTPUT; only the structural regs
//      (SP, DS/CS/ES/SS, DF) are unaffected -> register values thread through calls as typed params.
//   2) Assign __allregs to every function and reset its signature source to DEFAULT (the auto-analysis
//      "Decompiler Parameter ID" pass locked them to (void) under the stock model; we unlock so the
//      decompiler re-detects inputs against __allregs).
//   3) Decompile each function, read the decompiler's INPUT VARNODES directly (hf.locRange, isInput) and
//      install them as CUSTOM_STORAGE parameters (the per-function SUBSET of GP registers + stack slots
//      actually read-before-write), preserving the detected AX/EAX return. We deliberately do NOT use
//      HighFunctionDBUtil.commitParamsToDatabase: the decompiler only promotes a register to a formal
//      parameter when the model's input pentries fill contiguously from slot 1, so a function using e.g.
//      DX/SI/DI but not AX/BX/CX would keep them as free-floating in_DI/in_SI. Reading the input varnodes
//      captures every used register regardless of slot order. Iterated to a fixpoint (a function's
//      register inputs only surface once its callees have committed signatures).
//
// Variant (env FIST_CONV_VARIANT, default A):
//   A = faithful/aggressive: SI/DI/BP are inputs and NOT unaffected (killed-by-call). Threads pointer
//       registers (e.g. rep-stosw DI dst) as params. Default.
//   B = conservative: SI/DI/BP are inputs AND unaffected (callee-saved-and-arg, Watcom-like). Fewer
//       caller-side reloads but leaves unaff_SI/DI on genuine pointer-in-SI/DI callees.
// FLAGS/CF: x86-16 has no flag-register calling-convention slot in Ghidra; CF/status returns stay as
//   in_CF (documented patch class). See report.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.database.SpecExtension;
import ghidra.program.model.symbol.SourceType;
import java.util.*;

public class ApplyConv extends GhidraScript {
    public void run() throws Exception {
        String variant = System.getenv().getOrDefault("FIST_CONV_VARIANT", "A");
        String xml = protoXml(variant);

        int tx = currentProgram.startTransaction("install __allregs");
        try { new SpecExtension(currentProgram).addReplaceCompilerSpecExtension(xml, monitor); }
        finally { currentProgram.endTransaction(tx, true); }
        println("APPLYCONV installed __allregs variant=" + variant);

        FunctionManager fm = currentProgram.getFunctionManager();

        // Phase 1: assign convention + unlock signatures on ALL functions first, so every call site
        // uses __allregs during the phase-2 detection pass.
        int t1 = currentProgram.startTransaction("assign __allregs");
        int assigned = 0;
        try {
            for (Function f : fm.getFunctions(true)) {
                if (f.isThunk() || f.isExternal()) continue;
                try {
                    f.setCallingConvention("__allregs");
                    f.setSignatureSource(SourceType.DEFAULT);
                    assigned++;
                } catch (Exception e) {}
            }
        } finally { currentProgram.endTransaction(t1, true); }
        println("APPLYCONV assigned+unlocked " + assigned + " functions");

        // Phase 2: iterate to a fixpoint, committing the FULL detected input set per function.
        //
        // We do NOT use HighFunctionDBUtil.commitParamsToDatabase: the decompiler classifies a register
        // read as a formal PARAMETER only when the model's input pentries fill contiguously from the
        // first slot, so a function that uses e.g. DX/SI/DI but not AX/BX/CX gets protoParams=0 and the
        // registers stay as free-floating in_DI/in_SI. Instead we read the decompiler's INPUT VARNODES
        // directly (hf.locRange, isInput) -- which reliably include every read-before-write register --
        // and install them as CUSTOM_STORAGE parameters ourselves. This captures the exact per-function
        // used-register subset regardless of slot ordering. Stack inputs are carried through too, and the
        // decompiler's detected return (AX/EAX) is preserved.
        //
        // Register inputs of a function only become visible once its CALLEES have committed signatures
        // (args carrying the registers must survive dead-code elimination), so we loop to a fixpoint:
        // each pass re-detects against the previous pass's committed callee signatures.
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        int iters = (int) envHex("FIST_CONV_ITERS", 5);
        List<Function> funcs = new ArrayList<>();
        for (Function f : fm.getFunctions(true)) if (!f.isThunk() && !f.isExternal()) funcs.add(f);

        for (int pass = 0; pass < iters; pass++) {
            int t2 = currentProgram.startTransaction("commit params pass " + pass);
            int updated = 0, failed = 0, changed = 0, n = 0;
            try {
                for (Function f : funcs) {
                    n++;
                    try {
                        DecompileResults r = di.decompileFunction(f, 45, monitor);
                        if (r != null && r.decompileCompleted() && r.getHighFunction() != null) {
                            if (commitInputs(f, r.getHighFunction())) changed++;
                            updated++;
                        } else failed++;
                    } catch (Exception e) { failed++; }
                }
            } finally { currentProgram.endTransaction(t2, true); }
            println("APPLYCONV pass " + pass + " updated=" + updated + " failed=" + failed
                + " sig-changed=" + changed + " of " + n);
            if (changed == 0 && pass > 0) break;   // fixpoint reached
        }
        di.dispose();
    }

    static final Set<String> GP = new HashSet<>(Arrays.asList("AX","BX","CX","DX","SI","DI","BP"));

    // Install every detected input varnode (GP register + stack) as a CUSTOM_STORAGE parameter, and
    // preserve the decompiler's detected return. Returns true if the signature changed.
    boolean commitInputs(Function f, ghidra.program.model.pcode.HighFunction hf) throws Exception {
        // 1) GP register inputs, deduped by base register keeping the widest use
        Map<String,ghidra.program.model.lang.Register> regs = new LinkedHashMap<>();
        // 2) stack inputs, deduped by (offset,size)
        Map<Long,ghidra.program.model.pcode.VarnodeAST> stack = new LinkedHashMap<>();
        Iterator<ghidra.program.model.pcode.VarnodeAST> vit = hf.locRange();
        while (vit.hasNext()) {
            ghidra.program.model.pcode.VarnodeAST v = vit.next();
            if (!v.isInput()) continue;
            ghidra.program.model.address.Address a = v.getAddress();
            if (a.isRegisterAddress()) {
                ghidra.program.model.lang.Register rg = currentProgram.getRegister(a, v.getSize());
                if (rg == null) continue;
                ghidra.program.model.lang.Register base = rg;
                while (base.getParentRegister() != null) base = base.getParentRegister();
                String bn = base.getName();
                String gp16 = bn.startsWith("E") ? bn.substring(1) : bn;
                if (!GP.contains(gp16)) continue;            // skip segment/flag/FPU/etc -> patch classes
                ghidra.program.model.lang.Register prev = regs.get(gp16);
                if (prev == null || rg.getBitLength() > prev.getBitLength()) regs.put(gp16, rg);
            } else if (a.isStackAddress()) {
                stack.put(a.getOffset(), v);
            }
        }
        List<ghidra.program.model.listing.Parameter> params = new ArrayList<>();
        for (ghidra.program.model.lang.Register rg : regs.values())
            params.add(mkParam(new ghidra.program.model.listing.VariableStorage(currentProgram, rg),
                        rg.getMinimumByteSize()));
        for (ghidra.program.model.pcode.VarnodeAST v : stack.values())
            params.add(mkParam(new ghidra.program.model.listing.VariableStorage(currentProgram, v.getAddress(), v.getSize()),
                        v.getSize()));

        // detected return (AX/EAX) — preserve if non-void
        ghidra.program.model.listing.Variable retVar = null;
        ghidra.program.model.pcode.FunctionPrototype proto = hf.getFunctionPrototype();
        if (proto != null && proto.getReturnType() != null
                && !(proto.getReturnType() instanceof ghidra.program.model.data.VoidDataType)) {
            ghidra.program.model.listing.VariableStorage rs = proto.getReturnStorage();
            if (rs != null && rs.isValid() && !rs.isVoidStorage())
                retVar = new ghidra.program.model.listing.ReturnParameterImpl(proto.getReturnType(), rs, currentProgram);
        }

        String beforeSig = f.getPrototypeString(false, false);
        f.updateFunction("__allregs", retVar, params,
            Function.FunctionUpdateType.CUSTOM_STORAGE, true, SourceType.USER_DEFINED);
        return !beforeSig.equals(f.getPrototypeString(false, false));
    }
    ghidra.program.model.listing.Parameter mkParam(ghidra.program.model.listing.VariableStorage st, int sz) throws Exception {
        return new ghidra.program.model.listing.ParameterImpl(null,
            ghidra.program.model.data.Undefined.getUndefinedDataType(sz), st, currentProgram, SourceType.USER_DEFINED);
    }
    static long envHex(String k, long d){ String s=System.getenv(k); try{ return (s==null||s.isEmpty())?d:Long.decode(s);}catch(Exception e){return d;} }

    static String protoXml(String variant) {
        // dual-width inputs: 2-byte AX..BP first (so 2-byte uses stay undefined2), then 4-byte EAX..EBP
        // for genuine 32-bit (66-prefix) register reads. Output AX / EAX.
        StringBuilder in = new StringBuilder("  <input>\n");
        for (String r : new String[]{"AX","BX","CX","DX","SI","DI","BP"}) in.append(reg(r,2));
        for (String r : new String[]{"EAX","EBX","ECX","EDX","ESI","EDI","EBP"}) in.append(reg(r,4));
        in.append("    <pentry minsize=\"1\" maxsize=\"500\" align=\"2\"><addr offset=\"2\" space=\"stack\"/></pentry>\n  </input>\n");
        String out = "  <output>\n" + reg("AX",2)
            + "    <pentry minsize=\"3\" maxsize=\"4\"><register name=\"EAX\"/></pentry>\n  </output>\n";
        StringBuilder un = new StringBuilder("  <unaffected>\n");
        for (String r : new String[]{"SP","DS","CS","ES","SS","DF"}) un.append(u(r));
        if (!variant.equals("A")) for (String r : new String[]{"SI","DI","BP"}) un.append(u(r));
        un.append("  </unaffected>\n");
        return "<prototype name=\"__allregs\" extrapop=\"unknown\" stackshift=\"2\">\n"
            + in + out + un + "</prototype>\n";
    }
    static String reg(String r, int mx){ return "    <pentry minsize=\"1\" maxsize=\""+mx+"\"><register name=\""+r+"\"/></pentry>\n"; }
    static String u(String r){ return "    <register name=\""+r+"\"/>\n"; }
}
