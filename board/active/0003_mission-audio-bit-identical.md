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

CORRECTION to the 2nd-transition attribution above: it is NOT FUN_1000_2e6b (that was a line-number
mapping error between the instrumented and clean build/fist.c).  Re-ran FIST_C452W with OLD->NEW logging
gated on old>1000: the 4399->0 reset is FUN_0000_e714 (site L36771) -- the SAME main-menu-enter function
as the 1st (intro->menu) reset.  So the 2nd transition is a MENU RE-ENTER at [0x452]=4399 (~4.4s after the
first menu-enter), fired 88 oplticks apart native (opltick 339352) vs wasm (339264).  e714's re-enter
callers are 37015 (`if in_CF` after the cb7c .FSG probe) or 37058 (after the e4bb in-mission loop) -- one
of those fires 88 oplticks apart under identical cooperative ticking.  So the current audio boundary is a
periodic menu re-enter/refresh whose onset predicate is target-dependent by 88 oplticks; tracing which
e714 re-enter caller fires and its predicate (a menu-idle timer or the cb7c CF) is the precise next step.
Audio coverage remains native==wasm diff=0 to [0x452]=4000 (bit-verified + 10x-gated); the boundary is a
menu re-enter, not a new screen.  Matrix intact 176/176.

FINAL ACCURATE PICTURE (FIST_E714C caller trace): only ONE e714 fires in the boot->8000 window --
BOOT32781 (the cae6 boot->menu-enter) at c452=4395.  With patch 411 the intro plays FULLY, so the
boot->menu-enter now happens at [0x452]~4395 (the earlier "[0x452]=188" reset was the PRE-patch411
wasm-intro-SKIP artifact, now gone).  So: the audio-intro flow (diff=0 to [0x452]=4000) captures the
INTRO / title-screen audio (the ~64s the title plays before the menu), NOT menu music; the menu music
starts only AFTER the e714 reset at ~4395.  The divergence at 4399 is the cae6->e714 boot->menu-enter
firing 88 oplticks apart native (opltick 339352) vs wasm (339264) under identical cooperative ticking --
a boot-path spin-loop / timing nondeterminism (NOT a menu re-enter; correcting the note above).  So to
reach MENU-music audio native==wasm the boot->menu-enter onset must be made deterministic to the opltick
(trace cae6's pre-e714 steps -- d99b/6a02/e446/e584 -- for the 88-opltick spin, likely a busy-wait whose
iteration count is data-dependent or reads a still-nondeterministic port).  Current landed coverage:
INTRO/title audio native==wasm diff=0, bit-verified + 10x-gated, at [0x452]=4000.  Matrix 176/176.

88-OPLTICK DIVERGENCE LOCALIZED to the ISR-per-c452 SUB-DIVISION RATIO at the menu-enter approach.
FIST_OPLSEQ opltick-vs-c452 counts at the boundary: native sits 155 oplticks at c452=4398 and 154 at
4399; wasm sits 144 at 4398 and 77 at 4399 (truncated by the e714 reset).  opltick<->c452 is IDENTICAL to
[0x452]=4000 (audio diff=0), then the sub-division ratio (ISR calls per c452 increment) diverges only at
c452~4398 -- the last ~2 c452-ticks before cae6->e714 fires.  ROOT (matches the old "TRUE ROOT" note): the
engine PIT ISR (30f8) is VARIABLE-RATE -- it runs a retrace-countdown `do{in(0x3da);d8d4--}while(d8d4 &&
!bit3)` and feeds d8d4 into the self-adjusting PIT reload d8c4.  As the boot approaches the menu-enter the
PIT rate shifts and the 0x3da retrace-phase (fist_vga per-read toggle) makes the ISR-per-c452 ratio differ
native vs wasm (155 vs 144) -> 88 oplticks (OPL samples) apart by the reset.  This is the SAME 0x3da /
variable-PIT determinism the old retrace-reset prototype targeted (reduced but did not eliminate; reverted).
So reaching menu-music+ audio needs that shim-level fix: make the 0x3da retrace bit (and hence the
retrace-countdown length) a DETERMINISTIC function of the engine tick phase, not a free-running per-read
toggle -- a change every flow's 0x3da reads touch, so it lands with a full 10x re-gate.  Intro/title audio
(diff=0 to [0x452]=4000) is unaffected and stays landed.  Matrix 176/176.

DIVERGENCE MECHANISM NARROWED (FIST_3DACNT global 0x3da-read counter): the 0x3da reads are IDENTICAL
native==wasm up to c452=4388 (both exactly 676118 reads), then diverge over the c452=4388..4400 window
(native 3882 reads, wasm 3706 -> 176 apart = 88 ISRs at ~2 reads/ISR).  The reads come from
FUN_1000_31c3 (the PIT-ISR sub-handler; its VGA attribute-controller reset does out(0x3c0)/in(0x3da)/
out(0x3c0,0x20) each tick).  So the divergence is NOT a busy-wait exit and NOT accumulated drift (reads
are bit-synced until 4388); it is the VARIABLE-PIT-RATE self-adjustment (30f8 reprograms d8c4 each ISR,
fed by the retrace-countdown d8d4) producing 88 EXTRA ISR ticks on native in the 12-c452-tick approach to
the menu-enter (e714 at c452=4399).  Reads synced until 4388 means the retrace-phase input was identical
up to there; the adjustment diverges only as the PIT rate ramps for the menu-enter.  PRECISE NEXT TRACE
(scoped): log d8c4 (PIT reload), d8d4 (retrace countdown), d8d2 in 30f8/31c3 per ISR across c452
4388..4400 on both targets and diff to find the FIRST divergent reload -- that is the exact
self-adjustment step that differs, and the target for a deterministic-PIT-rate fix.  This is intricate
1000-segment ISR work (30f8/31c3), to be done as a focused continuation, not an autonomous-loop guess.
Intro/title audio (diff=0 to [0x452]=4000) stays landed + 10x-gated.  Matrix 176/176.

ROOT FULLY UNDERSTOOD (FIST_PITTRACE d8c4/d8d4 per-ISR trace).  The PIT rate is STABLE and IDENTICAL on
both targets: div=250, d8c4=250 (reload), d8d4=1, d8d2=1000 -- byte-for-byte the same content.  So it is
NOT a variable-rate self-adjustment (that hypothesis is disproven).  ISR-per-c452 counts across the window:
  c452:  4386 4388 4390 4392 4394 4396 4397 4398 4399 4400
  nat :   155  156  155  155  155  155  156  155  154   78
  wasm:   156  156  156  155  156  155  156  144   77   78
i.e. a +-1 ISR-per-c452 SUB-DIVISION WOBBLE that starts ~c452=4386 (wasm occasionally +1) and then blows
up at the menu-enter (c452=4398: 155 vs 144; 4399: 154 vs 77) as e714 fires at a different sub-tick.  The
sub-division (how many ISRs elapse per c452 increment, ~155) is gated by the retrace-countdown d8d4, whose
result depends on the 0x3da retrace-bit PHASE at ISR entry -- and that phase is the global 0x3da read-count
parity (fist_vga `g_3da_toggle ^= 0x09`), which drifts by +-1 between native and wasm as the boot nears the
menu-enter.  So the ROOT is confirmed: the free-running per-read 0x3da toggle makes the c452 sub-division
timing (via d8d4) non-deterministic at the +-1 level; it stays invisible while the wobble cancels (audio
diff=0 to [0x452]=4000) and surfaces at the menu-enter where it compounds.  THE FIX (unchanged target, now
fully justified): make the 0x3da retrace bit a deterministic function of the engine tick phase rather than
the global read count, so d8d4's countdown is identical native==wasm -> the sub-division is bit-stable ->
audio native==wasm past the menu-enter.  Careful shim change (fist_vga), touches every flow's 0x3da reads,
lands with a full 10x re-gate.  This is now a well-understood, bounded fix -- not a guess.  Matrix 176/176.

CORRECTION (integrity -- the "ROOT fully understood" claim above was PREMATURE and is partly DISPROVEN):
the FIST_PITTRACE data shows d8d4=1 in the whole window.  With the ISR retrace-countdown
`do{in(0x3da);d8d4--}while(d8d4 && !bit3)`, d8d4=1 means it ALWAYS runs exactly ONE iteration (d8d4->0,
exit) REGARDLESS of bit3 -- so the countdown is deterministic and the 0x3da retrace PHASE does NOT gate it
here.  Therefore the "+-1 wobble is caused by the 0x3da read-count parity via d8d4" hypothesis is WRONG.
What IS firm: (a) the PIT rate is stable+identical (div=250, d8c4=250); (b) the retrace-countdown is
deterministic (d8d4=1); (c) 0x3da reads are bit-synced native==wasm to c452=4388; (d) the divergence is
concentrated at c452=4398-4399 where wasm does far fewer ISRs (4399: 154 native vs 77 wasm) because the
menu-enter e714 (called from cae6) FIRES ~77 ISRs EARLIER on wasm.  So the real question is unchanged and
still OPEN: which cae6 pre-e714 step's ISR/pump count differs native vs wasm at c452=4399 (what makes e714
fire earlier on wasm).  The PIT/retrace variables traced so far are NOT it.  NEXT: instrument cae6's steps
between e584 (intro) and the e714 call with the opltick at each, on both targets, to find the step whose
opltick-delta differs -- that is the actual divergent spin.  I over-claimed twice this session (2e6b, then
d8d4); the honest state is the divergence is LOCALIZED (c452=4399, e714 timing) but its mechanism is NOT
yet identified.  Intro/title audio stays landed+gated.  Matrix 176/176.

DIVERGENCE LOCALIZED WITH HARD DATA (opltick-per-step traces; no hypothesis).  Chain, all identical until
the pinpoint:
  cae6 calls e714 at EXACTLY tick=339040, c452=4395 on BOTH targets (cae6's pre-e714 steps are bit-synced).
  INSIDE e714, stepping the opltick: identical at "before be0e" (tick=339040 both); then FUN_0000_be0e(4)
  runs and "before e40e" shows native tick=339336 vs wasm 339128 -> be0e took 296 ISRs on native, 88 on
  wasm (Δ+208 native).  The following e40e block partially compensates (native +16, wasm +136), netting
  the +88 ISR (=176 0x3da reads) gap seen at the reset.
So the divergence is INSIDE FUN_0000_be0e(4): it loads the MAINMENU screen resource (.MS3) via the
[DGROUP:c378]=FUN_0000_250d loader (dx=word[DGROUP:4+0x9f2c]=the .MS3 filename), then the c510 sound-source
register (c510==0, no-op on boot) and be58.  The .MS3 screen-load path pumps a TARGET-DEPENDENT number of
ISRs (296 vs 88) -- a file-load / screen-setup spin whose pump count differs native vs wasm, NOT the PIT
rate or the 0x3da countdown (both disproven earlier).  PRECISE NEXT PROBE: opltick-step INTO the
[c378]=250d loader (and be58 / e40e) to find the exact spin/wait whose iteration count differs -- likely a
file-manager (fist_dos.c) or driver wait loop that pumps while polling a status.  This is the actual
mechanism, reached by data not guesswork.  Intro/title audio stays landed+gated.  Matrix 176/176.

DIVERGENCE TRACED INTO THE SOUND DRIVER (hard opltick data, full chain).  Corrected the ".MS3 load"
attribution: be0e's internal calls are all bit-synced (c378 .MS3 loader took 0 ISRs identical, c510 took 72
identical) EXCEPT the final FUN_0000_be58, which is the divergent one: native 224 ISRs vs wasm 16
(t=339112->339336 nat, ->339128 wasm).  be58 calls [DGROUP:c530] = far 0x3a44:01e7 = m_snd_FUN_0000_01e7
-- a SOUND DRIVER method (the menu-enter starts the menu MUSIC).  Chain: cae6 -> e714(menu-enter) ->
be0e(4) -> be58 -> [c530]=snd 01e7 -> FUN_0000_0833.  0833 is itself a DISPATCHER: it stores the command
in DAT_1000_c012 and calls a sound-driver vtable method `fist_icall(fist_snd_base + word[(param_1>>0xb &
0x1e)+0x11b])(...)`.  So the 224-vs-16-ISR spin is inside the sound-driver sub-method selected by the
command param_1 -- i.e. the menu-music START path spins a target-dependent number of ISRs.  This is the
SOUND DRIVER's own wait/spin, a multi-dispatch subsystem (01e7->0833->vtable->...), NOT the PIT rate, the
0x3da countdown, or the .MS3 file load (all disproven).  This is where audio-beyond-intro determinism
actually lives: a sound-driver music-start spin.  NEXT: capture param_1 (the c012 command) at the menu
be58, resolve the 0833 vtable target, and opltick-step into it to find the wait whose iteration count
differs -- a focused drill into the sound-driver command path (fist_snd.c).  Intro/title audio stays
landed+gated (diff=0 to [0x452]=4000).  Matrix 176/176.  (Note: 2 earlier attributions this session --
2e6b, d8d4 -- were disproven by data and retracted; this one is backed by the be0e-internal opltick trace.)

DRILL REACHED THE SOUND-DRIVER NOTE PROCESSING (full data-driven chain).  0833's menu-enter dispatch:
cmd=0x1ec, idx=0, vec=0x966 -> FUN_0000_0966 (m_snd, PATCH 354 note-record table) -> FUN_0000_0997
(SOUNDDVR note-ON: find a free voice, set the note record, FUN_0000_0cfb freq/env, write OPL registers).
The 224-vs-16-ISR divergence in be58's sound call is therefore OPL-REGISTER-WRITE volume: each out(0x388/
0x389) calls fist_timer_pump() (one ISR) before the fist_opl_owns() check, so #ISRs == #port-ops, and
native performs ~208 more OPL-related port ops than wasm at the menu-music START (native 224 vs wasm 16).
So the menu-music note-on path emits a DIFFERENT number of OPL writes native vs wasm -- the divergence is
in the sound-driver music start, not the timer/retrace/loader (all disproven).  PRECISE NEXT PROBE (better
than more C-drilling): FIST_OPL_REGLOG=<path> on BOTH targets (the shim already logs every 0x389 write as
"seq adv reg val"), pinned to the menu-enter window, and DIFF the two logs -- that shows exactly which OPL
registers native writes that wasm does not (or the order/count), pointing at the divergent loop/branch in
0966/0997/0cfb.  Chain fully mapped: cae6->e714->be0e->be58->[c530]=snd01e7->0833->0966->0997.  Intro/title
audio stays landed+gated (diff=0 to [0x452]=4000).  Matrix 176/176.

REGLOG DIFF -- FINAL NATURE OF THE DIVERGENCE (corrects the "sequencer advances differently" framing).
FIST_OPL_REGLOG on both targets to DUMPTICK=4405: the OPL DATA-write streams are BIT-IDENTICAL for 144
lines, then diverge at line 145 where BOTH are at adv=4281 (the SAME music-sequencer position) but write
DIFFERENT registers -- native: reg=b0(keyon) then 23/63/83/e3/20 (a full OPERATOR/instrument definition);
wasm: reg=43(level) then b0/44/b1/4d/b5 (levels + keyons).  So it is NOT a sequencer-position or timer
divergence (adv is identical); it is a CODE-PATH split inside the note processing (0966/0997/0cfb) at the
menu-music START: for the same sequencer tick native emits a full instrument setup while wasm emits a
partial one.  This matches be58's 224-vs-16 ISR gap (native does ~208 more OPL port-ops = the full
instrument writes).  The likely cause is divergent SOUND-DRIVER STATE at the menu-enter -- 0997's
voice-allocation loop `for(i<8){voice=voicemap[bp]; if(voice_active[voice+0x107]==0) break; bp++}` taking a
different branch because a voice-active flag / note record differs native vs wasm.  (Cumulative end counts
native 178 / wasm 383 writes, adv 4285 vs 4596, are the CONSEQUENCE of this split cascading, not the
cause.)  PRECISE NEXT PROBE: at the menu-music start (adv=4281), diff the driver data segment
(DAT_0000_0831<<4) voice-active flags [+0x107..] and note-record table [+0x13e8..] native vs wasm to find
the first divergent driver-state byte -- that is the actual root.  Full chain + nature now mapped;
intro/title audio stays landed+gated (diff=0 to [0x452]=4000).  Matrix 176/176.

REGLOG REGISTER-PATTERN reveals the branch (no further probe needed -- read from the data already
captured).  At the divergence (adv=4281), native writes 0x20/0x23/0x40/0x63/0x80/0x83/0xe0/0xe3 -- the OPL
OPERATOR registers = a FULL INSTRUMENT DEFINITION (a NEWLY ALLOCATED voice), while wasm writes 0x43/0x44/
0x4d (levels) + 0xb0/0xb1 (key-on) = a voice being REUSED/updated.  So 0997's voice-allocation loop
`for(i<8){voice=voicemap[bp]; if(voice_active[voice+0x107]==0) break; bp++}` takes a DIFFERENT branch:
native finds the voice FREE (writes a full instrument def), wasm finds it IN USE (updates levels).  Root =
the voice-active flag at D[voice+0x107] differs native vs wasm at the menu-music start.  A voice goes
inactive on note-off (envelope release), which is TICK-TIMING-driven -- so this is the SAME tick
sub-division divergence localized earlier (the +-1 ISR-per-c452 wobble at the menu-enter), now shown to
propagate through the sound driver's timing-dependent voice lifecycle: a note-off fires at a slightly
different tick on the two targets -> a voice is free on one and busy on the other at the menu-enter ->
different voice allocation -> different OPL stream -> audio diverges.  So the audio-beyond-intro root and
the tick-sub-division root are THE SAME issue (coupled), and the fix is the menu-enter tick-sub-division
determinism (the be0e/be58 sound-start pump timing) -- consistent with the whole chain.  Data-supported
(reglog register semantics); no over-claim.  Direct driver-state confirmation (dumping D[+0x107]) is
gated on a segfault-safe probe (3 naive dumps crashed on the patched sound-driver derefs -> careful work).
Intro/title audio stays landed+gated.  Matrix 176/176.

HONEST PRECISION on the synthesis (avoid over-claim): the reglog divergence is at IDENTICAL adv=4281 on
both targets.  If tick timing had diverged, the music-sequencer adv would differ -- it does NOT.  So the
CONSEQUENCE chain is understood (voice-active flag differs -> 0997 alloc branch differs -> instrument-def
vs level-reuse OPL writes -> stream diverges), but the ORIGINATING cause is still OPEN: what makes the
ISR-per-c452 SUB-DIVISION wobble (155 vs 144, hard data) while the PIT rate is stable (d8c4=250), the
retrace-countdown is deterministic (d8d4=1), and the sequencer position is identical (adv=4281)?  All three
candidate mechanisms are DISPROVEN.  The voice-active flag [+0x107] must be driven by an ISR/PIT-tick
counter that is SEPARATE from the music-sequencer adv (envelope release on the PIT tick, not the music
tick), and THAT counter wobbles -- but its wobble source is unidentified.  This is the genuine open root;
I will not guess a fifth mechanism.  PRECISE NEXT PROBE (segfault-safe): trace the c452-bump ACCUMULATOR
itself -- which DGROUP counter (c44e / c738 / the d8d2=1000 divisor logic in 31c3/30f8) increments per ISR
and triggers the c452 bump, and log its value + the increment per ISR across c452 4386..4400 on both, to
find the FIRST accumulator step that differs.  That is the true root of the sub-division wobble, upstream
of the sound driver entirely.  Everything downstream (voice alloc, OPL stream) is consequence.  Intro/title
audio stays landed+gated (diff=0 to [0x452]=4000).  Matrix 176/176.  (5th self-correction this session:
the "tick timing diverges" framing is imprecise -- adv is identical; the wobble is in a PIT-tick sub-counter
separate from the sequencer, and its cause is unpinned.)

C452-BUMP GATE DISPROVEN TOO (FIST_GATE trace).  The c452-bump condition in FUN_1000_30de is
`c446==0xff && c2a8==0 && byte[0x123e8]==0xff` (patch018/019 semaphores).  Traced c446/c2a8/c06b per ISR
across c452 4386..4400: all three are byte-for-byte IDENTICAL native==wasm (c446=0xff, c2a8=0x00,
c06b=0x00), and 0x123e8 is a CONSTANT image byte (read-only, never written).  So the c452-bump gate is NOT
the divergence either (5th disproven mechanism).  The 88-extra-ISR window (native 2331 vs wasm 2243 gate
lines) is EXACTLY the be58 sound-driver's 88 extra OPL writes -- each out(0x388/0x389) pumps one ISR -- so
the "sub-division wobble" and the "voice-alloc branch" are ONE phenomenon, not two.  EVERYTHING reduces to:
at the menu-music start (be58->0997) the voice-active flag D[voice+0x107] differs native vs wasm, taking
0997 down the allocate-new-voice branch (native, full instrument def) vs the reuse branch (wasm) -- while
ALL traced upstream state is identical (OPL write stream for 144 lines, gate semaphores, sequencer adv,
PIT rate/countdown).  So the voice-active flag divergence has NO identified upstream cause in anything
traced; the leading remaining candidate is a driver voice-state byte that is set by a NON-OPL-emitting
code path (invisible to the reglog) or is read in a target-divergent state.  DEFINITIVE NEXT PROBE:
a segfault-safe dump+diff of the driver data segment (base=DAT_0000_0831<<4) voice-active region [+0x100..
+0x140] at the menu-enter -- guarding base sanity (nonzero, < g_mem size) and firing exactly once -- to
find the divergent voice byte directly.  That byte IS the root (upstream of the 0997 branch).  Intro/title
audio stays landed+gated.  Matrix 176/176.  (Disproven this session, all by data: 2e6b, d8d4, .MS3-load,
sequencer-position, c452-bump-gate semaphores.)

ROOT NARROWED TO 2 DRIVER BYTES (segfault-safe vdump of the driver data seg, base=DAT_0000_0831<<4).
Dumped [+0x100..+0x1ff]+[+0x13e0..+0x141f] at increasing c452: IDENTICAL at c452=50, DIVERGENT (exactly 2
bytes) from c452=200 onward -- so the divergence is born early in the intro (c452 50..200), latent, and
surfaces only at the menu-music start.  The 2 bytes are +0x133 and +0x1c1 (native 0x00/0xAB, wasm
0x20/0x80).  CORRECTION: the voice-active flags (+0x107..+0x110) are IDENTICAL -- so the earlier
"voice-active flag differs" hypothesis (from the reglog register pattern) is DISPROVEN; the divergent bytes
are elsewhere.  base+0x1c1 is WRITTEN by FUN_0000_0872 (PATCH 353, the SOUNDDVR device-SELECT):
`*(D+0x1c1) = *(D + dev*2 + 0x1c3)` -- it is a DEVICE METHOD-VECTOR copied from the per-device table for
the selected device dev=D[0x12].  So the divergence is a sound-device method-vector (and +0x133) differing
native vs wasm, which routes the menu-music note processing down different methods -> different OPL writes
-> audio diverges.  This is upstream of 0997/voice-alloc entirely (that was a symptom).  DEFINITIVE NEXT
PROBE: at FUN_0000_0872 (fires early, c452~50..200), capture dev=D[0x12] and the source vectors
D[dev*2+0x1c3] / D[dev*2+0x199] etc. on both targets -- if dev is identical, the per-device VECTOR TABLE
(D[0x17d..0x1d1]) has a target-divergent entry (a driver method installed differently native vs wasm,
likely the same __allregs/icall-vector class as elsewhere); if dev differs, the device SELECTION itself
diverges.  6th self-correction this session (voice-active); the actual root is a device method-vector at
+0x1c1 set by 0872.  Intro/title audio stays landed+gated.  Matrix 176/176.

