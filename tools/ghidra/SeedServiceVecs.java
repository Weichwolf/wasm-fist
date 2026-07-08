// wasm-fist postScript: seed the DGROUP service-dispatch table target routines as functions.
//
// PROBLEM (correctness gap): at load, the Doug-Huffman loader fills the engine's DGROUP
// service-dispatch table -- 12 far pointers at DGROUP:0x0a,0x0e,0x12,...,0x36 (4-byte off:seg
// stride) -- all pointing into the engine's own service-code segment (runtime seg load_seg+0xf69,
// base-0 segment 0xf69, linear window 0xf690..). These vectors are NULL/unrelocated in the static
// image (re_out/fist_dat_image.bin) and the routines they target are reached ONLY via
// `call far [DGROUP:0xNN]` at runtime -- so RecoverAll's static jump/call/prologue discovery never
// seeds them and they are never promoted to functions. Result: fist_icall_far cannot resolve them
// (they are not in the FUN_ map), logs "TRAP unmapped indirect call -> linear 0x0fXXX", returns 0,
// and the service's real side effects are SILENTLY SKIPPED (a latent correctness bug on the boot /
// MEMMGR path). Only 0xf842 (DGROUP:0x12), 0xf7ef, 0xf860, 0xf98b happened to be reached by other
// static references and got recovered; the other eight were not.
//
// FIX: create a function at each of the 12 service-table target linear addresses so they decompile
// as real FUN_0000_XXXX and land in the assemble_fist.py {linear -> &FUN} map, letting the indirect
// dispatcher resolve `call far [DGROUP:0xNN]` to the right C function instead of trapping. Idempotent
// (skips addresses that are already functions). Runs BEFORE RecoverAll so RecoverAll's fixpoint
// analysis and ApplyConv's __allregs register threading both cover the new functions.
//
// The 12 targets are the base-0 linear addresses (0xf690 + service offset) of the DGROUP table
// entries; the service offsets are the low words of the oracle-confirmed runtime table
// (tools/oracle/samples/dgroup_0x0_0x100.bin, rebased seg 0x2382->0xf69) and match the seeded table
// in tools/native_main.c g_dgroup_init. See docs/stage1.md "DGROUP service-routine recovery gap".
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.FunctionManager;
import ghidra.app.cmd.function.CreateFunctionCmd;

public class SeedServiceVecs extends GhidraScript {
    // DGROUP:0x0a,0x0e,0x12,0x16,0x1a,0x1e,0x22,0x26,0x2a,0x2e,0x32,0x36 -> service offsets in seg 0xf69
    // PLUS the frame/event-SCHEDULER method-vector targets installed by reloc section si=0x1d8 (applied
    // by FUN_1000_3446 at its `mov si,0x1d8; lcall [DGROUP:0x12]`), all far pointers off:0xf69 into the
    // same service segment: DGROUP:0x3fe=0x3e0b, 0x402=0x3e65, 0x406=0x3e05, 0x412=0x3ed6, 0x40a=0x3f17
    // (the frame/vsync POLL that FUN_1000_38cc spins on), 0x40e=0x422c, 0x416=0x423c (=38cc itself),
    // 0x41a=0x424e, 0x41e=0x4265.  These are reached ONLY through the DGROUP:0x40a family of `call far
    // [DGROUP:0xNN]` sites, so static discovery never seeds them; seeding the entries lets RecoverAll's
    // fixpoint pull in the whole scheduler cluster (0x3f17->calls 0x4290/0x41b0/... = 0x13920/0x13840).
    static final long BASE = 0xf690L;
    static final int[] OFFS = {
        0x15f, 0x18d, 0x1b2, 0x1d0, 0x1d9, 0x1ff, 0x224, 0x242, 0x279, 0x2a2, 0x2d0, 0x2fb,
        // reloc section si=0x1d8 (frame/event scheduler method vectors 0x3fe..0x41e):
        0x3e0b, 0x3e65, 0x3e05, 0x3ed6, 0x3f17, 0x422c, 0x423c, 0x424e, 0x4265,
        // reloc section si=0x174 (DISPLAY-LIST ELEMENT method vectors DGROUP:0x344..0x394, seg 0f69),
        // incl. DGROUP:0x388 = 0f69:0x306c = the screen-resource OPEN (INT 21h AX=4300 probe on the
        // driver-built filename @ DGROUP:0x740, e.g. MAINMENU.MRL) that stores the loaded segment into
        // the display-list descriptor word0; and the per-extension loader thunks 26fc dispatches to.
        0x2cf0, 0x2cf7, 0x2d0c, 0x2d15, 0x2d2a, 0x2d2f, 0x2d83, 0x2dc2,
        0x2e7d, 0x2f03, 0x2f7d, 0x30a3, 0x306c, 0x30de, 0x3152, 0x3167,
        0x2e78, 0x3230, 0x365d, 0x318e
    };

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        int made = 0, had = 0;
        for (int off : OFFS) {
            long lin = BASE + off;
            Address a = toAddr(lin);
            try {
                if (getInstructionAt(a) == null) disassemble(a);
                if (fm.getFunctionAt(a) != null) { had++; continue; }
                CreateFunctionCmd cmd = new CreateFunctionCmd(a);
                if (cmd.applyTo(currentProgram, monitor) && fm.getFunctionAt(a) != null) {
                    made++;
                    println(String.format("SEEDVEC created FUN @ linear 0x%05x", lin));
                } else {
                    println(String.format("SEEDVEC FAILED @ linear 0x%05x", lin));
                }
            } catch (Exception ex) {
                println(String.format("SEEDVEC EXC @ linear 0x%05x: %s", lin, ex.getMessage()));
            }
        }
        println(String.format("SEEDVEC done: %d created, %d already-present (of %d service vectors)",
                made, had, OFFS.length));
    }
}
