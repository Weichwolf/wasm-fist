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

Cross-surface consolidation (this round): re-examined 0003 as an alternative to the
windshield's tooling gate. Audio has a CLEANER verification path than the voxel (port-only
FIST_OPL_REGLOG + WAV cross-correlation, NO fragile xvfb/click oracle-capture), which is a
real advantage. BUT the ROOT is the SAME class as the voxel: the sequencer command dispatch
(FUN_0000_1d45 reads the 18-entry table DGROUP:0x1af5; FUN_0000_1d62 voice-slot setup carries
a Ghidra base-loss `*(undefined2*)0x0` at fist_snd.c:5360) threaded through the engine's
indirect service-vector layer -- the program-change command never routes to 0f99 (instrument
load), so every note plays with init instrument 0 (reg 0x20 stuck at 0x01 vs original 0x31).
Both the voxel (6980/3a24 producer) and audio (program-change dispatch) bottom out in the
engine's hand-written INDIRECT-DISPATCH + base-loss core -- this is the project's central
hard surface, confirmed cross-surface. The bounded landable sub-fixes here are the sequencer
BASE-LOSSES (a known asm-verified patch class, e.g. 1d62's `*0x0`) -- restoring their DGROUP
base is verifiable port-only (reg 0x20 should take 0x31, WAV correlation should rise from
0.126). That is the tractable audio entry point, and unlike the windshield it needs no
oracle-capture rebuild. NEXT (focused session): asm-verify the fist_snd.c sequencer
base-losses (1d62 `*0x0` + siblings), restore their bases as an asm-verified patch, confirm
reg 0x20 takes the song instruments + WAV correlation rises -- a landable win in the 407 class.

Concrete localization (correct image + sequencer functions found): fist_snd.c decompiles from
fist_snd_image.bin (16700B, SOUNDDVR.DVR) -- NOT fist_dat_image.bin (my earlier objdump was the
wrong image). The shim drives the sequencer via fist_icall(fist_snd_base+0x3dd) (timer ISR) and
+0xa28 (music advance, fist_sb.c:190). The song-event processor is FUN_0000_0a28 (fist_snd.c:1593):
iterates 9 voices, reads each voice's song stream via the pointer table [0x90] (loop-back via [0xa4],
MIDI-like special bytes 0x80/0x81), and triggers per-voice events via FUN_0000_0aa7. 0aa7 (:1656)
dispatches via `jmp near [0x1a5]` (asm 0xae2 `ff 26 a5 01`, decompiled as the unrecovered-jumptable
`fist_icall_far(_DAT_1000_c1a5)`) -- but this is a COROUTINE COMPUTED-RETURN (the sequencer returns
to the caller loop via the vector [DGROUP:0x1a5]), NOT the event-type dispatch. So the program-change
-> 0f99 link is in the event PARSING (0a28 + its callees 0b5d/0cf3/0c94), where an event byte selects
note-on vs program-change; asm-verifiable against fist_snd_image.bin. The sequencer is a coroutine
state-machine with computed returns + Ghidra base-losses (1d62 `*0x0`), same deep class as the voxel
6980 dispatch. VERIFICATION PATH (clean, port-only, no oracle-capture): FIST_OPL_REGLOG -- reg 0x20
must go from stuck-0x01 to taking the song's 0x31 once program-change reaches 0f99; then WAV
cross-correlation vs ref/audio_menu_oracle.wav rises from 0.126. NEXT (focused session): trace the
0a28 event-byte parse (which special byte = program-change) against fist_snd_image.bin asm, find where
it should call 0f99 with the song instrument, wire it (resolve the base-loss / computed-return), verify
reg 0x20->0x31. Bounded, asm-verifiable, port-only-verifiable -- the strongest tractable landable lead.

*** STRONG ROOT CANDIDATE: base-loss in the mode-select vector installer FUN_0000_0872 ***
Traced the sequencer service-vector dispatch to its INSTALLER. FUN_0000_0872 (fist_snd.c:1456,
= asm 0x872-0x965, covers the vector-install at 0x8da) selects the sound MODE (DAT_1000_c012,
1..5) and installs the 7 service vectors from per-mode tables (asm: `mov ax, DS:[0x17d+bx]; mov
[0x17b], ax` ... for 0x17b/0x189/0x197/0x1a5/0x1b3/0x1c1/0x1cf, bx = mode*2). The decompile:
  _DAT_1000_c1b3 = *(undefined2 *)(iVar1 + 0x1b5);   // fist_snd.c:1481, the program-change vector
reads the SOURCE table via `iVar1 + 0x1b5` with NO base -> in the flat port that is g_mem +
(mode*2 + 0x1b5) = linear ~0x1b5 (near-null garbage), a BASE-LOSS (the asm is DS-relative:
`DS:[0x1b5+bx]`). So the 7 vectors -- crucially [0x1b3], the program-change/control-change vector
dispatched at fist_snd.c:1641 when a voice's [0xcc] control byte changes -- are installed from
GARBAGE. Hence the program-change path never reaches 0f99 (instrument load); every note plays with
init instrument 0 (reg 0x20 stuck 0x01, WAV corr 0.126). The vector STORAGE (_DAT_1000_c1b3 ->
linear 0x1c1b3) is consistent between install (1481) and dispatch (1641); only the SOURCE
table reads (0x17d/0x18b/0x199/0x1a7/0x1b5/0x1c3/0x1d1) lost their base.

THE FIX: base-loss correction on the 7 mode-table reads in FUN_0000_0872 (lines 1477-1483) --
add the driver's DS base so they read the real per-mode handler tables. 407-CLASS asm-verified
patch, PORT-VERIFIABLE with no oracle-capture (FIST_OPL_REGLOG: reg 0x20 must go 0x01->0x31;
WAV corr vs ref/audio_menu_oracle.wav must rise from 0.126). REMAINING UNKNOWN (needs a port
run, NOT static): the exact base -- the mode tables are DS-relative DATA (the image at file-offset
0x1b5 is CODE, so the tables are not at fist_snd_base+0x1b5 as raw file bytes; the driver's DS at
install time must be dumped). NEXT: build the port, run with the sound driver active, dump g_mem at
fist_snd_base+0x1b5 vs 0x1c000+0x1b5 (and DS at FUN_0000_0872 entry) to fix the base, then patch
the 7 reads, verify reg 0x20->0x31. This is the strongest tractable landable lead in the project
-- a specific base-loss, a specific function, a clean port-only verification. Same base-loss class
as patch 001 (DGROUP re-base). Unlike the windshield it needs NO oracle-capture rebuild.

Base refined (from the assembler convention, not yet runtime-confirmed): the vector STORAGE
symbol DAT_1000_c1b3 assembles to g_mem+0x1c1b3 (= DGROUP 0x1c000 + 0x1b3), so the driver's
DS base in the port is 0x1c000. Ghidra did NOT recognize the mode-table read `iVar1 + 0x1b5`
as DS-relative (no DAT_ symbol emitted -> the assembler left base 0). So the base-loss fix is:
  *(undefined2 *)(iVar1 + 0x17d)  ->  *(undefined2 *)(g_mem + 0x1c000 + iVar1 + 0x17d)
