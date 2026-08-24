// wasm-fist postScript: set the CS segment-register CONTEXT per real-mode code segment so the decompiler
// resolves `mov [mem],cs` / `push cs` to the actual segment CONSTANT instead of emitting `unaff_CS`.
//
// PrepAnalysis already sets DS/SS globally; CS is per-code-segment (the engine runs multiple: the 0-page,
// the 0x1000 main engine, and the 0xf69 CRT/service cluster window 0xf690..0x1f68f).  This script:
//   1) baseline: CS = the page segment (linear>>16 << 12) over each 64 KB page -- 0 for 0..0xffff,
//      0x1000 for 0x10000..0x1ffff, ...  (matches how Ghidra decoded FUN_<pageseg>_*).
//   2) override: for every non-zero CS cluster discovered from direct far call/jmp targets (same rule as
//      SegWrapFixup: seg != own page seg, >= FIST_CS_MINSITES sites), set CS = seg over its CERTAIN
//      sub-window [seg<<4, next-64K-page-boundary) -- the part the page grid cannot form, so those bytes
//      certainly execute under this CS (e.g. 0xf690..0xffff => CS=0xf69, giving the `mov [mem],cs`=0xf69
//      the hand-patches restore by hand).  The overlap region (0x10000+) keeps the page baseline; its
//      per-flow 0xf69-vs-0x1000 discrimination is SegWrapFixup's job and is not needed for the CS constant.
//
// Runs AFTER SegWrapFixup / BEFORE ApplyConv so register threading sees the resolved (non-unaff) CS.
// Env: FIST_CS_MINSITES (default 8), FIST_CS_SEGS (pin a comma/space seg list), FIST_CODE_END (0x1c000).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import java.math.BigInteger;
import java.util.*;

public class SetCSContext extends GhidraScript {
    private long envInt(String k, long d){ String s=System.getenv(k); try{ return (s==null||s.isEmpty())?d:Long.decode(s.trim()); }catch(Exception e){ return d; } }
    Address MIN;
    long lin(Address a){ return a.subtract(MIN); }
    Address at(long l){ return toAddr(l & 0xfffffL); }

    public void run() throws Exception {
        Program p = currentProgram;
        MIN = p.getMinAddress();
        Listing lst = p.getListing();
        Register cs = p.getRegister("CS");
        if (cs == null) { println("SETCS no CS register"); return; }
        long minSites = envInt("FIST_CS_MINSITES", 8);
        long codeEnd  = envInt("FIST_CODE_END", 0x1c000);
        Address lo = p.getMinAddress(), hi = p.getMaxAddress();
        long linHi = lin(hi);

        int tx = p.startTransaction("set CS context");
        long baseSet = 0, clusterSet = 0;
        try {
            // 1) baseline: CS = page segment over each 64 KB page
            for (long pg = 0; pg <= (linHi >> 16); pg++) {
                long a = pg << 16, b = Math.min(((pg + 1) << 16) - 1, linHi);
                int pageSeg = (int)(pg << 12);
                p.getProgramContext().setValue(cs, at(a), at(b), BigInteger.valueOf(pageSeg));
                baseSet++;
            }
            // 2) discover non-zero CS clusters from far call/jmp targets
            Map<Integer,Integer> segSites = new HashMap<>();
            String pin = System.getenv("FIST_CS_SEGS");
            Set<Integer> pinned = new HashSet<>();
            if (pin != null && !pin.isEmpty())
                for (String s : pin.split("[ ,]+")) if (!s.isEmpty()) pinned.add((int)Long.decode(s.trim()).longValue());
            InstructionIterator it = lst.getInstructions(true);
            while (it.hasNext()) {
                Instruction ins = it.next(); byte[] b = ins.getBytes();
                int op = b[0] & 0xff;
                if ((op != 0x9a && op != 0xea) || b.length < 5) continue;
                int off = (b[1]&0xff)|((b[2]&0xff)<<8), seg = (b[3]&0xff)|((b[4]&0xff)<<8);
                long tgt = ((long)seg<<4)+off;
                if (seg == 0 || tgt < 4 || tgt >= codeEnd) continue;
                int pageSeg = (int)((tgt>>16)<<12);
                if (seg == pageSeg) continue;
                segSites.merge(seg, 1, Integer::sum);
            }
            List<Integer> csList = new ArrayList<>();
            for (Map.Entry<Integer,Integer> e : segSites.entrySet())
                if (pinned.contains(e.getKey()) || e.getValue() >= minSites) csList.add(e.getKey());
            if (!pinned.isEmpty()) { csList.clear(); csList.addAll(pinned); }
            Collections.sort(csList);
            // 3) override the CERTAIN sub-window of each cluster with CS = seg
            for (int seg : csList) {
                long base = (long)seg << 4;
                long certainHi = Math.min((((base >> 16) + 1) << 16) - 1, base + 0xffff);
                if (certainHi < base) continue;
                p.getProgramContext().setValue(cs, at(base), at(certainHi), BigInteger.valueOf(seg));
                clusterSet++;
                println("SETCS cluster CS=0x" + Integer.toHexString(seg) + " over 0x"
                        + Long.toHexString(base) + "..0x" + Long.toHexString(certainHi));
            }
            println("SETCS baseline pages=" + baseSet + " clusters=" + clusterSet + " discovered=" + csList.size());
        } finally { p.endTransaction(tx, true); }
        println("SETCS_DONE");
    }
}
