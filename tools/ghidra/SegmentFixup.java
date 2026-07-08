// wasm-fist postScript: fix real-mode near-pointer SEGMENT-BASE LOSS at the decompile stage.
//
// THE PROBLEM (root cause, evidence-derived — see the deliverable report):
//   PrepAnalysis sets a GLOBAL DS = DGROUP (0x1c00) context over the whole image, which is correct for
//   the majority of functions (they run with DS = DGROUP). BUT a class of hand-asm functions re-point DS
//   at their OWN code segment with a `push cs; pop ds` or `mov ax,cs; mov ds,ax` prologue, then do near
//   data accesses (`mov [0x2b54],bx`) into scratch/vector-save storage that lives INSIDE the code segment
//   (CS = 0x1000, linear 0x12b54), NOT in DGROUP. Ghidra never propagates the real-mode CS constant
//   through `mov ax,cs; mov ds,ax`, so for those instructions it either (a) mis-folds against the stale
//   global DS=0x1c00 (`[0x2b50]` -> DAT_1000_eb50 @ 0x1eb50, WRONG) or (b) loses the segment base
//   entirely (`*(undefined2 *)0x2b54`, base 0) -> a wild dereference of host low memory -> SIGSEGV in the
//   flat g_mem model. Proof the correct base is CS: the paired vector-RESTORE function reads the very same
//   slots with an explicit CS override (`mov cs:0x2b54,dx`) which Ghidra folds to DAT_1000_2b54 @ 0x12b54.
//
// THE FIX (systematic, faithful, no per-site patch): per function, statically track the DS segment value
//   through the instruction stream (a tiny abstract interpreter over push/pop + mov-into-DS), and wherever
//   DS is provably equal to CS (the function's own segment), OVERRIDE the DS context register for exactly
//   those instructions to CS. The decompiler then folds every near DS-relative access there to the correct
//   DAT_<cs>_off / g_mem linear address, exactly as it already does for the CS-override reads. Instructions
//   where DS is the global DGROUP, or is genuinely dynamic (loaded from memory: `mov ds,[mem]` / `lds`),
//   are left untouched (DGROUP stays folded; dynamic stays the separate far-pointer class).
//
// Disambiguation is by the TRACKED SEGMENT REGISTER, never by offset magnitude: a small constant like
//   0x2b54 is rebased to CS only inside a DS=CS window; a DGROUP `[0x2b54]` and BIOS/absolute accesses are
//   left alone. Runs AFTER ApplyConv, BEFORE ExportDecomp; the context lives in the program DB so both a
//   fresh `make decompile FIST_FRESH=1` and a reuse re-export emit the same folded C.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.*;
import java.math.BigInteger;
import java.util.*;

public class SegmentFixup extends GhidraScript {
    static final int UNKNOWN = Integer.MIN_VALUE;
    static final int DGROUP  = 0x1c00;   // PrepAnalysis global DS; overridable, kept in sync below