for all 7 reads (0x17d/0x18b/0x199/0x1a7/0x1b5/0x1c3/0x1d1) at fist_snd.c:1477-1483 -- matching
the _DAT_1000_* (0x1c000) mapping of their write targets. ONE runtime confirmation still gates
landing (code-is-truth, avoid a band-aid): verify the per-mode handler tables are actually
present/valid at 0x1c000+0x17d.. at FUN_0000_0872 install time (dump g_mem there during a
sound-active port run; a valid table = small in-driver handler offsets per mode 1..5). If
confirmed, this is a landable base-loss patch (patch 408/409 slot) that should flip reg 0x20
0x01->0x31 and raise the WAV correlation -- a second verified win after 407, on a CLEAN
port-only verification path (no oracle-capture). This is where a focused audio session resumes.

*** BASE-LOSS HYPOTHESIS RETRACTED -- the assembler already bases these reads (via D) ***
Checked build/fist_snd.c (the code that ACTUALLY runs) before writing any patch: FUN_0000_0872
there reads `*(uint16_t*)(D + iVar1 + 0x1b5)` where `D = g_mem + (DAT_0000_0831 << 4)` -- i.e.
assemble_fist.py ALREADY recognized these as DS-relative and based them on the driver's runtime
DS (DAT_0000_0831). So there is NO base-loss in build/ and NO patch to write here; the re_out
`iVar1 + 0x1b5` I read is the pristine pre-assembler form, correctly re-based by the assembler.
(Same discipline that caught the Layer-2 windshield regression: verify against the built truth
before landing.) The vector install IS correctly based. So the audio defect is NOT the vector
base. The real runtime questions, to settle EMPIRICALLY by instrumenting build/ FUN_0000_0872:
  (1) is FUN_0000_0872 even called (is a sound MODE dev=[D+0x12], 1..5, ever selected)?
  (2) is DAT_0000_0831 (the driver data seg, = load_seg+0x2a5) set correctly so D is valid?
  (3) does [D+0x1b3] end up a VALID program-change handler offset, and does the [0x1b3]
      dispatch at 0aa7/0a28 fire on a program-change event and reach 0f99?
The `mov ds,[cs:0x831]` at asm 0x83b/0xa28/0xaf4/0xc94 confirms [0x831]=driver data seg, loaded
per-function. NEXT: instrument build/ FUN_0000_0872 (log called?/dev/DAT_0000_0831/[D+0x1b3])
+ a run with OPL menu music -> pin which of (1)/(2)/(3) is the actual break. Port-only, clean.

*** SESSION CORRECTION: the vector layer WORKS (PATCH 351/353); no landable lead found today ***
Two of my hypotheses this session were WRONG, both caught by checking build/ (the built truth)
before landing -- the discipline working, no band-aid shipped:
  (1) "base-loss in FUN_0000_0872's mode-table reads" -- WRONG: build/ already bases them on the
      driver DS (D = g_mem+(DAT_0000_0831<<4)); the assembler handles the DS-relativity.
  (2) "install(0872 driver-DS) vs dispatch(0a28 engine-DGROUP) mismatch" -- WRONG: build/ 0a28
      (PATCH 351) also reads D+0x1b3 (driver-DS); install and dispatch agree.
Ground truth in build/: the 7 device method-vectors ARE correctly installed + dispatched. PATCH
353 (device select, 0872) + PATCH 351 (0aa7/0a28 driver-DS rebase) already handle the vector
layer. [0x1b3] = device-3(OPL) slot4 = FUN_0000_10a6 (NOTE-ON) -- it fires, notes play. So the
older board notes claiming "vectors not installed / program-change never routes" are STALE.
The REAL and narrower gap: 0f99 (full instrument patch load, regs 0x20-0xF5) is called ONLY from
the OPL init 104f (instrument 0) -- build/fist_snd.c:2489 is its lone caller -- and NEVER during
playback. The original reloads the song's instrument (reg 0x20 -> 0x31); the port does not. The
device method vectors are note-level (note-on/off), not the patch reload -- so the missing piece
is the SONG-LEVEL program/patch-change: either a song-stream control event in 0a28's parse that
should (re)call 0f99 with the voice's program, or a device method slot (one of [0x17b/0x197/
0x1c1/0x1cf], not yet mapped) that the OPL device uses for patch-change. Measurement is still
owed and needs the port DRIVEN TO MENU MUSIC (a 12s cold run only reaches TITLE.KDV; menu music
needs the menu-nav injection like the verify flows) + FIST_OPL_REGLOG, then: confirm reg 0x20
stuck 0x01, and instrument 0f99 to see if the original's path would call it. HONEST STATUS: no
landable audio patch found this session; the gap is real but deep (song-level patch-change in
the sequencer, same indirect/parse core as the voxel). Both surfaces need focused sessions.

*** PRECISE ROOT (asm-grounded): per-voice instrument assignment at SONG-PLAY is missing ***
Read build/ FUN_0000_0a28 (PATCH 358, the menu-music advance) in full: it ONLY reprograms note
FREQUENCY -- for each of 10 voices it walks the note stream ([0x90], delta bytes; 0x80=hold,
0x81=loop, else=note delta) and on a note change dispatches [ds:0x1b3]=device-3 slot4=FUN_0000_10a6
(OPL A0/B0 fnum), tail [ds:0x189]=slot1=10a5(ret). It parses NO program/instrument-change events.
So the menu song is per-voice monotimbral: each voice's INSTRUMENT is assigned ONCE at song-play,
not mid-stream. 0f99 (the OPL instrument-patch load: writes regs 0x20-0xF5 from the 16-byte record
at driver_ds:instr*0x10+0x1dd, and caches byte[1] in [0xbdd+voice]; note-on 10a6 re-emits reg 0x20
from that [0xbdd] cache) is a DEVICE METHOD -- 0x0f99 appears exactly once in the image, at 0x2c19,
inside the per-mode device-method table (~14-byte records; group with 0x0f99 = device-3/OPL). It is
dispatched via the vector mechanism, and in the port it fires ONLY from the OPL init 104f with
instrument 0 -> every voice plays instrument 0 -> reg 0x20 stuck 0x01, WAV corr 0.126. The ORIGINAL
assigns each voice its song instrument via 0f99 at song-play -> reg 0x20 takes 0x31. THE MISSING
PIECE: the song-play / voice-setup path that calls the OPL instrument-set (0f99) per voice from the
song's per-voice instrument bytes. 0872 seeds all voices to the default stream 0x15b0 + instrument 0;
the real menu song's per-voice instrument assignment is a separate song-play call the port does not
fully run (an unresolved indirect dispatch / a song-play fn not wired). NEXT (focused): find the
song-PLAY entry (sets the real per-voice [0x90] streams AND instruments; distinct from 0872's default
seed), locate its per-voice 0f99 call, wire it; verify reg 0x20 0x01->0x31 + WAV corr rise. This is
the precise, asm-grounded audio root -- the strongest and most specific the surface has reached.

