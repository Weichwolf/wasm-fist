// wasm-fist postScript: seed RUNTIME-INSTALLED interrupt-handler routines as functions.
//
// PROBLEM (correctness gap, same class as SeedServiceVecs but for hardware IRQ vectors): the engine
// installs its OWN interrupt service routines at runtime via DOS set-vector (INT 21h AH=25). The
// timer ISR is installed for INT 8 (PIT) by FUN_1000_32c1 with DS:DX = 0xf69:0x3a68 -> base-0 linear
// 0x130f8. That entry is reached ONLY through the installed INT-8 vector (an asynchronous hardware
// IRQ in the original; our host timer pump drives it -- see tools/native_main.c / re_out/fist_dos.c),
// so RecoverAll's static jump/call/prologue discovery never seeds it: it sits in the gap between
// FUN_1000_30de (ends 0x130f8) and FUN_1000_31c3 as un-promoted code. Its call-tree subs (31c3, 3218,
// 3a00, ...) WERE recovered from other references, but the ISR ENTRY was not, so the timer pump has no
// C function to call and DGROUP:0x452 (the tick counter the engine busy-waits on in FUN_1000_3346)
// never advances -> boot hang.
//
// FIX: create a function at the ISR entry linear so it decompiles as a real FUN_1000_30f8, gets
// __allregs register threading, and lands in the assemble_fist.py {linear -> &FUN} map. The host
// timer pump then resolves the installed INT-8 handler (fist_icall(0x130f8)) to this C function and
// runs it, so the tick counter advances and the engine's timer waits clear. Idempotent (skips
// addresses already functions). Runs BEFORE RecoverAll so its fixpoint + ApplyConv cover the entry.
//
// The ISR entry is asm-verified (objdump of re_out/fist_dat_image.bin @ 0x130f8): pushes AX..ES,
// sets DS=0x1c00 (DGROUP -- matches the static DS context, so its DAT_1000_cXXX accesses fold
// correctly), reprograms the PIT, sends EOI, runs the tick bookkeeping (inc DGROUP:0x44e/0x452 via
// the 0x131c3/0x13218 subtree), rotates the VGA palette (out 0x3c0/0x3d4), chains installed sound
// vectors (call far [DGROUP:0x432]/[0x5e4]), and IRETs. See docs/stage1.md "TIMER-DRIVEN EXECUTION".
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.FunctionManager;
import ghidra.app.cmd.function.CreateFunctionCmd;

public class SeedRuntimeVecs extends GhidraScript {
    // base-0 linear addresses of runtime-installed IRQ handlers (see header)
    static final long[] LIN = {
        0x130f8L,   // INT 8 (PIT timer) ISR, installed by FUN_1000_32c1 (DS:DX = 0xf69:0x3a68)
    };

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        int made = 0, had = 0;
        for (long lin : LIN) {
            Address a = toAddr(lin);
            try {
                if (getInstructionAt(a) == null) disassemble(a);
                if (fm.getFunctionAt(a) != null) { had++; continue; }
                CreateFunctionCmd cmd = new CreateFunctionCmd(a);
                if (cmd.applyTo(currentProgram, monitor) && fm.getFunctionAt(a) != null) {
                    made++;
                    println(String.format("SEEDRTVEC created FUN @ linear 0x%05x", lin));
                } else {
                    println(String.format("SEEDRTVEC FAILED @ linear 0x%05x", lin));
                }
            } catch (Exception ex) {
                println(String.format("SEEDRTVEC EXC @ linear 0x%05x: %s", lin, ex.getMessage()));
            }
        }
        println(String.format("SEEDRTVEC done: %d created, %d already-present (of %d runtime vectors)",
                made, had, LIN.length));
    }
}
