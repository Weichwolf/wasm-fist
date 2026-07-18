/* re_out/fist_opl_dbopl.cpp -- extern "C" bridge to the DOSBox 0.74-3 DBOPL OPL2/OPL3 emulator core.
 *
 * The Armored Fist sound driver's device 3 (AdLib/OPL, SOUND.CFG letter 'C') programs the OPL chip via
 * port 0x388/0x389 (the writer FUN_0000_0f21).  fist_opl.c traps those ports and feeds the register
 * stream here.  We drive DBOPL::Chip directly (Setup/WriteReg/GenerateBlock2) -- the SAME OPL core the
 * DOSBox oracle uses by default (oplemu=default -> DBOPL), so identical register writes -> byte-identical
 * FM output (the requirement for a real bit-exact menu-music compare).  The DSP core (Chip/Channel/
 * Operator/InitTables/GenerateBlock) in opl/dbopl.{cpp,h} is verbatim DOSBox 0.74-3; only DBOPL::Handler
 * (the DOSBox mixer glue) was removed.  Compiles native (g++ -m32 -nostdinc++) and wasm (em++). */
#include "opl/dbopl.h"
namespace DBOPL { void InitTables(void); }  /* file-local in dbopl.cpp, namespace-scope extern linkage */

static DBOPL::Chip g_chip;   /* global -> Channel/Operator ctors run at load (no operator new/RTTI) */
static int g_inited = 0;

extern "C" void fist_dbopl_reset(unsigned rate)
{
    static int tables = 0;
    if (!tables) { DBOPL::InitTables(); tables = 1; }
    g_chip.Setup((Bit32u)rate);
    g_inited = 1;
}

extern "C" void fist_dbopl_write(unsigned reg, unsigned char val)
{
    if (!g_inited) fist_dbopl_reset(44100);
    /* WriteAddr(0x388,reg) == reg for OPL2 (port&3==0); the AdLib latch is the raw register index. */
    g_chip.WriteReg((Bit32u)reg, (Bit8u)val);
}

/* Generate nsamp MONO samples (Bit32s) -> out.  DBOPL GenerateBlock2 is mono (opl2); opl3Active would
 * need GenerateBlock3 (stereo) -- the game (device 3) never enables OPL3, so mono is faithful. */
extern "C" unsigned fist_dbopl_generate(unsigned nsamp, int *out)
{
    if (!g_inited) return 0;
    unsigned done = 0;
    while (nsamp) {
        unsigned n = nsamp > 512 ? 512 : nsamp;
        g_chip.GenerateBlock2((Bitu)n, (Bit32s*)(out + done));
        done += n; nsamp -= n;
    }
    return done;
}

extern "C" int fist_dbopl_is_opl3(void) { return g_chip.opl3Active ? 1 : 0; }
