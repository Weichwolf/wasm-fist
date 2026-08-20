Type: feature
Area: audio

The mission + menu audio stream (SB/GUS + OPL, driven by the ported sound
engine) is bit-identical to the original given the same input and RNG state,
captured and compared against a DOSBox/QEMU reference and driven by a
tools/verify.sh flow. Currently unmeasured — no audio flow exists in the matrix.

## Comments

Audio is NOT 0% -- prior infrastructure exists: fist_opl.c (DBOPL FM synth, 44100Hz,
PCM ring + WAV sink via FIST_AUDIO_WAV) and fist_sb.c (SB PCM + WAV sink), plus a
reference ref/audio_menu_oracle.wav (12.7s stereo, the DOSBox full-mixer menu music)
and tools/oracle/capture_audio.sh. The port DOES produce OPL menu music
(FIST_OPL=1 FIST_SB=1 FIST_AUDIO_WAV -> a live 62.5s mono stream, rms 775, active).

Baseline compare (this round): a 4s oracle window cross-correlated against the port
OPL stream peaks at only 0.126 (near-uncorrelated) -- so the port's OPL output does
NOT match the oracle. CAVEAT: the oracle WAV is the FULL mixer (OPL+SB, stereo) and
the port WAV here is OPL-ONLY (mono); if the menu music is SB digital rather than
OPL FM, this compares the wrong stream. NEXT: determine whether the menu track is
OPL or SB (check what the engine writes -- FIST_OPL_REGLOG for OPL register writes,
the SB DMA path for digital), capture the matching port stream, and compare stream-
aligned; if OPL, diff the register-write log vs a DOSBox OPL reg trace (the port and
DOSBox share DBOPL, so equal register writes at equal times => bit-exact FM). Audio,
like the voxel, is a deep per-surface RE effort, but the harness + reference exist.

AUDIO ROOT LOCALIZED (OPL register-write compare, dosbox-fist opl_trace patch on
the ORIGINAL via FISTOPLLOG vs the port's FIST_OPL_REGLOG): the menu music IS OPL
(the original writes 2870 OPL DATA writes incl. 980 key-on b0-b8 values; the port
plays too -- 301 key-on, A0-A8 both 13 distinct). The NOTES match, but the OPL
INSTRUMENT DEFINITIONS do NOT: over the 89 common instrument registers (0x20-0x35
AM/VIB/EG/KSR/mult, 0x40-55 KSL/level, 0x60-75 attack/decay, 0x80-95 sustain/
release, 0xC0-C8, 0xE0-F5 waveform) only 15/89 MATCH. The port writes truncated
low values -- e.g. reg 0x20: original 0x31, port 0x01; reg 0x23: original 0x61,
port 0x11 -- consistently DROPPING the high bits (AM/VIB/EG-type/KSR), so the FM
timbre is wrong and the waveform is uncorrelated (0.126) even though the note
sequence is right. Since the port and DOSBox share DBOPL, fixing the instrument
bytes should make the FM output bit-exact. NEXT: find where the sound driver writes
the instrument registers (fist_snd.c sequencer / the SOUNDDVR.DVR or song
instrument table load) and why the high nibble is lost -- likely an instrument-byte
decode/mask bug or a wrong stride reading the patch table. This is a specific,
bounded fix, not open-ended.

Refinement: the shim OPL (fist_opl.c fist_opl_out -> fist_dbopl_write) is FAITHFUL
-- it passes the 0x389 data byte UNMASKED to DBOPL. So the wrong instrument value
(0x01 vs 0x31) already reaches the engine's `out(0x389, param_1)` call
(re_out/fist_snd.c ~2340/5678) -- i.e. the ENGINE sound sequencer computes/reads
the wrong instrument byte, not the shim. NEXT: trace the sequencer's instrument-
table read in fist_snd.c against the asm (where param_1 to the 0x389 writer comes
from) -- the high nibble (AM/VIB/EG/KSR bits) is dropped, so look for a wrong
mask/shift or a byte-stride error reading the OPL patch table (from SOUNDDVR.DVR or
the in-DAT patch bank). A base-loss or an `& 0x0f` on the instrument byte would
produce exactly the observed 0x31->0x01. Fixable as an asm-verified patch.

Root refined (correcting the "instrument bytes decoded wrong" framing -- the
last-val-per-reg compare was timing-confounded): the OPL instrument PATCH TABLE at
DGROUP:0x1dd (param_1*16+0x1dd, read by FUN_0000_0f99) is STATIC in the DAT's
DGROUP, so it is identical port<->oracle -- not a decode bug. The real gap:
FUN_0000_0f99 (instrument load) is ONLY called from FUN_0000_104f, the OPL INIT
(9 channels <- instrument 0). It is NOT called during song playback. So the port
plays every note with the init instrument 0 (reg 0x20 stays 0x01), while the
ORIGINAL reloads the song's instruments per voice (reg 0x20 also takes 0x31). The
per-voice instrument reload path is missing/unported -- likely reached via the
engine's indirect dispatch (fist_snd.c RULE 7 "OPEN: unresolved indirect dispatch"
/ fist_icall), which the port does not fully resolve for the sequencer. NEXT: find
the song sequencer's per-note instrument-select call (an indirect call to 0f99 or a
direct OPL-instrument write path) and wire/port it; then the FM timbre matches and,
sharing DBOPL, the audio goes bit-exact. Bounded but in the sequencer/icall layer.

