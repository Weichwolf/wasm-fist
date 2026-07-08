// wasm-fist postScript: thread software-interrupt (INT n) register dataflow into visible C.
// Runs AFTER RecoverAll (functions discovered) and BEFORE ApplyConv (so INT register reads are
// detected as function inputs and thread as __allregs params).
//
// THE PROBLEM: Ghidra's x86 sleigh models `int n` as  `intloc = swi(n); call [intloc];`  where
// `swi` is an OPAQUE CALLOTHER intrinsic. The live DOS/BIOS register operands (AH=function,
// DS:DX=ptr, CX, BX, ...) do NOT flow into it, so the shim can never see which DOS call was made,
// and the return values (AX/CF/DX/CX) never flow back to the engine.
//
// THE FIX (fully in the program DB via a compiler-spec extension -- NO edit to the shared Ghidra
// install, NO hand-edit of the decompile): install a `<callotherfixup targetop="swi">` that rewrites
// each INT into an explicit, register-threaded sequence against a fixed reg-file in g_mem:
//     reg-file[VEC] = n;                         // which interrupt
//     reg-file[AX..ES] = AX,BX,CX,DX,SI,DI,BP,DS,ES;   // live inputs  (threaded, visible in C)
//     (*0xF0100)();                              // the shim dispatcher (barrier + runtime hook)
//     AX,BX,CX,DX,SI,DI = reg-file[AX..DI];      // results back into registers (multi-return)
//     CF = reg-file[CF];                          // carry/error flag back into the flag dataflow
//     goto inst_next;                            // an INT resumes at the next instruction; this
//                                                //   drops the sleigh's bogus trailing `call[intloc]`
// Because the dispatcher call is an opaque indirect call, the decompiler keeps every store (reading
// the LIVE register varnodes -> inputs threaded) and every load (post-call -> outputs threaded incl.
// DX/CX/CF), turning the opaque `swi(0x21)` into readable, marshalled C. The reg-file lives at linear
// 0xF0000.. (BIOS-ROM hole, unused by the engine; max engine symbol ~0xaf9fe, VGA fb 0xA0000). The
// dispatcher magic address 0xF0100 is mapped to the shim's fist_int_dispatch() by assemble_fist.py.
//
// REG-FILE LAYOUT (linear; MUST match re_out/ghidra_compat.h + fist_dos.c):
//   0xF0000 AX  0xF0002 BX  0xF0004 CX  0xF0006 DX  0xF0008 SI  0xF000A DI
//   0xF000C BP  0xF000E DS  0xF0010 ES  0xF0012 CF(1B)  0xF0014 VEC   dispatcher @ 0xF0100
import ghidra.app.script.GhidraScript;
import ghidra.program.database.SpecExtension;

public class InstallIntFixup extends GhidraScript {
    static final String BODY =
        " *[ram]:2 0xf0014:4 = zext(vec);" +   // interrupt vector
        " *[ram]:2 0xf0000:4 = AX;" +
        " *[ram]:2 0xf0002:4 = BX;" +
        " *[ram]:2 0xf0004:4 = CX;" +
        " *[ram]:2 0xf0006:4 = DX;" +
        " *[ram]:2 0xf0008:4 = SI;" +
        " *[ram]:2 0xf000a:4 = DI;" +
        " *[ram]:2 0xf000c:4 = BP;" +
        " *[ram]:2 0xf000e:4 = DS;" +
        " *[ram]:2 0xf0010:4 = ES;" +
        " call [0xf0100:4];" +                 // shim dispatcher: reads reg-file, runs the handler
        " AX = *[ram]:2 0xf0000:4;" +
        " BX = *[ram]:2 0xf0002:4;" +
        " CX = *[ram]:2 0xf0004:4;" +
        " DX = *[ram]:2 0xf0006:4;" +
        " SI = *[ram]:2 0xf0008:4;" +
        " DI = *[ram]:2 0xf000a:4;" +
        " CF = *[ram]:1 0xf0012:4;" +          // carry/error flag back into the flag dataflow
        " res = 0:2;" +                        // satisfy the swi output (dropped by the goto below)
        " goto inst_next;";
    public void run() throws Exception {
        String xml = "<callotherfixup targetop=\"swi\"><pcode><input name=\"vec\"/><output name=\"res\"/>"
                   + "<body><![CDATA[" + BODY + "]]></body></pcode></callotherfixup>";
        int tx = currentProgram.startTransaction("install swi callotherfixup");
        try { new SpecExtension(currentProgram).addReplaceCompilerSpecExtension(xml, monitor); }
        finally { currentProgram.endTransaction(tx, true); }
        println("INSTALL_INT_FIXUP installed <callotherfixup targetop=swi> (INT register threading)");
    }
}