    public void run() throws Exception {
        long dgroup = envHex("FIST_DS", DGROUP);
        Program p = currentProgram;
        Register ds = p.getRegister("DS");
        if (ds == null) { println("SEGFIX no DS register"); return; }
        Listing lst = p.getListing();
        FunctionManager fm = p.getFunctionManager();

        int tx = p.startTransaction("segment fixup: DS=CS context");
        int nFns = 0, nInsn = 0;
        try {
            for (Function f : fm.getFunctions(true)) {
                AddressSetView body = f.getBody();
                int csSeg = segOf(f.getEntryPoint());
                int curDs = (int) dgroup;          // functions are entered with DS = DGROUP
                Deque<Integer> stk = new ArrayDeque<>();
                Set<String> regCS = new HashSet<>();  // GP regs currently holding the CS constant
                boolean touchedFn = false;

                InstructionIterator it = lst.getInstructions(body, true);
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    int seg = segOf(ins.getAddress());          // this instruction's CS
                    // (1) The memory operand of THIS instruction uses the CURRENT ds. If ds is provably CS
                    //     (a non-DGROUP known constant), override the DS context here so it folds to CS.
                    //     BUT skip any instruction that carries an explicit segment-override prefix
                    //     (ES:/CS:/SS:/DS:/FS:/GS:) -- its access is NOT DS-relative, so rebasing DS both
                    //     does nothing useful and can corrupt the override's own resolution (e.g. the
                    //     `lcall *ss:0x12` far-dispatch in FUN_1000_223c).
                    if (curDs != UNKNOWN && curDs != (int) dgroup) {
                        // rebase to CS, EXCEPT restore DGROUP on segment-override instructions (their
                        // access is not DS-relative). Writing DGROUP explicitly keeps re-runs idempotent.
                        int v = hasSegOverride(ins) ? (int) dgroup : curDs;
                        p.getProgramContext().setValue(ds, ins.getMinAddress(), ins.getMaxAddress(),
                                                       BigInteger.valueOf(v & 0xffff));
                        if (v != (int) dgroup) { nInsn++; touchedFn = true; }
                    }
                    // (2) apply this instruction's effect on the tracked DS / helper state
                    String m = ins.getMnemonicString().toUpperCase();
                    if (m.equals("PUSH")) {
                        Register r0 = ins.getRegister(0);
                        if (r0 != null && r0.getName().equals("CS"))      stk.push(seg);
                        else if (r0 != null && r0.getName().equals("DS")) stk.push(curDs);
                        else                                              stk.push(UNKNOWN);
                    } else if (m.equals("POP")) {
                        Register r0 = ins.getRegister(0);
                        int v = stk.isEmpty() ? UNKNOWN : stk.pop();
                        if (r0 != null && r0.getName().equals("DS")) curDs = v;
                    } else if (m.equals("MOV")) {
                        Register d = ins.getRegister(0), s = ins.getRegister(1);
                        if (d != null && d.getName().equals("DS")) {
                            // mov ds, <reg>: CS iff that reg holds CS; else (incl. mov ds,[mem]) unknown
                            curDs = (s != null && regCS.contains(s.getName())) ? seg : UNKNOWN;
                        } else if (d != null && !d.getName().equals("DS")) {
                            // track GP regs that hold the CS constant (mov r,cs sets it; any other write clears)
                            if (s != null && s.getName().equals("CS")) regCS.add(d.getName());
                            else                                       regCS.remove(d.getName());
                        }
                    } else if (m.equals("LDS")) {
                        curDs = UNKNOWN;                    // far load into DS = dynamic segment
                        Register d = ins.getRegister(0);   // also loads an offset reg; clear its CS flag
                        if (d != null) regCS.remove(d.getName());
                    } else if (m.equals("INT") || m.startsWith("CALL")) {
                        // DOS/BIOS and engine near-calls preserve DS by convention -> keep curDs.
                        // (a callother/INT reads DS but does not write it; see InstallIntFixup.)
                    } else {
                        // any other instruction: clear CS-tracking for a GP register it writes
                        Register d = ins.getRegister(0);
                        if (d != null && ins.getResultObjects().length > 0) regCS.remove(d.getName());
                    }
                }
                if (touchedFn) nFns++;
            }
        } finally {
            p.endTransaction(tx, true);
        }
        println("SEGFIX rebased DS->CS for " + nInsn + " instructions across " + nFns + " functions");
    }

    // True if the instruction begins with an x86 segment-override prefix (past any operand/addr-size or
    // rep/lock prefixes). Such an access targets the named segment, not the tracked DS -> leave it alone.
    private boolean hasSegOverride(Instruction ins) {
        try {
            byte[] b = ins.getBytes();
            for (int i = 0; i < b.length; i++) {
                int x = b[i] & 0xff;
                switch (x) {
                    case 0x26: case 0x2e: case 0x36: case 0x3e: case 0x64: case 0x65:
                        return true;                       // ES/CS/SS/DS/FS/GS override
                    case 0x66: case 0x67: case 0xf0: case 0xf2: case 0xf3:
                        continue;                          // size/lock/rep prefix -> keep scanning
                    default:
                        return false;                      // first non-prefix opcode byte
                }
            }
        } catch (Exception e) { /* fall through */ }
        return false;
    }

    private int segOf(Address a) {
        if (a instanceof SegmentedAddress) return (int) ((SegmentedAddress) a).getSegment();
        return (int) ((a.getOffset() >> 16) & 0xffff) << 12;   // fallback (flat)
    }
    private long envHex(String k, long def) {
        String s = System.getenv(k);
        try { return (s == null || s.isEmpty()) ? def : Long.decode(s); } catch (Exception e) { return def; }
    }
}