Localized precisely: the note-play handler FUN_0000_10a6 writes A0 (freq, |0xa000)
and B0 key-on (|0xb020 at fist_snd.c:2536), reading the voice's instrument from the
CACHED byte [0xbdd] (set by 0f99 at :2431 = puVar6[1]). 0f99 (the full instrument
load) is only invoked from the OPL init (104f) with instrument 0, so [0xbdd] stays
the init instrument. The ORIGINAL reloads the song's instrument (its reg 0x20 takes
0x31), so its sequencer calls 0f99 on a program-change event; the port never does
during playback. So the missing piece is the sequencer's PROGRAM-CHANGE handler
that calls 0f99 with the song's instrument number -- not reached in the port,
consistent with the unresolved indirect dispatch (RULE 7). NEXT: find the song
event loop (the fn that reads the song stream and dispatches note-on / note-off /
program-change), see how a program-change reaches 0f99 in the asm (likely an
indirect call / jump-table the port's fist_icall doesn't resolve), and wire it;
then instruments reload per voice and the FM output matches. Deep in the sequencer/
dispatch layer, like the voxel's 6980 -- both are the engine's paged/indirect core.

CONFIRMED (not a duration artifact): re-ran the port for 55s (matching the oracle,
12487 OPL writes / 4899 key-on) -- reg 0x20 still only ever = 0x01, and across the
op1 regs 0x20-0x25 the port emits exactly ONE instrument each {01,01,01,11,11,11}
while the ORIGINAL emits several {01,31 / 01,21,31 / 01,31,b1 / 11,61 / ...}. So the
port genuinely plays the whole song with the init instrument and NEVER triggers a
program-change; 0f99 (instrument load) is only reached from the OPL init (104f), not
from the note sequencer. Both the note-on handler (10a6) and 0f99 are dispatched
INDIRECTLY -- note-on resolves (notes play, 4899 key-on) but the program-change path
to 0f99 does not fire. The sequencer's event loop (the fn that reads the song stream
and indirectly calls 10a6) is the large indirect-dispatched core still to be found;
FUN_0000_1e5e @5456 is the sound-DEVICE dispatch (SB detect), not it. NEXT: locate
the note sequencer (indirect caller of 10a6) and its program-change case, and wire
its 0f99 dispatch. Audio and voxel both bottom out in the engine's indirect-dispatch
core -- the highest-leverage direction is completing that dispatch, not per-surface.

Dispatch mechanism found: the sound driver routes via SERVICE VECTORS -- fist_icall_far
through function pointers at DGROUP [0x012]/[0x0f4]/[0x0d4] (fist_snd.c:433-435) and
_DAT c50c/c520/c0f8/c016 (:460-463). Both 10a6 (note-on) and 0f99 (instrument load)
are in the icall fmap, so both are reachable; note-on fires but the program-change
never routes to 0f99. So one of these service vectors (or the sequencer's per-event
selection among them) is not installed/routed to the instrument-reload path in the
port. NEXT: dump the port's vector table [DGROUP:0x012/0x0f4/0x0d4] + c50c/c520 at
playback and compare to the oracle's (dosbox-fist guest-RAM, the same page-walk used
for the voxel TCB) -- find the vector that should equal 0f99's entry and does not,
then fix its install. This is the engine's indirect service-vector layer, the SAME
class as the voxel 6980 dispatch -- completing/verifying that vector layer is the
cross-cutting high-leverage fix.

Consolidation: the audio defect lives in the sound driver's command/sequencer
dispatch -- a 18-entry command table (FUN_0000_1d45 reads DGROUP:0x1af5 by index),
voice slots (1d62), and service vectors, threaded through the engine's indirect
dispatch with ~dozens of RULE-7 unresolved targets + near-offset base-losses (e.g.
1d62's `*(undefined2*)0x0`). The program-change command index does not route to
0f99 in the port. This is systematic decompile-fidelity work on fist_snd.c (resolve
the indirect dispatch + base-losses of the sequencer command table), a dedicated
multi-step effort of the SAME class as the voxel 6980 raycaster -- both are the
engine's deep dispatch core. The port-vs-oracle verification path is proven
(FISTOPLLOG register-stream diff shows exactly which instrument writes are missing);
the fix is bounded but large. Not a quick win, tracked for a focused session.