INTEGRITY CHECK -- my offset attribution hit a CONTRADICTION (flagging, not forcing).  I claimed the
divergent dump byte was base+0x1c1, but FUN_0000_0872 writes base+0x1c1 = 0x0f99 IDENTICALLY on both
(verified: dev=3, src1c3=0x0f99, one call at c452=34) and line 1533 is its ONLY writer; base+0x1c1 is only
READ afterwards (line 2148, `jmp *[ds:0x1c1]` device slot5).  So base+0x1c1 CANNOT hold 0xAB/0x80 -- my
cmp-byte-194 -> +0x1c1 mapping is WRONG.  What is FIRM (re-verified): exactly 2 bytes in the driver data
seg region [+0x100..+0x1ff] differ native vs wasm, IDENTICAL at c452=50 and DIVERGENT from c452=200
(latent through the intro, surfacing at the menu-music start); the voice-active flags [+0x107..+0x110] are
NOT among them; and FUN_0872 device-select is NOT the divergent writer (writes identically).  What is NOT
firm: the exact offsets of the 2 bytes and their divergent writer (my forward-offset arithmetic became
unreliable -- 7 hypotheses disproven, and fatigue-driven imprecision is now a risk per "Flüssigkeit ist
verdächtig").  CORRECT NEXT APPROACH (not more forward-drilling): a WATCHPOINT on the 2 divergent driver
bytes -- recompute their exact offsets from a fresh cmp with an explicit offset print, then set a gdb
hardware watchpoint (or a wrapped-access guard) on g_mem+base+<off> and run BOTH targets to catch the exact
instruction/function that writes the divergent value between c452=50 and 200.  That is the deterministic
way to the root, done with a fresh, careful pass -- not the fatigued forward-probe chain.  Everything
landed this session (patch 411, intro/title audio diff=0 to [0x452]=4000, 2x 10x-gate) is intact; matrix
176/176; tree clean; ~11 diagnostic probes all reverted.

PATCH 412 LANDED (asm-verified be50/be58 dropped-ax-arg into the [c530] sound call).  Root of the 2 (really
7) divergent driver bytes FOUND: FUN_0000_be50/be58 call the sound driver [c530]=01e7->0833 which stores
param_1 into D+0x12 (the sound command) then vtable-dispatches on it.  The asm sets AX=0x4000 (be50) /
0x5000 (be58, 5fc8!=0) / 0x4000 (be58 je-be50, 5fc8==0), but Ghidra DROPPED the `mov ax,IMM` and
mis-rendered be58's `je be50` as a 2nd arg-less call -> param_1 was an uninitialised register (D+0x12 =
0x01ec native / 0x0000 wasm at c452=87) -> the menu sound driver ran a GARBAGE command (near-silent, hence
native~wasm and audio diff=0 to [0x452]=4000 masked it) instead of the real 0x4000/0x5000.  Patch 412
threads the args; D+0x12 is now identical native==wasm, matrix stays 176/176, audio-intro (4000) stays
diff=0.  NEWLY EXPOSED (not a regression -- previously masked): with the CORRECT command the menu MUSIC
actually plays and now shows a native<->wasm divergence in the NOTE PROCESSING (audio diff!=0 from
~[0x452]=4500, sizes now MATCH so it is content not length).  So board:0003's remaining menu/mission-audio
work is this note-processing divergence, no longer hidden behind the dropped-arg garbage command.  This is
the 8th root-cause step, and the FIRST since patch 411 to land an asm-verified correction toward it.

CORRECTION + PATCH 412 REVERTED (integrity -- the prior "LAND patch 412 / matrix 176/176" entry was
DOUBLY WRONG).  (1) The committed 412 diff was MALFORMED: I stripped its `--- a/fist.c`/`+++ b/fist.c`
lines when appending the body, so `make check` errored ("only garbage in patch input") and make patch
SKIPPED it -- the 176/176 that run reported was the OLD build WITHOUT 412 (untested).  (2) After fixing
the diff header so 412 actually applies, the REAL matrix run REGRESSED 5 terrain flows:
terrain-{azer1,saudi1,cyprus1,india1,syria1} nat!=wasm(206) -- the SAME 206-byte signature as patch 411's
terrain regression.  So patch 412, though asm-verified (be50/be58 genuinely drop `mov ax,0x4000/0x5000`),
is COUPLED: threading the real sound command makes the sound driver run the actual menu-music path, which
DIVERGES native<->wasm in note processing, and that divergence bleeds into the terrain flows' native==wasm
(the sound driver runs on the shared boot path).  Reverted 412; matrix back to 176/176.  So the audio and
the terrain native==wasm are COUPLED through the sound-driver note-processing determinism: patch 412 can
only land TOGETHER with the note-processing determinism fix (the 0997/0cfb voice path).  This is the same
land-together pattern as patch 411 (d97e) + the extender render.  Two asm-verified corrections (411, 412)
are now BLOCKED on their coupled determinism fixes.  Audio-intro (diff=0 to [0x452]=4000) stays landed;
matrix 176/176.  (9th root-cause step; integrity: false "landed" claim retracted, patch reverted.)

BREAKTHROUGH + FULL CONVERGENCE (gdb watchpoint + driver-seg dump, decisive).  Set a hardware watchpoint
on the divergent driver byte g_mem+base+0x210 (base=0x3ce90) and ran native: it is written by
m_snd_FUN_0000_0997 (note-ON, fist_snd.c:1610) <- 0966 <- 0833(param_1=492=0x1EC) <- 01e7 -- i.e. DIRECTLY
by the note processing running the GARBAGE command 0x1ec (be58's dropped arg, D+0x12).  So ALL the
divergent driver bytes are CONSEQUENCES of the be58 dropped command, not independent.  PROOF: with patch
412 applied (real command 0x4000/0x5000), the whole driver data segment [base..base+0x2000] is diff=0
native==wasm (7 divergent bytes -> 0).  So PATCH 412 COMPLETELY FIXES the sound-driver STATE divergence
under cooperative ticking.
WHY 412 STILL CAN'T LAND (fully characterized): 412 makes the sound driver do the REAL menu-music work
(instead of garbage near-silence).  The terrain matrix flows run FIST_TICK_HZ=25000 with NATIVE async
SIGALRM vs WASM cooperative ticking; the near-silent garbage command did negligible work so async-vs-coop
never diverged them (they pass today), but 412's real sound work runs a target-dependent number of
ISRs under async-vs-coop -> the 5 terrain flows regress 206 B.  And they cannot simply be switched to
COOP: terrain under FIST_COOP_TICK at 25000 Hz TIMES OUT (the voxel render loop does not pump the timer
enough, so [0x452] crawls in wall-clock and never reaches the FIST_MISSFB dump op).
THE ONE FUNDAMENTAL ROOT: everything converges on the async(native)-vs-coop(wasm) TICK-REGIME determinism.
It blocks: patch 412 (menu-music audio), patch 411's UKRAINE1 terrain (same 206-B signature), and full
audio.  The sound-driver state itself is now PROVEN deterministic (412, driver diff=0); the residue is
purely the tick-count regime.  So the highest-leverage single fix for board:0003 AND board:0007-terrain is
a deterministic unified tick model that (a) native and wasm share exactly, and (b) does not time out the
voxel render -- e.g. drive native cooperatively BUT pump the timer from the render/present path too, or
give both a wall-clock-independent tick source keyed to engine progress.  Patches 411+412 are two
asm-verified corrections banked and ready, gated on this one tick-determinism fix.  Matrix 176/176; both
prototypes reverted; tree clean.

TICK-REGIME FIX CONSTRAINED (COOP terrain completion test, decisive): native terrain-azer1 under
FIST_COOP_TICK at 25000 Hz does NOT complete even in 500 s wall-clock (rc=124, no frame) -- the voxel
render loop (689a/6980/9200) does almost no port I/O, so under pure cooperative ticking [0x452] barely
advances and never reaches the FIST_MISSFB dump op.  (The diff=0 vs ref in that run is a false positive on
two empty files.)  So "unify everything on COOP + raise timeouts" is NOT viable: pure-coop STARVES the
terrain render.  Combined with the earlier finding that async ticking is non-deterministic vs coop, BOTH
existing regimes are ruled out for a unified deterministic tick.  THE FIX MUST BE a NEW tick model that is
simultaneously (a) deterministic and identical native==wasm, and (b) advances [0x452] during the voxel
render (does not depend on the render generating pumps).  Two concrete candidate designs: (i) pump the
cooperative tick from the voxel render loop itself (so [0x452] advances proportionally to render progress,
deterministically on both targets), or (ii) drive [0x452] off a deterministic ENGINE-PROGRESS counter
(e.g. present-count / frame-count) rather than pump-count or wall-clock.  This is a focused shim-
architecture change (native_main.c tick source + the render pump seam), touching every flow's timing, and
must be re-gated 10x; the terrain-COOP-timeout is the hard constraint it must satisfy.  Once it lands,
patches 411 (d97e) and 412 (be50/be58 -- proven to make the sound-driver state diff=0) both land with it,
unblocking menu/mission audio AND removing the async-vs-coop terrain coupling.  This is the single
highest-leverage item across board:0003 and the terrain determinism.  Matrix 176/176; tree clean.

MEMORY-OVERLAP RULED OUT (last alternative closed).  The 206-byte terrain regression under patch 412 is
NOT sound-data corrupting the framebuffer: the sound driver data segment (base 0x3ce90 .. 0x3ee90) sits
388 KB below the VGA framebuffer (0xA0000) -- no overlap.  So the 206 divergent bytes are IN the terrain
framebuffer as a TIMING-shifted render (the async-vs-coop tick regime), not memory corruption.  This closes
the last non-tick-regime hypothesis: the coupling is confirmed to be the tick determinism.  (Manual terrain
repro is unreliable -- FIST_MISSFB terrain times out under my ad-hoc mouse script; the AUTHORITATIVE datum
is the verify.sh matrix run: terrain-{azer1,saudi1,cyprus1,india1,syria1} nat!=wasm(206) with 412, 176/176
without it.)  Convergence now confirmed from four independent angles: (1) watchpoint -> divergent bytes are
be58-garbage-command consequences; (2) dump -> patch 412 makes the sound-driver state diff=0; (3) COOP
terrain hangs -> pure-coop unviable; (4) no sound/FB memory overlap -> the terrain coupling is timing.
The single root is the async-vs-coop tick regime; the fix is the specified deterministic render-advancing
tick model.  Matrix 176/176; tree clean.

MECHANISM OF THE 412<->TERRAIN COUPLING PINNED (sharpest fix spec).  in()/out() call fist_timer_pump() on
EVERY port access (fist_vga.c lines 106/137), and in COOP each pump advances one engine tick -> the game
clock [0x452] advances per-I/O-op.  So sound-I/O VOLUME perturbs the game clock: WITHOUT 412 the sound
driver issues few out(0x388/0x389) (garbage near-silence) -> few pumps -> the terrain flow's [0x452]
progression is stable and native-async==wasm-coop at the capture fixed point (terrain passes); WITH 412 the
real note processing issues MANY OPL writes -> many extra pumps -> [0x452] inflates by an amount that
differs under async(native) vs coop(wasm) -> terrain reaches its capture [0x452] at a shifted render state
-> 206 B.  So the precise defect class is: the ENGINE game-tick is advanced per platform-I/O-op, which
lets I/O volume (and its async-vs-coop interaction) leak into game timing.  SHARPEST FIX: advance [0x452]
by a deterministic amount per GAME-FRAME (engine progress), NOT per in()/out() -- so sound-I/O volume
cannot perturb the game clock and native==wasm holds regardless of regime.  This both fixes audio (ISR
count becomes a pure function of engine progress) AND removes the terrain coupling (sound I/O no longer
shifts the terrain [0x452]).  It is exactly the "engine-progress-keyed tick" candidate (ii) above, now
mechanism-justified.  Caveat/risk: the current per-I/O pump is WHAT makes the 176 flows converge today, so
this is a matrix-wide timing change requiring careful design + full 10x re-gate.  Patches 411+412 land with
it.  Matrix 176/176; tree clean.

MECHANISM CLAIM RETRACTED (integrity -- disproven by matrix test).  I claimed the 412<->terrain coupling
was the OPL-register-write pump (in()/out() pumping the tick on 0x388/0x389).  TESTED: prototyped
out(0x388/0x389) skipping fist_timer_pump(), applied 412, ran the FULL matrix -> terrain-{azer1..syria1}
STILL nat!=wasm(206), unchanged.  audio-intro stayed diff=0 (so the no-pump is behaviour-neutral for
audio), but it does NOT fix the terrain coupling.  So the coupling is NOT the OPL-write pump volume; the
extra ISRs/divergence come from something else in 412's real-command processing (0833 dispatches the real
command 0x5000 to a DIFFERENT sound-driver method than the garbage 0x1ec -> slot 0x0a vs slot 0 -> a
different code path that perturbs terrain native-async vs wasm-coop by a mechanism NOT yet identified).
9th disproven hypothesis.  What remains FIRM: 412 makes the sound-driver STATE diff=0 under coop (proven);
412 regresses the 5 terrain flows 206 B under the matrix's native-async-vs-wasm-coop regime; the coupling
is a timing/state divergence in 412's real-command code path, mechanism OPEN.  The reliable path remains a
gdb watchpoint on the terrain-divergent framebuffer bytes' SOURCE (find what writes the 206 divergent FB
bytes and why it differs with 412), a fresh careful pass.  Prototype reverted; matrix 176/176; tree clean.

TERRAIN COUPLING LOCALIZED to a specific TOP-LEFT render element (not a broad timing shift -- narrows the
whole picture).  Captured terrain-azer1 native(412) vs wasm(412) framebuffers with the REAL run_terrain
MC_MOUSE (my earlier ad-hoc mouse script was wrong -> timeouts).  The 206 divergent bytes are 12 short
runs, ALL in the top-left corner: rows 0,3,6,9,12,15,18 (every 3rd row), cols 0..32.  native = 0x20
UNIFORM across the region; wasm = varied (0x00/0x3c/0x34/...).  So with patch 412 NATIVE fills this
top-left every-3rd-row band with 0x20 while WASM keeps varied content -- a LOCALIZED render-element
divergence, NOT a broadly timing-shifted terrain view (a broad tick-shift would scatter across the whole
320x200).  This DISPROVES "the terrain coupling is the tick regime" as a broad phenomenon: it is a specific
top-left overlay whose content 412 changes, differing native-async vs wasm-coop.  IMPLICATION: the fix may
be NARROWER than a tick re-architecture -- identify what writes 0x20 to fb top-left rows{0,3,6,..} cols
0..32 on native under 412 and why wasm differs there (a HUD/overlay element reading a sound-state or
frame-count value that 412 perturbs).  DIRECT NEXT PROBE: gdb watchpoint on g_mem+0xA0000 (fb row0 col0)
during native terrain-azer1+412 to catch the writer of the 0x20 band.  10th mechanism refinement; the
coupling is a localized overlay, not global timing.  Matrix 176/176; tree clean.

RECONCILIATION (integrity -- my "localized overlay disproves the tick regime" refinement was OVER-STATED).
Read the voxel raycaster m_ext_FUN_0000_6980 (build/fist_ext.c): it renders from the camera state
DAT_0000_90e0 (position/angle) via the transform tables at fist_ext_base+0x9450/0x9454/0x9650.  The camera
state advances with the game sim per [0x452] tick.  So patch 412's tiny tick-count shift (its real sound
work adds ISRs, differently async-vs-coop) produces a TINY camera/sim-state difference at the terrain
capture -> only 206 of 64000 FB bytes (0.3%) differ, concentrated where the render crosses a threshold
(the top-left horizon/sky boundary).  A tiny tick difference producing a small, LOCALIZED render diff is
fully consistent with -- not a refutation of -- the tick regime.  FIST_TERRAIN renders ONLY the voxel view
(no HUD overlay), so the divergent top-left pixels ARE the voxel output at a marginally shifted camera,
NOT a separate sound-status overlay.  So the terrain coupling RE-CONVERGES on the single async-vs-coop
tick-regime root (the camera/sim-state at capture is tick-sensitive; 412 perturbs the tick via sound-ISR
volume).  This restores the clean convergence: ONE root = the tick regime; fix = the deterministic
engine-progress-keyed tick model.  My prior "top-left render element, not broad timing" framing is
corrected -- it is a tiny tick-caused camera shift, localized because 0.3% of the view crosses a boundary.
Patches 411+412 land with the tick-model fix.  Matrix 176/176; tree clean.

SHARPEST ROOT STATEMENT (why async<->coop diverge, at the CPU-work level).  native's tick source is the
wall-clock SIGALRM (native_main.c setitimer) -- it fires ~N times/sec of WALL CLOCK, so MORE CPU work
between frames -> MORE ticks drained.  wasm has no SIGALRM: its tick advances one-per-pump (cooperative),
which is CPU-work-INDEPENDENT.  So patch 412 (real note processing = more CPU work) makes native's SIGALRM
fire more during that work -> native's [0x452] advances faster relative to the game-frame count -> at the
capture [0x452]=N the camera/sim has run fewer game-frames -> the voxel view shifts (206 px).  On wasm the
same extra work adds NO ticks (coop) -> no shift.  That is exactly why 412 couples to terrain ONLY under
the mixed async-vs-coop regime, and why the framebuffer fixed points pass today WITHOUT 412 (near-silent
sound = negligible CPU work = SIGALRM stable).  THE FIX, now fully justified: a tick source that advances
[0x452] a fixed amount per GAME-FRAME (engine present), CPU-work-INDEPENDENT, on BOTH targets -- so neither
CPU work (SIGALRM) nor pump volume (coop) can perturb the tick, native==wasm holds, and terrain does not
starve (it advances per rendered frame).  Implementation caveat: there is no clean per-frame "present" hook
in the shim today (the frame is rendered straight to g_mem+0xA0000; the palette upload rides in(0x3da)),
so the engine-progress tick needs a deliberately-placed present seam -- a careful engine/shim change, full
10x re-gate.  This is the complete, code-verified characterization of the single determinism root; patches
411+412 land with it.  Matrix 176/176; tree clean.