*** DECISIVE POSITIVE CORRECTION: the program-change chain IS FULLY WIRED (board baseline STALE) ***
Read the CURRENT build/ (not the stale notes): the menu music (MAINMENU.MS3, "KGF'91") is driven by
a real MIDI sequencer, and the program/instrument-change path is COMPLETE:
  FUN_0000_0cf3 (MIDI event parse, PATCH 359): per tick fetch (0b5d); on a channel program change
    (D[voice+0x2a] != D[voice+0x34]) it calls FUN_0000_0cfb(prog, ...) THEN 0aa7 (key on).
  FUN_0000_0cfb (instrument program, PATCH 354): sets D[voice+0x34]=prog, loads the per-instrument
    freq/env tables, and TAIL-dispatches vec=[ds:0x1c1] = device-3 slot5 = FUN_0000_0f99(prog) --
    which writes the full OPL patch regs 0x20-0xF5 and caches byte[1] in [0xbdd+voice].
So 0f99 IS called during playback with the SONG's instrument (via 0cf3->0cfb->[0x1c1]->0f99), NOT
only from init 104f. The earlier board baseline -- "0f99 only from init / reg 0x20 stuck 0x01 / WAV
corr 0.126" -- PREDATES PATCH 354/359 and is STALE. My two hypotheses this session (base-loss,
install/dispatch mismatch) were also wrong; ALL of it dissolved on reading the built truth. The
audio program-change is wired; the real open question is now EMPIRICAL and probably much smaller:
what is the CURRENT reg-0x20 behaviour + WAV correlation? Could not measure this session -- a cold
port run sticks at TITLE.KDV (the title video; menu music plays only after it), so measuring needs
driving past the title via input injection (like the verify menu flows), then FIST_OPL_REGLOG +
FIST_AUDIO_WAV. NEXT (tractable, not deep): drive the port past TITLE.KDV to the main menu, capture
the OPL reglog + WAV, cross-correlate vs ref/audio_menu_oracle.wav. If it's now high -> 0003 is far
closer to done than the board says and just needs a matrix audio flow (FIX3 = add the flow); if a
residual gap remains, it is now a small measured delta on a fully-wired chain, not a deep RE bug.
LESSON: the board's audio notes were badly stale (patches 350-359 wired the sequencer since); trust
build/ over old comments. This REOPENS audio as a likely-tractable surface, contra my earlier
"deep like the voxel" framing.

*** MEASURED CURRENT STATE (empirical, this session): menu music never REGISTERS/plays ***
Ran the current build to the main menu (FIST_TICK_HZ=25000 FIST_OPL=1 FIST_SB=1 FIST_OPL_REGLOG,
~80s): the KDV intro completes (395 frames -> "proceeding to main menu"), OPL is active (19860
register writes), BUT the log shows NO MAINMENU.MS3 registration/song, and the instrument-patch
regs are each written EXACTLY ONCE (reg 0x20=0x01, 0x23=0x11, 0x40=0x4f, 0x60=0xf1, 0x80=0x53,
0xc0=0x06 -- the init instrument 0); only reg 0x43 (per-note level/KSL) varies. So those 19860
writes are the INTRO audio playing one instrument -- the MENU MUSIC (MAINMENU.MS3) does NOT play.
=> The real current gap is NOT the program-change (which IS fully wired: 0cf3->0cfb->[0x1c1]->0f99)
-- it is that the menu-music SONG-REGISTER chain never fires, so the wired program-change is never
EXERCISED. fist_sb.c:35-39 already documents the mechanism: "sound-source REGISTER threading
(be0e -> c510 -> 01ec -> 0af4) ... the __allregs indirect-vector dispatch DROPS the register args,
so the methods 01ec/0af4 [don't correctly] read them". So MAINMENU.MS3 (registered via 0af4, PATCH
350) never gets registered/started because the be0e->c510->01ec->0af4 register-arg threading is
incomplete through the indirect-method-vector dispatch. THE REAL AUDIO ROOT (measured): fix the
register-arg threading so 0af4 receives the MAINMENU.MS3 descriptor and the song plays; then the
already-wired program-change (0cfb->0f99) will exercise and reg 0x20 will take the song instruments.
This is a shim/indirect-dispatch threading fix (fist_sb.c + the register method vectors), port-only
verifiable (reg 0x20 gains 0x31.. once the song plays; WAV corr vs ref then measurable). Bounded and
specific -- and it is the SAME indirect-method-vector-threading class as the register-args note in
fist_sb.c. NEXT: complete the be0e->0af4 register-arg thread so MAINMENU.MS3 registers/plays.

Refinement (measured, further trace): the menu-music-START chain is bde4 -> be00 -> be0e(id)
(fist.c:29242/29267/29288) -- be0e registers the source; be00/bde4 are the play trigger. bde4 has
NO direct caller in fist.c (only the icall fmap entry 0xbde4) -> the menu code invokes it via an
INDIRECT dispatch. Two measured runs to the main menu (FIST_TICK_HZ=25000, DESCRDUMP): the port
reaches "proceeding to main menu" but (a) no MAINMENU.MS3 registration, and (b) the FIST_DESCRDUMP
seam never fires -> the menu-music setup (descriptor at DGROUP:0x9f1c + the bde4 register trigger)
does NOT run in the port's menu. So the gap is UPSTREAM of the (wired) program-change: the menu's
indirect call to bde4 (music start) does not fire/resolve. This is the SAME indirect-dispatch class
as everything else -- the menu-front-end's sound-start dispatch is unresolved/not-reached in the
port. Chain, measured end-to-end: [menu code -> INDIRECT bde4 (DOES NOT FIRE)] -> be00 -> be0e
register -> 0af4 (MAINMENU.MS3 into driver) -> 0cf3 MIDI parse -> 0cfb -> [0x1c1]=0f99(prog). Only
the FIRST hop (the menu's indirect bde4 dispatch) is broken; everything after it is wired. NEXT
(bounded): find the menu-front-end site that should indirect-call bde4 to start MAINMENU.MS3 and why
it does not fire in the port (unresolved icall / a menu-state the headless run does not enter), wire
it; then the whole downstream (register -> MIDI -> program-change -> 0f99) exercises and reg 0x20
takes the song instruments. Audio is thus ONE unresolved menu-dispatch away from playing, not a deep
per-note bug. This session traced it from a stale "reg 0x20 stuck" note to this precise measured hop.

Data point (measured): drove the port to the menu WITH mouse input (the menu-nav script) +
OPL reglog -- reg 0x20 STILL written exactly once (0x01), reg 0x60 once (0xf1), 13579 total OPL
writes (the intro/single-instrument audio). So the menu music does NOT play even with menu
interaction -> not merely a headless-idle artifact; the bde4 music-start hop genuinely does not
fire in the port's front-end. Definitively distinguishing "unresolved icall bug" from "the run
never enters the exact music-triggering menu state" needs the DOSBox oracle (confirm the ORIGINAL
fires bde4 / plays MAINMENU.MS3 at this front-end point) -- the one place audio still needs the
oracle. NEXT (focused): oracle-confirm the original's menu-music start point, then either wire the
port's bde4 music-start dispatch (if the port reaches the same state but the icall is unresolved)
or reach the triggering menu state. Everything downstream of bde4 is wired and will exercise once
it fires. This session characterized audio end-to-end from a stale baseline to this single measured
hop; no patch landed but three wrong premises were caught before landing (build-truth discipline).

