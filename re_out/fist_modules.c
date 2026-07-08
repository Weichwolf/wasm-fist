/* re_out/fist_modules.c -- Armored Fist overlay-driver wiring table.
 *
 * Maps each loadable driver overlay's DOS basename to its decompiled unit's {module-offset -> &FUN}
 * fmap, fmap count, and base global (the accessor base the assemble step gives the driver unit). The
 * INT 21h AH=4B AL=03 loader (fist_dos.c -> fist_ovl_register in fist_icall.c) looks a just-loaded
 * overlay up here and, IF the driver unit is compiled into this build, wires its fmap and sets its
 * base global to the runtime load address so its base-relative g_mem accessors resolve.
 *
 * The driver-unit symbols are WEAK: an engine-only build (no driver .c on the link line) still links
 * -- the weak-undefined refs resolve to 0, fist_ovl_register skips wiring (guarded), and a far-call
 * into the loaded-but-unwired overlay traps honestly ("overlay NAME+0xOFF (no fmap)"). Adding a driver
 * to the build (its fist_<mod>.c on the link line) is all it takes to wire it -- no edit here.
 */
#include "ghidra_compat.h"

#define WEAK __attribute__((weak))

/* MGAVIDEO.DVR -- Matrox MGA video driver (re_out/fist_mga.c, when built) */
extern const struct fist_fent fist_mga_fmap[] WEAK;
extern const unsigned fist_mga_fmap_n WEAK;
extern uint32_t fist_mga_base WEAK;

/* SOUNDDVR.DVR -- Sound Blaster / GUS sound driver (re_out/fist_snd.c, when built) */
extern const struct fist_fent fist_snd_fmap[] WEAK;
extern const unsigned fist_snd_fmap_n WEAK;
extern uint32_t fist_snd_base WEAK;

/* EXT -- Doug-Huffman extender kernel (re_out/fist_ext.c, when built): the KDV intro-FMV player.
 * NOT a DOS-loaded overlay -- native_main registers it directly (fist_ovl_register("EXT", ...))
 * at init, using this table to wire its fmap/base exactly like the .DVR overlays. */
extern const struct fist_fent fist_ext_fmap[] WEAK;
extern const unsigned fist_ext_fmap_n WEAK;
extern uint32_t fist_ext_base WEAK;

const struct fist_ovl_known fist_ovl_known[] = {
    { "MGAVIDEO.DVR", fist_mga_fmap, &fist_mga_fmap_n, &fist_mga_base },
    { "SOUNDDVR.DVR", fist_snd_fmap, &fist_snd_fmap_n, &fist_snd_base },
    { "EXT",          fist_ext_fmap, &fist_ext_fmap_n, &fist_ext_base },
    { 0, 0, 0, 0 },
};
