// wasm-fist preScript: seed auto-analysis with the real entry so flow-following starts from
// live code instead of guessing. Runs BEFORE analysis in analyzeHeadless.
//   Engine (FIST.DAT, x86:LE:16 Real Mode): entry = linear 0x0004 (verified DOS-CRT startup).
//   Kernel (FIST.RUN, x86:LE:32):           entry = 0x0d9d.
// Entry comes from env FIST_ENTRY (hex, e.g. 0x4); default 0x4 (engine). See docs/recon.md.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class MarkEntry extends GhidraScript {
    public void run() throws Exception {
        String s = System.getenv("FIST_ENTRY");
        long e = Long.decode((s == null || s.isEmpty()) ? "0x4" : s);
        Address ad = toAddr(e);
        try {
            disassemble(ad);
            if (currentProgram.getFunctionManager().getFunctionAt(ad) == null)
                createFunction(ad, "app_entry");
            addEntryPoint(ad);
            println("MARK_ENTRY app_entry @ " + ad);
        } catch (Exception ex) {
            println("MARK_ENTRY FAIL @ " + ad + ": " + ex.getMessage());
        }
    }
}