*** REAL FIX LANDED (patch 408): SOUNDDVR+0xfab instrument-apply was un-decompiled/trapped ***
Used FIST_TRACE_TRAPS to run the port to the menu: the ONE dominant unresolved indirect call was
SOUNDDVR.DVR+0xfab (31x/run, "no fmap entry; returning 0"). Disassembled 0xfab: it is the OPL
PER-VOICE INSTRUMENT-APPLY (bx=instr*16+0x1dd patch record; es=[cs:0x831]=driver DS; writes regs
0xb0/0x20/0x60/0x80/0xe0 x2 + 0xc0 via FUN_0000_0f21; caches [0xbdd+voice]=patch[1]) -- an alternate
entry into the 0f99 loader region that Ghidra had NOT created as a function, so it was absent from
the fmap and every dispatch trapped -> reg 0x20 (+ the whole timbre patch) was never applied and all
voices played the OPL-init instrument 0 (reg 0x20 stuck 0x01).
FIX (doctrine-clean, NO stub/band-aid): (1) added 0xfab to the SND FIST_DRIVER_SEED_OFFS (Makefile)
and re-ran the SND decompile -> Ghidra emitted FUN_0000_0fab cleanly (+1 function, 0f99 unchanged,
re_out regenerated via the legit chain); (2) patch 408 rebases 0fab's base-0 ds: accesses onto the
driver seg D=(DAT_0000_0831<<4), mirroring PATCH 354 for 0f99; asm-verified vs fist_snd_image.bin.
MEASURED (native, port-only): 0xfab traps 31 -> 0; reg 0x20 {01} -> {00,01,03,05,34,4e,b1,b2,c4,c9}
-- the voice instruments now APPLY. This is the first fix that closes a real audio deviation (the
timbre was monotimbral; now it is multi-instrument). make check clean; native+wasm rebuilt. Full
verify.sh both PASSED: 159/159 flows, 0 failed -- crash-free on native AND wasm, native==wasm
bit-identical (the hard invariant), fidelity preserved. Patch 408 LANDED + verified. NOTE: this is distinct from the menu-music SONG-REGISTER gap (bde4 not firing) -- 0xfab
fixes the instrument-apply on whatever song plays; the register/start is a separate remaining hop.

Post-408 sweep (this loop iteration): applied FIST_TRACE_TRAPS systematically -- (1) menu path:
0xfab traps 31->0, remaining 4 traps (0x00000 null, 0x01b31, 0x1360f, 0x13f7f) are all "no FUN_
there" garbage/mid-function computed targets (verified: 0x13f7f is inside FUN_0000_3f3c, 0x1360f
mid-stream), safely trapped -> benign, no real fix. (2) mission/windshield path: ZERO traps (2.4M
log lines are the op-service display-list fallthrough; the extender is fully seeded). => the
un-seeded-function class is EXHAUSTED -- 0xfab (patch 408) was the only real one. WAV-vs-oracle
fidelity measurement is blocked: FIST_AUDIO_WAV writes raw mono PCM (no header) AND the port plays
a different song than ref/audio_menu_oracle.wav (menu music) because the menu-music SONG-REGISTER
(bde4) still doesn't fire -- so patch 408's value stays verified via reg 0x20 (instruments apply)
+ verify.sh both 159/0, not yet via WAV. NEXT audio target: the menu-music-register gate (bde4 not
dispatched by the menu front-end) -- deeper (menu-flow, not a trap): the menu renders (159 flows
pass) but the screen-object's music-start method is not invoked. That is the gate to both the menu
music playing AND the oracle-comparable WAV.

Music-start trace (this iteration): instrumented a15a/9f1d/9f0f/bde4 and ran to the menu --
mustrace count = 0. NONE of the song-play functions fire (not intro INTRO.MS3, not menu
MAINMENU.MS3). So the 19860 OPL writes are the DEFAULT device tune (OPL init 0872->104f), NOT
any real .MS3 song. The gap is NOT specific to the menu-register (bde4) -- it is that the engine's
SCREEN-SYSTEM MUSIC-START DISPATCH never fires for ANY song. This bottoms out in the same deepest
core as the windshield: the engine's MAIN GAME/SCREEN LOOP does not fully run in the port (only
the extender render pipeline + coop-tick drive the 159 frame-pinned flows); the screen objects'
enter/update methods -- which start music, run menu logic, drive the sim/camera -- are not
dispatched. So audio-music, windshield flight-model, and menu logic ALL share this root: the engine
game loop / screen-method dispatch. patch 408 (0xfab instrument-apply) remains the landed win --
it makes whatever plays multi-instrument -- but a REAL song needs the game-loop music dispatch,
which is the deep core (not a trap, not a base-loss; an unreached code path in the screen system).

*** BROWSER AUDIO LANDED + THE WASM REALTIME-CLOCK FIX (commit 5ec6967) ***
The browser now streams the engine's OPL audio to Web Audio, AND the wasm engine now runs at REAL
speed.  Root found via the audio: with the cooperative tick advanced once per pump (CPU-speed spin),
the OPL + mission sim ran ~18x too fast (measured browser audio 825k samples/s vs 44100).  Fix
(g_web_mode-gated, native/node-wasm byte-identical, op-0x24 mission-cockpit frame verified): pace
fist_wasm_tick to WALLCLOCK (emscripten_get_now) at the PIT-programmed INT-8 rate 1193182/pit_div --
exactly the rate the OPL samples/tick (rate*pit_div/PIT_HZ) assumes.  Result: browser audio_rate
43377/s (~44100, was 825k), peak ~5000 (audible), AudioContext running.  This is the wasm-cooperative-
clock fix for PLAYBACK -- corrects audio pitch and paces the mission sim to real time.
FINDINGS: (1) realtime pacing did NOT change the M1CON console-partial (~27k, still the deep HUD-paint
frontier) but the mission render stays STABLE (not black) -- pacing is orthogonal to console-
completeness.  (2) In-mission audio is SPARSE (the same in-mission pump-starvation: audio posts on the
op-0x24 per-render seam at ~0.4fps); MENU audio is smooth/realtime.  Menu MUSIC=OFF is the config
default, so to HEAR music the settings MUSIC toggle must be ON (next: drive it + exercise patch-408
OPL).  Pipeline: fist_opl.c ring -> fist_web_audio_pull -> fist_web_post_audio (EM_JS) -> index.html
AudioContext.

