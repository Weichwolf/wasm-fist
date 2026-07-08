// wasm-fist postScript: seed a driver OVERLAY's runtime entry-vector + registered-method functions.
//
// PROBLEM (correctness gap, the driver analog of SeedServiceVecs/SeedRuntimeVecs): a driver overlay
// (MGAVIDEO.DVR / SOUNDDVR.DVR) is a 16-bit MZ module whose code past the tiny fixed entry vector
// (module offsets 0x0..0x8: `ret/retf/jmp init/jmp method` thunks) is reached ONLY at runtime, two ways:
//   (1) the ENGINE far-calls the driver's INIT via `call far load_seg:0x0002` (FUN_1000_5051), which is
//       the `jmp <init>` thunk at module offset 0x2; and
//   (2) the init REGISTERS its method entry points into the engine DGROUP via the service call
//       `mov ax,<method_off>; mov bx,<dgroup_slot>; call [DGROUP:0x36]`, and the engine later far-calls
//       those DGROUP method vectors -> driver_load_seg:<method_off>.
// Ghidra's static discovery (RecoverAll) never sees these runtime far-call targets, so the init body
// and the registered methods sit as un-promoted bytes in the 0x9..0x1xx gap -> no C function -> the
// engine's overlay dispatch traps ("no fmap entry") and the driver never initialises (its DGROUP
// method/struct slots stay 0 -> the engine's driver-dispatch subsystem null-derefs).
//
// FIX: create functions at (a) the entry thunks 0x2/0x5 and the targets of every jmp thunk in 0x0..0x8,
// and (b) an explicit list of registered method offsets passed via the JVM property `fist.seed.offs`
// (comma-separated hex, set per-driver in decompile.sh/Makefile from the asm).  Seeding BEFORE RecoverAll
// lets its fixpoint + ApplyConv cover the new functions (and any near-call callees they reach), so they
// land in the module fmap and the overlay dispatcher resolves them. Idempotent (skips existing funcs).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.lang.OperandType;
import ghidra.app.cmd.function.CreateFunctionCmd;
import java.util.LinkedHashSet;

public class SeedDriverVecs extends GhidraScript {
    private void seed(long off, LinkedHashSet<Long> done) {
        if (!done.add(off)) return;
    }

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        LinkedHashSet<Long> offs = new LinkedHashSet<>();
        // (a) the fixed entry thunks + the target of every short/near jmp in the 0x0..0x8 thunk table.
        offs.add(0x2L); offs.add(0x5L);
        for (long a = 0; a <= 8; a++) {
            Address ad = toAddr(a);
            try {
                if (getInstructionAt(ad) == null) disassemble(ad);
                Instruction ins = getInstructionAt(ad);
                if (ins != null && ins.getMnemonicString().toLowerCase().startsWith("jmp")) {
                    Address[] fl = ins.getFlows();
                    for (Address t : fl) offs.add(t.getOffset());
                }
            } catch (Exception ex) { /* ignore */ }
        }
        // (b) explicit registered-method offsets from the asm (per-driver, via JVM property).
        String p = System.getProperty("fist.seed.offs", "");
        for (String s : p.split(",")) {
            s = s.trim();
            if (s.isEmpty()) continue;
            try { offs.add(Long.decode(s.startsWith("0x") ? s : "0x" + s)); }
            catch (Exception ex) { println("SEEDDRV bad offs token: " + s); }
        }
        int made = 0, had = 0;
        for (long off : offs) {
            Address a = toAddr(off);
            try {
                if (getInstructionAt(a) == null) disassemble(a);
                if (fm.getFunctionAt(a) != null) { had++; continue; }
                CreateFunctionCmd cmd = new CreateFunctionCmd(a);
                if (cmd.applyTo(currentProgram, monitor) && fm.getFunctionAt(a) != null) {
                    made++;
                    println(String.format("SEEDDRV created FUN @ 0x%04x", off));
                } else {
                    println(String.format("SEEDDRV FAILED @ 0x%04x", off));
                }
            } catch (Exception ex) {
                println(String.format("SEEDDRV EXC @ 0x%04x: %s", off, ex.getMessage()));
            }
        }
        println(String.format("SEEDDRV done: %d created, %d already-present (of %d entry points)",
                made, had, offs.size()));
    }
}
