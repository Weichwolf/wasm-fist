// wasm-fist preScript: turn on aggressive discovery + fix 16-bit segmented decompile BEFORE analysis.
// Runs in analyzeHeadless BEFORE auto-analysis (so option/context changes are honoured by the analyzers).
//
// Two jobs:
//  1) Enable the aggressive Ghidra analyzers that a bare BinaryLoader import leaves off: Aggressive
//     Instruction Finder, Decompiler Parameter ID, scalar/address-table reference creation, and lower the
//     absurd default address-table minimum so 16-bit near/far dispatch (jump) tables are recovered.
//  2) Set the segment-register CONTEXT over the whole image so the decompiler resolves DGROUP data refs to
//     real DAT_ symbols instead of emitting unaff_DS/unaff_SS garbage. DS = DGROUP paragraph 0x1c00
//     (linear 0x1c000); SS = 0x2ba9 (from the CRT startup, see docs/recon.md). ES left unset (varies).
//
// Values are overridable via env: FIST_DS (default 0x1c00), FIST_SS (default 0x2ba9).
import ghidra.app.script.GhidraScript;
import ghidra.framework.options.Options;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.MemoryBlock;
import java.math.BigInteger;

public class PrepAnalysis extends GhidraScript {
    private long envHex(String k, long def) {
        String s = System.getenv(k);
        try { return (s == null || s.isEmpty()) ? def : Long.decode(s); } catch (Exception e) { return def; }
    }
    public void run() throws Exception {
        Program p = currentProgram;
        Options o = p.getOptions("Analyzers");

        // --- 1) aggressive discovery ---
        o.setBoolean("Aggressive Instruction Finder", true);
        o.setBoolean("Decompiler Parameter ID", true);
        o.setBoolean("Decompiler Switch Analysis", true);
        o.setBoolean("Scalar Operand References", true);
        // Build near/far dispatch (jump) tables: default min table size is 1048576 (never fires).
        o.setBoolean("Reference.Create Address Tables", true);
        o.setInt("Reference.Address Table Minimum Size", 2);
        o.setBoolean("Data Reference.Create Address Tables", true);
        o.setInt("Data Reference.Address Table Minimum Size", 2);
        o.setBoolean("Data Reference.Switch Table References", true);
        o.setBoolean("Reference.Switch Table References", true);
        // let the constant-ref analyzer speculate harder about segmented pointers
        o.setBoolean("x86 Constant Reference Analyzer.Create Data from pointer", true);
        println("PREP_OPTS aggressive analyzers enabled");

        // --- 2) segment register context over the whole image ---
        long ds = envHex("FIST_DS", 0x1c00);
        long ss = envHex("FIST_SS", 0x2ba9);
        Address start = null, end = null;
        for (MemoryBlock b : p.getMemory().getBlocks()) {
            if (start == null || b.getStart().compareTo(start) < 0) start = b.getStart();
            if (end == null || b.getEnd().compareTo(end) > 0) end = b.getEnd();
        }
        setReg("DS", ds, start, end);
        setReg("SS", ss, start, end);
        println("PREP_CTX DS=0x" + Long.toHexString(ds) + " SS=0x" + Long.toHexString(ss)
                + " over " + start + ".." + end);
    }
    private void setReg(String name, long val, Address s, Address e) {
        try {
            Register r = currentProgram.getRegister(name);
            if (r == null) { println("PREP_CTX no reg " + name); return; }
            currentProgram.getProgramContext().setValue(r, s, e, BigInteger.valueOf(val));
        } catch (Exception ex) { println("PREP_CTX FAIL " + name + ": " + ex.getMessage()); }
    }
}