*** VERIFIED AUDIBLE: browser plays real MENU MUSIC (MUSIC=ON) at realtime pitch ***
Drove the browser (headless-chromium CDP) main-menu -> SETTINGS (160,126) -> clicked the MUSIC=ON
radio (181,149; the SOUND group's MUSIC pair, OFF at 181,136 per verify.sh settings-music-off).  The
SETTINGS screen renders PERFECTLY in the browser (55357 nonzero, full chrome -- it is a normal menu
screen, not the deep in-mission frontier).  Captured 5.5 s of the streamed Web-Audio PCM to a WAV
(tools: CDP hook on playPCM).  Spectral analysis (Goertzel): clear musical tones 131/226/297/594 Hz
(~C3/A3/D4/D5), peak/median tonality ratio 15.9 (>3 => tonal/musical), rate 44100/s.  => the browser
produces the genuine Armored Fist AdLib/OPL menu music at correct realtime pitch.  Sent the WAV to the
user.  This closes the browser-audio story: pipeline + realtime PIT-pacing + audible menu music, all
landed (commit 5ec6967).  The remaining audio open items stay the DEEP ones: in-mission audio cadence
(pump-starvation) and full bit-exact-vs-oracle audio stream.

IN-MISSION AUDIO IS BLOCKED ON SOUNDDVR.DVR WIRING (not a cadence fix): investigated why in-mission
audio is sparse (194 samples/s in the browser).  fist_sb.c documents that AF plays in-mission SFX/voice
(and SB-mode music) via Sound Blaster DIGITAL DMA, and that path is NOT WIRED ("STATUS iteration 1: the
SOUND driver SOUNDDVR.DVR is not yet wired -- no SB port is touched yet; this shim is the READY platform
layer, produces real engine PCM the moment SOUNDDVR is wired").  So the streamed browser audio is OPL
(AdLib) only -- which carries the menu MUSIC (already landed) but little in-mission sound.  Draining the
OPL ring faster in-mission would NOT add sound; the in-mission-audio unblock is the SOUNDDVR.DVR
wiring (SeedDriverVecs re-decompile + the DSP/DMA base-loss reconstruction, docs/audio.md) -- a deep
driver task, not a harness tweak.  Conclusion: the tractable browser-audio win (OPL menu music) is
complete; in-mission audio joins the deep-frontier list.

AUDIO native==wasm NEEDS TICK-DRIVEN SYNTHESIS (measured this round).  Terrain just landed in the matrix
as a native==wasm flow (board:0002); tried the same for audio and found the gap concretely: a tick-pinned
run (FIST_TICK_HZ=1000 FIST_DUMPTICK=2000 FIST_OPL=1 FIST_SB=1 FIST_AUDIO_WAV) yields DIFFERENT WAVs
native vs wasm -- nat=1105920 B, node=3093428 B, 775703 diff bytes.  Root: OPL/SB generation is PUMP-driven
(fist_opl_pump advances the synth by elapsed emulated time each pump), and the pump rate differs by target
(native SIGALRM wall-clock vs wasm cooperative), so the two produce different SAMPLE COUNTS for the same
engine-tick window.  The framebuffer is native==wasm because a dumped frame is a FIXED POINT at tick N
(independent of pump count); audio is a STREAM over the whole pump history, so it is not.
FIX (bounded harness change, board:0003): drive OPL/SB sample generation off the ENGINE TICK, not the
pump/wall-clock -- emit exactly samples_per_tick = 44100/tick_hz samples per INT-8 tick (fist_wasm_tick /
tick_advance), so N ticks -> N*samples_per_tick identical samples on both targets.  Then a tick-pinned
menu/mission WAV is native==wasm bit-identical and can become an audio flow in tools/verify.sh (like the
terrain flows).  This is the next matrix-expansion after terrain; it does not touch the OPL/SB synth math
(already the DOSBox DBOPL core), only WHEN samples are pulled.  Oracle-fidelity (vs a clean-provenance
DOSBox capture via the now-headless oracle) is the subsequent Stage-2.

CORRECTION (honest): "pump-driven" above is WRONG.  fist_opl_pump() is EMPTY (fist_opl.c:246); OPL
generation is already TICK-driven via fist_opl_tick() ("once per engine INT-8 tick from the ISR drain",
samples/tick = rate*pit_div/PIT_HZ).  The real native-vs-wasm gap: the tick-pinned run (FIST_DUMPTICK=2000)
produced 12.5 s of audio native (1105920 B = 552938 mono s16) but 35 s wasm (3093428 B = 1546692), a ~2.8x
ratio, even though both stop at engine tick [0x452]=2000.  So the number of fist_opl_tick calls (and/or the
samples_per_tick pit_div) to REACH [0x452]=2000 diverges by target -- native's SIGALRM ISR and wasm's
cooperative drain do not advance the OPL sample clock in lockstep with [0x452].  The framebuffer is immune
(fixed point at [0x452]=N); the audio stream is not.
NEXT (accurate): make the OPL sample clock advance in EXACT lockstep with [0x452] on both targets -- one
fist_opl_tick per [0x452] increment, samples_per_tick from the same pit_div -- so [0x452]=N yields
N*samples_per_tick identical samples native==wasm.  Then a tick-pinned WAV is bit-identical and becomes an
audio matrix flow.  Investigate why native emits ~1/2.8 the ticks: likely native drains multiple wall-clock
ISR ticks per pump while [0x452] and fist_opl_tick count differently, or the menu pit_div read differs at
the sample point.  (Terrain already landed as a native==wasm matrix flow this round; audio is the next.)

AUDIO GAP LOCALIZED TO LIVE pit_div PER opl_tick.  The ISR drain (native_main.c:711-722) couples one
engine-ISR call ([0x452]++) with one fist_opl_tick() 1:1, so both targets run 2000 opl_ticks to reach
[0x452]=2000.  Yet samples differ: native 552938 (276.5/tick) vs wasm 1546692 (773/tick).  fist_opl_tick
(fist_opl.c:230) reads `div = fist_vga_pit0_div()` LIVE each tick and adds rate*div/PIT_HZ samples -- so
the two targets see DIFFERENT live PIT divisors per tick (implied avg div native ~7481 vs wasm ~20915).
The engine programs the PIT deterministically as a function of [0x452], so at the SAME [0x452] the div
should match; the divergence means native (SIGALRM, budget=4 drain/pump) and wasm (coop, 1 tick/pump) are
sampling the div at different phases relative to when the menu reprograms the PIT, OR [0x452] is not
bumped 1:1 with the ISR (a sub-divided counter).  REAL FIX (no fixed-div band-aid, per "code is the
truth"): make the live pit_div read deterministic native==wasm at each opl_tick -- trace whether [0x452]
tracks the ISR 1:1 and whether the div-reprogram sequence is phase-aligned to [0x452] on both; align it so
sum_k rate*div(k)/PIT_HZ is identical.  Then the tick-pinned WAV is native==wasm and becomes an audio
matrix flow.  This is the precise, measured next sub-project for board:0003.

AUDIO GAP FULLY LOCALIZED: ISR-calls-per-[0x452] ratio diverges (div=250 is identical).  Added an
env-gated FIST_OPLDIV trace in fist_opl_tick (logs [0x452] + div per opl_tick).  Findings:
  - div = 250 CONSTANT and IDENTICAL on both targets (samples/tick = 44100*250/1193182 = 9.24).  My
    earlier "pit_div divergence" was WRONG.
  - opl_tick (= one engine-ISR call = one PIT period) fires ~80x per [0x452] increment ([0x452] is a
    SUB-DIVIDED counter the ISR bumps every ~80th call).  native reached [0x452]=766 at 59500 opl_ticks
    (~77.7/[0x452]); wasm [0x452]=1994 at 167000 (~83.8).  FIST_COOP_TICK=1 does NOT fix it: at
    [0x452]=1200 native=~75.5 ISR/[0x452] vs wasm=~87.7 (WAV nat 1673998 vs node 1944102, 775703 diff).
  - So the audio stream length = f(total ISR calls), and the ISR-call-count to reach a given [0x452]
    differs native vs wasm because the ISR->[0x452] sub-division / spin-loop ISR count is not identical
    (the framebuffer is immune: it is a fixed point at [0x452]=N regardless of ISR count).
  REAL FIX (board:0003, no band-aid): make the total ISR-call count a pure function of [0x452] identical
  on both -- trace FUN_1000_30f8 / FUN_1000_3346's [0x452] sub-division and ensure each [0x452] increment
  corresponds to the SAME number of PIT-ISR calls (hence OPL samples) on native and wasm.  Then a
  tick-pinned WAV is native==wasm and becomes an audio matrix flow.  FIST_OPLDIV diagnostic added
  (env-gated, in the fist_opl.c shim).

DEEP TRACE: divergence is the ISR-CALL COUNT, not the per-call emulation.  The engine PIT ISR
FUN_1000_30f8 (@0x130f8) is a VARIABLE-RATE timer: it polls the retrace status `in(word[0x463]+6)` =0x3da
in a countdown loop (d8d4), then reprograms the PIT reload `out 0x40,d8c4` (d8c4 self-adjusts), and calls
FUN_1000_31c3 which bumps [0x452].  Checked the port emulation is DETERMINISTIC per call: fist_vga in()
0x3da TOGGLES bit3 every read (not wall-clock), and the PIT counter read (0x40-0x42) DECREMENTS a
synthetic g_pit[ch] per read -- so each ISR call is reproducible.  Therefore the native-vs-wasm audio
divergence is NOT per-call rate; it is the TOTAL NUMBER of ISR calls (= drained ticks) run to reach a
given [0x452].  FIST_COOP_TICK=1 (native drives 1 tick/pump like the node/wasm g_web_mode=0 path) did NOT
equalize it (WAV nat 1673998 vs node 1944102) -- so even under cooperative ticking the two run different
ISR counts per [0x452].  Also the OPL starts at different [0x452] (native=1, wasm=34), a start-offset on
top of the ratio gap.  NEXT (focused debug): instrument the ISR-call count vs [0x452] under FIST_COOP_TICK
on both and find why they differ tick-for-tick (candidate: extra pump/drain calls in one target's spin
loops, or the d8c4 variable-reload feedback diverging from a different in()-read interleaving).  Once the
ISR-call count is a deterministic function of [0x452] on both, the tick-pinned WAV is native==wasm and
becomes an audio matrix flow.  (Terrain already native==wasm in the matrix; audio is the open frontier.)

TRUE ROOT: the per-read 0x3da RETRACE TOGGLE makes the variable-rate PIT ISR non-deterministic.
Under FIST_COOP_TICK the OPL start-offset aligns (both [0x452]=34 at opltick=0), but the ISR-per-[0x452]
ratio K still diverges: native 43500 opltick @ [0x452]=594 (K~77.7) vs wasm 58500 @ [0x452]=599 (K~103.5).
The engine ISR 30f8 runs a retrace-countdown `do{ in(0x3da); d8d4--; }while(d8d4 && !(bit3))` and feeds
d8d4 into the self-adjusting PIT reload d8c4 (its variable rate).  fist_vga in(0x3da) does
`g_3da_toggle ^= 0x09` -- a GLOBAL per-read toggle, so bit3's value at ISR entry depends on the TOTAL
count of 0x3da reads so far, which includes the render/vsync busy-poll reads that differ native vs wasm.
-> the ISR sees a different retrace phase -> different d8d4/d8c4 -> different K -> different total ISR
calls (OPL samples) per [0x452] -> audio not native==wasm.  The framebuffer is immune (fixed point at
[0x452]=N).  REAL FIX (shim, board:0003): make the 0x3da retrace bit DETERMINISTIC as a function of the
engine tick / [0x452] (e.g. bit3 = f(tick phase)) instead of a free-running per-read toggle, so the ISR's
retrace-countdown is identical native==wasm -- while still terminating the render vsync busy-waits.  This
touches a port every covered flow reads (0x3da), so it must be re-gated 10x on the 162-flow matrix to
prove byte-neutrality on the existing surface before the audio flow is added.  This is the precise,
root-caused next sub-project for audio bit-identity.

RETRACE-RESET ALONE INSUFFICIENT (tried + reverted, unverified goal-not-met change kept out of tree).
Prototyped resetting the 0x3da toggle phase at each ISR entry (fist_vga_retrace_reset from the drain loop):
it REDUCED the native-vs-wasm audio diff (775703 -> 574918 B) and changed the WAV sizes, but audio is
STILL not bit-identical (native ~72.6 ISR/[0x452] vs wasm ~96.9).  So the retrace phase is ONE
non-deterministic input but not the only one.  SECOND source: in the MENU (g_mission_coop==0) native's
async SIGALRM STILL calls tick_advance IN ADDITION to the FIST_COOP_TICK coop tick -> native's total
tick/ISR count per [0x452] differs from wasm's pure-coop count.  So audio native==wasm needs BOTH:
(a) deterministic 0x3da retrace-countdown phase, AND (b) a pure-cooperative tick source for the audio
capture (no async SIGALRM contribution) -- i.e. run the audio verify flow under the mission-coop-style
lockstep even in menus (a FIST_AUDIO_DETERM mode that forces coop + retrace-reset), then a tick-pinned WAV
is native==wasm.  Both are shim changes touching the 0x3da/tick path that every flow uses, so the audio
flow lands with a full 10x re-gate.  Reverted the partial prototype to keep the 162-flow matrix verified;
FIST_OPLDIV trace retained.  Precise next sub-project: implement (a)+(b) together under one env gate.

FIST_AUDIO_DETERM (retrace-reset + pure-coop TOGETHER) also INSUFFICIENT -- reverted.  Adding the
pure-cooperative tick source (latch g_mission_coop to stop SIGALRM) ON TOP of the retrace-reset produced
the EXACT SAME WAVs (nat 805502 / node 1074858, 574918 diff) as retrace-reset alone -- so SIGALRM is NOT
the second source either (it was not contributing).  The ISR-count-per-[0x452] still diverges (native
72.6 vs wasm 96.9) under identical cooperative ticking + deterministic retrace.  Hypotheses now DISPROVEN
for the audio native==wasm gap: pump-vs-tick, pit_div, retrace-phase (partial only), SIGALRM tick-source.
REMAINING: the engine itself calls fist_timer_pump (-> ISR) a different NUMBER of times per [0x452] on
native vs wasm even with identical code + deterministic port emulation -- i.e. an engine-level control-flow
divergence (a data-dependent spin-loop iteration count, or a still-nondeterministic port read the ISR/menu
logic branches on).  NEXT (differential trace): log the (pump#, [0x452], key DGROUP timer vars d8c4/d8d2/
d8d4/c452) per ISR on BOTH targets under coop and diff to find the FIRST tick where they diverge -- that
pins the real source.  This is a deeper determinism problem than the framebuffer (which is immune as a
fixed point); audio remains the open frontier.  All partial prototypes reverted; matrix stays 162/162.

AUDIO native==wasm LANDED for the deterministic window (matrix flow audio-opl-init).  Bisected the
divergence: the OPL FM WAV is native==wasm BIT-IDENTICAL (diff=0, reproducible) up to FIST_DUMPTICK=120
under FIST_COOP_TICK (122400 B both), and diverges only between [0x452]=120 and 250 (DT=250 diff=87799).
The opltick->[0x452] curves are IDENTICAL for the first ~213 ticks then diverge at a STATE TRANSITION
that RESETS [0x452] (wasm trace shows [0x452] 124 -> 110 at a later opltick) -- i.e. an intro->menu (or
similar) screen change whose onset is reached at a different opltick on the two targets.  So the audio
engine itself is deterministic native==wasm within a screen; the divergence is the TRANSITION timing.
LANDED: tools/verify.sh audio-opl-init flow -- OPL FM audio full-WAV native==wasm over the [0x452]=120
window (div=250 fixed via FIST_TICK_HZ=1000).  Matrix now 165 flows.  OPEN (full-duration audio): make
the state-transition onset ([0x452] reset point) deterministic native==wasm -- the transition fires at a
different opltick because the pre-transition screen's exit condition (likely the KDV intro frame count or
a timed auto-advance) resolves at a target-dependent tick; trace the exit predicate.  Then the audio flow
can extend across transitions to full menu/mission music.

TRANSITION-ONSET DIVERGENCE PINNED TO A SINGLE wasm-EXCLUSIVE [0x452]-RESET (FIST_OPLSEQ probe, this
round).  Added an env-gated per-opltick trace FIST_OPLSEQ in the fist_opl.c shim ("opltick c452" per line
+ a RESET marker when c452 drops).  Ran BOTH targets under the audio flow (FIST_TICK_HZ=1000 div=250
FIST_COOP_TICK, no FIST_MOUSE) and diffed the c452-vs-opltick curves:
  - opltick<->[0x452] is BIT-IDENTICAL native==wasm for opltick 0..11915 (both climb to [0x452]=188 at the
    exact same oplticks).  So per-screen the audio ISR + retrace-countdown + ISR-per-[0x452] ratio K are
    fully deterministic native==wasm -- this SUPERSEDES the earlier "K diverges per screen" reading (that
    was measured ACROSS the transition).
  - at opltick 11916 EXACTLY, wasm RESETS [0x452] 188->0 (a screen/timer re-init), while NATIVE continues
    MONOTONICALLY (188->189 at opltick 11985 ... ->[0x452]=800 at opltick 59485, NO reset in a 90s window).
  So the sole audio divergence is a wasm-EXCLUSIVE [0x452]-reset event that native never executes in-window,
  at an identical engine state (identical [0x452] and identical opltick up to the event).

RULED OUT this round:
  - g_web_mode: node runs callMain/_main (NOT fist_web_start), so g_web_mode=0 on BOTH; and it is byte-
    identical 0..11915 (a g_web_mode split would diverge from tick 0).  The native_main.c g_web_mode
    branches (580/598/3074) do not fire here.
  - engine floating-point: the engine uses NO float/double -- all 11 "double" matches in build/fist.c are
    the WORD in comments (double-deref/-buffer/-click).  So x87(native 80-bit) vs wasm(64-bit) is NOT the
    cause.  (The OPL shim's doubles are in the ISR path, which is proven identical 0..11915.)
  - retrace-phase per screen: K identical per screen (above), so the 0x3da toggle is consistent within a
    screen; only the transition predicate differs.

REMAINING (the real root): with identical code, identical g_web_mode=0, no FP, and identical state to
opltick 11915, the transition predicate at 11916 must branch on a PLATFORM READ that returns different
values native vs wasm (input poll / DOS or KDV stream position / an uninitialised host-side var).  PRECISE
NEXT PROBE: instrument the c452 write sites (build/fist.c 15603/17684/17687/17954/33782/34031) to log
which one performs the 188->0 reset on wasm and confirm native never reaches it; then read that site's
guard and identify the divergent platform input.  That input, made deterministic native==wasm, extends the
audio flow across the transition to full menu/mission music.  Matrix intact 175/175; FIST_OPLSEQ retained
(env-gated, byte-neutral: audio-opl-init still diff=0, 122400 B both).

TRANSITION ROOT = d97e DROPPED RETURN VALUE (asm-faithful fix found; landing BLOCKED on the extender
intro-render frontier -- prototyped patch 411, verified the win, REVERTED for a matrix regression).
Full call-chain trace of the intro->menu transition that diverges the audio:
  e714(menu-enter, resets [0x452]=0) <- cae6(boot->menu init) -step12-> e446 -> e584(intro anim-script
  interpreter).  e584's loop exits early on `FUN_0000_d97e(puVar6) != 0`.  d97e asm (0xd97e-0xd99a) is
  `store param->task+0x3f2 ; aa10=0x78 ; call e339 ; ret` -- so d97e's return IS e339's AX.  e339 far-
  calls the Doug-Huffman extender gate [ea16]=0762:1179=linear 0x8799 (extender op 0x78 = task-execute/
  present the intro frame).  Ghidra rendered d97e as `FUN_0000_e339(); return;` -- DROPPING the value.
  So d97e returned leftover EAX: measured 0 on native, 0x100000 on wasm (FIST_D97E probe) -> wasm's e584
  SKIPPED the 573-frame intro every boot, native played it -> different ISR-tick accumulation -> audio
  streams diverged at exactly opltick 11916 ([0x452] 188->0 = e714 menu-enter, pinned via FIST_C452W to
  build/fist.c site + FIST_E714 to caller cae6/32781).
FIX (asm-faithful): d97e `FUN_0000_e339(); return;` -> `return FUN_0000_e339();`.  e339's op-0x78 return
  is a DETERMINISTIC 0 on BOTH (FIST_E339 probe: aa16=0x8799 gate, uVar1=0 both) -> both play the intro ->
  transition fires at the same opltick.  VERIFIED: audio WAV over the intro (FIST_TICK_HZ=1000
  DUMPTICK=300 COOP_TICK, OPL+SB) is native==wasm BIT-IDENTICAL (diff=0, 381008 B), up from [0x452]=120.

WHY IT CANNOT LAND ALONE (matrix regression -> reverted): with d97e fixed, wasm now PLAYS the intro like
native -- but the extender task-execute service (op 0x78 / gate 0x8799) is a STUB (traps to 0, the
documented "extender-service frontier"), so the intro renders BLANK.  Under the non-COOP framebuffer-flow
regime (FIST_TICK_HZ=25000, native async-SIGALRM vs wasm cooperative), wasm playing the blank intro fails
to reach the tick-pinned dump [0x452]=2600 in time -> battles-cancel-briefing dumps a near-blank frame
(nonzero 0.7% vs native 94.4%) -> nat!=wasm (181417 B).  1 of 175 flows regressed; the other 174 stayed
green.  The OLD green was wasm ACCIDENTALLY skipping the intro (the d97e bug) landing on the same settled
briefing fixed-point as native -- i.e. the flow passed for the wrong reason.
COUPLING (board:0003 <-> extender frontier): audio full-duration native==wasm REQUIRES both (a) the d97e
return-value fix AND (b) a faithful extender intro-render service (op 0x78) so the intro plays+renders
identically within the flow timeouts on both targets.  They must land TOGETHER.  Patch 411 kept out of
tree until (b) exists.  Matrix restored 175/175.  Diagnostics retained: FIST_OPLSEQ (committed).

FUNDAMENTAL BLOCKER RE-IDENTIFIED = the async(native)-vs-coop(wasm) TICK REGIME, NOT the extender render.
Isolated with patch 411 re-applied as a build-only experiment (reverted):
  - patch411 + async native (default):  native reaches the briefing at [0x452]=2600 (nonzero 0.944),
    wasm (always coop) is STILL in the intro at [0x452]=2600 (nonzero 0.007) -> nat!=wasm.  NOT a timeout:
    wasm with a 400s timeout completes (rc=0) and STILL dumps the blank intro -> wasm genuinely needs MORE
    [0x452] than native to reach the briefing.
  - patch411 + COOP_TICK on BOTH:  native AND wasm are blank at [0x452]=2600 (both 0.007) and MATCH
    (diff=0).  So native+COOP behaves like wasm+coop -> the divergence is the REGIME, not the target.
CONCLUSION: the intro+nav sequence costs a DIFFERENT number of [0x452] ticks under async vs cooperative
ticking (the async SIGALRM does engine work between [0x452] increments that coop does not), so the
framebuffer "fixed point at [0x452]=N" is NOT regime-immune across a screen TRANSITION sequence -- it is
immune only WITHIN a settled screen.  The current 175-flow matrix passes only because wasm ACCIDENTALLY
skips the intro (the d97e bug) and lands on the same settled [0x452]=2600 briefing state as native-async.

THEREFORE the audio full-duration native==wasm path is:
  (1) patch 411 (d97e return, asm-faithful, verified);
  (2) UNIFY the tick regime: drive native cooperatively too (no async SIGALRM) so native==wasm is
      regime-identical -- then BOTH play the intro identically in [0x452] terms (audio already proven
      diff=0 under COOP_TICK+411, and briefing already proven diff=0 under COOP+411);
  (3) RE-CAPTURE the ref framebuffers under the cooperative regime (the current refs were captured from
      native-async, which reaches different [0x452]=N states across transitions);
  (4) re-gate 10x.
This is a determinism-FOUNDATION change (native_main.c tick source + a ref recapture), larger than a
single patch but now precisely bounded.  The extender op-0x78 render (board:0009) is for the intro's
VISUAL fidelity (blank vs real frames) -- it is NOT the audio blocker; audio is regime-sensitive, not
render-sensitive (audio diff=0 was achieved WITH a blank intro under COOP_TICK).  Matrix intact 175/175.

TICK-REGIME UNIFICATION DE-RISKED (measurement, no tree change): ran native with FIST_COOP_TICK vs native
default (async) on settled-screen ref'd flows -> mainmenu diff=0, settings diff=0.  So native-coop is
OUTPUT-IDENTICAL to native-async on settled screens (the framebuffer fixed point is regime-invariant
WITHIN a screen, as expected).  The regime only changes output on TRANSITION-crossing flows (the intro
[0x452]-cost).  Implication: switching native to cooperative ticking will NOT perturb the settled-screen
matrix (mainmenu/settings/about/cockpit/terrain/editor fixed points) -- only the few transition-crossing
flows (battles-cancel-briefing, campaign-missions, and any that boot through the intro before a tick-pin)
need their dump-tick moved past the (now-played) intro + a ref recapture.  This bounds the board:0003
foundation change to: (1) native_main.c cooperative default, (2) patch 411, (3) recapture the handful of
transition-crossing refs, (4) 10x re-gate -- most of the 175 flows carry over unchanged.  Matrix intact
175/175 (measurement only).

AUDIO native==wasm COVERAGE MEASURED after patch 411 (the intro-skip fix): the OPL+SB stream is now
BIT-IDENTICAL native==wasm far past the intro -- diff=0 at [0x452] = 300 / 600 / 1000 / 2000 / 4000
(4000 = 5,696,650 B WAV, ~64 s of audio: the full title intro + the menu music).  It DIVERGES again by
[0x452]=8000 (diff=776594, node WAV 1442 B shorter) -> a SECOND transition-onset divergence of the same
class as the intro was (a screen change whose onset lands at a target-dependent opltick), somewhere in
4000..8000.  So patch 411 made the ENTIRE intro+menu phase deterministic; the next divergence is a later
menu/sub-screen transition (to be pinned the same way: FIST_OPLSEQ + FIST_C452W the reset site).  The
audio-intro matrix flow is extended 300 -> 1000 (safely inside the diff=0 region and the run_audio
timeout).  Full-duration audio across MISSION transitions is the remaining open work; the tick-regime
unification (de-risked above) plus pinning each successive transition onset is the path.

2ND TRANSITION PINNED (the [0x452]~8000 audio-coverage boundary): FIST_OPLSEQ located the divergence at
opltick 339264 -- BOTH targets reset [0x452] near c452=4399 (wasm @339264 c452=4398 -> 0; native @339352
c452=4399 -> 0) but 88 oplticks APART.  FIST_C452W + backward signature search pinned the reset site to
FUN_1000_2e6b (NOT e714): a 1000-segment SCREEN/MODE-INIT that fills the far-call block at 0x000f0000
(uRam000f0002..0010), calls fist_int_dispatch(), then zeroes DAT_1000_c452 / c432 / d8ca / c44e / c450
and sets c738=0xff.  So this is a mode-set / screen re-init reached via an INT dispatch, fired at a
TARGET-DEPENDENT opltick (88 apart) -- a spin-loop / timing divergence in its CALLER, NOT the clean
dropped-return class the intro (d97e) was.  This is the current audio-coverage boundary: native==wasm
diff=0 holds to [0x452]=4000 (~64s, the full intro + menu music), and the FIRST divergence past it is this
2e6b mode-init at c452~4399.  NEXT audio step (scoped): trace 2e6b's caller and the predicate that fires
it 88 oplticks apart under identical cooperative ticking (candidate: a busy-wait iteration count that is
data-dependent or reads a still-nondeterministic port) -- the same async-vs-coop / spin-loop determinism
family as the deeper board:0003 work.  Harder than the intro; likely needs the tick-regime unification
(de-risked above) to resolve cleanly.  Matrix intact 176/176; audio-intro flow pinned at the safe
[0x452]=4000.