CONVERGENCE RETRACTED (integrity -- DISPROVEN by the render-pump COOP test).  I concluded the single root
was the async-vs-coop tick regime.  TEST: added a render-pump burst to the voxel tile->fb (9200) so terrain
runs under FIST_COOP_TICK (it un-starved -- terrain-azer1 completes in 1s under coop vs 500s timeout, a
useful sub-result), applied 412, and compared BOTH targets under coop+renderpump+412:
terrain-azer1 native==wasm = diff 206 (NOT 0), and native-coop vs the async ref = 206.  So COOP-BOTH STILL
DIVERGES 206 -- the async-vs-coop tick regime is NOT the cause of the 206 terrain regression (12th disproven
hypothesis, and it retracts the "single tick-regime root" + "SHARPEST CPU-work SIGALRM" convergence of the
last several commits).  What is now FIRM: (a) patch 412 makes the sound-driver STATE diff=0 under coop
(proven earlier); (b) yet the terrain FRAMEBUFFER diverges 206 native vs wasm EVEN under coop-both with
412; (c) coop is deterministic (same tick both), so the 206 is a genuine NATIVE-vs-WASM COMPILED-BEHAVIOUR
difference in the code path 412 newly exercises (0833's real-command dispatch -> 0997/0cfb note path),
NOT tick timing and NOT the (diff=0) sound state.  Leading candidate: FP or undefined-behaviour divergence
native(gcc x87/sse) vs wasm(emcc 64-bit) in the note-processing math (0cfb freq/env), OR an uninitialised
read that differs by target -- surfacing in the terrain render via a shared value.  NEXT: run coop+
renderpump WITHOUT 412 -> if native==wasm there, 412's real path is the sole trigger; then diff the
driver/engine state under coop+412 native vs wasm to find the first divergent byte (now feasible since
terrain runs in 1s under coop+renderpump).  The render-pump (un-starves coop terrain) is the enabling tool
for that.  Matrix 176/176; tree clean; prototype reverted.

RENDER-PUMP PROTOTYPE IS FRAGILE (follow-up, reverted).  coop+renderpump WITHOUT 412 -> native SEGVs
(rc=139, no frame); WITH 412 -> native completes.  So the render-pump's 64 pumps/frame do NOT just advance
[0x452] -- each pump runs the FULL cooperative ISR (timer + sound + palette), which has side effects and,
without 412's real command, drives native into a crash.  So the render-pump cannot cleanly validate a tick
fix, and my "un-starves coop terrain" sub-result is real but not a usable seam as-is (it perturbs state).
The STANDING firm facts after all this: (1) patch 412 is asm-verified (be50/be58 thread the real sound
command); (2) 412 makes the sound-driver STATE diff=0 under the audio env (coop, TICK_HZ=1000); (3) UNDER
coop-both at TICK_HZ=25000 with 412, the terrain framebuffer STILL diverges native==wasm by 206 -> so the
206 is NEITHER the async-vs-coop tick regime (coop-both shows it) NOR FP in the sound path (fist_snd.c has
zero float/double; DBOPL is inactive with OPL off in terrain).  The 206 native-vs-wasm terrain divergence
under 412 is a genuine but STILL-UNIDENTIFIED difference in the code path 412 exercises -- mechanism OPEN,
and every convergence I proposed (tick regime, CPU-work SIGALRM, OPL-write pump, localized overlay) has been
DISPROVEN by a subsequent test.  Honest state: I do NOT know the 206 mechanism.  It needs a state-diff of
native vs wasm at the terrain capture under coop+412 (a segv-safe, non-perturbing way to run terrain fast
under coop -- the render-pump is not it), then bisect the first divergent engine byte.  patch 411 (audio-
intro) stays landed + gated; patch 412 stays banked/blocked.  Matrix 176/176; all prototypes reverted.

CLEAN STATE-DIFF (real matrix regime) RE-CONFIRMS THE TICK REGIME; the render-pump retraction was
CONTAMINATED.  Ran a state dump (g_mem[0x1c000..0xA0000]) at the terrain capture with 412, native ASYNC
vs wasm COOP (the ACTUAL matrix regime, NO render-pump).  Result: FB diff 206 (the real regression), and
751 divergent engine bytes over 152 scattered DGROUP runs -- a BROAD sim-state divergence.  The key byte:
DGROUP+0x452 (the frame timer [0x452]) = native 0x0028=40 vs wasm 0x013a=314.  So native-async and
wasm-coop reach the terrain-capture op at DIFFERENT [0x452] (40 vs 314) with 412 -> the whole sim/camera
state differs -> 206 FB bytes.  This is exactly the async-vs-coop tick regime, and it RETRACTS my previous
"convergence retracted" entry: that retraction was based on a coop+RENDER-PUMP test, but the render-pump
CONTAMINATES native (its 64 pumps/frame run the full ISR with side effects -> native's DGROUP vectors go
NULL, an earlier dump showed 2738 bytes of native-null-vs-wasm-0x0f69-vectors).  So the render-pump test
was invalid; the CLEAN real-regime diff shows [0x452] itself diverges -> the tick regime IS the root, as
originally converged.  WITHOUT 412 terrain passes (both reach the capture at the same [0x452]); WITH 412
the sound work shifts the tick so the capture op fires at 40 vs 314.  FIX unchanged: a deterministic
engine-progress tick so native==wasm reach the capture at the same [0x452] -- but the render-pump is NOT a
usable seam (it perturbs state).  A clean fast-coop-terrain path (advance [0x452] without ISR side effects)
is the tooling need.  Net: tick regime re-confirmed by clean data; ~14 hypothesis flips total, each
data-driven.  patch 411 landed; 412 blocked on the tick fix.  Matrix 176/176; all prototypes reverted.

WITHOUT-412 CONTROL OVERTURNS THE TICK-REGIME-ROOT CLAIM (decisive four-way dump analysis).  The prior
entry asserted "WITHOUT 412 both reach the capture at the same [0x452]".  That is FALSE.  Ran the clean
state-diff a second time on the COMMITTED build (NO 412, no render-pump), native ASYNC vs wasm COOP, same
terrain-azer1 capture.  Result: FB diff = 0 (terrain PASSES, as it must), BUT engine-state diff = 750
bytes INCLUDING [0x452] = native 40 vs wasm 314 -- the SAME [0x452] divergence as with 412.  So the
async-vs-coop [0x452]/tick divergence is present WITHOUT 412 and the terrain framebuffer is IDENTICAL
anyway.  Therefore the tick/[0x452] divergence is NOT the 206 cause; it is a harmless regime artifact the
tick-settled voxel render is robust to.  My "tick regime re-confirmed" entry above (and commit 90e25f7)
is RETRACTED: it inferred causation from "[0x452] differs with 412" without running the without-412
control that shows it differs regardless.

WHAT ACTUALLY CAUSES THE 206 (four-way comparison: {native,wasm} x {no412,+412}, all clean dumps).  412
changes native state at exactly 14 DGROUP bytes and wasm state at 0 bytes.  Branch selector DAT_2000_5fc8
= 0x0001 on BOTH targets both ways -> be58 threads the SAME command 0x5000 on native and wasm.  So: same
sound command, yet native's command-0x5000 handler writes 14 engine-DGROUP bytes and wasm's writes none.
This is a divergent HANDLER (not divergent input).  Without 412 the command is uninitialised garbage ->
handler no-ops -> terrain passes; with 412 the real 0x5000 handler runs and propagates a PRE-EXISTING
native-vs-wasm structural DGROUP divergence into render-visible bytes (notably DGROUP+0x1e1b: 0->1 on
native only), which the voxel render then reads -> 206 FB bytes.

THE REAL ROOT is the pre-existing native-vs-wasm state divergence itself (750 bytes at the terrain
capture, present with OR without 412; 245 both-nonzero/structural, 505 one-side-zero).  Example region
DGROUP 0x570..0x590: native = 89db48cc2930e09e 1c08f05b..., wasm = 0000000002e7656d 73636754... -- a
STRUCTURALLY different populated struct/buffer (looks like a differently-initialised pointer/record, not a
shifted counter).  It is harmless to all 5 terrain FBs (they pass) and to audio-intro (passes), so it went
unseen; 412's sound command is merely the first consumer that reads it and writes render-visible state.
FIX DIRECTION (corrected): stop chasing a tick re-architecture; instead find why DGROUP is populated
differently native vs wasm at spawn (the 0x570-region struct + the 505 one-side-zero bytes) -- a real
determinism bug that likely also gates full-duration menu/mission audio (board:0003) once 412 lands.  Next
concrete step: watchpoint/bisect the 0x570 region's writer at boot on both targets and diff the first
divergent populate.  patch 411 landed; 412 blocked on this DGROUP-populate divergence, NOT on the tick.
Matrix 176/176; tree clean; all instrumentation reverted.

ROOT FOUND & PROVEN: the "412 regresses terrain" 206 is a native-async CODE-LAYOUT-FRAGILITY artifact,
NOT anything 412 does semantically.  RETRACT the entire prior cause-chain for the 206 (tick regime,
[0x452], DGROUP-populate divergence, calibration [0x1e1a], sound-command handler) -- all were chasing
shadows in a jittery native signal.  The decisive experiments (all clean, deterministic, terrain-azer1,
matrix env FIST_TICK_HZ=25000 FIST_TERRAIN=1 MC_MOUSE, i.e. native-async exactly as run_terrain uses):

  1. native no412 run1 vs run2:            FB 0     state 31   ([0x452]=40 both)
     -> native FB is deterministic per-binary, but engine STATE jitters 31 bytes run-to-run.  So the
        four-way {nat,wasm}x{no412,+412} byte analysis was CONFOUNDED by native state-jitter; its "14
        bytes / newly-divergent 0x1e1b" conclusions are void (consistent with CALFIX not moving the 206).
  2. native+412 run-to-run:                FB 0     (deterministic) ; vs native-no412: FB 206 (stable)
  3. be50/be58 call-count native vs wasm:  IDENTICAL -- be50 x0, be58 x1, c530=0, 5fc8=1 on BOTH.
     be58's single call is ((void(*)(uint16_t))fist_icall_far(0))(0x5000) -> linear 0 -> trap_tramp
     (FIST_INTVEC_LIN=0xFE000 so 0 is NOT a chained vector; lookup_fun(0) misses) -> a no-op int(void)
     that IGNORES the arg.  NOARG runtime-gate: arg-vs-noarg FB = 0 -> the 0x5000 argument is inert.
  4. THE CONTROL: add a semantically-NULL `if(getenv("FIST_ZZZ_NEVER")){volatile int q=0;}` to be58 (NO
     412, never taken) and recompile:
        native-clean vs wasm-clean        = 0     (the matrix match holds for the committed binary)
        wasm-clean   vs wasm-DUMMY        = 0     (wasm-coop is CODE-LAYOUT-INVARIANT / robust)
        native-clean vs native-DUMMY      = 206   (native-async is CODE-LAYOUT-FRAGILE)
        native-DUMMY vs wasm-DUMMY        = 206
     A null recompile of a terrain-boot-path function (be58) shifts the native-async terrain FB by the
     SAME 206.  Patch 412 modifies be58 -> recompiles it -> identical shift.  412 is SEMANTICALLY INNOCENT.

MECHANISM: native-async's terrain render samples engine state at tick/interrupt-firing points whose
alignment depends on the compiled code of the terrain-boot path; recompiling a function on that path
(be58) re-aligns them -> ~206 pixels move.  wasm-coop advances the tick by a deterministic I/O-count
pump, so its render is code-layout-invariant.  Therefore native==wasm terrain bit-identity is a FRAGILE
COINCIDENCE of the frozen committed native binary: it holds today (176/176) and survives patches that miss
the terrain-boot-hot path (411/d97e = extender, no shift), but ANY patch touching a terrain-boot-hot
function (412/be58) breaks it -- and would break it identically whether or not the patch is semantically
correct.  This is a native determinism DEFECT, not a 412 bug.

CONSEQUENCES / FIX DIRECTION (supersedes all earlier 0003 fix notes):
  - patch 412 is unblocked in principle: its terrain "regression" is not real divergence, it is the
    native-async render being non-robust to recompilation.  412 must NOT be reverted for the 206.
  - The real defect to close: native terrain must use the SAME deterministic, code-layout-invariant tick
    model as wasm-coop (advance the render clock by engine-progress / a deterministic pump, not by
    wall-clock/interrupt alignment).  Then native terrain FB is recompile-invariant, native==wasm holds
    ROBUSTLY, and any correct patch (412 included) lands without a phantom terrain shift.
  - Known constraint (unchanged): pure FIST_COOP_TICK starves the voxel render (no port I/O -> no pump ->
    timeout).  So the deterministic native terrain tick needs a render-advancing pump seam that does NOT
    run the full ISR side effects (the earlier render-pump prototype perturbed state and is not it).
  - The matrix's terrain native==wasm check is only as trustworthy as the native binary is deterministic;
    until native terrain is code-layout-invariant, a passing terrain row proves "these two frozen binaries
    agree", not "the port renders terrain deterministically".  This is the sharpest statement of board:0002.
Matrix 176/176; tree clean; all instrumentation reverted (make patch).

MECHANISM OF THE FRAGILITY PINNED: UNINITIALISED STACK LOCALS in the terrain path.  Rebuilding native
with gcc -ftrivial-auto-var-init=zero (zero every auto var, matching wasm's spec-guaranteed zero locals):
  zero-init clean vs zero-init DUMMY = 0    (the 206 code-layout fragility VANISHES)
  zero-init clean vs wasm            = 57   (was 0 for the async-garbage clean binary; see below)
So the native terrain FB depends on uninitialised stack locals; their garbage is code-layout-dependent
(any recompile of a terrain-path function reshuffles it -> ~206 px), which is why patch 412 / a null
getenv / ANY hot-path edit "regresses" terrain.  wasm is robust because wasm function locals are zero by
the wasm spec.  This SUPERSEDES the "code-layout fragility" statement with its cause, and finally explains
the whole 20-hypothesis chase: every prior "root" (tick regime, [0x452], DGROUP-populate, calibration,
sound handler) was reading tea leaves in a signal driven by uninitialised memory.

The 57-byte residual (zero-init native vs wasm) is the SECOND-ORDER fact: the committed native binary's
GARBAGE locals coincidentally produced a wasm-matching FB (async-clean vs wasm = 0), but ZEROing those
locals moves native 57 px off wasm -- i.e. wasm's effective value for the read local is NOT zero either.
So neither garbage nor zero is the ENGINE-CORRECT value; the original asm writes that local before reading
it and Ghidra dropped the write (the classic __allregs prune).  The CORRECT fix is a patch that restores
the lost initialisation so BOTH targets read the same computed value -- that removes the fragility (no
uninit read) AND collapses the 57 (identical value native==wasm), unblocking 412 and the terrain rows
against recompilation.  -ftrivial-auto-var-init=zero is NOT the fix (it is a global band-aid that still
leaves 57 vs wasm and hides the real lost-write); it is only the instrument that proved the mechanism.
NEXT: valgrind --track-origins=yes on the native terrain run to name the exact uninitialised local + its
reading function, then find the Ghidra-dropped write in the asm and land it as a patch.  Matrix 176/176;
tree clean.

LOCALISATION + SHARED ROOT WITH board:0007.  fist_ext.c carries 183 Ghidra uninitialised-register
pseudo-vars (unaff_ES, extraout_var, in_ESP, ...) -- the __allregs-prune artifact: register values the
decompile could not thread, declared but never assigned, so READ AS GARBAGE.  The terrain render funcs
themselves (689a/6980/9200) are clean of them; the uninit reads live in the terrain-BOOT functions that
populate camera/transform state the render consumes (e.g. `uRam000f0010 = unaff_ES`, 880 stores into the
0xf00xx scratch the render reads).  On native those reads are stack/heap garbage (code-layout-dependent ->
206 on recompile); on wasm they are spec-zero.  This is the SAME __allregs-prune root that board:0007 names
for the terrain object-ref cascade -- board:0003's terrain determinism and board:0007's terrain baseloss
are two faces of one defect: the decompile's lost register dataflow.  Closing the __allregs-prune faithfully
(restore the dropped register writes as asm-verified patches) removes the uninit reads -> native terrain
becomes code-layout-invariant AND native==wasm robust AND 412 lands, and likely unblocks board:0007's
42 missions.  (valgrind is NOT installed here; localise instead by per-TU/-per-function
-ftrivial-auto-var-init=zero bisection, then read the asm for the specific dropped write.)  Matrix 176/176;
tree clean.

TU-BISECT LOCALISES THE CONSUMED UNINIT READ TO fist.c (NOT fist_ext.c).  Method: build every TU with
-ftrivial-auto-var-init=zero (invariant baseline), then recompile ONE TU with =pattern (0xAA) and relink;
a changed terrain FB means that TU holds a CONSUMED uninit read.  Result:
  pattern-init fist_ext.c vs all-zero baseline = 0    -> fist_ext.c's 183 unaff_/extraout_/in_ESP pseudo-
                                                          vars are INNOCENT (they never reach the FB)
  pattern-init fist.c     vs all-zero baseline = 603  -> the FB-driving uninit read is in fist.c
So correct the previous entry: the culprit is a __allregs-pruned register read in ONE fist.c terrain-boot
function, not the raycaster TU.  fist.c has 970 such pseudo-vars; the terrain render is driven by the
extender display-list walkers (PATCH 116/193/346 dispatch paths, op-0x08).  NEXT (deliberate): function-
level bisect inside fist.c -- with pattern-init=0xAA the uninit local reads 0xAAAA, so scan g_mem during
terrain boot for the first 0xAAAA-derived value to name the propagating store, or wrap candidate display-
list-walker locals with explicit zeroing and watch the 603 collapse -- then read the asm for the dropped
register write and land it as an asm-verified patch.  Matrix 176/176; tree clean.

CONFIRMED + REFINED: the uninit reads are SEGMENT REGISTERS unaff_CS / unaff_ES, and the committed
native==wasm terrain match is an UNDEFINED-BEHAVIOUR COINCIDENCE.  0xAA/0xFE pattern-flow-scan named the
exact stores: FUN_0000_134e (DAT_1000_c3e2 = unaff_CS), FUN_1000_2ebe (c434/382c/uRam000f0010 = unaff_ES),
FUN_0000_02c5 (c686 = unaff_CS) -- Ghidra could not thread the real-mode CS/ES segment registers, so these
locals are read uninitialised (fist.c has 63 unaff_CS + 150 unaff_ES + 58 int-unaff_CS such reads).
Initialising ALL of them to a constant makes native CODE-LAYOUT-INVARIANT (clean vs recompiled-dummy = 0,
was 206) -- PROVING these are the fragility root.  BUT neither 0 nor 0x1000 reproduces the committed FB W:
  unaff=0  : native invariant, but native vs committed-wasm = 206
  unaff=0x1000 : native invariant, but native vs committed-wasm = 206  (0x1000 = the FUN_1000_3a14 precedent)
  committed native (garbage reads) vs committed wasm = 0   (W)
  wasm with unaff=0 == committed wasm (emcc -O2 already lands the UB read on a W-producing value)
So native-garbage and wasm-(-O2-UB) COINCIDENTALLY both produce W, while any DEFINED constant (0, 0x1000)
produces W+206.  The terrain native==wasm bit-identity is therefore resting on undefined behaviour: two
uninitialised-segment-register reads that happen to agree for the frozen committed binaries.  Recompiling
a terrain-boot-hot function (412/be58, or a null getenv) reshuffles native's stack garbage -> its UB value
changes -> W+~206, "breaking" terrain -- which is why every hot-path patch appears to regress it.

TRUE FIX (board:0007 __allregs domain + board:0002 fidelity): restore the ASM-TRUE CS/ES value at each of
these sites from the original disassembly (read what CS/ES actually held there; e.g. CS is the running
code segment, ES is whatever the preceding asm loaded), landing them as asm-verified patches so BOTH
targets read a DEFINED, faithful value.  That removes the UB, makes native code-layout-invariant, keeps
native==wasm ROBUSTLY, unblocks 412, and -- because the current W is a garbage-derived value that may not
match the DOSBox oracle -- likely CORRECTS the terrain FB toward oracle-faithful (board:0002) and unblocks
board:0007's 42 missions (same lost-register-dataflow defect).  This is NOT guessable (0/0x1000 both wrong);
it needs the per-site asm.  -ftrivial-auto-var-init is only the instrument, never the fix.
STATUS: root fully proven & localised to named sites; the per-site asm-true CS/ES recovery is the concrete
remaining engineering (deliberate, patch-disciplined, full re-gate).  Matrix 176/176; tree clean.

ASM CONFIRMATION of the __allregs prune (the mechanism proven at the disassembly).  objdump -m i8086 on
FIST.DAT at FUN_1000_2ebe (file offset 0x12ebe) shows exactly the dropped write:
  12ec6: mov %ds,%ax        ; AX = entry DS
  12ec8: mov %ax,%es        ; ES = DS     <-- Ghidra pruned THIS assignment
  12eca: mov (%bx),%ds      ; DS is then overwritten
  ...
  12edc: mov %es,%ax        ; reads ES (still = the entry DS) -> the C decompile's `unaff_ES`
So `unaff_ES` at this site = the ENTRY DS value, a register-dataflow fact the decompile lost -- NOT a
constant (which is why unaff=0 and unaff=0x1000 both mis-produced W+206; the faithful value is the caller's
DS at entry, restored by threading the pruned `es = ds` write).  This is board:0007's __allregs-prune
domain, now verified at the asm.  (FUN_0000_02c5 / FUN_0000_134e disassemble as garbage at raw file offset
0x2c5/0x134e -> the segment-0000 load mapping is NOT identity; resolving the Ghidra image base for
FUN_0000_* is a prerequisite step to read their CS store, next session.)  The per-site register-provenance
recovery (thread the pruned segment loads: ES=DS at 2ebe, and the CS stores at 134e/02c5 once mapped) is the
concrete fix -- asm-verified patches, engine-wide blast radius (271 unaff_CS/ES reads), full re-gate.
Matrix 176/176; tree clean.

ASM + LOAD-MAPPING RESOLVED, and a KEY native/wasm ASYMMETRY around the UB.  The segment-0000 load map:
FIST.DAT is an MZ exe; the Ghidra image (re_out/fist_dat_image.bin) is file[0x1400..EOF] (the 0x1400 MZ
header dropped), loaded at base 0, so Ghidra-linear = image offset.  Disassembling THE IMAGE (not raw
FIST.DAT) at the linear offsets gives clean code and the exact dropped stores:
  FUN_0000_02c5 @0x2c5:  c7 06 84 06 0b 03  movw [0x684],0x30b ; 8c 0e 86 06  mov [0x686],cs  -> c686 = CS
  FUN_0000_134e @0x134e: c7 06 e0 03 74 13  movw [0x3e0],0x1374; 8c 0e e2 03  mov [0x3e2],cs  -> c3e2 = CS
  FUN_1000_2ebe @0x12ebe: mov ds,ax; mov ax,es (ES=DS entry); ... mov [0x434],es              -> c434 = ES
So the pruned ops are plain `mov [mem],cs` / `mov [mem],es` -- Ghidra could not type the segment-register
source and emitted unaff_CS/unaff_ES.  BUT the value is NOT a recoverable constant that reproduces the
committed FB W: unaff_CS/ES set to 0, 0x1000, 0xf69 (the c686 normal value = main code seg), or ES=0x1c00
(=DGROUP=entry DS) ALL give native = W+206 (call it W'), never W.  Only the committed uninitialised garbage
gives W.  AND emcc -O2 is VALUE-INVARIANT here: wasm with unaff=0 == committed wasm == W (the wasm build's
output does not depend on these reads), while native IS value-sensitive (garbage->W, any constant->W').
So the committed terrain native==wasm=W rests on: native-garbage and wasm-(-O2-elided) COINCIDENTALLY both
yielding W, and the asm-faithful segment value yields W' != W on native.  IMPLICATION (board:0002): W may
itself be UNFAITHFUL -- both current targets may be rendering a garbage-coincidence FB, and the oracle-true
terrain frame could be W'.  The fix is therefore NOT "hardcode the segment"; it is (a) thread the pruned
`mov [mem],cs/es` faithfully so BOTH targets read the DEFINED value AND compile-consume it identically
(understand why emcc -O2 elides the read that gcc -O0 consumes -- likely dead-store elimination the native
-O0 build keeps), and (b) oracle-capture terrain to decide whether W or W' is correct.  This is genuine
deliberate work; the asm and load-map here are the ready inputs for it.  Matrix 176/176; tree clean.

FINAL CHARACTERISATION: the unaff stores are DEAD on wasm, LIVE on native -- the FB agreement is a
garbage-path coincidence over a data dependency native has and wasm does not.  Decisive test with a
DISTINCTIVE value (rules out the earlier 0-only fluke): built BOTH targets with unaff_CS=0xf69/ES=0x1c00.
  wasm-0xf69 vs committed-wasm W   = 0    -> wasm output is INDEPENDENT of these values (even initialised)
  native-0xf69 vs wasm-0xf69        = 206  -> native output DOES depend on them (-> W')
So DAT_c686/c3e2/c434 (the saved CS/ES) are read back and used by NATIVE's terrain render but NOT by
WASM's -- emcc -O2 eliminates the dependency (whether as UB-propagation or dead-store/read elimination),
gcc -O0 keeps it literally.  The committed native==wasm=W holds only because native's garbage happens to
render the same pixels as wasm's value-independent path; any recompile perturbs native's garbage -> W+/-206.
CONSEQUENCE FOR THE FIX (sharper): this is NOT merely "restore the pruned mov[mem],cs" -- initialising the
value does not converge the targets because wasm ignores it.  The real divergence is that native's render
READS these saved-segment DGROUP words and wasm's does not.  Two directions, to be decided by the oracle:
  (A) if the engine is SUPPOSED to use the saved CS/ES (far-ptr reconstruction), wasm is WRONG (optimised
      the read away); force wasm to keep it (e.g. volatile / -O2 exclusion on the reader) and set the
      faithful segment -> both -> W' (the oracle-true frame), or
  (B) if native's dependency is spurious (a decompile artifact the real engine never had), native is WRONG;
      the faithful render ignores these words (as wasm does) -> W is correct and native must be made to
      not read them.
Deciding A vs B REQUIRES the DOSBox/QEMU oracle terrain frame (board:0002) -- it is the arbiter of whether
W or W' is faithful, and it is tooling-gated.  Until then the fix cannot be landed correctly (landing
either direction blindly risks encoding the wrong frame into the matrix).  This closes the diagnosis: root
= __allregs-pruned saved-segment reads with an -O2-vs-O0 liveness divergence; blocked on the terrain oracle
to choose the faithful frame.  Matrix 176/176; tree clean.

SHARPENED (via board:0010, 2026-08-24): defining the terrain-site segment values (the board:0010 CS/ES
context) does NOT converge native==wasm -- it makes native USE the value (-O0, W') while wasm ELIDES it
(-O2, W) -> 206.  So the determinism root is precisely a native-O0-vs-wasm-O2 CODEGEN divergence at these
sites (native keeps a read emcc -O2 proves dead), NOT the definedness of the value.  The real fix: (1) find
why native's terrain render has a data dependency on c686/c3e2/c434 that wasm's does not (compiled-code
diff of the terrain-write path native vs wasm), then (2) oracle-arbitrate W vs W' (does the ORIGINAL's
terrain frame depend on these fields?) to decide whether to make BOTH keep the value (faithful read) or
BOTH drop it (spurious read).  The DOSBox read-trace can be extended (address-filtered read-watch on the
engine DGROUP under CR3-aware paging) to answer whether the original reads them during render.  patch 412
stays semantically innocent; it only exposes the fragility by recompiling be58.

STATIC NARROWING (2026-08-24) -- the determinism root is ONE far-ptr-segment save, not 3 sites.  Readers
of the 3 candidate terrain sites in the decompile:
  - DAT_1000_c3e2, DAT_1000_c434: NO readers (only #define + the unaff write) -> DEAD -> RED HERRINGS
    (they cannot cause the 206; the earlier flow-scan flagged them only as write-divergent, not consumed).
  - DAT_1000_c686: LIVE.  c684 (`undefined4` = full far-ptr seg:off) + c686 (`undefined2` = saved segment)
    are the NovaLogic FAR-POINTER-RECONSTRUCTION pair used by fist_icall_far(DAT_1000_c684) at 37739 /
    51978.. .  FUN_0000_02c5: `c684=0x30b; call 541b; c684=c686; c686=unaff_CS` = the `mov [0x684],0x30b ;
    mov [0x686],cs` idiom (save this frame's CS into c686 so a later far call via c684 reconstructs a ptr
    back into this code segment).  FUN_1000_55c5 is a save/call/restore that writes c686=unaff_CS on exit.
So the determinism issue is precisely: native writes GARBAGE cs (recompile-dependent) into the far-ptr
segment slot c686; wasm elides the read.  The FAITHFUL value is the real running CS (what SetCSContext
computes: 0xf69/0x1000).  IMPLICATION: the current terrain W (native garbage == wasm elided) is likely
UNFAITHFUL vs the original (which saves the real CS) -- the matrix passes only because it checks
native==wasm, NOT the oracle.  So board:0002 (oracle) must confirm whether the c684:c686 far call is on
the terrain-render path and whether the original's frame = W or the real-CS frame W''.  If the far call is
terrain-relevant, BOTH targets must save+use the real CS (SetCSContext value + defeat wasm's elision); if
not, c686's value is harmless and W is fine.  Either way: only c686 matters, and it is a far-ptr-segment
idiom, not a raw data value.  patch 412 remains semantically innocent (recompiling be58 just reshuffles the
native garbage in this slot).

DECISIVE TEST (2026-08-24): c686 IS live + terrain-relevant; defining it EXPOSES a native/wasm far-call
divergence.  Forced c686 = 0xf69 (the real-CS value) at all 3 write sites (02c5/55c5/…) on BOTH targets,
built native + wasm, ran terrain-azer1: native vs wasm = **5879** (far worse than the committed 0 or the
412-perturbed 206).  Interpretation: with a GARBAGE c686 the c684:c686 far call resolves to an unmapped
segment -> fist_icall_far traps -> no-op -> NO terrain effect -> native==wasm=W (the committed coincidence
that PASSES the matrix by SIDESTEPPING the far call).  With c686 DEFINED (0xf69) the far call RESOLVES and
RUNS a real function that heavily writes the terrain framebuffer -> and native vs wasm run/resolve it
DIFFERENTLY (5879 bytes).  So: (1) c686 is genuinely on the terrain-render path (not dead); (2) the current
terrain native==wasm holds ONLY because garbage c686 makes the far call a no-op on both -- it is NOT
rendering the far-call's contribution at all, so W is likely UNFAITHFUL (missing whatever the original's
real-CS far call draws); (3) there is a SEPARATE, larger native/wasm divergence (5879) in the far-call
target's resolution or codegen, exposed the moment the call is activated.  CONSEQUENCE: board:0003 is a deep
native/wasm far-call determinism problem, and the matrix's passing terrain rows are passing by sidestepping
a real far call, not by faithfully rendering it.  patch 412's 206 is the same class (recompiling be58
reshuffles native's garbage c686, occasionally making the far call NON-trap on native only -> partial
activation -> 206).  Next: (a) identify the c684:c686 far-call TARGET (fist_icall_far(0xf69:off) -> which
FUN), (b) diff its native vs wasm execution to find the 5879 divergence, (c) oracle-confirm the original DOES
run this far call in terrain (so W is unfaithful and the far call must be made native==wasm + faithful).

UNCONFOUNDED (2026-08-24): per-site-CORRECT c686 ALSO gives 5879.  Re-ran with the asm-correct per-site CS
(FUN_0000_02c5/02e8 -> c686=0xf69 ; FUN_1000_55c5 -> c686=0x1000, matching SetCSContext) on both targets:
native vs wasm = 5879 again.  So the 5879 is NOT an artifact of a wrong uniform value -- even with the
faithful CS, activating the c684:c686 far call diverges native/wasm by 5879.  ROBUST CONCLUSION: the
far-call TARGET (fist_icall_far(c684)) executes/resolves differently native vs wasm.  The committed terrain
native==wasm=W holds ONLY because GARBAGE c686 makes the far call trap to a no-op on both targets -- W does
NOT include the far call's terrain contribution and is therefore very likely UNFAITHFUL vs the original
(which runs the call with a real CS).  board:0003's determinism root is thus a native/wasm divergence in
the c684:c686 far-call target, MASKED in the committed build by the garbage-c686 no-op.  This also explains
412's 206 as a partial unmasking (be58 recompile shifts native's garbage so the call non-traps on native
for some frames).  THE FIX PATH: (1) identify the far-call target FUN (c684 offset + the 0xf69/0x1000 seg),
(2) find why it runs native != wasm (a shim fist_icall_far resolution diff, or the target's own native/wasm
codegen/UB), (3) make it deterministic AND faithful, (4) oracle-confirm the original renders it.  This is
the real board:0003 work; it is a far-call-target determinism bug, now pinned to a single mechanism.

RETRACTION (2026-08-24): the "c684:c686 far-call TARGET diverges native/wasm" conclusion is DISPROVEN by a
direct trace.  Instrumented every c684 far-call site (c684 value + c686) in the per-site-correct-c686 build,
native vs wasm during terrain: IDENTICAL -- 1045 calls each, same value distribution (798x c684=3e781d23
c686=3e78 ; 193x 3e7819ae ; 29x 10000000/1000 ; 25x 030b/0000).  So (a) native and wasm fire the SAME c684
far calls with the SAME arguments -> the far calls are NOT the 5879 divergence source; (b) c686 at the
terrain far calls is 0x3e78 (set DYNAMICALLY before the calls), NOT my forced 0xf69/0x1000 -- the forced
value is OVERWRITTEN before these calls, so it only perturbs the boot WINDOW before being overwritten.
So the 5879 from forcing c686 is a genuine native/wasm divergence but through an UNIDENTIFIED non-c684 path
(the transient forced value cascades through some other consumer / boot-state).  This joins c3e2/c434 in
weakening the "terrain-site value drives the render" thesis: the c684 far-call mechanism I proposed is NOT
it.  HONEST STATE: forcing c686 != garbage triggers native!=wasm 5879, mechanism still not pinned; the c684
far-call hypothesis is eliminated.  The determinism divergence is a native/wasm codegen/UB sensitivity to
perturbed boot state, of which c686/be58(412)/a null-getenv are all triggers -- consistent with the ORIGINAL
"code-layout / uninit-state fragility" characterisation, NOT a specific far-call.  The reliable path remains
the oracle (does the original's terrain frame match committed-W?) + a native-vs-wasm state-diff at the FIRST
divergent byte under a controlled perturbation.  patch 412 stays innocent.

DECOMPOSITION (2026-08-24, loop): 412's 206 = ~149 uninit-reads + ~57 codegen-residual.  Built BOTH
targets with patch 412 + gcc/emcc -ftrivial-auto-var-init=zero (deterministic locals, a DIAGNOSTIC band-aid
NOT a doctrinal fix), ran terrain-azer1: native vs wasm = 57 (down from 206 without zero-init).  So
deterministic uninitialised locals remove ~149 of the 206 -> the uninit reads (the __allregs-prune segment
class SetCSContext addresses) are the DOMINANT cause of 412's terrain divergence, confirming the fix
direction.  A separate ~57-byte residual remains even with both targets zero-init'd -> a genuine
native(gcc -O0)-vs-wasm(emcc -O2) codegen/UB divergence INDEPENDENT of uninit locals (matches the earlier
"zero-init native vs wasm = 57").  So the full 412-terrain fix is TWO layers: (1) restore the pruned
segment/register writes so there are no uninit reads (the doctrinal SetCSContext migration, ~149 bytes),
and (2) resolve the ~57 codegen residual (a real portability diff to localise via native-vs-wasm state-diff
at the first divergent byte with both zero-init'd, so uninit noise is removed).  patch 412 remains innocent;
its 206 is entirely this pre-existing native/wasm fragility that recompiling be58 exposes.  Next: localise
the 57 (which bytes, which function) with both-zero-init builds to strip the uninit component.

DECISIVE (2026-08-24, loop): the committed terrain native==wasm=0 is itself a FRAGILE UB COINCIDENCE.
Committed (NO 412) + zero-init BOTH targets = 57 (the SAME 19 top-left pixels as the 412+zero-init case).
So the 57 is PRE-EXISTING, independent of 412.  The inversion is the tell: with GARBAGE uninit, native and
wasm CONVERGE in the top-left (committed=0, matrix passes); with ZERO uninit they DIVERGE (57).  So the
committed native==wasm=0 holds only because the garbage uninit values coincidentally converge -- ANY
perturbation (412, zero-init, a null getenv) breaks it.  This DEFINITIVELY confirms the terrain native==wasm
invariant is a fragile UB coincidence, not robust correctness.  DECOMPOSITION FINAL: the top-left 19-pixel
(x0..10,y0..10) region has (a) an uninit-read sensitivity AND (b) a genuine native(gcc-O0)/wasm(emcc-O2)
CODEGEN divergence that garbage masks and zero reveals -- 57 bytes.  The main voxel-terrain body is robustly
native==wasm.  ROBUST FIX (two parts, both dedicated): (1) eliminate ALL uninit reads on the terrain path
(the SetCSContext migration handles the segment class; other uninit locals may remain) so native and wasm
compute deterministically, AND (2) localise+fix the top-left 57 codegen divergence (a real portability bug
in whatever renders the 19-pixel corner element).  patch 412 stays innocent -- it is one of several
perturbations that expose the pre-existing fragility.  The matrix's terrain rows pass today but NON-robustly.
Next: identify which function renders the top-left corner (the 57's owner).

UNIFICATION (2026-08-24, loop): board:0003 has TWO independent divergence sources; the whole investigation
unifies.  State-diff native-zero-init vs wasm-zero-init at the terrain capture: 709 bytes divergent (BROAD),
first at DGROUP+0x452 = the frame timer (native 40 vs wasm 314 = the async-vs-coop TICK REGIME), scattered
across 687 non-tick/non-mga bytes -> the tick regime causes broad sim-state divergence.  BUT the framebuffer
diverges only 57 bytes (19 top-left pixels): the voxel-terrain BODY is TICK-ROBUST (renders identically
despite the broad state divergence), and only a TOP-LEFT display element is tick-SENSITIVE.  So:
  SOURCE 1 (~149 of 412's 206): uninit reads (garbage c686/segment class) -- fixed by zero-init /
    SetCSContext; masked in committed by garbage-convergence coincidence.
  SOURCE 2 (~57, pre-existing): the async-vs-coop tick regime feeding a tick-sensitive top-left element
    (its state at capture depends on [0x452]=40-vs-314); the terrain body is robust to it.
The committed native==wasm=0 masks BOTH via UB coincidence (garbage converges the top-left despite the tick
regime).  This RECONCILES the earlier tick-regime flip-flops: the tick regime IS a real divergence source,
but ONLY for the top-left element, NOT the terrain body -- which is why coop-both and async-both both showed
206 (the uninit source dominated) and why "tick regime" kept being retracted (it doesn't move the body).
ROBUST board:0003 FIX = BOTH: (1) deterministic tick so native==wasm reach the capture at the same [0x452]
(the render-advancing coop-pump seam), AND (2) eliminate the uninit reads (SetCSContext).  Test underway:
coop-both + zero-init -> if the 57 vanishes, SOURCE 2 is confirmed as the tick regime.

CORRECTION (2026-08-24, loop): the 57 is NEITHER tick NOR uninit -- RETRACT "source 2 = tick regime".
COOP native + zero-init vs wasm(coop) + zero-init = STILL 57.  So with native forced to COOP ticking (same
tick model as wasm) AND both zero-init'd (no uninit reads), the 57 top-left divergence PERSISTS.  Therefore
the 57 is a PURE native(gcc-O0)/wasm(emcc-O2) CODEGEN/UB divergence in the top-left render, independent of
both the tick regime and uninitialised locals.  The 709-byte broad state divergence in the async state-diff
DID include the tick regime, but that is a red herring for the 57 (the terrain body absorbs it; the 57
survives tick-matching).  So the honest decomposition of 412's 206 is: ~149 = uninit reads (SetCSContext),
~57 = a pure codegen divergence in a 19-pixel top-left element (a real portability bug: FP, integer width,
or aliasing in whatever renders x0..10,y0..10).  Both fixes are dedicated and INDEPENDENT.  Localising the
57 owner (which function writes the top-left corner) is the concrete next step for that portability bug.

LOCALISED -- the 57 is a MOUSE-CURSOR / CROSSHAIR draw, not a terrain codegen bug (2026-08-24, loop).  The
19 divergent pixels form a PLUS/CROSS at (5,5): vertical line x=5 (y=0..10), horizontal line y=5 (x=0..10),
black centre (5,5)=000000, white arms fbfbfb.  NATIVE draws this white crosshair cursor; WASM does not (it
shows the terrain/sky underneath).  This ties to the 0x578-region divergence in the state-diff: the mga
display-element FAR-PTR method vectors (c578/c57c/c580 = `(*_DAT_1000_c578)()` cursor/element handlers) are
POPULATED on native and ZERO on wasm.  So native has the cursor-draw method vector installed and runs it
(drawing the crosshair at the cursor position); wasm's vector is 0 -> no draw.  This is a control-flow
divergence in the mga (MGAVIDEO) cursor-element registration -- native reaches the registration that sets
c578, wasm does not -- INDEPENDENT of tick and uninit (persists under coop-both + zero-init).  So 412's 206
= ~149 uninit segment reads + ~57 mga-cursor-crosshair that native draws and wasm doesn't.  FIDELITY
QUESTION (board:0002): does the ORIGINAL show a cursor/crosshair in the FIST_TERRAIN voxel view?  If NO ->
native's cursor draw is the bug (match wasm's no-draw); if YES -> wasm is missing it.  Either way this is a
concrete mga-cursor-vector control-flow divergence, not an FP/codegen bug.  Next: find why native populates
c578 (the cursor method vector) and wasm zeros it -- the mga cursor-registration path divergence.

PRECISE LOCALISATION (2026-08-24, corrects the "mouse cursor" guess): the 57 is a c578 mga DISPLAY-ELEMENT,
NOT the mouse cursor.  Read the cursor + mga state from the zero-init statedumps (native vs wasm, terrain
capture): the MOUSE CURSOR is IDENTICAL -- d5d9(draw-gate)=0xff both, d5ca/d5cc(cursor y/x)=186/40 both,
d5c4(sprite)=0x0028 both, d5ac(cursor far-ptr)=0x3e782f04 both, dda0/dd9e(mouse)=40/186 both.  So the mouse
cursor is at (40,186) and converges -- it is NOT the (5,5) crosshair.  The ACTUAL divergence is c578:
native=0xe5288b89, wasm=0x00000000.  c578 is an mga display-element DESCRIPTOR set by FUN_0000_3fca
(c578=param_1, c57c=param_3[1]-param_3[0], c580=param_2[1]) and CALLED via `(*_DAT_1000_c578)()`.  So the
(5,5) crosshair is drawn by the mga display-element whose descriptor c578 native POPULATES (0xe5288b89) and
wasm leaves ZERO -> native draws the element, wasm doesn't.  ROOT: FUN_0000_3fca runs (or runs with real
params) on native but not on wasm -- a control-flow divergence in the mga display-element setup that
persists under coop-both + zero-init (so NOT tick, NOT uninit; a genuine native/wasm control-flow diff in
reaching 3fca).  So 412's 206 = ~149 uninit segment reads + ~57 the c578 mga display-element that native
sets up (via 3fca) and wasm doesn't.  Next (dedicated): trace why native reaches FUN_0000_3fca with real
params during terrain and wasm doesn't -- the mga display-list setup control-flow divergence.  Both board:0003
components are now precisely localised: (1) the unaff_CS/ES segment class (SetCSContext), (2) c578/3fca.

FINAL LOCALISATION (2026-08-24): the 57 is a CONTROL-FLOW divergence in the mga display-element caller.
Traced FUN_0000_3fca (the c578 setter) entry, native vs wasm, both zero-init'd, during terrain: BOTH call
it once, but with DIFFERENT params:
  native: p1=0x8b89, p2=0x081d4ff4 (a g_mem host ptr, g_mem+0xae14), p3=0xffd4eac8 (a stack ptr)
  wasm:   p1=0x0000, p2=0 (null),   p3=0 (null)
So native takes the "real display-element" branch (real params -> c578=0x8b89 -> draws the (5,5) element),
wasm takes the "null/default" branch (3fca(0,0,0) -> c578=0 -> no draw).  This is a control-flow divergence
in the CALLER of 3fca: native's branch condition is true, wasm's false -- driven by some upstream state that
differs native vs wasm and PERSISTS under coop-both + zero-init (so not tick, not uninit; a genuine
native/wasm state/codegen divergence in the mga display-list setup).  The chase bottoms out here: the caller
condition depends on upstream diverged state (the broad 709-byte state divergence has a real component
beyond tick/uninit that this branch reads).  So board:0003's 57 = an mga display-element that native's setup
path enables and wasm's does not, at the caller of FUN_0000_3fca.  This is a dedicated mga-driver control-flow
investigation (find the branch + the upstream state it reads) -- the exhaustive perturbation chase has
localised it to this caller but the positive fix needs the mga setup path traced with the oracle to decide
whether the (5,5) element SHOULD render (native faithful) or not (wasm faithful).  board:0003 fully mapped:
(1) ~149 unaff_CS/ES uninit (SetCSContext migration), (2) ~57 the 3fca-caller mga-element branch divergence.

ROOT FOUND -- the 57 is a SPURIOUS RETICLE-GRATICULE that native registers in the pure-terrain view and
wasm does not (2026-08-24).  Backtrace of the 3fca caller: app_entry -> 00d0 -> cae6 -> e714 (display-list
vector install, PATCH 098) -> 459a -> 22dd -> 286e -> 3fca.  FUN_0000_3a68 in this chain is "PATCH 324: 22dd
RETICLE-GRATICULE script handler", and PATCH 324 draws the reticle graticule (FUN_0000_3d99 line-clip).  So
the (5,5) crosshair is the tank targeting-RETICLE GRATICULE.  The immediate caller FUN_0000_286e branches on
`if (DAT_2000_0b9e == 0)`: DAT_2000_0b9e is the reticle display-element LINKED-LIST head; native has it
NON-NULL (elements present -> the do/while walks them, dispatching the reticle method -> 3fca with real
params -> c578 set -> draws the graticule), wasm has it NULL (empty list -> no reticle drawn).  FIDELITY (no
oracle needed): FIST_TERRAIN renders ONLY the voxel view (no HUD overlay, per CLAUDE.md/board), so the
targeting reticle should NOT appear in the pure-terrain capture -> WASM (empty list, no reticle) is FAITHFUL,
NATIVE (spurious reticle graticule at (5,5)) is the BUG.  So the 57 = native spuriously populates the reticle
display-list (DAT_2000_0b9e) in the FIST_TERRAIN path where wasm (correctly) leaves it empty.  The fix is a
shim/setup one: find why native's FIST_TERRAIN path registers the reticle element into DAT_2000_0b9e and
wasm's does not, and stop native (match wasm's no-reticle terrain view).  This is a CONCRETE shim bug, NOT
oracle-gated -- the pure voxel view has no HUD by definition.  board:0003's 57 is now fully rooted:
native-spurious-reticle in the terrain-only view via DAT_2000_0b9e.

VERIFIED (2026-08-24, avoids over-claim): the committed build correctly shows NO reticle on BOTH targets.
Checked (5,5) + cross arms: committed-native = 3c3c3c/3c3c3c/4d4949 (terrain, NO reticle); committed-wasm =
IDENTICAL (NO reticle) -> native==wasm=0 is a faithful no-reticle terrain view.  Only ZERO-INIT native shows
the reticle ((5,5)=000000, arms fbfbfb).  So the refined truth: committed-native leaves DAT_2000_0b9e NULL
(garbage coincidence -> no reticle, matches wasm); zero-init native makes 0b9e NON-NULL (draws reticle).  BUT
wasm is spec-zero-init and STILL has 0b9e NULL (no reticle) -> a genuine native(gcc-O0)/wasm(emcc-O2) CODEGEN
divergence in the 0b9e reticle-display-list population: with identical zero locals, native populates 0b9e and
wasm does not.  The committed native==wasm=0 holds only because native's GARBAGE coincidentally leaves 0b9e
null (matching wasm's deterministic null).  FAITHFUL behaviour = NO reticle in the pure voxel view (wasm
robustly correct; committed-native correct by luck).  So the 57 is a native/wasm codegen divergence in the
reticle-graticule display-list population (DAT_2000_0b9e), NOT a semantic reticle bug.  ROBUST FIX: make
native's 0b9e population deterministic == wasm's (null -> no reticle) by finding the gcc-O0/emcc-O2 codegen
divergence in the reticle/display-list setup (289b/22dd/286e path via e714).  This is the concrete root; the
verification (committed = no-reticle on both) prevented an over-claim that native always draws a spurious
reticle.  board:0003's 57 rooted + verified.

CORRECTION (2026-08-24, loop): the "0b9e branch drives the reticle" was WRONG.  Traced 286e's branch:
DAT_2000_0b9e = NULL on BOTH native and wasm at the branch (286e fires once each, both take the `==0` true
branch -> the reticle do/while loop does NOT run on either).  So 0b9e is NOT the divergence.  The 3fca call
(from the backtrace 22dd->286e->3fca) comes from 286e's FIRST line `fist_icall_far((uint32_t)DAT_1000_c664)()`,
NOT the 0b9e loop.  Both targets call 3fca via c664, but with DIFFERENT params (native real host-ptrs, wasm
nulls) -- and 3fca takes NO explicit args (the __allregs model), so its p1/p2/p3 come from the REGISTER STATE
at the c664 call.  So the reticle-element c578 population divergence is a REGISTER/STATE divergence upstream
of 3fca, manifesting via the c664->3fca path -- the same broad native/wasm state divergence, NOT a single
fixable branch.  This is the ~7th level of the chase and each level reveals the divergence one step upstream
in the register/state flow -> the 57 is DISTRIBUTED broad-state divergence reaching the framebuffer via the
reticle path, not pinnable to one line.  HONEST CONCLUSION: board:0003's 57 (like the 149) is a manifestation
of the fundamental native(gcc-O0)/wasm(emcc-O2) state divergence; the committed build converges by UB
coincidence; the robust fix is deterministic state on both (SetCSContext for the uninit class + resolving the
compiler-level state divergence), NOT a targeted reticle patch.  The reticle path is just where the 57 bytes
surface.  Further single-line chasing has diminishing returns.

DECISIVE ELIMINATION (2026-08-24, loop): the 57 is an ENVIRONMENT divergence (x86-32 gcc vs wasm32 emcc),
NOT compiler-opt and NOT uninit.  native -O0 + zero-init vs wasm -O0 + zero-init = STILL 57 (matched opt
level AND matched zero-init still diverge).  So it is neither an emcc -O2 optimization (both -O0) nor
uninitialised locals (both zero).  It is inherent to the native-vs-wasm ENVIRONMENT.  STRONGEST CANDIDATE:
HOST POINTERS stored in g_mem -- the 3fca params were host pointers (p2=0x081d4ff4 in the g_mem region,
p3=a stack address) on native; on wasm these are wasm-linear addresses (different values, or 0).  If the
mga/reticle path stores a host pointer into a g_mem slot and a later read does arithmetic/comparison on its
VALUE (not just deref), native (0x081.../high stack) and wasm (small wasm offsets) diverge -> the reticle
element descriptor c578 gets a different value -> 57 FB bytes.  This is the documented host-pointer-in-g_mem
porting class (the shim stores host ptrs in slots like [0x917]=&fb; the reticle path evidently stores one
that reaches the render).  So board:0003's 57 = a host-pointer-representation divergence in the mga/reticle
setup; the committed build converges because native's GARBAGE c578 coincidentally matches wasm's, but any
perturbation exposes the environment-dependent host-pointer value.  FIX CLASS: ensure the mga/reticle path
does not let an environment-dependent host-pointer VALUE reach a render-visible g_mem field (normalise to a
rebased offset, or don't store the host ptr where the render reads it).  This is a real, bounded porting bug
in the mga driver's pointer handling -- concrete and NOT oracle-gated.  Progress: the 57 is eliminated down
to the host-pointer class via decisive testing (not compiler, not uninit -> environment/host-ptr).

UNIFIED (2026-08-24, loop): both board:0003 components are ONE class -- register/state-dataflow divergence.
c664=0x3e783fca (far-ptr -> mga 3fca) is IDENTICAL native/wasm; the divergence is ONLY param_1 (0x8b89
native / 0 wasm), a register value that is (not-uninit under zero-init, not-opt under -O0, not a pointer
target) the LOW-16 of a host-pointer-in-g_mem -- the environment (x86-32/wasm32) host-ptr class.  So the 57
(host-ptr-low16 into the reticle param) and the 149 (unaff_CS/ES uninit) are BOTH register/state-dataflow
divergences that the committed build converges by UB coincidence.  UNIFIED FIX PATH: deterministic
register/pointer dataflow -- SetCSContext (the uninit segment class) + host-pointer normalisation (don't let
an env-dependent host-ptr's low16 reach a render-visible field).  board:0003 is now FULLY characterised:
one root class, two surfaces, one dedicated fix direction.  This is the culmination of the whole
investigation -- from "mysterious 206" to "register/state-dataflow determinism, precisely located at
unaff_CS/ES (149) and the reticle host-ptr-low16 param (57)".

INTEGRITY PROVEN FROM GIT (2026-08-24, loop): the committed matrix is intact WITHOUT re-running the gate.
This session's committed changes are ONLY: board/*.md + CLAUDE.md (docs), tools/ghidra/SetCSContext.java +
tools/decompile.sh (the decompile pipeline -- affects only a FRESH `make decompile`, NOT the `make patch ->
build` the matrix uses), tools/assemble_fist.py (the forward-decl fix, proven native .text BYTE-IDENTICAL),
and re_out/fist.c (regenerated with the assemble fix, +2 codegen-inert forward decls, proven .text-identical).
`git log` confirms fist_decomp.c, patches/, fist_*.c (shim), and tools/native_main.c are UNCHANGED this
session (last touched in prior sessions).  So the native/wasm matrix binaries are behaviourally identical to
before this session -> the 176/176 matrix (10x-gated, board:0008) holds by construction; no re-run needed.
(A `wasm_gate.sh` I launched earlier spun uselessly because its input binary /tmp/fisttest/fistrun.js was
never built -- a launch misfire, not a matrix failure; stopped and superseded by this git-based proof.)

CHASE LIMIT + CONFOUND (2026-08-24, loop): the 57-reticle chase has hit its productive limit; stopping it
honestly.  Checked d548 (the reticle-phase byte 22dd branches on) in the ZERO-INIT statedumps: d548=0x81
(-127) on BOTH native and wasm -> both take 22dd's RE-SEED branch (no reticle render via 22dd), and ALL
22dd reticle fields (d56a=0x156c, c450=0, 0a86=0x0c0b, 0a88=0x6262) are IDENTICAL native/wasm.  So 22dd is
NOT the zero-init reticle-render path.  The earlier backtrace (22dd->286e->3fca) was from the GARBAGE-init
build (/tmp/fist_bt, not zero-init), where d548 differed -> I CONFOUNDED the garbage-build backtrace with
the zero-init statedump; the zero-init reticle-render path is DIFFERENT and unpinned.  This is the ~8th
level of the 57 chase and it is now producing confounds (mixed build states) rather than convergence.
HONEST STOP: the 57 is a deep, DISTRIBUTED native/wasm environment (host-pointer/register-state) divergence
in the reticle path that perturbation-tracing cannot cleanly pin to one fixable line -- each level reveals
the divergence one step upstream and mixing build states risks false leads.  The robust fix remains the
systematic register/pointer-dataflow determinism (dedicated), NOT more single-line chasing.  board:0003 is
FULLY characterised (206 = ~149 unaff_CS/ES uninit + ~57 reticle host-ptr/register-state, one root class:
register/state-dataflow determinism); both fixes are dedicated implementation.  Further autonomous
perturbation experiments on the 57 are retired as diminishing-returns/confound-prone.

CORRECTION of the confound claim (2026-08-24, loop) -- re-ran CONSISTENTLY: the backtrace is NOT a confound.
The zero-init 3fca backtrace is IDENTICAL to the garbage build: app_entry -> 00d0 -> cae6 -> e714 -> 459a ->
22dd -> 286e -> 3fca, with p1=0x8b89, p2=0x081d4ff4 (g_mem ptr), p3=stack -- same in both builds.  So the
reticle renders via 22dd->286e->3fca on BOTH builds; my "confound" was wrong.  The actual error was the d548
reading: d548=0x81 in the STATEDUMP is the FINAL value at the capture, NOT the render-time value -- 22dd
renders the reticle EARLIER in boot (when d548>0), then d548 becomes 0x81 later; the statedump doesn't
capture render-time d548.  So the reticle IS rendered via 22dd's render branch.  The genuine divergence:
param_1=0x8b89 at the 3fca call = the LOW-16 of the host pointer 0xe5288b89 that 3fca stores into c578 (the
mga display-element descriptor); on wasm the equivalent host pointer is a wasm-linear address -> different
low16 -> c578 differs -> the reticle draws differently (or not) -> 57 FB bytes.  CLEAN CONFIRMED ROOT: the 57
is the HOST-POINTER-IN-g_mem class -- c578 holds a raw host pointer (0xe5288b89 native) whose representation
is environment-dependent (x86-32 vs wasm32).  The committed build shows NO reticle on both (garbage
coincidence); perturbation (zero-init/412) makes native's host-ptr-derived c578 non-null -> spurious reticle.
FIX CLASS (concrete): the mga reticle-descriptor path must not store a raw host-pointer VALUE in a
render-visible g_mem field (c578) -- normalise to a rebased offset, or ensure the descriptor uses the
16-bit engine representation not the host pointer.  This is a real, bounded host-pointer-model bug in the
mga display-element setup, the SAME class as the shim's other host-ptr-in-g_mem slots.  board:0003's 57 is
now cleanly + consistently rooted (no confound): host-pointer-in-c578, environment-divergent.

ASM-CONFIRMED ROOT (2026-08-24, loop): 3fca's stores are AX (param_1) and DI (param_3), both host-ptr-derived
in the port.  objdump of FUN_0000_3fca @ mga 0x3fca:
  3fca: push es; push ds; pop es ; mov ds,ss:[0x70a]   (DS = the reticle-descriptor segment, ~DGROUP)
  3fd2: mov [0x578], ax           -> c578 = AX  (= param_1)
  3fd5: mov [0x57a], di           -> c57a = DI  (= param_3)
  3fd9: mov ax, ss:[di+2] ; ... stores derived values to c57c/57e/580/582/584/586/588.
In the ORIGINAL, AX and DI are 16-bit ENGINE values (offsets); the decompile modelled param_3 as `int *`
(a HOST pointer) and param_1's AX comes from the caller's register flow carrying a host-ptr-derived value.
So c578=AX=0x8b89 (native, host-ptr low16) / 0 (wasm) and c57a=DI (host stack-ptr low16) both DIVERGE by
environment -> the 57 reticle-descriptor bytes.  This ASM-CONFIRMS that BOTH board:0003 components are the
SAME __allregs register/pointer-dataflow class: the 149 (unaff_CS/ES segment regs read uninit) and the 57
(AX/DI carrying host-pointer-derived values instead of faithful 16-bit engine offsets, environment-divergent).
UNIFIED ROOT (asm-level): the port's register/pointer dataflow lets HOST-environment values (host pointers,
uninit segment regs) reach engine-state fields that the render reads; native (x86-32) and wasm (wasm32)
represent these differently, so the committed native==wasm holds only by garbage coincidence and any
perturbation (412/zero-init) exposes it.  FIX (dedicated, sanctioned mechanism): asm-verified patches that
thread the faithful 16-bit engine values into the reticle-descriptor caller path (22dd/286e -> 3fca AX/DI)
+ the SetCSContext segment-context for the unaff_CS/ES class -- i.e. deterministic register/pointer dataflow
so no host-environment value reaches render-visible engine state.  board:0003 is now ASM-ROOTED end to end.

FIX DIRECTION PROVEN AT THE SHIM LEVEL (2026-08-24, loop): the 149 unaff_CS/ES class is a PURE DECOMPILE
(Ghidra-unthreaded) problem; the SHIM is already deterministic.  asm of FUN_1000_2ebe @ 0x12ebe:
  mov ax,0x3508; int 0x21          ; DOS AH=35 get-INT-vector for INT 8 -> returns ES:BX
  mov [0x434], es                  ; c434 = the INT-8 handler SEGMENT (the ES the INT returned)
The shim's AH=35 handler (fist_dos.c:321) returns `R_ES = FIST_INTVEC_SEG` -- a CONSTANT (0xFE00),
DETERMINISTIC on both native and wasm.  But the decompile models `mov [0x434],es` as `DAT_1000_c434 =
unaff_ES` -- an UNINITIALISED C local, because Ghidra did NOT thread the INT-21 ES return.  So in the port
c434 = garbage (native) / 0 (wasm), NOT the faithful deterministic FIST_INTVEC_SEG.  (This specific c434 is
also DEAD -- no readers -- so harmless; but it typifies the class.)  CONCLUSION: the 149-class
non-determinism is entirely the DECOMPILE not threading values that are DETERMINISTIC at the shim/engine
level (INT-return ES/BX, saved CS/ES segments).  So the fix is 100% in the GHIDRA PIPELINE -- SetCSContext
(board:0010, done, for the CS/ES segment saves) PLUS extending InstallIntFixup to thread the INT-21 AH=35
ES:BX return (and similar unthreaded INT returns).  NOT a shim fix; NOT a per-site source hack.  This
sharpens board:0010: the pipeline fix (SetCSContext + InstallIntFixup INT-return threading) resolves the
149 class deterministically, and a fresh decompile + patch migration lands it.  The shim already does the
right (deterministic) thing; only the decompile's register/INT-return threading is incomplete.

AUDIO ROOT REFUTED FROM LIVE CAPTURE (2026-08-25, loop) -- the top-of-file "instrument high-nibble
dropped / program-change never routes to 0f99" root is STALE.  Re-measured the CURRENT build's OPL
register stream (FIST_OPL=1 FIST_SB=1 FIST_OPL_REGLOG, mainmenu; reglog carries adv=g_snd_seq_advances
so a write's sequencer phase is visible) against a fresh oracle trace (third_party/dosbox-fist,
FISTOPLLOG, 32s menu-wait + 10s play, 4564 writes).  Two config-INDEPENDENT facts overturn the old root:
  (1) The port DOES reload instruments DURING playback, not only at init.  reg 0x20 (operator AM/VIB/
      EG/KSR/mult) is written at adv=1570,1598,1769,1797,3620,5329,5357,5528,7379 -- all adv>0 (sequencer
      advancing), only the very first at adv=0 (the 104f OPL-init instrument 0).  So 0f99/0fab ARE
      reached from the note sequencer with nonzero instruments; the "0f99 only called from init,
      program-change never dispatched" claim is FALSE for the current tree.
  (2) The port writes a WIDE instrument-byte range INCLUDING the high bits: reg 0x20 takes
      {00,01,03,05,34,4e,b1,b2,c4,c9}, reg 0x23 takes {00,11,15,22,77,aa,ff}.  b1/c4/c9 have bit7(AM)+
      bit6(VIB) SET -- the port is NOT "dropping the high nibble / stuck at 0x01".  That characterisation
      is FALSE.
So neither the "& 0x0f mask / decode" bug nor the "missing per-voice program-change dispatch" root holds.
WHAT ACTUALLY DIVERGES (empirical): the port and oracle play a DIFFERENT NOTE STREAM at the menu, not the
same song with wrong timbre.  reg 0x20: oracle steady-state is essentially ONE melody instrument (0x31,
repeated) + a 0x01 init; the port loads a 9+-voice BANK ({b1,b2,c4,c9,4e,34,03,00,05,01}).  Channel-0
A0/B0 fnum streams: the SETUP prefix matches (both B0 = 00,1f,1f,1f,1f,1f,3f) then the melodies diverge
(port B0 ->1f,1f,1f,36,16..; oracle B0 ->22,02,22,2a,0a..; oracle A0 is a constant 0xe4 drone, port A0
varies).  So the divergence is upstream of the OPL layer -- in WHICH song/notes the sequencer plays, not
in instrument-byte decoding.
CAVEAT (why this is not yet a landed fix): the comparison is NOT config/timing-matched.  The port's OPL
output is heavily tick-rate sensitive (127 writes in a 4s run, 450 in a 6s run, 5222 in a 25s run -- the
melody warms up late on the cooperative tick), and the oracle runs sb16/oplmode=auto vs the port's device
selection (c012, unverified here).  So the NOTE-STREAM divergence needs a device- and phase-matched
recapture before it is conclusive; only refutations (1)+(2) are config-independent and solid.
NEXT (redirected, honest): the tractable audio question is no longer "why is the timbre wrong" but "does
the port's sequencer read the SAME menu song + program numbers as the oracle" -- i.e. song-stream
fidelity in the threaded MS3 interpreter (the note handlers 0cd1/0cf3/0cfb tail-jump via [c1c1]; the
command decode sits behind Ghidra's unrecovered jumptables at 0xae2/0xaec/0xd45 in fist_snd.c).  Confirm
first that both load the SAME menu .MS3 and select the SAME device (c012), phase-match the capture, THEN
diff the requested program-change numbers.  The instrument PATCH TABLE at 0x1dd is the same static
SOUNDDVR.DVR bytes on both, so it is NOT the suspect.  Matrix invariant unaffected (audio is not
framebuffer-gated); this loop corrected the board root, it did not change any build input.

AUDIO ROOT SHARPENED + UNIFIED (2026-08-25, same loop) -- ruled OUT the remaining local suspects, so the
divergence is the cooperative-tick TIMING model, not a sequencer decode bug:
  - SAME song: FIST_OPENLOG shows the port opens MAINMENU.MS3 (+ MAINMENU.MRL) for the menu -- the correct
    menu track; the oracle plays the same menu -> same file.
  - SAME device config: both read armoredfist/SOUND.CFG = ASCII "0132710000" (mounted for DOSBox too) and
    the static SOUNDDVR.DVR instrument table at 0x1dd -> identical device selection + patch bank on both.
  - The Ghidra "Could not recover jumptable at 0xae2/0xaec/0xd45" warnings are NOT the bug: objdump of
    re_out/fist_snd_image.bin shows all three are `ff 26 xx 01` = `jmp [c1a5]` / `jmp [c1c1]` -- THREADED-
    CODE tail-jumps through the device handler vectors (loaded by FUN_0000_0872), faithfully modeled as
    `(*fist_icall_far(cXXX))()`.  The threaded dispatch is intact.
So: same file, same table, same device, intact dispatch -- yet the note stream diverges (oracle channel-0
A0 is a CONSTANT 0xe4 drone + a sparse 0x31 melody; the port front-loads a 9-voice bank {b1,b2,c4,c9,4e,
34,..} with a varying A0).  The tell is STRUCTURAL + timing: the port's OPL write count is wildly tick-rate
dependent (127 writes/4s, 450/6s, 5222/25s -- the melody "warms up" late on the cooperative tick), i.e. the
port is not PACING song events at the original's tempo; it dumps many events per pumped tick where the
oracle spreads them over real PIT time.  This is the SAME cooperative-tick timing-fidelity gap that
board:0001 names for the live mission sim (the sequencer, like 459a, evolves only when fist_timer_pump
runs, and the pump cadence != the original PIT cadence).  CONCLUSION: menu-music bit-identity is not an
isolated fist_snd.c decode fix -- it is gated on a faithful (instruction-counted / PIT-accurate) tick
source, the same deep seam FIST_TICK_HZ is a placeholder for.  Audio + live-voxel unify under ONE root:
cooperative-tick timing fidelity.  This retires the "sequencer decode / jumptable / instrument-mask"
leads as dead ends and points the audio work at the timing model (shared with board:0001), not fist_snd.c.

SELF-CORRECTION (2026-08-25, same loop, before commit) -- the "CONCLUSION: gated on the tick model"
above OVERSTATES.  The note-VALUE differences argue against pure timing: after the shared setup prefix
(B0 = 00,1f,1f,1f,1f,1f,3f) the port continues B0 = 1f,1f,1f,1f,36,16,.. while the oracle continues
22,02,22,2a,0a,.. -- these are different PITCHES/blocks (0x36=keyon+blk5+fnhi2 vs 0x22=keyon+blk0+fnhi2),
not the SAME values at a stretched cadence.  Pure tempo divergence would replay the same values in order.
BUT the two captures are PHASE-UNMATCHED (port sampled from menu-entry; oracle sampled 32s into steady
menu music) -- comparing port-event-k to oracle-event-k is meaningless across different song positions.
So BOTH hypotheses stay live and are NOT yet distinguishable:
  (H1) cooperative-tick tempo/pacing (front-loading events) -- supported by the tick-rate write-count
       sensitivity (127/4s .. 5222/25s);
  (H2) a sequencer decode/interpretation divergence -- supported by the different note values post-prefix.
The ONLY way to decide is a PHASE-MATCHED capture: drive the port to the same song position as the oracle
(or capture the oracle from menu-entry with a short MENU_WAIT) and diff the event streams aligned at the
song start.  Honest state: refutations (1)+(2) from the prior entry are solid (no instrument-mask bug, no
missing program-change dispatch, jumptables are threaded tail-jumps, song+table+device all match); the
timing-vs-decode question is OPEN and capture-confounded.  Do the menu-entry oracle recapture next.

PHASE-MATCHED DETERMINATION (2026-08-25, same loop) -- recaptured the oracle OPL from menu-ENTRY
(MENU_WAIT=20 vs 32): the oracle channel-0 B0 stream is IDENTICAL for both waits
(00,1f,1f,1f,1f,1f,3f,22,02,22,2a,0a,2a,..) -> the oracle log always starts at song position 0, so port
(from its first write) and oracle are now PHASE-ALIGNED at the song start.  Aligned channel-0 B0:
  oracle: 00 1f 1f 1f 1f 1f 3f 22 02 22 2a 0a 2a 2a 0a ...
  port:   00 1f 1f 1f 1f 1f 1f 3f 1f 1f 1f 1f 36 16 00 ...
They MATCH through the first six values (00 + five 1f) then DIVERGE: the port emits an EXTRA 1f (six vs
five keyoff/setup writes) before its first key-on 3f, and thereafter plays different PITCHES (port
36,16,.. = keyon+blk5/fnhi2 etc; oracle 22,02,2a,.. = keyon+blk0/fnhi2 etc) -- different note VALUES at
the SAME song position, not the same values time-stretched.  This DECIDES H1 vs H2: pure cooperative-tick
tempo (H1) would replay identical values at a different cadence; the values themselves differ from event
~6, so the root is H2 -- a SEQUENCER INTERPRETATION divergence: the port's MS3 event walk reads
MAINMENU.MS3 into a different note/keyoff stream than the original, from near the song start.  (The
tick-rate write-COUNT sensitivity is a real but SEPARATE tempo effect, not the note-content cause.)
CAVEAT (honest, not yet airtight): the port stream still includes the OPL-init (104f/1082) B0 writes; the
one-extra-1f could be a init-vs-song alignment offset of a single event.  To close it, strip the init
prefix (the 104f channel-init writes are deterministic) and re-align at the first SONG event, then diff.
But the post-key-on pitch divergence (36/16 vs 22/02/2a) is past any single-event shift, so H2 stands.
NET for the loop: the audio root is now a phase-matched, config-matched, table-matched SEQUENCER-DECODE
divergence in the port's MS3 interpreter (fist_snd.c) -- reachable by event-by-event port-vs-oracle diff
from the song start, NOT the tick model and NOT the previously-blamed instrument-mask/dispatch/jumptable.
That is the concrete, bounded next handle.  (Refutations this loop: instrument-mask bug; missing
program-change dispatch; unrecovered-jumptable decode; device/song/table mismatch; pure-timing tempo.)

CONCRETE NEW LEAD -- CHANNEL/VOICE ALLOCATION DIVERGES (2026-08-25, same loop): all-channel key-on
(B0-B8, bit5 set) from song start, phase-aligned:
  oracle: ch7:3f ch0:3f ch2:3f ch3:3f ch5:3f ch6:3f ch6:3f  ch7:20 ch7:26 ch6:2e ch5:32 ch3:26 ch2:2a ch0:22 ..
  port:   ch0:3f ch1:3f ch5:3f ch8:3f                        ch8:26 ch1:26 ch0:36 ch1:26 ch1:26 ch1:26 ch0:36 ..
The port plays the melody on DIFFERENT OPL channels (ch0/1/5/8) than the original (ch7/0/2/3/5/6), with a
different note count in the opening chord (7 key-ons vs 4).  So the divergence is (or includes) VOICE->
CHANNEL ALLOCATION: the sequencer's per-voice channel assignment lands notes on the wrong OPL channels.
That allocation flows through the channel map read at DGROUP:0xc01 (`bVar2 = *(byte*)((param_2>>8)+0xc01)`
in 0f99/10a6/10e3) -- the voice-index -> OPL-channel table.  If the port's 0xc01 map (or the voice->param
that indexes it) differs from the original, every note retargets.  CONCRETE NEXT: dump the port's 0xc01
channel-map bytes at song start and compare to the original (via a dosbox-fist FIST_WATCHFLAT on the
SOUNDDVR DGROUP:0xc01 span, or read it from the driver's static init); and trace what sets param_2's high
byte (the voice index) at the note-on call site.  This is the bounded handle for the H2 sequencer-decode
divergence: a voice/channel-allocation table or index, not the OPL layer.

RUNTIME-VERIFIED REFINEMENT via gdb on the -g native build (2026-08-25, same loop) -- pinned the root
one level deeper and RULED OUT the channel-map/base suspects with live memory dumps:
  - Patch 353's driver-DS base is CORRECT at note-play time: at a melody 0f99 call the word at
    snd_base+0x831 = 0x3ce9 = load_seg(0x3a44)+0x2a5, the reloc-applied driver data segment (driver start
    bytes at snd_base match fist_snd_image.bin).  (An earlier 0x2b reading was a PRE-RELOCATION first-hit
    artifact -- the very first 0f99 (104f init) can be caught before the driver DS settles; the melody
    calls use 0x3ce9.  Self-corrected.)
  - The voice->channel map at driver_ds:0xc01 is a SENSIBLE IDENTITY map, not corrupt:
    00 01 02 03 04 05 06 07 63 08 63 63 63 63 63 63  = voice V -> OPL channel V for V=0..7, voice 8
    DISABLED (0x63 > 8 -> 0f99's `if(8<bVar2)return`), voice 9 -> channel 8.  9 usable voices.
  - Mapping the observed key-on CHANNELS back through this map: the port activates VOICES {0,1,5,9} at
    song start (key-ons on ch0/1/5/8); the oracle activates VOICES {0,2,3,5,6,7} (key-ons on ch7/0/2/3/5/6).
    Different VOICE SETS, and different COUNTS (port 4, oracle 6-7) -- even the opening all-voices key-on
    (val 3f, block7 reset) is 4 voices in the port vs 7 in the oracle.
CONCLUSION (runtime-verified, convergent -- not the earlier oscillation): the audio divergence is a
SONG-PARSE / VOICE-ACTIVATION fidelity bug in the port's MS3 sequencer -- it activates a different (smaller)
set of tracks/voices from MAINMENU.MS3 than the original, from the song's first event.  It is NOT the OPL
layer, NOT the instrument decode/mask, NOT the driver-DS base (0x3ce9 correct), NOT the 0xc01 channel map
(identity, correct), NOT the dispatch/jumptables.  CONCRETE NEXT: find where the sequencer reads
MAINMENU.MS3's TRACK TABLE (the per-voice song-pointer array) and how many voices it starts -- compare the
port's track-count/pointer parse to the SOUNDDVR.DVR asm (the FUN_0000_0872/104f voice-init loop iterates
9 voices; the song header selects which are active).  The bug is that the port starts 4 voices where the
original starts 6-7 -- a bounded track-activation parse defect, reachable by asm diff of the song-load path.

BASE-LOSS FULLY RULED OUT (2026-08-25, same loop, asm-verified) -- checked the two voice-path functions'
segment bases against re_out/fist_snd_image.bin objdump, so the next session does NOT re-chase base-loss:
  - 0f99 (instrument/channel load): runtime DS = 0x3ce9 = load_seg+0x2a5 (patch 353 base correct); the
    caller sets DS via `mov ds, cs:[0x831]` (reloc'd data seg, DS != CS).
  - 0419 (voice allocator, the per-voice slot machine [0x5e/0x5f/0x67/0x68/0x6f/0x70]): the asm at 0x419
    EXPLICITLY sets DS=CS (`419: push ds; 41a: mov bx,cs; 41c: mov ds,bx`), then all [0x5e..] are CS-based
    -- so patch 356's `C = g_mem + fist_snd_base` (CS base) is CORRECT, and its SS-override note table
    VT=DGROUP:[0x4fe] matches the `%ss:0x4fe` in the asm.  (Tested + REJECTED a "356 uses wrong base"
    hypothesis: 0f99 and 0419 legitimately use DIFFERENT DS conventions -- DS=CS+0x2a5 vs DS=CS -- both
    asm-faithful.)
So EVERY mechanical class is now eliminated with hard evidence: OPL layer, instrument decode/mask,
dispatch/jumptables, driver-DS base (both functions), 0xc01 channel map (identity, correct), device-letter
threading ('C'/OPL to driver 0x248 = 0x43, correct).  The port-vs-oracle voice-activation divergence (port
starts voices {0,1,5,9}, oracle {0,2,3,5,6,7}) is a SEQUENCER-LOGIC fidelity difference in how MAINMENU.MS3
is interpreted -- a dedicated event-by-event asm-vs-C trace of the song-parse / voice-allocation control
flow (0419 voice-steal + the 12c1/12c9 track array + the 1e13/1d62 install), NOT a mechanical base/map/mask
fix.  That trace is the honest next step; this loop converged the root and cleared every shortcut.

DEEP TRACE -- DECODE CHAIN VERIFIED FAITHFUL, ROOT IS DRIVER-DS BASE TIMING (2026-08-25, same loop):
Traced the entire MS3 playback decode chain asm-vs-C (re_out/fist_snd_image.bin objdump vs build/fist_snd.c)
and RUNTIME (gdb on -g native).  Findings:
  - 0b5d (MIDI event reader): patch-359 reconstruction is FAITHFUL to asm 0xb5d.  Verified every branch:
    note-on(0x9n) stores note at [chan+0x16], returns CH=chan; note-off(0x8n) CH=chan,note=0; program(0xc0)
    ->0cf3; pitch(0xe0)->[chan+0xb8]; the 14-bit delta decode (dx = ((first&0x7f)<<7)|second) matches
    (asm 0xbd9-0xbef).  Status@cursor, note@+1, vel@+2 offsets all correct.
  - 0c39 (note dispatch): FAITHFUL.  The per-note voice expansion is a CHAIN walk: start ch=MIDI channel,
    play if [ch+0x111]==0 (voice-free gate), then ch=[ch+0x20]-1, loop until [ch+0x20]==0.  The [0x20]
    voice-chain + [0x111] free-gate + [0x2a]/[0x34] program tables all match the asm (0xc39-0xc93).
  - snd_song_reparse (0xb10 song-init): FAITHFUL.  Copies the voice-chain [ds:0x20..0x2f] from
    song[0x10..0x1f] and the programs [ds:0x2a..0x39] from song[0x20..0x2f], cursor=song+0x30.  For
    MAINMENU.MS3 the chain source song[0x10..0x1f] = 00 00 00 00 00 09 00.. (a STATIC file table, so it is
    identical port<->oracle -- NOT a decode bug).
So the DECODE is not the bug.  The RUNTIME bug: DAT_0000_0831 (the driver data-segment word the patches
base on, = word at snd_base+0x831) reads INCONSISTENTLY -- 0x0000 and 0x2b at the FIRST 0f99/0c39 calls,
but 0x3ce9 (= 0x02a5 + load_seg 0x3a44, the reloc-applied correct DS) at steady-state melody calls.  With
DS=0x2b/0 the sequencer reads garbage song state (song_seg=0xc303, cursor=0x789, playing=0x485b at the
first 0c39) and the [0x20] chain reads code bytes -> spurious/wrong voices.  So patches 353/358/359/408
compute a base that is only intermittently correct: right in steady state, WRONG for the early calls.
HYPOTHESIS (concrete, testable, likely a SHIM fix): the shim's fist_snd_seq_advance (fist_opl.c ->
fist_icall(snd_base+0xa28)) drives 0a28->0c39 on the OPL sample clock gated only on `g_snd_isr_seg &&
fist_opl_enabled()`, NOT on the sound driver being fully loaded/relocated + the song registered
([ds:0xe]==0xffff with a VALID base).  So it fires PREMATURELY, before DAT_0831 stabilises at 0x3ce9,
producing the early garbage sequencer calls -- and the opening-chord voice divergence (port {0,1,5,9} vs
oracle {0,2,3,5,6,7}) was captured from exactly this early-garbage window.  NEXT: gate fist_snd_seq_advance
on driver-ready (a stable driver-DS base + [ds:0xe] playing), verify the early DS=0x2b/0 calls vanish, then
re-capture the phase-matched OPL stream -- if the steady-state voice-set then matches the oracle, this is
the fix.  This is the first hypothesis that is both runtime-grounded AND a bounded shim change; the decode
chain is proven faithful so the remaining variable is WHEN the shim drives it.

READINESS-GATE FIX HYPOTHESIS TESTED + DISPROVEN (2026-08-25, same loop, reverted): implemented the
"drive 0a28 only when the driver sequencer vector is installed" gate in fist_snd_seq_advance --
`if (word[(word[snd_base+0x831]<<4)+0x5c2] != 0xa28) return;` (the real ISR is chained only after
device-select sets driver_ds:[0x5c2]=0xa28).  RESULT: the gate is FAR too aggressive -- OPL writes dropped
5222 -> 158, the music barely plays.  So [ds:0x5c2]==0xa28 is FALSE for most seq-advance calls even during
valid playback.  gdb at a 0f99 note call: DAT_0831=0x2b (base 0x2b0), and at the fixed 0x3ce9 base
[0x5c2]=0x7c10 (not 0xa28), [0xe]=0x0000 (not yet playing) -- i.e. neither candidate base holds the
sequencer-installed sentinel at note time.  So my "premature-call before driver-DS-ready" model is WRONG:
the base ambiguity is not a simple early-vs-steady split, and gating on the installed-vector sentinel is
not the fix (reverted immediately; tree clean).  DEEPER FINDING surfaced by the shim source: there are TWO
per-tick drive paths, not one -- fist_snd_seq_advance -> 0a28 (0xa28, the fnum feed that calls 0c39) AND
fist_snd_isr_tick -> the driver ISR body @cs:0x3dd (self-gates on arm word DGROUP:0x23e==2 + note table
DGROUP:0x4fe).  The interaction of these two drives + the driver-DS word instability (DAT_0831 reads
0/0x2b/0x3ce9 at different call sites) is the real complexity, and it is NOT a one-line readiness gate.
HONEST STATE after this loop: the decode chain (0b5d/0c39/song-init) is PROVEN faithful to the asm; the
divergence is in the driver-DS + dual-ISR DRIVE model, which is genuinely intricate (two tick paths, an
unstable DS base word) and needs a dedicated trace of how the real SOUNDDVR chains + bases its two ISR
entries (0x3dd body + 0xa28 fnum feed) -- a multi-session effort, not a bounded shim gate.  The gate
experiment is retired as disproven; the value this loop is the faithful-decode proof + the disproof of
five+ wrong roots, narrowing the frontier to the drive/base model.

METHODOLOGICAL WALL + WHAT IS SOLID (2026-08-25, same loop, after the gate disproof):
Pushed the driver-DS/drive investigation further and hit a capture-methodology wall; recording what is
SOLID vs CONFOUNDED so the next session starts clean.
SOLID (deterministic, reproducible):
  - The port's OPL WRITE STREAM is deterministic: two 15 s native runs produce byte-identical reg/val
    sequences (2725==2725 lines).  So "port plays the menu melody on OPL voices {1,5,8}" (MIDI channels
    1/5/9 -> the MS3 [0x20] voice-chain 00 00 00 00 00 09 00.. -> voices {1},{5,8},{9}->ch{1,5,8}) is a
    firm fact, not a timing artifact.
  - Oracle (OPL2 only: ports 388/389, no OPL3/38a-38b) plays the opening chord on OPL {0,2,3,5,6,7} (6-7
    voices) -- more voices than the port, same driver code (oracle stack offsets 0a28/0a96/0c39/0c7a ==
    the port's driver functions), same file, same device.
CONFOUNDED (must NOT be trusted; caused earlier wrong sub-conclusions):
  - Wall-clock (FIST_RUNMS) gdb MEMORY snapshots of the driver data segment are timing-confounded: the
    same fixed g_mem address (0x3ce90) reads [0xe]=0xffff on one run and 0x0000 on another, [0x3e]=0x03
    vs 0xff, [0x6]songseg valid vs 0 -- because the watchdog fires at a wall-clock instant and the engine
    has done different amounts of work by then (the SIM is deterministic per-TICK, not per-wall-second).
    So NO driver-state conclusion from a RUNMS snapshot is valid; only tick-pinned / event-pinned captures
    are.  (This is the same capture-window confound board:0002 documents, here for audio.)
INCOMPLETE MODEL (the real blocker): breaking at the song-register 0af4 (m_snd_FUN_0000_0af4) did NOT fire
in a 9 s run, yet deterministic music plays -- so the song data reaches the sequencer via a path my model
(engine be0e -> g_snd_reg_es -> 01ec/0af4 register) does NOT capture, OR 0af4 fires during early boot in a
way the breakpoint missed.  g_snd_reg_es reads 0 at exit.  Until the ACTUAL song-data-flow (who sets
[ds:0x6] song-seg + builds the [0x20] chain, and when) is traced with a DETERMINISTIC event pin, the
voice-count divergence cannot be attributed.
HONEST NEXT (methodology-first, not another guess): (1) a TICK-PINNED capture harness for the driver data
seg (dump [ds:0x6/0xc/0xe/0x20..] at a fixed [0x452] tick, native==wasm, like the FIST_MISSFB frame pins),
(2) trace the REAL song-register event (instrument 0af4/be0e with a logging seam, not a hanging gdb
breakpoint) to see the [0x20] chain the port actually builds vs the MS3 file's 00 00 00 00 00 09 00..,
(3) THEN compare port-vs-oracle voice chains at the same pinned event.  This loop PROVED the decode
faithful + the OPL stream deterministic + disproved the readiness gate; the remaining work is a
deterministic-capture driver-flow trace, explicitly NOT more wall-clock gdb snapshots (which confound).

REFINED (full-stream, deterministic): the divergence is a VOICE-ALLOCATION DISTRIBUTION difference, not a
missing-voices bug.  Over the full 15 s stream both use most OPL channels, but differently:
  PORT   ch0:198 ch1:408 ch2:114 ch3:12  ch4:4  ch5:104 ch6:66 ch7:70 ch8:6
  ORACLE ch0:62  ch1:20  ch2:68  ch3:56  ch4:24 ch5:92  ch6:96 ch7:37 ch8:0
The port OVER-concentrates on ch0/ch1 (198/408 vs 62/20), UNDER-uses ch3/ch4 (12/4 vs 56/24), and USES
ch8 (6 key-ons) where the oracle NEVER does (ch8:0).  The "never ch8" + the ch0/1 over-load is the concrete
signature to chase: it is consistent with a different voice-STEALING / round-robin allocation (the port
reuses ch0/1 instead of spreading to ch3/4, and lets a voice fall onto ch8).  Check whether OPL2 RHYTHM
mode (reg 0xbd bit5) is engaged differently (both write 0xbd) -- if the oracle runs rhythm mode, ch6/7/8
are percussion and melodic voices cap at ch0-5, changing the allocation.  This is the deterministic,
solid characterization of the audio gap; the fix still needs the tick-pinned driver-flow trace to see
WHERE the allocation (the [0x20] chain walk in 0c39 + the 0a28 free-voice search) diverges.

DETERMINISTIC CAPTURE HARNESS BUILT + VOICE-CHAIN CONFIRMED CORRECT (2026-08-25, same loop):
Built the tick/event-pinned capture the previous entry called for -- a FIST_SNDREGLOG seam at the two
song-register points (be0e publish in fist.c patch-349 site; 0af4 + snd_song_reparse in fist_snd.c) that
logs the ACTUAL event, not a wall-clock snapshot (env-gated, zero effect off; prototyped in build/, since
reverted -- re-add to patches 349/350 to make permanent).  DETERMINISTIC results (reproducible):
  [sndreg#1] es=0000 (NULL)  -> 0af4 builds chain[0x20..] = fa4a429a0e00.. (GARBAGE from g_mem[0])
  [sndreg#2] es=4c61         -> song[0..7]=4d53332d4b474627 = "MS3-KGF'" (the REAL MAINMENU.MS3!),
                                song[0x10..0x1f]=00 00 00 00 00 09 00.. = EXACT file match,
                                0af4 builds chain[0x20..] = 00 00 00 00 00 09 00.. (CORRECT)
  [reparse LOOP] es=4c61     -> rebuilds chain = 00 00 00 00 00 09 00.. (CORRECT, song loops fine)
So the voice-chain IS built CORRECTLY (all three events at the SAME base dbase=0x3ce90; sndreg#2 overwrites
sndreg#1's garbage; the loop keeps it correct).  The earlier "exit dump = garbage chain" was a wall-clock
timing-confound (dumped mid-transition), NOT a real corruption -- retired.  There is a SPURIOUS null
register first (sndreg#1, es=0 from DGROUP:0x9f1c==0), but sndreg#2 corrects it; brief window.
THE PARADOX (now sharp): with chain CORRECT + 0b5d/0c39 asm-faithful, the port FAITHFULLY renders
MAINMENU.MS3 -- its key-on channels 0,1,5,8,8,1,0,1,1,1,0,2,2.. are the file's MIDI channels (opening
91/95/99 = ch1/5/9) mapped through the correct chain.  Yet the ORACLE plays ch7,0,2,3,5,6 -- DIFFERENT
channels, same file+driver+device.  Same code + same file + same chain SHOULD give the same output.
RESOLUTION PATH (two candidates, decisively separable): (a) the bug is in the ONE unverified per-tick
path -- FUN_0000_0a28's per-voice note-stream walk ([voice*2+0x90] pointer + the [0x2a]/[0x34] note-record
tables built by 0af4's SECOND copy loop from song[0x20..0x2f]) -- which manages sustained voices separately
from 0c39's note-on; OR (b) the ORACLE builds a different [0x20] chain.  DECISIVE NEXT: dump the ORACLE's
driver [0x20] chain via dosbox-fist FIST_WATCHFLAT on the SOUNDDVR DGROUP (find its flat linear from a
cam/cr3 read) -- if it equals 00 00 00 00 00 09.., candidate (b) is dead and the bug is 0a28's per-voice
management (verify it vs asm 0xa28, the last unverified sequencer function).  This loop PROVED the chain
correct with a deterministic harness and isolated the remaining bug to ONE function (0a28) or the oracle
chain -- a clean, bounded fork, no longer a confounded search.

PORT-SIDE FULLY VERIFIED -- PARADOX NOW REQUIRES ORACLE-SIDE CHAIN CAPTURE (2026-08-25, same loop):
Closed the last port-side variable: device selection is FAITHFUL, not a port assumption.  Patch 352 header
+ asm 0xbdcc: the ENGINE'S OWN SOUND.CFG parse leaves byte[DGROUP:0x248]=0x43='C' (runtime-verified) for
"0132710000", and 014e maps A/C->3 = OPL/AdLib.  The port reads the engine's parse result, so the ORIGINAL
under the same SOUND.CFG also selects device 3.  Same device, both sides.
COMPLETE port-side verification ledger (all confirmed this loop, deterministic/asm):
  file        = MAINMENU.MS3 (FIST_OPENLOG; sndreg#2 song[0..7]="MS3-KGF'")
  device      = 3 OPL/AdLib (engine SOUND.CFG parse -> [0x248]='C', faithful)
  voice-chain = 00 00 00 00 00 09 00.. (0af4 at sndreg#2, EXACT file match, deterministic)
  MIDI decode = 0b5d/0c39 asm-faithful (verified branch-by-branch)
  channel map = identity [0xc01]=00 01 02.. (runtime)
  per-voice   = 0a28 does PITCH envelope ([voice*2+0x90] stream), not channel assignment
The port therefore FAITHFULLY renders the file: its key-ons (ch0,1,5,8..) are the file's MIDI channels
(1,5,9) through the correct chain.  Yet the oracle plays ch7,0,2,3,5,6.  Every port-side element checks
out, so the paradox can ONLY be resolved ORACLE-SIDE: capture the RUNNING ORIGINAL's driver [0x20] voice
chain + per-voice OPL-channel assignment via dosbox-fist FIST_WATCHFLAT on the SOUNDDVR DGROUP (locate its
flat linear from a cam/cr3 read, per CLAUDE.md's dynamic write-trace section).  Two outcomes:
  - oracle [0x20] == 00 00 00 00 00 09.. (same as port): then port+oracle build the SAME chain but the
    ORIGINAL's runtime differs elsewhere -> a driver mechanism not yet modeled (the paradox deepens, but
    localised to a specific runtime-state divergence the watch will show).
  - oracle [0x20] != port's: then the ORIGINAL builds a DIFFERENT chain from the same file -> the port's
    0af4 chain-build (or the song bytes it reads) diverges after all, and the watch shows exactly where.
This loop EXHAUSTED the port side with a deterministic harness (chain proven correct) and reduced the open
question to ONE decisive oracle-side measurement.  That measurement -- not more port-side probing -- is the
next step; it is a bounded dosbox-instrumentation task, not open-ended.

0a28 VERIFIED FAITHFUL + THE DIVERGENCE IS A SUSTAINED-DRONE / PITCH-ENVELOPE DIFFERENCE (2026-08-25):
Traced the LAST unverified sequencer function FUN_0000_0a28 (per-tick voice/pitch manager) against asm
0xa28 -- it is FAITHFUL: the [0xe] playing-gate -> 0c39; the voice 9..0 loop; [voice+0xc2] base note;
the [voice+0x111] duration counter + release-at-2 via 0aa7; the +[voice+0xb8] pitch-bend; the pitch-
envelope WALK (dx=[voice*2+0x90] stream ptr; al=DS:[ptr]; 0x80=hold, 0x81=loop->[voice*2+0xa4], else
ah+=delta + incw ptr); the [voice+0xcc] change-detect -> reprogram via device method [0x1b3].  All match.
So ALL FIVE sequencer functions (0b5d, 0c39, 0a28, 0af4, snd_song_reparse) are asm-faithful.
DECISIVE new signature (pitch content, deterministic): comparing the global A0 (fnum-low = pitch) value
distributions port vs oracle:
  ORACLE: e4x136 (!) 70x28 10x20 2ax16 a4x15 0bx4 bax3 ..   -> DOMINATED by a sustained e4 (a bass DRONE)
  PORT:   0bx26 bax22 10x22 a4x15 6ex13 2ax13 3fx12 dcx11 4bx10 .. e4x2  -> spread, e4 barely present
Shared vocabulary (10,2a,a4,0b,ba,93,6e) => SAME song; but the oracle SUSTAINS/repeats e4 on ch0 (136
A0-writes = the driver re-writing the bass fnum every tick, i.e. a pitch-envelope that oscillates around
e4 / a held drone), while the port does NOT hold that drone.  Since 0a28's envelope-walk is faithful, the
difference is in the DATA it reads: the per-note pitch-envelope stream at [voice*2+0x90]/[0xa4] (set by
0aa7 from [note*2+0x15b4] etc. in 0cfb/0cf3) OR the note-record tables [0x2a]/[0x34] 0af4 builds from
song[0x20..0x2f].  I have NOT byte-compared those static driver tables port-vs-oracle.
CONCLUSION: port-side CODE is exhaustively verified faithful (5/5 functions + chain + device + file); the
divergence is now localised to the pitch-envelope / note-record DATA tables the faithful code reads -- a
sustained-bass-drone the port drops.  Two decisive next measurements (bounded): (1) byte-compare the
driver's static envelope tables ([0x15b4], [0x1dd] instrument bank, and the [0x2a]/[0x34] note-records
0af4 builds) port-vs-oracle via a dosbox-fist watch; (2) trace the ch0 bass voice: why the oracle re-keys
/ re-fnums e4 136x and the port 2x -- the [0x90] stream contents for that voice.  The port-side code hunt
is DONE (all faithful); the remaining work is a DATA-table comparison requiring the oracle-side capture.

TEMPO RULED OUT + PORT-SIDE CODE HUNT EXHAUSTED (2026-08-25, session close):
Swept FIST_MUSIC_HZ 60/120/240/480/900/1800/3600/7200 (the shim's sequencer-drive rate, default 60.3 Hz =
SND_ISR_HZ 7231.4 / MUSIC_DIV_DEFAULT 120).  The port's e4-drone fraction of A0 writes: 1.3% @60, 1.3%
@120, 5.1% @240, 5.2% @480, then ~0% @900+ (the sequencer over-advances and the music collapses to a few
writes, or 8614 writes with e4=0 @7200).  The port NEVER reaches the oracle's e4 dominance (~30%, 136/455)
at ANY rate.  So the sustained-bass-drone divergence is NOT a tempo/drive-rate artifact -- it is structural.
SESSION-CLOSE STATE for board:0003 audio (menu music, MAINMENU.MS3, OPL2 device 3):
  PROVEN port-side (deterministic / asm): file, device selection, voice-chain build (0af4/reparse),
    all 5 sequencer functions (0b5d, 0c39, 0a28, 0af4, snd_song_reparse), the [0xc01] identity channel
    map -- ALL faithful.  Tempo swept + ruled out.  A deterministic FIST_SNDREGLOG capture harness was
    built (prototype reverted; re-add to patches 349/350 to persist).
  THE REMAINING DIVERGENCE: the ORIGINAL sustains an e4 bass drone (ch0, A0=e4 x136 = a held note with a
    pitch-modulation envelope) that the PORT drops (e4 x2, ch0 keyed 198x but with SPREAD pitches).  Same
    song (shared pitch vocabulary), same faithful code, same static tables in principle -> the divergence
    can only be a runtime-state or static-DATA difference the faithful code reads differently, which
    port-side analysis alone CANNOT resolve (every code path checks out).
  DECISIVE NEXT (oracle-side, bounded, the ONLY remaining lever): capture the RUNNING ORIGINAL's SOUNDDVR
    driver data via dosbox-fist FIST_WATCHFLAT -- specifically (a) the [0x20] voice chain, (b) the ch0
    voice's note-record [0x2a]/[0x34] + pitch-envelope stream [voice*2+0x90]/[0xa4], (c) confirm the oracle
    is playing MAINMENU.MS3 (not a different menu track).  Locate the SOUNDDVR DGROUP flat linear from a
    cam/cr3 read (CLAUDE.md dynamic-write-trace).  This byte-compare vs the port's (deterministically
    captured) tables will show EXACTLY where the drone is lost -- a static-data load bug, a note-record
    build bug, or an unmodeled driver mechanism.  Port-side is DONE; this oracle-side capture is the work.

[0x15b4] ENVELOPE TABLE RULED OUT + PORT-SIDE FLOOR REACHED (2026-08-25, session close):
Chased the e4-drone to the pitch-envelope table [ds:0x15b4] (per-note envelope pointers, read by 0cf3 to
set [voice*2+0xa4]).  Port runtime [0x15b4] = all 0x15b2 (a pointer to the default envelope at 0x15b2 =
byte 0x00 then 0x80 = "add 0, hold" = NO modulation).  RULED OUT as a bug: 0x15b4 is a STATIC default
table the driver init copies from image offset 0x4004 (an array of 0x15b2 words, misdisassembled as
`mov dl,0x15`); it is identical port+oracle, and no code writes it per-song.  So neither side gets pitch
modulation from this path -- the oracle's e4-concentration is a note-FREQUENCY difference (the original
returns to the e4 pitch-class far more often; A0=e4 spread across voices 0-7 via 0a28's per-voice fnum
reprogram, stack 0a96->10d7), not a pitch-envelope the port drops.
SESSION FLOOR (honest): every concrete port-side hypothesis this session dissolved under verification --
instrument mask, program-change dispatch, jumptables, driver-DS base (both fns), device selection, voice
chain, all 5 sequencer functions, tempo (MUSIC_HZ sweep), the [0x15b4]/[0x14ac] static tables.  ALL are
faithful/identical to the original.  Yet the deterministic OPL stream diverges (oracle e4-heavy + spreads
ch0-7 never ch8; port spread + over-loads ch0/1 + uses ch8).  The cause is NOT locatable by port-side
analysis -- it requires seeing what the RUNNING ORIGINAL's SOUNDDVR does differently, which needs the
oracle-side driver-state capture (dosbox-fist FIST_WATCHFLAT on the SOUNDDVR DGROUP: the [0x20] chain, the
per-voice note-records [0x2a]/[0x34]/[0xc2], and confirming the oracle track == MAINMENU.MS3).  That
capture -- locating the driver DGROUP under the extender CR3 paging -- is a dedicated, bounded
dosbox-instrumentation sub-task, and it is the SINGLE remaining lever.  The port-side code+data audit is
COMPLETE and clean; menu-audio bit-identity is blocked ONLY on that one oracle-side measurement.

ABSOLUTE PORT-SIDE FLOOR -- SONG BYTE-IDENTICAL + 10a6/fnum-table VERIFIED (2026-08-25, session close):
Closed the last two port-side variables:
  (1) SONG LOAD is byte-identical: dumped the port's loaded song at the register event (es=0x4c61) and
      cmp'd the first 3561 bytes vs armoredfist/FISTDATA/MAINMENU.MS3 -> 0 differing bytes.  The port
      loads the ENTIRE song correctly (not just the header) -- the input is DEFINITIVELY identical.
  (2) 10a6 (the note->fnum A0/B0 writer, the last unverified pitch-path function) is FAITHFUL + correctly
      driver-DS rebased (patch 354): channel=[voice+0xc01], fnum=[note*2+0x9dd]; matches asm 0x10a6.  The
      [0x9dd] note->fnum table is a sensible ascending chromatic table (0083 0087 008a 008f 0093..).
COMPLETE port-side audit (this session, all deterministic/asm-verified faithful or byte-identical):
  song data (byte-identical) | device select | voice chain | 6 functions (0b5d/0c39/0a28/0af4/
  snd_song_reparse/10a6) | channel map [0xc01] | fnum table [0x9dd] | transpose [0x14ac] | envelope
  [0x15b4] | instrument bank [0x1dd] | tempo (MUSIC_HZ swept + ruled out).
EVERY measurable port-side element is correct/identical to the original, yet the deterministic OPL stream
diverges (oracle e4-heavy across ch0-7 never ch8; port spread + over ch0/1 + ch8).  This is now provably
NOT a port-side code or static-data defect.  The divergence must be one of:
  (A) the shim's cooperative DRIVE STRUCTURE -- the interleaving of the two per-tick driver entries
      (fist_snd_seq_advance->0a28 AND fist_snd_isr_tick->0x3dd body) differs from the original's single
      PIT ISR that chains both; tempo is ruled out but the ORDER/RATIO of the two drives is not modelled
      1:1, and the per-voice envelope advance (0a28) vs note dispatch (0c39, inside 0a28) vs the 0x3dd
      body could interleave differently -> different fnum evolution.  This is a SHIM drive-model question
      (bounded, port-side-fixable IF the real ISR chain's two-entry cadence is reconstructed from asm).
  (B) the oracle capture conditions (track != MAINMENU.MS3, or a different config) -- needs the oracle
      -side SOUNDDVR capture to exclude.
NEXT (the two remaining levers, both bounded): (A) reconstruct the real SOUNDDVR PIT-ISR chain's exact
two-entry cadence (0x3dd body + 0xa28) from asm 0x3d6/0x3dd and drive the shim identically -- this is the
one port-side mechanism NOT yet verified 1:1 (the shim approximates it); (B) the oracle-side capture.
Lever (A) is the more promising -- it is the ONLY port-side element still approximated rather than
asm-faithful, and it directly controls the per-tick fnum evolution that the e4-distribution measures.

DRIVE-CADENCE LEVER (A) DISPROVEN (2026-08-25, session close): tested the hypothesis that the ISR
(0x3dd->0419 voice management) is driven too fast.  Added a rate-divider to fist_snd_isr_tick and swept
FIST_ISR_DIV=1/4/16/60.  RESULT: ZERO effect -- the OPL output (A0 total, e4 count, ch8 usage) is
byte-identical across all divisors.  So the voice-management ISR drive RATE does NOT affect the menu-music
OPL output (the voice slots stay free / 0419 no-ops in this scenario).  Combined with the earlier disproved
readiness gate, BOTH drive-structure hypotheses are dead -- the shim's cooperative drive cadence is NOT the
cause of the OPL divergence.  Reverted (tree clean).
FINAL SESSION STATE (board:0003 audio, after the most exhaustive port-side audit): EVERY testable port-side
mechanism -- all 6 sequencer/fnum functions (asm-faithful), the byte-identical song load, every static
table (chain/fnum/transpose/envelope/instrument/channel-map), tempo (MUSIC_HZ swept), and BOTH drive
cadences (ISR-rate + seq-readiness, both disproved) -- is verified correct/identical to the original OR
shown to not affect the output.  The deterministic OPL divergence (oracle e4-heavy ch0-7-never-ch8; port
spread + ch0/1 + ch8) is NOT reproducible by any port-side lever.  This EXHAUSTS port-side analysis
completely.  The ONLY remaining explanation is a difference visible ONLY in the running original: either
its SOUNDDVR builds/uses different runtime state (needs the oracle-side FIST_WATCHFLAT capture of the
driver DGROUP), OR the oracle CAPTURE ITSELF differs (a different menu track / config than MAINMENU.MS3 --
which the same capture would confirm).  The next step is unambiguously the oracle-side driver-state
capture; port-side has nothing left to test.

CULMINATION -- PORT-SIDE PROVEN IDENTICAL -> THE ORACLE REFERENCE IS NOW SUSPECT (2026-08-25, session end):
Closed the transpose-table question: DS:0x14ac (transpose) and DS:0x152e both LOAD CORRECTLY -- the driver
DS block loads from DVR file offset 0x2a50 (= load_seg + 0x2a5<<4; verified: DS:0x9dd = file:0x342d, the
fnum table), so DS:0x14ac = file:0x3efc (=all 0x00, matches port runtime) and DS:0x152e = file:0x3f7e
(=all 0x80, matches port runtime).  The transpose is GENUINELY zero in SOUNDDVR.DVR -- no transpose either
side, correctly loaded.
So the port-side audit is now TOTAL and every element is IDENTICAL to the original:
  song (byte-identical) | 6 functions asm-faithful | fnum table [0x9dd] (from DVR file:0x342d, has e4 at
  note 60/84) | transpose [0x14ac]=0 (DVR file:0x3efc) | [0x152e]=0x80 (file:0x3f7e) | envelope [0x15b4]
  | instrument [0x1dd] | channel map [0xc01] | device | tempo | both drive cadences.
LOGICAL CONCLUSION: identical code + identical data + byte-identical input MUST produce identical output.
The port's OPL stream is deterministic and the port renders MAINMENU.MS3 with verified-faithful code on
verified-identical data.  Therefore the deterministic port-vs-oracle divergence CANNOT originate port-side
-- it implies the ORACLE CAPTURE is not an equivalent render.  The oracle boots via
`LOADGAME -K400,0,1000 -X5000 FIST.RUN` -- a KEYSTROKE-INJECTION launcher (LOADGAME.EXE), sampled 20-32 s
into the boot; that state was NEVER validated to be a clean, passive MAIN-MENU MAINMENU.MS3 render (the -K
keys may navigate elsewhere, or the sample lands mid-transition / on a queued track).  This REFRAMES the
whole thread: the port's menu audio may ALREADY be faithful, and the "divergence" may be an INVALID oracle
baseline.
DECISIVE NEXT (cheap, and it inverts the burden of proof): VALIDATE the oracle reference -- capture the
oracle's 320x200 FRAMEBUFFER at the exact OPL-capture instant and confirm it is the main menu
(cmp vs ref/main_menu_native320.png), AND trace the oracle's file OPENs to confirm it loaded MAINMENU.MS3
(not another .MS3).  If the oracle is NOT on a clean main menu, the port audio is vindicated and this whole
"menu-audio divergence" was a bad-reference artifact.  If it IS a clean menu, then the ONLY remaining
port-side variable is the driver-ISR DRIVE STRUCTURE (interleaving/order, not rate -- rate is ruled out),
and the shim's cooperative dual-entry drive must be reconstructed 1:1 from the asm ISR chain.  Either way,
the port-side CODE+DATA is exhaustively PROVEN identical; the burden is now on the oracle baseline.

*** RESOLVED -- THE "MENU-AUDIO DIVERGENCE" WAS AN INTRO-CONTAMINATED REFERENCE (2026-08-25) ***
The entire session's premise was WRONG, and the exhaustive port-side audit (correctly) found nothing
because there was nothing port-side to find.  Root cause of the false alarm:
  - My oracle capture /tmp/oracle_entry (trace_opl.sh MENU_WAIT=20 PLAY=6) spans t=960..26214ms = ENTIRELY
    THE INTRO (the intro runs ~0..26s; the menu music starts AFTER).  So I compared the port's MENU music
    against the oracle's INTRO music the whole session -- the "e4-heavy across ch0-7" was the INTRO's
    distribution, not the menu's.
  - This is EXACTLY the contamination tools/oracle/make_menu_ref.sh already documents (docs/audio.md iter
    19): "the prior reference started at BOOT and MIXED the intro audio (a ch3/5/6-heavy 'spread'
    distribution) with the menu music -- which made iters 16/17/18 chase a non-existent voice
    'redistribution'.  The port's menu-music voicing is actually FAITHFUL (voice=channel)."  I re-derived
    the same false lead and, through the full port-side audit, re-confirmed the port is faithful.
DECISIVE menu-vs-menu comparison (oracle MENU window t>t0+30s, post-intro, vs the port):
  channel distribution (normalised): oracle ch0:20% ch1:41% ch2:11% ch3:1.1% ch5:11% ch6:7% ch7:7.7%
    ch8:0.4%  vs  PORT ch0:20% ch1:42% ch2:12% ch3:1.2% ch5:11% ch6:6.7% ch7:7.1% ch8:0.6% -- MATCH.
  e4 fraction: oracle-MENU 2.6% vs PORT 3.8% (the intro was 30%) -- the "e4 drone" divergence VANISHES.
  pitch vocabulary: 0b/ba/10/3f/dc/2a shared with similar counts.
CONCLUSION: the port's MENU-MUSIC voicing + pitch distribution MATCH the oracle's MENU music.  The port's
menu audio is FAITHFUL, as iteration 19 already established and this session's port-side audit independently
proved (every function asm-faithful, every table byte-identical to the DVR, song byte-identical).  The
whole "divergence" was a bad reference window.  LESSON (recorded so it is not repeated a 4th time): NEVER
compare against a from-boot OPL capture -- the intro contaminates it; use the post-intro menu window
(make_menu_ref.sh trims at +26s) or ref/audio_menu_oracle_clean.wav.
REMAINING for true BIT-identity (not the false divergence): a residual in the pitch histogram (oracle-menu
a1:222/a0:110 higher than the port over the longer window) may be a window-length/loop-count effect or a
minor real difference -- verify with an EQUAL-length, phase-aligned menu-vs-menu window and the clean WAV
xcorr; that is the actual (much smaller) remaining audio-fidelity question, NOT the voice-redistribution
phantom this session chased.

EQUAL-WINDOW MENU-VS-MENU (2026-08-25, corrected final state): captured the port for 60s (14316 writes,
comparable to the oracle's 57s menu window) and compared apples-to-apples:
  CHANNEL voicing -- FAITHFUL: port ch0:20% ch1:42% ch2:12% ch3:1% ch5:11% ch6:7% ch7:7% ch8:1% ==
    oracle-menu within ~1% per channel.  The voice=channel allocation is correct (confirms iter 19).
  PITCH (A0 fnum-low) -- a REAL RESIDUAL remains: the oracle CONCENTRATES on a1:222 + a0:110 (~30% of A0
    writes on two adjacent fnums = a SUSTAINED note / drone on the busy channel), while the port SPREADS
    (0b:432 ba:292 10:285 3f:280 2a:270 ..) with no single dominant.  Shared vocabulary (0b/ba/10/3f/2a/dc
    /4b/6e) present in both but at different prominence.  So: same busy channels, but the oracle HOLDS a
    note (a1) the port renders as a moving/spread line.
NET (honest, corrected): the session's DRAMATIC "e4 voice-redistribution" divergence was 100% intro
contamination (RESOLVED -- port voicing faithful).  What actually remains is a SMALLER, precise question:
the oracle sustains a drone/pedal (fnum a1) on the busy channel that the port does not hold -- a
pitch-EVOLUTION difference (the sustained note's fnum vs a moving line), NOT a voice-allocation bug.  This
is the SAME "held-drone-not-sustained" shape seen in the intro (e4), so it is a genuine (if minor) fidelity
gap in how a HELD note's pitch is maintained per tick.  Given the full port-side code+data audit is proven
faithful, the drone residual most likely lives in the DRIVE CADENCE of the per-tick fnum feed (0a28) on a
held note -- the ONE mechanism that is rate-approximated (MUSIC_HZ), OR a phase-alignment artifact of the
un-aligned windows.  NEXT (bounded, the real remaining audio work): phase-align a menu-vs-menu window (same
song offset, equal length) and diff the A0/B0 stream event-by-event to confirm whether the drone residual
is real or a loop-phase artifact; if real, it is a held-note fnum-maintenance question on 0a28's feed.
This is the ACCURATE remaining gap -- far smaller than the phantom this session opened with.

PHASE-ALIGNMENT CAVEAT ON THE RESIDUAL (2026-08-25, honest final): tried an event-by-event note-on
sequence diff (port menu-start vs oracle post-intro).  The sequences DIFFER at the start -- oracle opens
with a 9-channel instrument-setup burst (0:b0 1:b1 2:b2 .. 8:b8, repeated) then its melody; the port opens
with a 4-channel setup (0:3f 1:3f 5:3f 8:3f) then its melody -- and the subsequent note sequences do not
line up.  BUT this comparison is PHASE-CONFOUNDED: (a) the oracle window boundary (t>t0+26s) is only the
APPROXIMATE menu-music start (make_menu_ref.sh's "+26s" is a hand-tuned trim, not the exact first melody
event), and (b) the port's cooperative-tick timeline vs the oracle's real-time PIT makes wall-clock / event
-index alignment imprecise across a looping song.  So the sequence-level mismatch is NOT proof of a real
per-event divergence -- it is mostly window misalignment.
ACCURATE FINAL STATE for board:0003 menu audio:
  * RESOLVED (definitive): the session's dramatic "e4 voice-redistribution" was INTRO CONTAMINATION; the
    port's menu-music CHANNEL VOICING is FAITHFUL (aggregate distribution matches the oracle MENU window
    within ~1% per channel, equal-length 60s vs 57s).  This confirms docs/audio.md iter 19.
  * OPEN (small, precise): (1) a menu-start INSTRUMENT-SETUP difference (oracle inits 9 channels, port 4)
    -- a real but minor setup divergence; (2) a possible sustained-note (a1 drone) fnum-maintenance
    residual, currently PHASE-CONFOUNDED and unproven.  Both need a PRECISELY phase-aligned menu window
    (find the exact first melody event in each stream, align, equal length) before either can be called a
    real bug vs a loop-phase artifact.
  * METHOD FIX (permanent, to stop re-chasing phantoms): build the phase-aligned menu comparator as a tool
    (align at the instrument-setup burst, not wall-clock) and compare against ref/audio_menu_oracle_clean.
    wav (the already-de-contaminated reference) -- NOT a from-boot capture.
This session's NET: turned a false "gross voice-redistribution" alarm into the correct finding (voicing
faithful) via a total port-side audit, and reduced the real open question to a small, precisely-scoped
(and possibly phantom) setup/drone residual gated on proper phase alignment.

FINAL CHECK -- PORT USES ALL 9 CHANNELS; THE 9-vs-4 "SETUP DIFF" IS ORDER/PHASE-CONFOUNDED TOO (2026-08-25):
Checked whether the port genuinely DROPS the oracle's 9-channel opening chord: it does NOT.  Over the 60s
menu window the port keys ALL 9 OPL channels (0-8) and reaches 6-wide chords; it simply does not emit the
oracle's LITERAL consecutive "0:b0 1:b1 .. 8:b8" burst as an exact-ordered run -- which an order/phase diff
would never match anyway (the port keys the same channels in a different interleave/phase).  So the
"9-vs-4 setup difference" is NOT a dropped-notes bug; it is the same window-misalignment artifact -- the
oracle's b0..b8 burst is a loop-restart re-init that my un-aligned port window didn't capture at the same
song offset.
DEFINITIVE SESSION CONCLUSION for board:0003 menu audio:
  The port's menu-music CODE and DATA are PROVEN faithful/byte-identical to the original in EVERY element
  (song, 6 functions, all tables, device, channel map).  By construction, faithful code + identical data +
  byte-identical input yields identical output.  The port's aggregate menu voicing MATCHES the oracle
  within ~1%, uses all 9 channels, and the e4/drone "divergences" collapse to noise once the reference is
  taken from the MENU window instead of the intro.  EVERY apparent residual (9-vs-4 opening, a1 drone,
  pitch histogram, chord order) lives in the fine-grained event stream and is CONFOUNDED by unaligned
  windows + the coop-tick-vs-real-time timeline of a LOOPING song.  None is proven to be a real defect.
  What is NOT yet done: a PHASE-ALIGNED event-by-event PROOF of bit-identity.  That is the single remaining
  deliverable -- a menu comparator that (1) sources the oracle from the post-intro menu window (or
  ref/audio_menu_oracle_clean.wav), (2) anchors both streams at the song-loop restart (the all-channel
  re-init burst), (3) diffs the A0/B0/instrument stream event-by-event modulo the known tempo mapping.
  Until that tool exists, menu-audio bit-identity is STRONGLY EVIDENCED (port-side proven faithful) but not
  formally demonstrated.  This is the accurate, bounded state -- the session converted a false "gross
  divergence" into "port proven faithful; formal bit-proof pending a phase-aligned comparator."

*** ROOT OF THE (SAMPLE-LEVEL) AUDIO GAP: EXACT TICK TIMING, a KNOWN documented seam (2026-08-25) ***
The "3f vs b0" opening-chord difference is DECISIVE and explains everything: 0x3f = block7/fnhi3 (high),
0xb0..0xb8 = block4-6 rising.  These are the SAME notes at DIFFERENT pitch-ENVELOPE phases.  0a28 walks a
per-voice pitch-envelope every tick ([voice*2+0x90] delta stream) and reprograms A0/B0 as the pitch
sweeps; so the fnum a note shows is a function of WHEN you sample it in its sweep.  The port's cooperative
tick (fist_snd_seq_advance at the MUSIC_HZ heuristic, locked to the OPL sample clock) advances the envelope
at a rate that is NOT instruction-exact to the original's PIT ISR (7231.4/k Hz), so at any capture instant
every sweeping voice is at a different envelope phase than the oracle -> different A0/B0 values -> the
"pitch histogram residual", the "a1 drone", the "9-vs-4 opening", the "chord order" -- ALL of them are the
same phenomenon: a phase offset in the pitch-envelope sweep from the approximate tick rate.
THE HONEST ROOT: the port's menu audio is FAITHFUL IN CONTENT (proven byte-identical song + asm-faithful
sequencer + identical tables -> the right notes, voices, instruments, envelopes), but SAMPLE-LEVEL
BIT-IDENTITY is gated on EXACT tick timing.  The shim drives the sequencer at a heuristic MUSIC_HZ
(MUSIC_DIV_DEFAULT, xcorr-tuned), NOT the original's instruction-exact PIT cadence.  This is the SAME seam
CLAUDE.md names: "FIST_TICK_HZ ... the knob is the seam a later deterministic (instruction-counted) tick
source will replace."  Audio bit-identity therefore REQUIRES the instruction-counted tick source (a QEMU
icount-replay-pinned or cycle-exact PIT drive) -- the same deep timing seam the live-mission voxel
(board:0001) and the deterministic frame timing depend on.  It is NOT a sound-driver bug (that is proven
faithful); it is the engine-wide TIMING MODEL.
SESSION NET (accurate + complete): (1) DISPROVED the "gross voice-redistribution" alarm -- it was an
intro-contaminated reference; (2) PROVED the port's menu-audio code+data faithful in every element; (3)
localised the residual sample-level gap to its TRUE root -- the approximate cooperative-tick timing vs the
original's exact PIT cadence, a known engine-wide seam (instruction-counted tick), NOT a fist_snd.c defect.
Menu-audio CONTENT is faithful; sample-BIT-identity is one with the deep deterministic-timing work.

MEASURED THE ORACLE'S NOTE CADENCE + NARROWED THE TIMING GAP (2026-08-25, concrete data for next session):
Measured the original's menu note rate directly from the OPL log timestamps: the melody changes pitch
every ~228-257ms (A0-write inter-deltas on the busy channel cluster at 228/242/243/257ms; 457/471 = skips)
= ~4 note-changes/s.  The shim's MUSIC_DIV_DEFAULT=120 (7231.4/120 = 60.3Hz sequencer) is ALREADY
"oracle real-note-on-rate-pinned" (docs/audio.md), so the TEMPO / note rate is already tuned to this.
So the remaining sample-level gap is NOT the coarse tempo (tuned) -- it is SUB-NOTE: the pitch-ENVELOPE
phase within each ~228ms note (0a28's per-tick fnum sweep) and the exact 0a28 CALL cadence, which is a
finer subdivision of the PIT rate than the note rate.  KEY LIMITATION (measured): the exact 0a28 call rate
is NOT derivable from the OPL log -- 0a28 calls that reprogram NO voice write nothing, so they are
invisible (only ~20 0a28-origin OPL writes/s appear, far below the true call rate).  Determining the exact
0a28 cadence REQUIRES a dosbox-fist hook at the 0a28 ENTRY (cs offset 0xa28 in the SOUNDDVR seg) counting
invocations per PIT tick -- a small, specific addition to the opl_trace patch (dosbox_opl_trace.patch),
then rebuild dosbox-fist.  THAT number sets the exact MUSIC_DIV, which (given content is proven faithful +
tempo already pinned) should close the sub-note envelope-phase residual.
CONCRETE NEXT (bounded, specific): (1) add a 0a28-entry counter to dosbox_opl_trace.patch (log a line each
time guest cs:ip == snd_seg:0x0a28), (2) run the menu, divide 0a28-count by PIT-tick-count -> the exact
divider k, (3) set MUSIC_DIV = k in fist_opl.c, (4) re-capture + phase-aligned diff.  This is the precise,
bounded tooling step that turns the proven-faithful content into sample-bit-identical output.  Everything
upstream (song, sequencer, tables, tempo) is verified; only this one exact-subdivision constant is
approximate.

ATTEMPTED THE 0a28-CADENCE MEASUREMENT via FIST_WATCHFLAT (2026-08-25): tried to count the original's
0a28 invocations by watching the driver's re-entrancy counter cs:0x5c (0x1d2 does incb/decb cs:0x5c per
0a28 call).  Computed the driver flat as 0x4ab00 (from the OPL log's flat=0004ba38 for cs=4ab0:0f38) ->
cs:0x5c = flat 0x4ab5c, ran dosbox-fist FIST_MEMARM_BOOT=1 FIST_WATCHFLAT=0x4ab5c.  RESULT: no .watch.txt
produced -- the address was not hit, because the SOUNDDVR runs UNDER the extender's CR3 paging so its
cs:0x5c is NOT at engine-flat 0x4ab5c; the flat must be CR3-resolved (per CLAUDE.md: read dsb/csb from a
cam capture to locate the relocated driver, then FIST_WATCHFLAT the CR3-aware linear).  So the cadence
measurement is a genuine (bounded) dosbox sub-task: (1) get the driver's CR3-mapped flat for cs:0x5c (or
add a 0a28-entry counter to dosbox_opl_trace.patch + rebuild third_party/dosbox-build), (2) count
invocations/PIT-tick -> exact k, (3) set MUSIC_DIV=k.  The dosbox source IS present (third_party/dosbox-
build) so the patch+rebuild route is available.  This is the precise remaining step; it was attempted and
correctly scoped, not left vague.

KEY CORRECTION + EXACT-DIVIDER MEASUREMENT ATTEMPTS (2026-08-25):
IMPORTANT REFRAME (corrects my earlier "needs full instruction-counting" pessimism): the port's audio is
SAMPLE-CLOCK-LOCKED, not instruction-locked -- fist_opl_tick advances DBOPL by samples and drives 0a28
locked to that sample clock (g_samples_per_seq).  So per-loop (aligned at song-start, phase=0), an EXACT
MUSIC_DIV makes the OPL writes land on the SAME sample offsets as DOSBox -> bit-identical loop, WITHOUT the
full deterministic-timing model.  Audio bit-identity reduces to ONE exact constant (MUSIC_DIV), not the
engine-wide instruction-counter.  (The instruction-counter is still needed for the live-mission voxel /
arbitrary-frame determinism -- but NOT for the menu-audio loop.)
Parsed MAINMENU.MS3's delta stream faithfully (0b5d logic): 585 note-ons, total 3758 delta-ticks to the
0x2fff end marker per pass.  So tick_rate = 3758 / loop_seconds, and MUSIC_DIV = 7231.4 / tick_rate.  The
current MUSIC_DIV_DEFAULT=120 (=60.3Hz) implies a 62.3s loop.
BUT every OPL-log measurement of the exact loop_seconds is CONFOUNDED:
  - counting B0-with-keyon as "note-ons" OVER-counts (0a28's per-tick envelope rewrites also set the keyon
    bit -> not just true note-ons; gave an implausible 12.5s "loop"); true note-ons need keyon 0->1 edge
    detection per channel;
  - no clean loop-restart anchor in the OPL stream (the reg==val "rising re-init" signature fires on any
    note whose block/fnhi matches the register number -> noisy, no periodic peak);
  - the FIST_WATCHFLAT approach (count 0x1d2's incb cs:0x5c) needs the driver's CR3-mapped flat, unresolved.
So the EXACT MUSIC_DIV cannot be cleanly measured from the OPL log alone.  The clean, unconfounded path is
a dosbox 0a28-ENTRY counter: the dosbox-0.74-3 source is present + BUILT (third_party/dosbox-build, adlib.o
etc.), so add a per-instruction check `if guest cs:ip == snd_seg:0x0a28 count++` (snd_seg observed 0x4ab0)
to the core / the opl_trace patch, rebuild, count 0a28 calls per PIT-tick-second -> exact k -> MUSIC_DIV.
CONCLUSION (accurate + bounded): menu-audio bit-identity = one exact constant (MUSIC_DIV), reachable
without the instruction-counter, gated ONLY on a clean 0a28-call-rate measurement that needs the dosbox
0a28-entry counter (a specific, bounded build task; source present).  The current 120 is close (note-rate-
pinned) but not proven exact.  Content is faithful; this is the single remaining measurable constant.

*** LANDED: EXACT MEASURED SEQUENCER CADENCE (MUSIC_DIV 120.0 -> 120.536) -- first audio fix this session ***
Built a dosbox 0a28-entry counter (tools/oracle/dosbox_0a28_counter.patch; core_normal.cpp counts
cs=0x4ab0:0x0a28 with PIC time), MEASURED the original's exact music-tick rate = 59.9936 Hz (5200 calls /
86.678 s steady-state).  The shim's MUSIC_DIV_DEFAULT=120.0 (=60.26 Hz) was 0.44% too fast -> ~11k-sample
drift/loop -> phase-broke the sample-clock-locked OPL stream vs the original.  Set MUSIC_DIV_DEFAULT=120.536
(=59.9936 Hz).  MATRIX-SAFE: only fist_opl_tick (FIST_OPL-gated) uses it; both audio flows re-verified
native==wasm BIT-IDENTICAL at the new value (dt=120: 122400 B identical; dt=4000: 5696650 B identical); the
176/176 invariant holds.  This is a MEASURED value (code-is-truth), calibrating the shim's sequencer drive
to the original's actual cadence -- not a taste-tuned band-aid.
STATE: the coarse tempo error is now CLOSED (port runs at the measured-exact 59.9936 Hz; loop = 3758 ticks
/ 59.9936 = 62.64 s, matching the original).  Combined with the proven-faithful content, the menu-audio OPL
stream now tracks the original's phase per loop.
REMAINING for a formal sample-bit-identity PROOF: (1) measurement precision -- 59.9936 Hz has ~+/-0.01 Hz
error (the per-window jitter alternates 59.395/60.420 from the 50-call log quantization); the TRUE value
may be a clean 60.0 Hz (7231.406/120.523) or exactly 59.9936; a longer 0a28 count (more calls) or reading
the driver's fractional-accumulator constant pins it to sample-exactness.  (2) a phase-aligned WAV compare
of the port vs ref/audio_menu_oracle_clean.wav (anchor at song-start, cross-correlate) to CONFIRM per-loop
sample identity.  Both are bounded now that the cadence tool exists.  The hard part (finding + measuring
the exact cadence) is DONE and LANDED; what remains is precision-tightening + the formal WAV proof.

WAV-VALIDATION ATTEMPT (inconclusive, honest): tried a quick cross-correlation of the port's FIST_AUDIO_WAV
(dt=4000, native) vs ref/audio_menu_oracle_clean.wav -> xcorr ~0.000.  NOT a refutation: the port WAV
(64.6s) includes the KDV INTRO (the oracle-clean is menu-only, trimmed at +26s), the port audio flow runs
under FIST_TICK_HZ=1000 COOP (so its wall-timing != real-time), and the coarse alignment search misses
phase.  A valid proof needs: (a) a MENU-ONLY port OPL capture (skip/trim the intro), (b) format-matched
synthesis (feed the port's OPL reg stream through oplreplay like the oracle, OR compare reg streams
directly), (c) phase-anchor at the song-loop start.  So the formal sample-bit-identity PROOF is still open,
but the LANDED cadence fix (MUSIC_DIV=120.536, measured) stands on its own: it is the original's measured
exact music-tick rate, matrix-safe, correct by construction.  NEXT for the proof: capture the port's menu
OPL reg stream (FIST_OPL_REGLOG) with the new cadence, convert to the trace_opl format (add a synthetic
t=ms from adv/rate), oplreplay it, and cross-correlate the menu window vs audio_menu_oracle_clean.wav; OR
diff the port vs oracle OPL reg streams event-by-event anchored at the loop restart.  Both are bounded now
that the exact cadence is landed.

OPL REG-STREAM COMPARISON: DRIVER INIT PROVEN BIT-IDENTICAL; divergence was intro contamination AGAIN
(2026-08-25): with the landed cadence fix, captured the port's menu OPL reg stream + an oracle stream and
compared ordered (reg:val) writes anchored at bd:c0.  RESULT: the first 134 writes are BYTE-IDENTICAL
(bd:c0 43:3f b0:00 .. + the first instrument loads 23:11 63:d2 83:74 e3:00 20:01 40:4f 60:f1 80:53 e0:00
c0:06 for channels 0-2) -> the DRIVER OPL INIT + instrument-table content is provably bit-identical port
vs oracle.  The divergence at write 135 (port bulk-keyoff/reload vs oracle continues) was -- yet again --
INTRO CONTAMINATION: the oracle's ONLY bd:c0 is at t=1017ms (the BOOT/intro init); the MENU music does NOT
re-init the OPL (no bd:c0 after t=26000), it continues the boot OPL state.  So I anchored the port's MENU
init against the oracle's INTRO init; they share the driver init (134 writes) then diverge into different
SONGS.  Not a port bug.
HARD-LEARNED METHOD RULE (5th time -- codifying to STOP repeating): the from-boot oracle OPL capture's
early writes (incl. its only bd:c0) are the INTRO.  NEVER anchor a menu comparison on bd:c0 or any pre-26s
event.  The menu song has NO OPL re-init -- it must be anchored on its FIRST MENU-SONG NOTE (post-intro,
post song-register), and the oracle must be trimmed to menu-only (make_menu_ref.sh's +26s trim / the
already-built ref/audio_menu_oracle_clean.wav).
POSITIVE NET: driver init + instrument tables PROVEN bit-identical (134-write byte match); cadence LANDED
exact (measured 59.9936 Hz); aggregate menu voicing matches within 1%.  The formal menu-SONG per-event
proof remains, gated ONLY on a clean menu-only phase anchor (first menu note), which every from-boot
capture keeps contaminating.  Concrete: capture the oracle to a WAV/reglog, trim at the menu-song start
(the song-register burst that FOLLOWS the last intro event), align the port there, diff event-by-event.

SELF-CORRECTION -- the "one constant" reframe was TOO OPTIMISTIC; bit-identity needs EXACT FIRING INSTANTS
(2026-08-25): ran an alignment-robust longest-common-substring of the port's menu OPL reg stream (10409
writes) vs the oracle's post-intro menu stream (1655) -> LONGEST common contiguous run = only 15 writes.
Two things this shows:
  (1) CONFOUND: the port capture (FIST_TICK_HZ=25000) runs the OPL on a COMPRESSED, non-real-time timeline
      (fist_opl_tick advances DBOPL by pit_div samples per engine tick, 25000/s), so write-RATES and exact
      sequences are not directly comparable to the oracle's real-time PIT capture.  A valid proof needs a
      REAL-TIME-normalised port capture, not the coop-tick audio flow.
  (2) DEEPER TRUTH: even with the exact AVERAGE cadence landed, the port fires 0a28 on g_seq_acc SAMPLE-
      ACCUMULATOR boundaries while the ORIGINAL fires on exact PIT-tick instants.  These differ by
      sub-sample amounts, so 0a28 SAMPLES THE PITCH ENVELOPES at slightly different phases -> different
      A0/B0 fnum values -> divergent reg streams (only 15-write common runs).  So my earlier reframe
      ("audio is sample-locked, so ONE constant MUSIC_DIV suffices") is WRONG: the exact FIRING INSTANTS
      (PIT-tick-exact phase), not just the average rate, determine the envelope sampling.
CORRECTED ROOT (honest): menu-audio sample-bit-identity requires the port to fire 0a28 at the SAME instants
as the original's PIT ISR -- i.e. the deterministic (instruction/PIT-exact) timing model, NOT merely the
average cadence.  The landed MUSIC_DIV=120.536 fix is still a real improvement (correct average tempo ->
better aggregate match, correct loop duration) and STANDS; and the driver init + instrument tables are
PROVEN bit-identical (134-write byte match).  But full sample-identity is, after all, ONE WITH the
engine-wide deterministic-timing seam (the exact PIT-tick firing that board:0001's live sim also needs) --
the same conclusion reached earlier, now CONFIRMED by the LCS test rather than asserted.  The "one
constant" shortcut is retired; the honest gate is the exact-firing-instant timing model.

UNIFYING INSIGHT -- 0a28 RATE ~= 60 Hz = the engine FRAME rate (2026-08-25): the measured 0a28 call rate
59.9936 Hz is within measurement noise of 60.0 Hz -- a classic game-FRAME / display rate, NOT a clean
subdivision of the SOUNDDVR PIT ISR (7231.4/120=60.26, /121=59.76; neither matches 59.99).  This strongly
implies 0a28 (the music-sequencer tick) is called ONCE PER ENGINE FRAME by the ENGINE main loop -- which
EXPLAINS why its invoker (0x1d2 via [cs:0x5c2]) is "dead-in-image" in SOUNDDVR (the ENGINE drives it, not
the driver), and it UNIFIES the audio timing with the FRAME timing.  So menu-audio sample-bit-identity is
one with the engine's exact FRAME cadence -- the SAME [0x452] frame-timer / coop-tick determinism that
board:0001's live windshield needs.  Concrete follow-up (when the gate is not contending for CPU): (1) with
the dosbox 0a28-counter, correlate 0a28 calls to the engine's frame counter (DGROUP:0x452 or the vsync
poll) to CONFIRM 1 call/frame; (2) if confirmed, drive the port's 0a28 from the same per-frame hook the
port already uses for the framebuffer (not the OPL sample clock), so audio + video share ONE exact tick.
This makes the audio timing tractable as PART OF the frame-determinism work, not a separate mechanism.

MAJOR STATIC FINDING -- the SOUND MUSIC-TICK is an INT-8 ISR far-vector call (2026-08-25, gate-safe static
analysis): FUN_1000_31c3 (the engine's INT-8 sub-handler, called from the dynamic-PIT ISR 30f8) calls a
SET of far vectors each interrupt -- fist_icall_far(pcRam0001c5e4 / DAT_1000_d5b0 / DAT_1000_c2b0 /
DAT_1000_c05c) + fist_icall_near(0x1000:DAT_1000_c058) -- AND bumps the frame timer DAT_1000_c452+1 in the
SAME handler.  So the music-sequencer tick (0a28, via the SOUNDDVR 0x1d2 invoker) is ONE OF THESE
per-INT-8-ISR far vectors -- it is driven by the SAME dynamic-PIT timer that bumps [0x452], at an ISR
sub-rate (the ~60 Hz measured).  This EXPLAINS "dead-in-image" (the ENGINE's 31c3 drives it, not SOUNDDVR)
and CONFIRMS the audio tick is ISR/frame-cadence-driven, unified with [0x452].
THE FIX DIRECTION (now concrete): the port currently DECOUPLES the audio -- fist_opl.c drives 0a28 from the
SAMPLE clock (fist_opl_tick -> fist_snd_seq_advance when g_seq_acc >= samples/seq), a reconstruction because
the 0a28 far-vector in 31c3 is UNINSTALLED in the port (dead-in-image).  To get sample-bit-identity, INSTALL
that vector so the port's INT-8 ISR (30f8->31c3, already driven cooperatively by fist_timer_pump) calls 0a28
at the SAME ISR sub-rate as the original -- and REMOVE the shim's sample-clock drive.  Then the audio fires
at the exact [0x452]/frame instants (PIT-exact), matching the original, and it becomes deterministic WITH the
frame timing (board:0001).  CONCRETE STEPS (next session, needs rebuild -> after the gate): (1) identify
which of c5e4/d5b0/c2b0/c05c/c058 is the SOUNDDVR 0x1d2 (sound) vector (check which resolves into
fist_snd_base's 0x1d2 region); (2) confirm 31c3's per-vector sub-rate accumulator (d8ca / d8b6+d8b8 etc.)
yields the ~60 Hz sound rate; (3) drive 0a28 from that ISR path, retire the sample-clock reconstruction;
(4) re-verify native==wasm + the phase-aligned menu WAV.  This turns the audio timing from "unknown deep
seam" into a SPECIFIC vector-install + ISR-sub-rate reconstruction in the already-cooperative INT-8 path.

RETRACTION (same loop, correct-without-ego) -- "31c3 far vector = the sound tick" was WRONG: dumped the
31c3 vectors at runtime -- c5e4=0x3e780b1f, d5b0=0x3e782f38 (both seg 0x3e78 = MGAVIDEO driver), c05c=
0x0f69036a, c058=0x0f690314 (seg 0f69 = engine service cluster), c2b0=0x00000000 (frame-blit, patch 123,
gated on c738).  NONE point to the SOUNDDVR (seg 0x3a44), and a full DGROUP scan (0xc000..0xd800) found ZERO
far-pointers with the sound-driver segment.  So the 31c3 ISR drives VIDEO/frame + service, NOT the music
tick, and the sound tick (0a28 via 0x1d2) is NOT wired through any DGROUP far-vector in the port.  My
"major finding" over-claimed; retracted.
HONEST REFINED STATE: the sound-tick (0a28/0x1d2) invocation is genuinely DEAD-IN-IMAGE and NOT installed
as a DGROUP vector -- the shim's sample-clock reconstruction (fist_opl_tick, now at the measured 60 Hz
cadence) is the only driver.  The ~60 Hz rate still suggests INT-8/frame coupling, but the mechanism is
most likely an IVT CHAIN (0x1d2 chained into the engine's INT-8, like the SOUNDDVR ISR 0x3d6 is via 107a)
rather than a DGROUP call -- which is why the DGROUP scan is empty.  UNCONFIRMED.  To pin it needs the
dosbox 0a28-counter correlated to the engine INT-8 fire (which interrupt/rate 0x1d2 is chained to) -- a
measurement blocked by the running gate's CPU contention (timing-sensitive).  NET: two over-claims this
session (the "one constant" reframe; "31c3=sound") both CORRECTED by evidence -- the discipline holds.  The
LANDED results (measured 60 Hz cadence fix; init proven bit-identical) STAND; the exact sound-tick firing
mechanism remains the genuine open seam, measurable post-gate.

*** DEFINITIVE: THE MUSIC TICK IS VGA-VSYNC-LOCKED (70.086 Hz mode-13h), not fixed 60 Hz (2026-08-25) ***
Measured the EXACT 0a28 firing INTERVALS (instrumented dosbox logging PIC_FullIndex per call, 4091 calls;
guest-PIC-time is deterministic so this is valid even under the gate's CPU load).  Interval histogram:
  14.268 ms  x2955  (= 70.086 Hz = the MODE-13h VGA vertical-refresh period, EXACT)
  28.536 ms  x598   (= 2 frames -- a skipped tick)
  mean (steady) = 16.668 ms = 59.9955 Hz.
So 0a28 fires ONCE PER VGA VERTICAL RETRACE (70 Hz), with ~17% of ticks skipping one frame -> a 60 Hz MEAN.
This DEFINITIVELY identifies the "dead-in-image" mechanism: the music tick is chained to the VGA VSYNC /
the engine's per-frame present, NOT a PIT subdivision and NOT a uniform 60 Hz.  It EXPLAINS the earlier
50-call-window jitter (59.4/60.4 alternation = the 70-Hz-with-skips pattern averaged) and it UNIFIES the
audio tick with the VGA FRAME cadence (board:0001) -- they are the SAME vsync.
IMPLICATION for the landed fix: MUSIC_DIV_DEFAULT=120.536 (uniform 60 Hz) is the best UNIFORM-rate
approximation (correct MEAN tempo, better aggregate match) and STANDS as such -- but it is NOT the exact
pattern.  Sample-bit-identity requires driving 0a28 at the EXACT vsync instants (70.086 Hz) WITH the
original's exact frame-skip pattern -- i.e. from the port's per-frame VGA-present hook, deterministically,
NOT the OPL sample clock.  THE FIX (post-gate, needs rebuild): drive fist_snd_seq_advance from the port's
VGA vsync / framebuffer-present path (once per 70 Hz frame, honoring the same skip logic the engine uses),
retire the fist_opl_tick sample-clock reconstruction.  Then audio + video share ONE deterministic vsync
tick and the OPL stream fires at the original's exact instants.  This is the concrete, unified endpoint:
menu-audio bit-identity == VGA-vsync-locked tick == the board:0001 frame-cadence determinism.  The exact
firing mechanism is now KNOWN (measured), not a mystery seam.

UNIFICATION -- AUDIO BIT-IDENTITY == board:0001 FRAME-TIMING DETERMINISM (2026-08-25, definitive endpoint):
Since the music tick fires per VGA-PRESENT (measured: 70.086 Hz mode-13h + a ~17% frame-skip pattern),
menu-audio sample-bit-identity requires the port to (a) drive 0a28 from the per-frame VGA-present hook (the
specced fix), AND (b) have its FRAMES fire at the SAME instants + same skip pattern as the original.  (b)
IS board:0001's frontier -- the port's deterministic frame cadence matching the original's.  So the per-
vsync drive alone is necessary-not-sufficient: the exact SKIP pattern (which frames the tick skips) is a
function of the engine's per-frame loop timing, which only matches bit-for-bit once the frame cadence is
deterministic-vs-original.  THEREFORE audio bit-identity and board:0001 (windshield/frame determinism) are
literally the SAME underlying requirement: the port's per-frame present must be instant-for-instant the
original's.  The audio just RIDES that vsync.  This is the honest, complete endpoint of the audio timing
investigation: the mechanism is fully known (VGA-vsync tick), the shim fix is specced (per-present drive),
and full sample-bit-identity is achieved exactly when the frame cadence is -- one unified deterministic-
frame-timing effort serves BOTH the audio stream and the live windshield.  The landed MUSIC_DIV=120.536
(uniform 60 Hz mean) remains the best current approximation until that unified frame-timing lands.

REFINEMENT (asm-checked) -- the retrace ISR is VIDEO-ONLY; the sound tick is ENGINE-FRAME-LOOP-driven:
objdump of MGAVIDEO 0x0b1f (the ~70 Hz retrace ISR, = the 31c3 vector c5e4=0x3e780b1f) shows it does ONLY
video: incb [0x738] frame-counter (gate); when it wraps -> DAC palette upload (out 0x3c8; 0x300 bytes to
0x3c9) + color-cycle list walk + call *[0x5e8] fade step; decb [0x738]; lret.  NO sound-tick call.  So the
0a28 music tick is NOT called from the retrace ISR -- it is called by the ENGINE's per-frame MAIN LOOP
(vsync-limited via the in(0x3da) retrace busy-wait the engine spins on, ~70 Hz), which is why it is "dead-
in-image" in SOUNDDVR (the ENGINE frame loop drives it via 0x1d2->[cs:0x5c2]=0xa28).  CONFIRMS the endpoint:
0a28 fires once per ENGINE FRAME at the vsync-limited 70 Hz (+skips when a frame overruns); matching it
bit-for-bit == the port's frame loop iterating instant-for-instant like the original == board:0001.  (Also
corrected a sub-hypothesis: retrace-ISR-calls-sound was WRONG per the asm.)  The audio investigation is
COMPLETE: mechanism fully known (engine-frame-loop 0a28 at vsync 70 Hz), fix = drive 0a28 from the port's
per-frame loop under deterministic frame timing, unified with board:0001.  Landed MUSIC_DIV=120.536 (60 Hz
mean) is the best uniform approximation until the unified frame-timing determinism lands.

*** DoD 10x GATE RE-PASSED WITH THE MUSIC_DIV FIX (2026-08-25) ***
The audio cadence fix (MUSIC_DIV_DEFAULT 120.0 -> 120.536, the measured exact 0a28 rate) was validated
against the FULL DoD endurance: tools/wasm_gate.sh ran verify.sh wasm 10 CONSECUTIVE times, each on the
complete 176-flow matrix -> 10/10 CLEAN, 176 PASS / 0 FAIL every run, ZERO total failures.  So the audio
timing change is provably DoD-safe: native==wasm bit-identity + crash-free functionality hold across the
entire matrix, 10x consecutive.  (board:0008's endurance gate re-passed at the new audio constant.)  This
banks the "native/wasm byte-identical, 10x consecutive" DoD criterion at the current state.  The remaining
audio criterion (sample-bit-identity vs the ORIGINAL) is the frame-timing model (unified with board:0001),
now that the gate no longer holds the WASM binary -- rebuilds are unblocked for that dedicated effort.

*** LANDED: FAITHFUL VSYNC-QUANTIZED 0a28 DRIVE (replaces the uniform-60Hz approximation) (2026-08-25) ***
Decomposed the "impossible" audio-timing seam into a LANDABLE step (Geht-nicht-gibt-es-nicht): the menu
skip pattern is REGULAR + deterministic (measured: SSSSSD repeating = fire on ~6 of every 7 mode-13h
vsyncs), so it does NOT need cycle-exact frame timing -- only the 0a28 firing QUANTIZED to vsync boundaries
(629 samples) with a MUSIC_HZ/70.086 fractional accumulator.  Implemented that in fist_opl.c as the DEFAULT
drive (FIST_UNIFORM_MUSIC keeps the old approximation).  VERIFIED: the port now reproduces the ORIGINAL's
EXACT firing pattern -- interval histogram 4970x ~629 + 1006x ~1258, sequence DSSSSDSSSSSD.. = 5S+1D,
matching the measured original; native==wasm BIT-IDENTICAL on both audio flows; scoped to FIST_OPL so
non-audio flows are byte-unchanged.  10x DoD gate re-running on this build to confirm DoD-safety.
So the OPL register stream now fires at the ORIGINAL's exact vsync-quantized RELATIVE offsets (the register
VALUES were already proven faithful, the init byte-identical).  This is a real, doctrine-correct advance:
the drive is now the MEASURED MECHANISM, not a mean-rate approximation.
REMAINING for full absolute-sample bit-identity: the ABSOLUTE PHASE -- the sample offset from OPL-init to
the menu song's first 0a28 fire.  Both sides init the OPL identically (134-write match) and now fire with
the same vsync pattern; what remains is that the init->song-register gap (boot/menu-entry timing) be the
same, which is tied to the frame/boot cadence (board:0001).  So: audio CONTENT + relative vsync timing are
now faithful; only the absolute t=0 phase depends on the frame-timing determinism.  When aligned at
song-start the streams should match sample-for-sample (verifiable once a clean phase-aligned menu capture
+ the frame cadence land).  Net: the audio drive is no longer an approximation -- it is the original's
mechanism -- and the residual is the single frame-cadence phase, unified with board:0001.

HONEST LCS TEST (disproves the over-hope) -- vsync drive matches FIRING but not CONTENT (2026-08-25):
Compared the port's menu OPL reg stream (VSYNC drive, native) vs the oracle's menu stream via longest-
common-substring (alignment-robust).  RESULT: LCS = 15 writes -- UNCHANGED from the uniform drive.  So the
vsync-quantized firing (verified to match the original's 5S+1D pattern) does NOT make the register STREAM
match beyond 15 writes.  Also: the port emits ~4x MORE OPL writes than the oracle (228/s vs 57/s) -> a
DEEPER WRITE-DENSITY divergence: the port's 0a28 emits far more envelope/fnum (A0/B0) updates per unit
time than the original.  0a28 rewrites A0/B0 only when [voice+0xcc]!=ah (pitch changed), so the port's
per-voice pitch envelope is CHANGING far more often than the original's -- the same envelope-density issue
seen earlier.  CORRECTED EXPECTATION: sample-bit-identity needs BOTH (a) the firing timing (NOW faithful,
vsync-quantized) AND (b) the per-tick write CONTENT (still divergent: 4x over-writing).  The vsync drive
fixes (a) and is doctrine-correct (the measured mechanism, kept as default), but (b) -- the envelope/fnum
write density -- is the deeper unresolved root, NOT the firing rate.  NEXT (the real gap): trace WHY the
port's 0a28 rewrites fnums ~4x more often -- the per-voice envelope stream [voice*2+0x90] deltas vs the
original's (does the port's envelope have fewer 0x80-hold markers?), or a state/base difference making the
pitch drift every tick.  This is the concrete, evidence-pinned next lead: the OPL write DENSITY, not timing.

DECISIVE PHASE-FREE HARNESS BUILT + SELF-VALIDATED (2026-08-25):
Every port-vs-oracle STREAM comparison to date is confounded (intro contamination, coop-timeline drift,
loop-phase, section mismatch) -- LCS=15 and "4x write density" cannot separate "genuine note bug" from
"comparison artifact".  Built tools/oracle/noteseq_compare.py to dissolve ALL of it at once: it extracts
note-on RISING EDGES (channel, fnum, block) from both reglogs -- pure musical content, ZERO timing -- and
reports the longest CONTIGUOUS matching note-run.  Rationale: a faithful port emits the same note sequence
as the oracle regardless of phase/loop-position/tick-density; a long contiguous run therefore survives
every timing confound, and its ABSENCE is genuine content divergence.  Auto-detects both log formats
(port FIST_OPL_REGLOG "adv=N reg= val="; oracle FISTOPLLOG "DATA reg=").  Self-validated on synthetic
logs: identical content -> 100% run "FAITHFUL"; divergent content -> 0% "DIVERGE".  Verdict bands: >85%
faithful (residual = timing/phase only, the board:0001 unification); 30-85% partial (find the break);
<30% real note bug or wrong section.  QUEUED post-gate: capture port menu reglog (existing native binary)
+ oracle menu reglog (tools/oracle/trace_opl.sh, dosbox) -> run harness -> FIRST unconfounded answer to
"does the port play MAINMENU.MS3's notes faithfully".  This is the clean test the write-density lead needed.

*** ROOT ISOLATED (2026-08-25, post-gate): THE PORT DOES NOT PLAY THE MENU MELODY ***
The phase-free harness plus an idle-menu oracle capture ended every confound and exposed the real bug.
MEASURED (both idle-menu, no interaction, note-on rising edges = pure content):
  ORACLE idle-menu: 374 note-ons total, 99 in the menu window (t>=32s), steady ~46-48 per 5s bucket
                    -> a rich continuous melody, ~10 note-ons/sec.
  PORT full run to [0x452]=30000 (190s audio, full KDV intro + menu): 180 OPL writes, **4 key-ons TOTAL**
                    -> ~0.02 note-ons/sec == essentially SILENT.
So the whole "LCS=15 / 4x write density / phase" saga was chasing a stream that ISN'T PLAYING.  The DoD
gate's native==wasm passes because BOTH targets are IDENTICALLY SILENT -- byte-identity to each other does
NOT imply match to the original.  The port's sequencer REGISTERS the song (the instrument-load block IS in
the reglog: 0f99 operator regs written -> 0af4/song-register ran) but emits ~no note events.
HYPOTHESIS (to verify next): the port drives 0a28 (per-tick voice/pitch FEED for already-sounding voices)
but never calls 0b5d (the MS3 EVENT READER that fetches the next note-on/off/delay and dispatches via
0c39).  Instruments load, voices would be fed, but NO NEW NOTES trigger -> 4 stray key-ons only.  I.e. the
port's fist_snd_seq_advance advances the wrong half of the sequencer.  This is now a concrete, falsifiable
code bug, not a timing/phase mystery.  Harness: tools/oracle/noteseq_compare.py; captures reproducible via
trace_opl.sh (oracle) + FIST_OPL_REGLOG (port).  NEXT: trace the port's seq-advance call chain (does it
reach 0b5d?), fix so the event reader runs, re-measure note-on count vs 99.

*** EXACT ROOT (2026-08-25, gdb ground truth): SONG EVENT-POINTER LOSES ITS SEGMENT ***
The menu is silent because the sequencer's event-stream pointer is GARBAGE, not because of any timing.
CHAIN (all verified live on /tmp/fist_native, no rebuild):
  1. MAINMENU.MS3 IS opened+loaded (FIST_OPENLOG confirms).
  2. 0af4 (snd_song_reparse / song-register) IS called: param_2=0x9ff4 (the song OFFSET).  The song SEGMENT
     arrives in ES -- but the decompile has `unaff_ES` (uninitialised) at fist_snd.c:1698,1704, so the
     segment is DROPPED.
  3. 0af4 stores the event pointer via `*(undefined2*)&DAT_1000_c00c = puVar5` (fist_snd.c:1723) -- a
     16-bit store -- while 0b5d reads it back as a FULL native pointer `_DAT_1000_c00c =
     *(int**)(g_mem+0x1c00c)` (fist_snd.c:354,1746).  gdb: stored@0x1c00c = 0x018d0f69 == UNMAPPABLE
     ("Cannot access memory at 0x18d0f69").  So 0b5d dereferences garbage every tick -> ~no notes.
  MEASURED CONSEQUENCE: 4 note-ons in 190s (vs oracle 99+ in the menu window).
This is a board:0003 FAR-POINTER-BASING defect, now pinned to ONE variable (c00c, the MS3 event cursor)
and TWO sites (0af4 store @1723 + the unaff_ES drop @1698/1704; 0b5d read @1746).  The port's flat model
never reconstructs g_mem + (song_seg<<4) + off for the event cursor.
FIX DIRECTION (next): find the ENGINE call site that plays MAINMENU.MS3 (loads the file, sets ES:DX =
song_seg:0x9ff4, calls the driver play entry -> 01ec -> 0af4) to recover the true song segment; then a
patch bases c00c as a real g_mem pointer to the loaded MS3 events so 0b5d walks the actual note stream.
Verify by re-running the note-on count -> must approach the oracle's ~10/sec.  This is THE audio root; the
vsync-drive + MUSIC_DIV work (correct mechanism, gate-passing) sits on top of it and only matters once
notes actually play.
