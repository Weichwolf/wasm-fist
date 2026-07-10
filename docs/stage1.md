# Stage 1 — making the engine execute (running notes)

Goal: `app_entry` runs the real game crash-free (native + wasm), reaching FIRST LIGHT (video mode +
resource load + pixels) and then full boot. Every engine correction = an asm-verified `patches/NNN-*.diff`;
the decompile stays pristine; shim files (`re_out/fist_dos.c`, `fist_vga.c`, `tools/native_main.c`) are
hand-written.

## DONE (STAGE 3 — THE BRIEFING MODAL'S ACCEPT/CANCEL BUTTONS ARE RECONSTRUCTED + FUNCTIONAL, AND THE MISSION-START CASCADE FRONTIER IS PINPOINTED: clicking CANCEL in the BATTLES→OK briefing now tears the dialog down and RESTORES THE MENU BIT-IDENTICALLY — AE = 0/64000 vs a genuine, reproducible DOSBox capture on BOTH native AND wasm, native↔wasm 0-diff (single md5 `21ce397f`), DETERMINISTIC (tick-pinned, 5× serial identical). One new verify.sh flow → `bash tools/verify.sh both` = 19/19. 2 new patches (152/153). `re_out/fist.c` pristine `61453e42`; NO regression.)

**THE KEY FINDING — THE ACCEPT CASCADE WAS NEVER WIRED (the handoff's "wire the dispatch tail" is exactly right, and here is where):** the real `FUN_0000_e87a` (@0xe87a) DOES contain the full mission cascade (`e43f`/`4754`/`459a`/`1631`/`e4bb`), but **e714's state machine does NOT call `FUN_0000_e87a`** — patch 151 reconstructed the BATTLES dispatch *inline* inside e714's `_tgt==0xe87a` block and it STOPS at the briefing: `e39f(); cb7c(".FSG",…); if(a85d<2){ 7088(0,0x4509); } b78e=10;` — after 7088 returns (ACCEPT **or** CANCEL) it just sets `b78e=10` (menu) and `continue`s.  So the cascade tail (`e43f…e4bb`) is simply absent from the executed path.  Confirmed empirically: with the briefing buttons fixed, clicking ACCEPT registers correctly (`7217`→`7249` with AX=0 → `DAT_2000_4be2=1`) but returns to the plain menu with **zero** post-briefing file opens.

**PATCH 152 — `FUN_0000_7217`, the briefing/dialog confirm-button activate method (asm 0x7217-0x7248).**  Clicking a briefing button SIGSEGV'd (fault 0x3b32).  `ax`=event flags, `bx`=element node, `bp`=dirty base — the last two are **DGROUP NEAR OFFSETS** Ghidra dereferenced as host pointers (`*(int*)(param_2+2)`, `*(param_3+iVar1)`), the identical base-loss class as 128/142/145.  Rebased elem/base to `g_mem+0x1c000+(uint16)off`; threaded the dropped CF (stc when the 0x80 bit is absent) and the dropped confirm-dispatch **AX** = `word[base+si+1]` (0=OK / 1=CANCEL).  For the briefing (id=0) the confirm vector `word[DGROUP:0x8c5b]=0x7249` reads AX → sets `DAT_2000_4be2` (=1 OK / =0xff CANCEL) so 7088 returns CF=0 ACCEPT / CF=1 CANCEL.  Reached only by a modal confirm button (no prior flow clicked one) → zero regression.

**PATCH 153 — the 7088 EXIT display-list RESTORE `FUN_0000_1cdb` (asm 0x70e5..0x70ed).**  When the briefing closes, its modal loop tears down the dialog list and restores the caller's: `lcall[0x594]; pop dx/cx/bx/ax; call 1cdb`.  The popped **AX = the SETUP 1cdb's return handle** (`iVar2`, pushed at 0x70b5), bx/cx/dx = the init geometry — so the restore is `1cdb(iVar2, 0x8cee, 0x94, 0x10, param_2, uVar8)` (the proven idiom, cf. patch 142 build:31232 `1cdb(iVar1,<same init geom>)`; 1cdb uses `__allregs` order ax,cx,dx,bx and returns the PREVIOUS handle).  Patch 151 had emitted a SCRAMBLED `1cdb(0,0x10,0x8cee,iVar2,…)` — param_1=0 (should be the saved handle) with the geometry rotated into the wrong slots → 1cdb rebuilt from a null handle, its paint walker `FUN_0000_201a` dispatched a corrupted element whose method vector resolved to `FUN_0000_9a27` which base-loss-dereferenced garbage → SIGSEGV (fault 0x9ae4).  This exit path is NEVER hit by a prior passing flow (battles-ok captures the OPEN briefing and never closes it) → the bug was latent; clicking a button is the first close.  Fixed → the menu is restored **bit-identically** (diff vs main_menu = only the cursor@78,186).

**NEW FLOW `battles-cancel-briefing`** (`ref/battles_cancel_briefing_native320.png`, `tools/refcapture_ok3.sh 160 100 205 128 78 186 40 8 8`, 3 independent DOSBox captures AE=0, non-circular; DOSBox also returns to menu on CANCEL): BATTLES(160,100)→OK(205,128)→briefing→CANCEL(78,186, the 2nd bottom button right of ACCEPT@40,186)→menu.  TICK-PINNED `tick=2600` (FIST_DUMPTICK; the post-teardown menu repaint settles by `[0x452]~1280` and stays AE=0 through ~5458, but a wall-clock dump rarely catches a mid-repaint frame → pin a fixed engine tick for native↔wasm + run determinism, per the campaign-missions precedent).  READ-only (7162 only reads AZER1.FSW).

**THE MISSION-START CASCADE FRONTIER — PINPOINTED by throwaway-wiring the ACCEPT tail (`if(4be2==1){ e43f;…;4754;…;1631;…;e4bb }`) into e714 and tracing (NOT committed — that is the multi-patch LOADING deliverable):**
- The cascade IS reachable and runs `e43f` → **opens `LOADING.MS3`** (the LOADING screen layout) — the first mission asset, matching the recon trace.
- **First cascade blocker = `FUN_0000_e459` (the LOADING-screen TITLE text, asm 0xe48d..0xe4b8):** draws a centered string via `a358`(width)+`a370`(draw).  Two base-losses: (a) the `a358` arg — asm `movzbw bx,byte[DGROUP:0xf75f]; mov si,word[DGROUP:(bx-0x8a0)]` with ES=STRSEG(`word[DGROUP:0x70]`) — Ghidra collapsed it to `word[b75f-0x8a0]` host-deref; the string offset is `word[DGROUP:(byte[0xf75f]+0xf760)]` and `a358` derefs its `char*` param → the caller must pass `g_mem+(STRSEG<<4)+si`; (b) `a370`/`5591` then draw that string (same STRSEG base-loss, next fix).  **`FUN_1000_a370` is at linear 0x1a370 (seg-0xf69 code), not 0xa370.**
- After e459 the cascade proceeds to **`FUN_0000_4754` → `FUN_0000_d501` = the FSG chunk parser** (opens `<battle>.FSG`, reads 8-byte `SHDR`… headers, dispatches via the **7-entry handler table `DAT_2000_a9e6` @ linear 0x2a9e6, stride 3 words** [type-lo, type-hi, handler-near-off] — the d501 decompile reads it correctly as `int*` (2-byte int here), the residual is the open DX host-ptr (`param_4=&DAT_2000_39a9` → the DGROUP:0x79a9 filename buffer)); then `459a`, **`1631`** (LOD model loader, `18dc(i,0x20/0x10/8/0)` = .M32/.M16/.M08/.M00), then **`e4bb` = the in-mission game loop** (template 0x2d1d, `1cdb`+`206f`/`c40a`/`1e4b` modal, whose paint methods are the voxel terrain + 3D models + cockpit).
- **VOXEL-ENTRY PIN:** the voxel raycaster is a paint method of the `e4bb` (template 0x2d1d) display list, invoked per-frame downstream of the `D32/C32.KLC` load in `d501`.  Its exact FUN offset is not yet reached (the cascade blocks at e459 before LOADING.MRL/KLC load); the `.KLC/.SKY/.PAL` suffix table is at image 0x163de/0x16419 and `FUN_0000_82ee` (radar text) is a confirmed sibling paint method — the raycaster entry sits in the 0x2d1d paint-method set, to be pinned once d501→KLC lands.
- **HONEST STATUS on the LOADING frame:** it is NOT landable this iteration — reaching a *rendered* LOADING frame needs e459 (title) + 4754/d501 (FSG parse, which loads `LOADING.MRL` + the map) + the paint, i.e. a multi-patch subsystem, and the LOADING screen is transient (it dissolves into the non-static voxel game loop).  The **furthest STABLE bit-verified frame before the sim is the BRIEFING** (already landed, battles-ok).  152/153 are the genuine forward step: the briefing modal's buttons now work, so the cascade's entry gate is unblocked; the cascade reconstruction (e459→d501→KLC→voxel) is the next multi-patch deliverable.

## RECON (STAGE 3 — HANDOFF PREMISE CORRECTED: BATTLES→OK→ACCEPT IS THE PLAYABLE MISSION WITH THE VOXEL TERRAIN RENDERER, **NOT** A MAP EDITOR. No engine change this iteration (the next honest frame is a whole subsystem — the voxel game loop — and no partial/artificial flow is landed); `re_out/fist.c` pristine `61453e42`, verify.sh stays 18/18, git changes are tools-only. Evidence + full cascade map below.)

**THE VERDICT (decisive — genuine DOSBox 3-click capture, write-isolated, `tools/oracle/trace_click3.sh 160 100 205 128 40 186`, evidence `ref/recon/battles_accept_gameplay.png`):** BATTLES@(160,100)→OK@(205,128) opens the mission BRIEFING (patch 151, bit-verified). Clicking **ACCEPT@(40,186)** does NOT open a level/mission editor — it **LOADS AND STARTS THE PLAYABLE MISSION**: the after-frame is the in-cockpit **VOXEL TERRAIN gameplay view** — brown desert hills under a cloudy sky in the top viewport, the full instrument dashboard (AUTO CONTROL, MPH gauge, FIRE button, radar screen reading "READY", a terrain-map inset bottom-left), "GOALS REMAINING:13", "TARGET". So the handoff's premise ("behind ACCEPT sits the LEVEL/MISSION-EDITOR BUILD — the e891 fall-through cascade") is **WRONG**: the e891/e87a fall-through cascade is the **MISSION-START / GAME-LOOP setup**, not an editor build. **The in-engine EDITOR (the CLAUDE.md deliverable — the 6 INT-33h tool sites ADD WAYPOINT / CUT UNIT OR PATH POINT / TANK ADDED @ FIST.DAT 0x14d37–0x14dc8) is reached by a SEPARATE, still-unlocated entry** (hidden key / command-line switch / menu item) and remains an open Phase-0 recon task — it is NOT the BATTLES flow.

**ASSET LOAD ORDER (LD_PRELOAD open-trace, extended filter `tools/oracle/opentrace.c` now logs .FSG/.FSW/.CAM/.FPL/.PAL/.M00-.M32/.SET/.CFG; full log `/tmp/fist_trace_click3.log`):**
- *After OK (briefing):* `AZER1.FSW` (briefing text) + `FLDCOMP.MRL` (panel) — patch 151.
- *After ACCEPT (mission load):* `LOADING.MS3` + `LOADING.MRL` (the LOADING screen) → `AZER1.FSG` (mission data) → `RESOURCE.RES` ×N → `PAL.RES` + `532.PAL` (palette) → **`D32.KLC` (heightmap) + `C32.KLC` (colormap) = MAP 32** → `5.SKY` → `WVSOUNDS.BIN` → the full 3D LOD model set (`M1_A..E`+`_DEAD`, `M3`, `T80`, `BMP`, `ARTILL`, `TREES`, `TARGETS`, `APACHE`, `HIND`, each `.MAL/.M00/.M08/.M16/.M32`) → gameplay.
- So the **AZER1** (Azerbaijan-1) battle uses **map 32** (`D32/C32.KLC`), palette `532.PAL`, sky `5.SKY`; `AZER1.FSG` is the mission descriptor that names them.

**THE ACCEPT CASCADE (`FUN_0000_e87a` @ 0xe87a, the branch after `FUN_0000_7088(0)` returns CF=0=ACCEPT; asm-mapped 0xe89b..0xe929):** `e43f`; `[0xf6ce]|=0x22`; device methods `[0x1594]`/`[0x1598]`; `be0e(8)`; `e459`; `[0x5450]=[0x5452]=3`; **`FUN_0000_4754`** (mission setup → `4779` resets the sim state [`4413/5f52/9be8/b158/61f7/9d3c/b2a2/9cdb`] → **`FUN_0000_d501`** = the FSG chunk parser); `63b8`; `e006/d755/9d26`; device `[0xe8]`; `[0xd57e]|=0x21`; device `[0x1594]`; `be75`; `[0x2da9]=[0x2ce7]=1`; `5d12`; **`FUN_0000_459a`**; device `[0x59c]`; **`FUN_0000_1631`** (the 3D-model LOD loader — loops idx 0x0..0x44 calling `18dc(i,0x20/0x10/8/0)` = load `.M32/.M16/.M08/.M00`, exactly matching the trace's model opens); `be86`; `616e` (.FPL write); `6220`; **`FUN_0000_e4bb`** (the in-mission HUD loop, below); then `e714` (menu — only reached after mission exit).

**THE FSG PARSER (`FUN_0000_d501` @ 0xd501):** opens `<battle>.FSG` (INT 21h AH=3D), reads 8-byte chunk headers (`SHDR` = `0x4853`/`0x5244`…), dispatches each chunk type against the **7-entry handler table `DAT_2000_a9e6` (DGROUP:0xa9e6, stride 3: type-lo, type-hi, handler-fn near-offset)**, seeking past unrecognised chunks (INT 21h AH=42). The handlers populate the mission model (map#, unit list, path/waypoint list) and drive the `D32/C32.KLC` + `532.PAL` + `5.SKY` + model loads.

**THE GAME LOOP (`FUN_0000_e4bb` @ 0xe4bb) — architecturally the SAME machinery as every menu dialog:** it copies template **`0x2d1d`** (the cockpit HUD display list), builds it (`1cdb`), then runs the **identical `206f`-paint / `3907` / `[0x40a]`(`c40a`)-poll / `1e4b`-dispatch modal loop** used by all the reconstructed menu screens — but template 0x2d1d's paint methods drive the **voxel terrain + 3D models + cockpit instruments** each frame. `FUN_0000_82ee` (draws the radar "LOADING"/"READY" text) is one such paint method; the gauge/radar/viewport paints are its siblings. This is good news for tractability: the loop skeleton is already proven; the frontier is the heavy PAINT METHODS + the sim tick, not new loop machinery.

**THE VOXEL / GAME-LOOP FRONTIER (a whole subsystem — multi-iteration, NOT a single-frame menu-style reconstruction):**
1. **Voxel terrain raycaster** (Comanche / "Voxel Space" lineage, hand-asm fixed-point; the public algorithm is the semantic oracle per CLAUDE.md): renders the top viewport per-column from heightmap `D32.KLC` + colormap `C32.KLC`, invoked from an `e4bb` (template 0x2d1d) paint method. Entry not yet pinned to an exact FUN offset — locate via the KLC-segment reader downstream of `d501`; the `.KLC/.SKY/.PAL` suffix table is at image `0x163de`/`0x16419`.
2. **3D LOD model renderer** (`.M00-.M32`) — draws units/vehicles over the terrain (loaded by `1631`/`18dc`).
3. **Cockpit instruments** — dials / MPH gauge / radar sweep / "READY" (`82ee` and siblings), dynamic per-frame.
4. **The sim tick** — terrain scrolls, radar animates, so a bit-identical gameplay frame needs the sim reconstructed + `FIST_DUMPTICK` phase-pinning (patch-150 precedent). This is why NO gameplay flow is landed this iteration: it is not a static frame.

**NEAREST PRE-TERRAIN BIT-VERIFIABLE CANDIDATE:** the `LOADING.MRL` screen (shown right after ACCEPT, before the map/model load). But reaching it faithfully still requires reconstructing the cascade through `4754`/`d501` (the FSG parser + map-setup subsystem), and the LOADING frame is transient in DOSBox (needs mid-load capture timing) → it is a multi-patch deliverable, not a clean one-shot; deferred with the rest of the game loop.

**NEXT-ITERATION ORDER:** (A) locate the actual in-engine EDITOR entry (separate recon — the genuine CLAUDE.md editor deliverable; try command-line switches, the INT-33h tool cluster's caller, hidden menu/keys). (B) For gameplay: reconstruct `d501` (FSG chunk parser + handler table) → the KLC map loader → the voxel raycaster (public Voxel Space algorithm as oracle) → the cockpit paint methods → phase-pin a settled frame. Each is a separate multi-patch deliverable. **Tools added this iteration:** `tools/oracle/opentrace.c` (extended asset filter) + `tools/oracle/trace_click3.sh` (3-click open-trace + after-frame capture); evidence `ref/recon/battles_accept_gameplay.png`.

## DONE (STAGE 3 — THE BATTLES→OK MISSION-BRIEFING SCREEN IS THE ENTRY INTO THE LEVEL/MISSION EDITOR AND ITS FIRST SETTLED FRAME IS BIT-IDENTICAL: AE = 0/64000 vs a genuine, reproducible DOSBox capture on BOTH native AND wasm, native↔wasm 0-diff (single md5 `bc73e2000f40`), DETERMINISTIC (5× serial + pattern-init byte-identical, crash-free). One new verify.sh flow → `bash tools/verify.sh both` = 18/18. 1 new patch (151). `re_out/fist.c` pristine `61453e42`; NO regression.)**

**WHAT BATTLES→OK ACTUALLY OPENS (recon — genuine DOSBox 2-click capture BATTLES@160,100 → OK@205,128 + opentrace):** NOT the voxel map editor directly — it opens the mission **BRIEFING** screen: a full FLDCOMP panel with the selected battle's briefing text (AZER1 → "BLOODFEUD! / AS ARMENIAN FORCES PUSH INTO AZERBAIJAN, MULTILATERAL FORCES HAVE BEEN CALLED UPON TO INTERVENE. …") and **ACCEPT / CANCEL** buttons. The in-engine LEVEL/MISSION EDITOR (the map view + INT-33h tools) is gated BEHIND ACCEPT — `e891` is `mov ax,0; call 7088; jb 0xe931` where `jb`(CANCEL)→menu and fall-through(ACCEPT)→the editor build (`e43f/4754/459a/1631/be86/616e/6220/e4bb`). So the briefing is the editor entry's first sub-screen and the tractable bit-verified frame this iteration; the editor build itself is the next deliverable.

**THE BRIEFING IS A 7088 MODAL DIALOG (id=0) — the SAME machinery as ABOUT (id=4, patch 131) / REVIEW (id=2, patch 136).** 7088 builds template `0x2c56`, calls the per-dialog SETUP `[id*2-0x73ab]` (id=0→`FUN_0000_7162`), paints the text via `[id*2-0x739f]` (id=0→`FUN_1000_6c2d`) and the buttons via `[id*2-0x7393]` (id=0→`FUN_1000_6d53`). **The briefing text is DYNAMIC, from `<battle>.FSW`** (plaintext, ~800 B; `.FSE` if `[0x6db4]!=0` = EAST side): `FUN_0000_7162` builds the name from the selected battle (DGROUP:0x79a9) → DGROUP:0x8bed, opens it (INT 21h AH=3D), reads ≤0x1800 B into **STRSEG(0x2d74):0x452c**, then `6c2d` renders it with the already-proven `5591` multi-line drawstring (the ABOUT-credits blitter). **The `.FSG` (10 KB, the SELECT BATTLE list entry) is the EDITOR'S battle/map data** (chunks `SHDR/DCBSR/PATH/STMP/PINF/BINF`, references `D32.KLC`/`C32.KLC` heightmap+colormap = map 32, `532.pal`, `5.SKY`, unit/path placements) — NOT opened for the briefing; it is the editor-load input behind ACCEPT.

**PATCH 151 (all asm-verified vs `re_out/fist_dat_image.bin` @ 0x7162 / 0x16d53):**
- **`FUN_0000_7162` (briefing SETUP) base-losses:** (a) the default "NO ORDERS RECEIVED FOR THIS MISSION…" `rep movsb` (DGROUP:0x8caf → ES=STRSEG:0x452c) — Ghidra rendered the SI/DI cursors as the reg-file slots `pcRam000f0008`/`000a`, whose 4-byte host-pointer views **OVERLAP** (0xf0008 vs 0xf000a) so the second write corrupts the first → wild src ptr → SIGSEGV; reconstructed with plain locals for the DS:SI/ES:DI effective addresses. (b) the AH=3D open **DX**: asm `mov dx,0x8bed` (DS:DX = DGROUP:0x8bed = the filename buffer = `&DAT_2000_4bed` linear 0x24bed) — Ghidra emitted the host pointer → DX=garbage → open '' fails; write the real DOS offset 0x8bed (host-ptr-into-DOS-register class, patch 077/146). (c) the read DX=0x452c / DS=STRSEG and close were already faithful.
- **`FUN_1000_6d00`/`6d0f`** (buffer clear / 0x0c/0x0e control-char remap): both operate on ES/DS=STRSEG:0x452c but Ghidra base-lost the buffer to host `0x452c` → deref of linear 0x452c; rebase to `g_mem + (STRSEG<<4) + 0x452c`.
- **`FUN_1000_6d53`** (id=0 button sprite draw, asm 0f69:0x76c3): exact mirror of `6d3d` (patch 131) — `si=ax*2; ax=word[si-0x7379]; if(ax!=0xffff){ ax+=[0x8be9]; lcall [0x6b4] ax=sprite,bx=rect }`; Ghidra deref'd the per-state sprite table as a host int* (base-loss) + dropped the +[0x8be9] and ax/bx args.
- **e87a dispatch (patch-140 body extended):** after `cb7c`, if `a85d < 2` (cc33 OK/select, vs ≥2 CANCEL) → `7088(0)` (the briefing modal). The editor build after ACCEPT stays deferred (b78e=10 → menu either way; the captured frame is the first settled briefing frame). READ-only (no write until the editor's own save) but isolatable.

**THE EDITOR FRONTIER MAP (what remains for the full LEVEL/MISSION-EDITOR deliverable, per CLAUDE.md):** the editor is reached by `e891` fall-through (7088 ACCEPT, `4be2==0xff`→CF=0). The build cascade (asm 0xe89b..0xe929): `e43f`; `[0xf6ce]|=0x22`; device methods `[0x1594](al=1)`/`[0x1598](al=0xff cursor-white)`; `be0e(8)`; `e459`; `[0x9450]=[0x9452]=3`; **`FUN_0000_4754` = the display-list build**; `0f69:6d28`; `e006/d755/9d26`; `[0xe8](ax=0x1000,bx=0x157c)`; `[0x157e]|=0x21`; `[0x1594](al=0)`; `be75`; `[0x6da9]=[0x6ce7]=1`; `5d12`; `459a`; `[0x59c](ax=0x10)`; `1631`; `be86`; `0f69:6ade` (WRITE .FPL); `0f69:6b90`; `e4bb`; `0f69:6ade`. This is the map-view UI. **OPEN QUESTIONS for next iteration:** (1) does 4754/459a invoke the **VOXEL terrain renderer** over the .FSG's D32/C32.KLC map? (recon: which fns, what data — this is the large subsystem); (2) the 6 INT-33h editor-tool sites (0x14d37–0x14dc8: ADD WAYPOINT / CUT UNIT OR PATH POINT / TANK ADDED); (3) the editor round-trip (create→save `.FSG`→reload→sim, byte-identical) — CLAUDE.md's editor acceptance bar. The briefing entry (this patch) is the gate; the terrain view + tools + round-trip are the remaining editor deliverables.

## DONE (STAGE 3 — THE CAMPAIGN MISSION-SELECT SCREEN f338 "INTRODUCTORY CAMPAIGN" IS BIT-IDENTICAL: AE = 0/64000 vs a genuine, reproducible DOSBox capture on BOTH native AND wasm, native↔wasm 0-diff (single md5 `8776939ecd1c`), DETERMINISTIC via a new tick-phase-pinned dump. One new verify.sh flow → `bash tools/verify.sh both` = 17/17. 1 new patch (150). `re_out/fist.c` pristine `61453e42`; NO regression.)**

**THE 114/142-px RESIDUAL WAS ROOT-CAUSED TO THREE ASM-VERIFIED RECONSTRUCTION BUGS (all fixed in patch 150; the handoff's "unresolved paint-order / palette nuance" premise for the c622 gray was WRONG — it is a dropped register-clobber):**
1. **The inner marker ring is ALWAYS palette index 2 (gray), by the 11ba asm itself — a dropped AL clobber (80 px).**  be47 draws each marker as three nested box outlines: `[0x61e]`=119f (inset-1, caller's al = state colour), `[0x61a]`=11db (outer, al=0 black), `[0x622]`=11ba (inset-2).  The 11ba asm is `mov ax,0x2` (the inset amount) `... add [bx],ax ...` `call 11db` — the `mov ax,2` sets the inset amount AND clobbers AL to 2, so 11db draws the innermost ring with **colour index 2 (the dark-gray bevel highlight)**, NOT the caller's al=0.  (Contrast 119f which uses `inc [bx]`/`dec [bx+4]` and never touches AX → preserves al; and 11db which push/pop's ax around its clip call.  Only 11ba reuses the inset constant as the colour.)  Patch 138 reconstructed 11ba as `11db(bx, al)` (passing the caller's al through), which is wrong — the real 11ba passes AL=2.  Proven: dropping c622 reveals TERRAIN (idx 175/165) not gray, so the gray is NOT the baked bg; it is c622 itself drawing idx 2.  Fix: `11db(bx, 2)`.  (c622/11ba is reached ONLY via be47 — verified no other caller — so this touches only the campaign markers; zero regression to the 16 flows.)
2. **The selected-marker blink was inverted (28 px + the determinism hazard).**  be47 asm 0x1be64: `cmp cx,[0x6db6]` (selected?) `jne`; `mov al,0`; `test [0x452],0x10`; `je 1be8a` — so the SELECTED marker's inset-1 ring is BLACK when `[0x452]&0x10 == 0`, state-colour when set.  Patch 149 had the phase inverted (`(tick&0x10)!=0 -> al=0`).  Fixed to match asm.
3. **The mouse cursor drew palette index 105 not 255 on the WHOLE f338 screen (34 px) — patch 149's far-call dummy 0xf69 leaked into AL.**  f338 entry asm: `mov al,0xff; call [0x1598]` (set cursor colour mask WHITE idx 255 via 2d1f/d5c2) then `mov al,0x0; call [0x1594]`.  Patch 149 called both far vectors with the segment dummy `(0xf69)` → the `__allregs` convention put 0xf69 in AX → **AL=0x69=105**, so the cursor rendered idx 105 (199,199,105) instead of idx 255 (158,105,89 in this palette).  This was the "34px cursor settle" residual — NOT a settle-position delta at all (both cursors are at the identical pixel), a colour-mask bug.  Fix: pass `(0xff)` / `(0x00)` = the real AL.

**PHASE-PINNING (the DETERMINISM fix — HARNESS feature, engine untouched):** be47 draws only when `[0x452]&0xf==0` and the selected marker blinks on `[0x452]&0x10`, so the marker layer depends on the engine frame-timer `[0x452]` mod 0x20.  A wall-clock `FIST_RUNMS` dump lands at a TARGET-DEPENDENT `[0x452]` (native's SIGALRM-driven counter vs wasm's cooperative one advance at different rates → different blink phase → native≠wasm by 80px).  New **`FIST_DUMPTICK=N`** (`tools/native_main.c` `fist_dump_and_exit`, shared with the FIST_RUNMS path) dumps when `[0x452]` FIRST reaches N — identical on both targets.  The flow uses `tick=8008`: N=8008 is 8 ticks past the mult-of-32 boundary 8000, so BOTH native and wasm have caught the `[0x452]==8000` be47 draw (bit4=0 → selected marker BLACK, matching the DOSBox capture) before dumping → the marker layer is native↔wasm bit-identical (verified AE=0; picking N=8000 exactly is a race on whether be47 fires at that exact tick → 28px divergence).  verify.sh's `run_target` parses `tick=N` in the 3rd flow field → `FIST_DUMPTICK`.  DOSBox ref reproducible: `tools/refcapture_ok3.sh 160 87 205 128 203 159` (3× AE=0, DOSBox blink phase = selected black at SETTLE=8, non-circular).

**WHAT f338 SHOWS (genuine DOSBox 3-click capture: CAMPAIGNS@160,87 -> roster OK@205,128 -> campaign OK@203,159):** a full-screen "INTRODUCTORY CAMPAIGN" screen — a left column of mission rows (TERRORDOME / LA DIABLA / INFERNO / TRIAD), each a small map-thumbnail square with a 3-ring bevel marker box (outer black / middle state-colour / inner gray idx2); a right column of per-mission briefing text; the SELECTED mission's name + two embossed briefing lines at the bottom; ACCEPT / CANCEL.  Reached from ec7e-OK: `6ade` writes the player campaign `.FPL`, then `f338` opens (its own template 0x2d37, dispatch flag `ba56`).  The deeper ACCEPT->`7088` screen / mission-start cascade (e7dd..) is a SEPARATE deliverable.

**PATCH 149 (all asm-verified vs `re_out/fist_dat_image.bin` + `re_out/fist_mga_image.bin`):**
- **The `.CAM` MISSION parser `FUN_0000_f0f1` + 13 handlers** (asm 0xf0f1..0xf334): f0f1 reopens the selected campaign's `.CAM` (bd14 open+op-0x80 decrypt, patch 147) and walks its keyword tokens — `f05f` copies a token, `f088` matches it against the 19 STRSEG keyword strings (`b91b[]`) -> byte-index bx, and the handler at `word[DGROUP:0xf822+bx]` fills the mission-record table (records at DGROUP:0x6dda, stride 0xfd, count `[0x6dd0]`).  Ghidra base-lost the whole family: the handler dispatch `*(word*)(bx-0x7de)` (bx a host ptr), every `[0xf89b]+off` record write (record base a host ptr), the STRSEG base of `efd8`/`f00c`, and folded every carry to a static false.  Reconstructed faithfully; CF via `g_fist_cf`.
- **`efa1` (parse-cursor reset) — a 2-BYTE store, not 4:** `DAT_2000_b899` is typed `int**`; the macro assign `= 0x452c` wrote 4 bytes and ZEROED the adjacent record ptr `b89b` (0x2b89b), so f0f1 built the records at DGROUP:0xfd (low-memory corruption -> SIGSEGV).  Write exactly 2 bytes (asm `movw`).
- **`FUN_0000_f338` modal loop** (reconstructed like ec7e/cc33 — Ghidra folded the carry to a static in_CF -> never terminated); **`bd7e`** mission-availability calc (rebase record base 0x6dda; `rec+0x31` is a WORD); **`bdbe`** bottom selected box (thread the dropped `[0x60a]` fill; rebase the record base from `unaff_CS`; draw name + 2 briefing lines with the 0xf8->0x17 fixup); **`be47`** per-mission marker boxes (reconstruct the dropped al/bx args for `[0x61e]`/`[0x61a]`/`[0x622]`, colour by availability `rec+0x3b`, selected blinks on `[0x452]&0x10`); **`bea4`** hover hit-test (rebase record base); **`f3a3`** screen enter (thread the dropped `[0x560]`/`[0x564]` bg-load args ax=0xffff,cx=ds,bx=0xfa52).
- **`ba7c`/`ba7d` retyped undefined2 -> uint8_t** (patch-142 a8ee class): the per-element dirty BYTES were typed as overlapping WORDs, so `be47`'s `ba7c=3` clobbered the bottom-box dirty byte `ba7d` -> `bdbe` never repainted.
- **The `[0x688]` EMBOSSED-text glyph blitter `1e8d`/`1ec9` (fist_mga.c)** — the `.CAM`-driven bottom briefing renders via `thunk_5547` -> `[0x688]` -> 1e8d/1ec9, a parallel to the transparent `[0x684]` blitter 1d5f (patch 131) that NO prior flow exercised: it deref'd the DGROUP:0x736 text-state VALUE as a host `int*` (base-loss) and word-scaled the byte fields.  Reconstructed the two-tone blit (fill = `(fontbyte^3)+colourLo`, shadow/border = colourHi) in the byte-offset model; `thunk_5547` threads the dropped AL char (mirror of thunk_5531/patch 139).
- **e78c dispatch:** on ec7e-OK (CF=0) -> `6ade` (.FPL write) + `b6c0=0` + `f338`; both f338 outcomes return to menu-idle (the deeper 7088 is deferred).

**THE FRONTIER (why no AE=0 flow yet) — the 4 mission MARKER boxes (`be47`), ~80/114 px:**
1. **`c622` inner-ring shade:** the marker is 3 nested box-outline rings — `[0x61a]`=11db (outer, al=0 black), `[0x61e]`=119f (inset-1, state-green), `[0x622]`=11ba (inset-2, al=0).  Native draws the inner ring **index-0 black**; DOSBox shows a **dark-gray (28,28,28)** there (the baked thumbnail border).  The asm is unambiguous `xor al,al` before `[0x622]`, and dropping the call is WORSE (AE 114->142), yet DOSBox's inner ring is gray while its OUTER ring (also al=0) is black — an unresolved paint-order / palette nuance (the terrain thumbnail interior matches exactly, so it is baked-bg).
2. **Tick-gated blink (DETERMINISM):** `be47` only draws the markers when `[0x452]&0xf==0`, and the SELECTED marker toggles on `[0x452]&0x10` — so the marker layer is PHASE-dependent.  **DOSBox IS deterministic with fixed capture timing** (3 identical captures = AE 0/0/0), but native vs wasm differ by 80 (the cooperative-tick `[0x452]` phase) and native vs DOSBox by 114 -> no clean bit-exact frame across all three yet.
The cursor (~34 px) is a tunable settle-position delta.  NEXT: root-cause the `c622` gray (likely a per-mission thumbnail re-blit AFTER `be47`, or a colour-table remap) + align the marker tick phase across native/wasm/DOSBox, then add the `campaign-missions` WRITE flow -> 17/17.  Then the deeper ACCEPT->`7088` screen and the BATTLES editor `e891`.


## DONE (STAGE 3 — THE "SELECT A CAMPAIGN" MODAL SCREEN RENDERS BIT-IDENTICAL) — **clicking CAMPAIGNS → OK opens the campaign modal screen `FUN_0000_ec7e` — a left LIST of the 7 decrypted campaign names (TRAINING [red-selected] / OVERWATCH / CROSSED SWORDS / AEGIS / CERTAIN FURY / FIRE HAMMER / BURNING FROST), a right HARDWARE group (WESTERN [red] / EASTERN radios) and OK / CANCEL — and its first settled frame is AE = 0/64000 vs a genuine DOSBox capture on BOTH native AND wasm, native↔wasm 0-diff, deterministic. One new verify.sh flow → `bash tools/verify.sh both` = 16/16.** 1 new patch (148). `re_out/fist.c` pristine (`61453e42`); `make check` = 148 patches apply, native + wasm green. NO REGRESSION: all 15 prior flows PASS.

**THE FLOW — campaigns-ok** (`ref/campaigns_ok_native320.png`, non-circular genuine DOSBox capture via `tools/refcapture_ok.sh 160 87 205 128`, write-isolated): CAMPAIGNS (row 87) opens the SELECT PLAYER roster; OK (205,128) picks the default player **D** → e78c copies the name (e9bd) + reads the .FPL (6a9c) → opens `ec7e` = the SELECT A CAMPAIGN modal. The captured frame is ec7e's first settled frame (READ-only: the campaign .FPL write 6ade fires only on ec7e's OWN OK; isolated anyway).

**PATCH 148 — all asm-verified vs `re_out/fist_dat_image.bin`.** The recurring class is the **b941 campaign-name table walk** `si = word[DGROUP:0xf941 + bx]` → a DGROUP NEAR-OFFSET Ghidra dereferenced as a HOST pointer (fault `0xfffff941`); rebase every byte read to `g_mem[0x1c000 + (uint16)si]`, the parsed name at DGROUP:(si+1).
- **DAT_2000_b848 retyped undefined2 → uint8_t:** ec7e's exit flag is a BYTE (`movb [0xf848]`); the WORD store at ec7e entry (`b848=0`) clobbered the adjacent HARDWARE-icon rect at DGROUP:0xf849. 0=idle / 1=OK / 0xff=CANCEL.
- **`FUN_0000_ec7e`:** fix the b941 walk base-loss (find the first LOADED campaign from `[0x77d6]`) + reconstruct the CF-driven modal loop faithfully (mirror the proven cc33 loop, patch 137): 206f paint → 3907 → if `g_fist_cf`: 096c → inner poll `[DGROUP:0x40a]`=35a7 (sets `g_fist_cf`, patch 073): CF=1 repaints, CF=0 dispatches (1e4b) until b848 set; exit CF = (b848==0xff)==CANCEL. f0f1 (OK submenu) unreached on the first frame.
- **`FUN_0000_ed28` (campaign-LIST paint, cdc4-lesson):** Ghidra DROPPED the selection-bar colour (`al=0xf8 if row==[0x77d6]` → c60a fill) and the glyph colour (`al=0x1f/0x20 from byte[DGROUP:0x77da+row]&0x80` → c684); reconstructed per-row bar + hardware glyph + NAME drawstring (`thunk_FUN_1000_5531`=f69:0x5e9b, patch 139) from DGROUP:(si+1).
- **`FUN_0000_edc2` (list HIT-TEST):** click Y at `[si+4]`, si = the drained coord node `word[DGROUP:0x18e2]` (ccde class, patch 142); `row=((Y-0x60)/7)*2`; if LOADED and != `[0x77d6]` → select (b882=3 BYTE dirty).
- **`FUN_0000_ed14`/`ee67`/`eeab` (HARDWARE box + WESTERN/EASTERN radio-icon PAINT):** Ghidra dropped the c6b4 sprite/rect args (ax=sprite id, bx=DGROUP rect, sig per patch 139): ed14→0x13e0@0xf849; ee67(WESTERN)→(DAT_2000_2db4!=0?0x13c8:0x13d0)@0xf812; eeab(EASTERN)→opposite@0xf81a.
- **`FUN_0000_ef39` (keyboard accel, off the mouse-flow, reconstructed faithfully):** scancode at `[si+2]`, si=word[DGROUP:0x18e2]; match `(key&0xf77f)` vs the 11-entry b851 table (stride 4, 16-bit compare) → near-dispatch.
- **e78c dispatch (state 0x0 CAMPAIGNS):** after cb74 OK (a85d<2) copy name (e9bd) + read .FPL (612c/6a9c), then call ec7e. The radios ee7c/eec0 + OK eeef / CANCEL ef14 were VERIFIED faithful (all DGROUP vars via correct DAT_2000_ accessors) — no change.

**NEXT FRONTIER:** the `ec7e`-OK tail (`6ade` write .FPL + `f338`/`7088` = the deeper campaign MISSION-SELECT subscreen via the `f0f1` submenu parser), then the **BATTLES mission EDITOR** (`e891`). Each is a separate cdc4-lesson deliverable.

## DONE (STAGE 3 — THE LINCHPIN: THE EXTENDER op-0x80 `.CAM` DECRYPT IS FOUND, IMPLEMENTED, AND PROVEN END-TO-END ON REAL ENGINE CODE + THE FULL CAMPAIGN-NAME PARSER CHAIN IS RECONSTRUCTED) — **the CAMPAIGNS roster (TRAINING / OVERWATCH / CROSSED SWORDS / AEGIS / CERTAIN FURY / FIRE HAMMER / BURNING FROST) now DECRYPTS + PARSES correctly, bit-identical on native AND wasm, with ZERO regression (verify.sh both = 15/15 PASS, git clean, `re_out/fist.c` pristine `61453e42`).** 1 new engine patch (147) + the shim decrypt/self-test (`tools/native_main.c`).

**THE DECRYPT (the blocker below is CLEARED).** Located the op-service dispatch table base = image **0xcb3** (the PM gate `FUN_00000f30`: `movzx ebx,bx; mov ebx,[ebx+0xcb3]; call [0xcaf]`, op used directly as a byte offset; ebx=TCB inbox at TCB+0x3f2). op 0x80 = `[0xcb3+0x80=0xd33]` = **`FUN_00007762`/`776e`** (`re_out/fist_image.bin` @ image 0x7762): a stream cipher over 8-byte blocks — keys `eax=0xfade2bad` / `ebp=0xace4dead`, per-block LFSR (`shl eax,1; cdq; and edx,0xbc9abb09; stc; adc eax,edx; ror ebp,1`), then `dword0 ^= eax` + byte-swap-low-word (`xchg bh,bl`), `dword1 ^= ebp` + `ror 16`.  The buffer is passed in **ESI = (word[DGROUP:0x70]<<4)+0x452c** (bd14 0x1bd58, NOT the inbox — e2a5 writes a garbage EBX to the inbox for this op), len in **ECX = word[DGROUP:0xf810]**.  Wired faithfully in `fist_extender_gate` op 0x80 (`tools/native_main.c`).  Validated: A.CAM → `DESC:  TRAINING / CAMPMAP: INTR_.MRL`.

**THE FIST.DAT PARSER CHAIN (patch 147, all asm-verified vs `re_out/fist_dat_image.bin`).** STRSEG=word[DGROUP:0x70]; the parse-ptr `b899`=word[DGROUP:0xf899] is a STRSEG NEAR-OFFSET Ghidra typed `int**` → host-pointer base-loss on every buffer byte; the `b941` campaign-name table (12 DGROUP near-offsets, stride 0x13, name = `[si+1]`, `[si]`=loaded flag) and `b91b` keyword table (19 STRSEG near-offsets, `[0]`="DESC:") are static in the DGROUP image; radix `ba50`=10:
- **`FUN_1000_bd14`** (0x1bd14): open DX = DOS offset **0xf889** ("A.CAM") not a host ptr (patch-077/146 class); read-buffer clear → **STRSEG:0x452c** (was host 0x452c = engine code); bytes-read → **DGROUP:0xf810**; CF (open/read fail vs success) → `g_fist_cf` so ef5e's `jb` works.
- **8-fn parser** `efa8`/`f040`/`f05f`/`f088`/`f0b0`/`f0cc` (+`efa1`): all rebased (source STRSEG, dest DGROUP:0xf902/di), CF via `g_fist_cf` (the decompile's local `in_CF` never reflected the real carry).
- **`FUN_0000_ef5e`** (0xef5e): the 12-file build loop; `b941` walk (`si=word[DGROUP:0xf941+bx]`) rebased; per file clear name-slot, bd14 (open+decrypt), seek section 0 ("DESC:"), copy name → DGROUP:(si+1), si[0]=1.

**PROOF (non-crashing, deterministic, both targets):** shim self-test `FIST_CAM_SELFTEST=1` (`tools/native_main.c`) runs the REAL `FUN_0000_ef5e` once the menu is up and dumps the parsed roster — identical on native and wasm (all 7 names correct; H-L.CAM absent → empty).  Default boot rc=0, no regression.

**REMAINING FRONTIER (the AE=0 flow → verify.sh 16/16 — the next deliverable):** the `ec7e` MODAL SCREEN itself (template 0x2d30 copy + 1cdb display-list + the c40a/1e4b modal loop + b941-walk row select) and its cdc4-lesson PAINT methods **`ed28`** (list paint: per-row selection-bar colour `al=0xf8` if row==[0x77d6] via c60a, glyph colour `0x1f/0x20` via c684, name drawstring `5e9b` from DGROUP:(si+1) — the current decompile DROPS the selection-colour logic + base-losses the b941 walk) and **`edc2`** (list hit-test, b941 walk).  All asm mapped below; the radios (`ee7c`/`eec0`), OK (`eeef`)/CANCEL (`ef14`), keyboard (`ef39`) are structurally faithful in the decompile.  Then: wire the CAMPAIGNS-OK dispatch → ec7e, capture `ref/campaigns_ok_native320.png` (`tools/refcapture_ok.sh 160 87 205 128`), drive AE=0 native+wasm, add the `campaigns-ok` flow.  The `f0f1` OK-submenu (deeper mission subscreen) is a later, separate deliverable.

## RECON (STAGE 3 — THE CAMPAIGN "SELECT A CAMPAIGN" SCREEN: FULL RECONSTRUCTION MAP + A HARD BLOCKER FINDING) — **the CAMPAIGNS→OK screen `FUN_0000_ec7e` is NOT "just base-loss like SELECT PLAYER"; its campaign names are decrypted by an UNIMPLEMENTED extender-kernel service (op 0x80), so it is a Route-1 + FIST.DAT-base-loss multi-patch deliverable, not a one-shot. No engine change this iteration (would be untested debt with no bit-verifiable flow); `re_out/fist.c` pristine (`61453e42`), verify.sh still 15/15, git clean.**

**TARGET (genuine DOSBox capture, 2-click CAMPAIGNS@(160,87)→OK@(205,128), write-isolated):** a "SELECT A CAMPAIGN" modal dialog — a left LIST of 7 campaign names (TRAINING [red-selected], OVERWATCH, CROSSED SWORDS, AEGIS, CERTAIN FURY, FIRE HAMMER, BURNING FROST), a right-side "HARDWARE" group with WESTERN [red box] / EASTERN radio, and OK / CANCEL buttons. Reached from the CAMPAIGNS button `FUN_0000_e78c`: on a fresh boot `[0x6dad]==0` → it first opens the SELECT PLAYER roster `cb74` (the existing `campaigns` flow's frame), then on OK runs `e9bd`(copy name→STRSEG:0xbf1)/`[0x6dad]:=1`/`6a9c`(read .FPL) → **`ec7e`** (this screen; own template `0x2d30`, dispatch var `b848`) → on ec7e-OK `6ade`(write .FPL)/`f338`/`7088`(the deeper mission subscreen). ec7e's first settled frame is the deliverable; it is a READ-only frame (6ade fires only after ec7e-OK) but isolate anyway.

**THE BLOCKER (the correction to the handoff — asm-verified vs `re_out/fist_dat_image.bin` + `re_out/fist_image.bin`):** ec7e→`ef5e` builds the campaign list by, per file A.CAM..(cx=0xc), calling `0f69:0xc684 = FUN_1000_bd14` (linear 0x1bd14): it INT-21h opens `DGROUP:0xf889`="A.CAM", reads 0x1800 B into `STRSEG:0x452c`, then **`call 0:0xe2a5` with esi=(STRSEG<<4)+0x452c, ecx=len** to decrypt the buffer in place; ef5e then `efa1`(parse-ptr=0x452c)/`f0b0(0)`(seek section 0)/`f0cc(0x10, b941[i]+1)`(copy the campaign NAME token out of the decrypted text into the `b941` name string). **`FUN_0000_e2a5` is NOT an in-engine decrypt — it is an EXTENDER GATE call: `*(TCB+0x3f2)=buf_lin; aa10(DGROUP:0xea10)=0x80; e339` → the 0x8799 gate → `fist_extender_gate`. op 0x80 is UNHANDLED there (only op 0 create-task + 0x64/0x78 KDV are) → the gate is a no-op → the .CAM stays ENCRYPTED → the parser finds no section-0 numeric key → every campaign name stays empty → the list renders blank ≠ AE=0.** The .CAM decrypt is a real algorithm in the Doug-Huffman extender kernel image `re_out/fist_image.bin` (NOT trivial: no constant-XOR/ADD/cumulative-XOR reveals "TRAIN" in A.CAM) — it must be located in the extender op-table (image ~0xcef..0xd2b, same table as the KDV `0x11cb`/`0x6f17`), decompiled, and wired into `fist_extender_gate` op 0x80 as a Route-1 ext-module service (the KDV precedent). **This is the linchpin: without it the screen cannot show campaign names.** e2a5 is called ONLY from bd14 (1bd6c) so op-0x80 is exclusively the .CAM decrypt.

**THE FIST.DAT-SIDE RECONSTRUCTION (routine once the decrypt lands; ALL asm-verified, ALL functions campaign-path-isolated ⇒ zero regression risk to the 15 flows — none is reached by any existing flow):**
- **`FUN_0000_ec7e`** (0xec7e): the campaign-name table WALK `bx=[0x77d6](37d6); si=word[bx-0x6bf]=word[DGROUP:0xf941+bx]; cmp byte[si],0` — Ghidra read `si` (a DGROUP NEAR offset into the b941 name table) as a HOST pointer (fault `0xfffff941`). Rebase every `*si`/`byte[si]` to `g_mem+0x1c000+(uint16)si`. Then `1e27(0x2d30)` template copy, `1cdb` build display list, `206f` paint, modal loop over `[DGROUP:0x40a]` sched poll; on activate (`b848`!=0) → `e39f`/`f0f1`.
- **`FUN_0000_ef5e`** (0xef5e): per-campaign loop; same `word[DGROUP:0xf941+bx]` walk base-loss; `byte[si]=0/1`; `f0cc(0x10, si+1)` dest is a DGROUP near offset (base-loss).
- **The text parser** `efa8`/`efd8`/`f00c`/`f040`/`f05f`/`f088`/`f0b0`/`f0cc` (0xefa8..0xf0f1): ALL read `es:[si]` where `es=word[DGROUP:0x70]`=STRSEG, `si=word[DGROUP:0xf899]`(b899). Ghidra typed `DAT_2000_b899` as `int**` and read `*pbVar` as a raw host pointer → STRSEG base-loss on every read; the write dests (`f05f`→`0xf902`=b902 DGROUP; `f0cc`→passed DGROUP di) are DGROUP-based. Rebase: byte at STRSEG:si = `g_mem[(STRSEG<<4)+ (uint16)si]`, DGROUP dest = `g_mem+0x1c000+off`. (efa8=skip-to-token, efd8=skip-to-digit, f00c=atoi radix `[0xfa50]`, f040=skip-to-EOL, f05f=copy-token-to-b902, f088=match-keyword vs the `b91b` table, f0b0=seek section N, f0cc=copy-value.)
- **`FUN_1000_bd14`** (0x1bd14): (a) the filename DX slot `pcRam000f0006 = (char*)s_A_CAM_2000_b889` is a HOST pointer into the 16-bit DX register (patch-077 host-ptr-into-DOS-register class) → store `R_DX=0xf889, R_DS=0x1c00`; (b) the STRSEG:0x452c buffer-clear loop writes host `0x452c` (base-loss) → STRSEG:0x452c; (c) the `e2a5` decrypt-gate call (currently reachable but no-op — see blocker).
- **`FUN_0000_f0f1`** (0xf0f1, ONLY on ec7e-OK, not first frame): the campaign SUBMENU parser — `f0b0`/dispatch `*(word*)(uVar2-0x7de)` handler table (`f141`/`f146`/`f154`/`f1a1`/`f1da`…); same STRSEG base-loss.

**PAINT/ACTIVATE METHOD SET (the cdc4-lesson enumeration for template 0x2d30 — must all be reconstructed BEFORE driving, else a base-lost method wild-fills/SEGVs):** the list-paint **`FUN_0000_ed28`** (walks `word[DGROUP:0xf941+b887]` → `c60a` drawstring per non-empty name + `52d1` setpos + `c684` glyph blit; b941 walk + host-ptr base-loss), the list hit-test **`FUN_0000_edc2`** (click-Y→row via the `[si+4]-0x60`/7 stride, then `word[DGROUP:0xf941+i]` base-loss), the HARDWARE radios **`ee7c`**(WESTERN, toggles `b80e`)/**`eec0`**(EASTERN, `b80f`), OK **`eeef`**/CANCEL **`ef14`** (set `b848`=1/0xff), and the keyboard handler **`ef39`** (matches `b851` accel table → near dispatch). Most are already structurally faithful in C; the b941-walk ones (ed28/edc2) carry the same base-loss as ec7e.

**NEXT-SESSION EXECUTION ORDER:** (1) locate + decompile the extender op-0x80 `.CAM` decrypt in `re_out/fist_image.bin` (reverse its op-table dispatch, mirror the KDV Route-1 ext-module wiring) and add it to `fist_extender_gate`; (2) reconstruct bd14 (host-ptr-DOS-register + STRSEG clear) so the decrypted text lands; (3) reconstruct the 8-function text parser (STRSEG base-loss) + ef5e (b941 walk); (4) reconstruct ec7e (b941 walk + modal) + the paint set ed28/edc2 + verify the radios/OK/CANCEL methods; (5) capture the write-isolated ref (`tools/refcapture_ok.sh 160 87 205 128` → `ref/campaigns_ok_native320.png`), drive to AE=0 both targets, add the `campaigns-ok` WRITE flow → verify.sh 16/16. The ref is already reproduced/validated at `/tmp` (matches the DOSBox capture). f0f1 (the deeper submenu) is only needed for the ec7e-OK → 7088 mission subscreen, a SEPARATE deliverable after this frame.

## DONE (STAGE 3 — THE SELECT PLAYER "OK" SAVE PATH + WRITE-ISOLATION HARNESS) — **CLICKING OK IN THE SELECT PLAYER LIST DIALOG NOW RUNS THE REAL PLAYER-PROFILE SAVE (read+checksum+write the `.FPL`), copies the chosen name into CURRENT PLAYER, and returns to the menu drawing "CURRENT PLAYER:<name>" — AE = 0/64000 vs genuine DOSBox references on BOTH native AND wasm, native↔wasm 0-diff, deterministic. Two new verify.sh flows → `bash tools/verify.sh both` = 15/15** (the 13 prior + **selplayer-ok** + **selplayer-ok-kkr**). 1 new patch (146). `re_out/fist.c` pristine (`61453e42`); `make check` = 146 patches apply, native + wasm green. NO REGRESSION: all 13 prior flows PASS. **`git status` stays CLEAN after a full verify run** (write-isolation).

**THE TWO FLOWS** (both exercise the reconstructed `6a9c`/`6ade`/`e9bd` save tail; refs are non-circular genuine DOSBox captures, write-isolated):
- **selplayer-ok** (`ref/selplayer_ok_native320.png`): SELECT PLAYER (row 74) → OK (205,128) on the default player **D** → menu with **"CURRENT PLAYER:D"**.  6a9c reads D.FPL (checksum matches → no reset), 6ade rewrites it BYTE-IDENTICAL.
- **selplayer-ok-kkr** (`ref/selplayer_ok_kkr_native320.png`): SELECT PLAYER (row 74) → select the **KKR** row (130,119, a838=4) → OK (205,128) → menu with **"CURRENT PLAYER:KKR"**.  Exercises the non-default name copy (`e9bd` from `STRSEG:(a838*0x10+0x3776)`) + the `.FPL` I/O for a different profile.

**PATCH 146 — the SELECT PLAYER OK tail, all asm-verified vs `re_out/fist_dat_image.bin`** (state-0xe handler 0xe97c, dispatched by e714's jumptable):
```
e98b: call cb74            ; modal .FPL list dialog (enumerate + cc33)
e98e: jae  0xe9a6          ; CF=0 (OK: cc33 sets CF=(a85d>=2), so a85d<2) -> save tail
e9a6: call e9bd            ; copy selected 8.3 name -> CURRENT PLAYER STRSEG:0xbf1, [0x6dad]=1
e9a9: lcall $0xf69,$0x6a9c ; open+read(0xe6)+checksum the .FPL   (FUN_1000_612c)
e9ae: lcall $0xf69,$0x6ade ; create+write(0xe6) the .FPL        (FUN_1000_616e)  <- REAL disk write
e9b3: jmp  0xe714          ; -> menu-idle (b78e=10)
```
Three faithful reconstructions:
- **FUN_0000_e9bd** (CURRENT-PLAYER name copy): reads DGROUP:0x77bc (the 8.3 name cb74 copied into di=0x77bc on OK), writes `ES:0xbf1` where `ES=STRSEG=word[DGROUP:0x70]` (≤8 chars, stop at '.'/' ', NUL-term), `movb [0x6dad],1`.  Ghidra DROPPED the ES=STRSEG base on the DESTINATION (raw host offset 0xbf1) — rebased to STRSEG:0xbf1.  (Source was already correctly based; [0x6dad]=1 = DAT_2000_2dad.)
- **FUN_1000_612c (6a9c)** + **FUN_1000_616e (6ade)** (.FPL read/checksum + create/write): both marshal INT-21h through the reg-file and set the DX slot (0xf0006) via `puRam000f0006 = &DAT_2000_37bc/37ce` — a HOST pointer stored into a 16-bit DOS register (R_DX = low16(hostptr)) → AH=3D/3C opened garbage.  asm `mov dx,0x77bc`/`mov dx,0x77ce`, R_DS=0x1c00 — store the real DOS offsets 0x77bc/0x77ce.  (**host-pointer-into-DOS-register reg-file artifact, patch-077 class** — the ~10-site class in CLAUDE.md; FUN_1000_6199 checksum was already faithful.)  Dispatch wiring: on OK (a85d<2) the e714 state-0xe97c case now runs e9bd + 612c + 616e; on CANCEL (a85d>=2) unchanged (menu-idle, patch 143).

**WRITE-ISOLATION POLICY (harness only — the engine keeps its faithful write behaviour).** 6ade is a REAL `INT 21h AH=3C/40` create+write of the player `.FPL` under FISTDATA/.  Both the port (`$FIST_DATADIR`, re_out/fist_dos.c `datadir()`) and DOSBox (mounted dir) resolve paths at the harness boundary, so isolation needs NO engine change:
- **`tools/verify.sh`** marks WRITE flows (`WRITEFLOWS`) and runs each against a FRESH `cp -a` scratch copy of `armoredfist/` (per (flow,target)); the repo is never mutated and every run starts from the pristine state → deterministic.  Read-only flows use the repo dir directly.
- **`tools/refcapture_ok.sh`** (2-click) / **`tools/refcapture_ok3.sh`** (3-click: item, row, OK) mount the SAME fresh `cp -a` copy for the DOSBox reference, so ref and port exercise the identical initial state.  Verified: `git status` clean after a full 15-flow verify AND after a ref capture.  (NB the KKR write path takes 6a9c's checksum-mismatch reset branch on the shipped file, so KKR.FPL is rewritten NON-identically — a deterministic, isolated side effect that does not affect the verified frame, which is drawn from STRSEG:0xbf1.)

**FRONTIER (honest, updated):**
1. **CAMPAIGNS OK → the campaign MISSION-SELECT screen (`ec7e`) — a LARGE unreconstructed screen, deferred.** CAMPAIGNS OK (e78c: `call e9bd; movb [0x6dad],1; lcall 6a9c; call ec7e; ...; lcall 6ade; ...; 7088; 4754 display-list build`) opens `FUN_0000_ec7e` = the campaign mission-select modal loop (its own template 0x2d30 + dispatch var b848).  Experimentally wiring it SEGVs immediately at fault-addr `0xfffff941` = **`DAT_2000_b941` (DGROUP:0xf941) dereferenced as a host pointer** — ec7e's campaign-name table walk `pcVar4 = *(word*)(&DAT_2000_b941 + b/37d6); *pcVar4` is base-lost, as are `ef5e` (the A.CAM/B.CAM… camera-file loader, heavy base-loss) and `f0f1`.  This is a full multi-base-loss + cdc4-lesson paint-method reconstruction on the scale of the SELECT PLAYER dialog (patches 137-139) — a separate multi-patch deliverable.  BATTLES OK → `e891` mission EDITOR is bigger still.
2. **Cursor ghost / scroll-release** — unchanged from prior (documented below).  (NB the OK flows return to the REBUILT menu, so the prior menu→dialog cursor ghost does NOT appear on the OK path — both OK frames are AE=0.)

## DONE (STAGE 3 — LIST-DIALOG SCROLLBAR: ARROW AUTO-REPEAT + TRACK PAGE-SCROLL) — **THE SCROLLBAR IS INTERACTIVE AND BIT-VERIFIED TWO WAYS: (1) holding the DOWN ARROW auto-repeats the list to its clamped bottom; (2) a single click on the TRACK below the thumb PAGES DOWN 8 rows — both AE = 0/64000 vs genuine DOSBox references on BOTH native AND wasm, native↔wasm 0-diff, deterministic. Two new verify.sh flows → `bash tools/verify.sh both` = 13/13** (the 11 prior + **battles-scroll** + **battles-page**). 2 new patches (144/145); `re_out/fist.c` pristine (`61453e42`); `make check` = 145 patches apply, native + wasm green. NO REGRESSION: all 11 prior flows PASS.

**THE FLOW — battles-page** (`ref/battles_page_native320.png`, md5 `f181262a` ≠ port `6227dbb4`, non-circular two-click DOSBox capture via `refcapture_click2.sh 160 100 172 115`): BATTLES → SELECT BATTLE list → ONE click on the scrollbar track below the thumb (172,115) → `ce37` press path → `d146` PAGE DOWN (+8 rows; `cef4` recomputes the thumb, dirty-marks, and clamps the selection into the window) → **CYPRUS2..INDIA2, red bar on CYPRUS2** (a83a=8, a838=8). A single click is a deterministic endpoint — no hold/tick dependence at all.

**PATCH 145 — `FUN_0000_ce37`, the SCROLLBAR TRACK/THUMB activate method, reconstructed (the cdc4 lesson fired exactly as predicted).** Clicking the track SIGSEGV'd: fault-addr **0xe8ed = the dialog element node 0xe8e9 + 4 dereferenced as a HOST pointer** (FIST_SEGV_BT: EIP in ce37, called from the 1eb4 hit-test activate dispatch). ce37 receives ax=event flags, bx=element node (DGROUP NEAR offset), si=the drained event/coord node with the click Y at [si+4] — Ghidra bound bx/si to raw int params (`param_2+4`/`param_3+4`), the identical base-loss class patch 142 fixed in ccde. The decompiled CONTROL FLOW was faithful and is kept 1:1; only the memory model is rebased (elem → `g_mem+0x1c000+(uint16)bx`; coord node ← `word[DGROUP:0x18e2]`, the ccde model). asm-verified 0xce37..0xcef3: press (ax&0xa) → a86d=8, a86c=0xff, then clickY < y0+a86f+2 → `d135` PAGE UP / clickY > y0+a86f+8 → `d146` PAGE DOWN / else THUMB GRAB (anchor a871, a86c=0, dirty); drag (a86c==0) → remap clickY through the track via the 32-bit denominator a87d (patch 142) → new a83a + `cef4`; release (ax&0x14) → a86d=0 + dirty.

**THE FLOW — battles-scroll** (`ref/battles_scroll_native320.png`): BATTLES (row 100) opens the SELECT BATTLE `.FSG` list (47 entries), then a PRESS-AND-HOLD on the down-scroll arrow (172,135) drives the game's own timer-throttled auto-repeat until the view CLAMPS at the bottom (offset 39 → **UKRAINE1-8**, red selection bar on UKRAINE1). The clamp is a **deterministic endpoint independent of the exact iteration/tick count** (any ≥39 scroll steps land the same frame), so native, wasm, and DOSBox all settle on the identical frame despite the modal loop's wall-clock-bounded iteration count. Ref captured **mid-hold** (button still down) via the new `tools/refcapture_scroll.sh` (screenshots during the hold so the arrow's pressed state matches; port md5 `26c1cf67` ≠ ref md5 `6300db4e`, non-circular).

**PATCH 144 — root-caused by runtime-tracing the modal-loop dispatch (all asm-verified vs `re_out/fist_dat_image.bin`).** The cc33 modal loop dispatches its per-frame handler slot **a85b** (DGROUP:0xe85b) every frame via `ax=[0xe85b]; call ax` (asm cc9f). a85b is a **WORD near code-offset** (cc33 `mov WORD PTR [0xe85b],0xd0d7`; the down-arrow activate `cd8f` sets `mov WORD PTR [0xe85b],0xd103`). **Ghidra typed `DAT_2000_a85b` as undefined4 (DWORD) — which OVERLAPS a85d/a85e — so the dispatch read `a85b_word | (a85d,a85e)<<16` (e.g. 0x00ffd103) and `fist_icall_far` mapped that bogus seg:off to garbage → the no-op trap trampoline.** d0d7 (idle) is an empty function so it looked identical to the no-op → the 11 working flows were unaffected, but the down-arrow's 0xd103 handler ALSO trapped → the scroll never dispatched (runtime-confirmed: 0 real d103 calls; the dispatch always saw a85b=0x00ff1da1). Fix: retype a85b to `undefined2` (word) — the `(uint32_t)a85b` dispatch then zero-extends to linear 0xd0d7/0xd103 and resolves via the fmap — and init a85b to the near offset 0xd0d7 (asm) instead of a host pointer. Then **recover FUN_0000_d103** (asm 0xd103..0xd134, never a Ghidra function boundary): `d103: ax=[0x452]-[0xe885]; cmp 8; jbe ret` (throttle) then falls through into the tail Ghidra DID recover as `FUN_0000_d107(param_1)` — so `d103 = FUN_0000_d107((uint16_t)([0x452]-[0xe885]))`, faithful, no new logic — and register 0xd103 in the fmap. No existing flow holds the down-arrow, and retyping a85b leaves the idle dispatch (d0d7, empty) byte-identical → zero regression.

**FRONTIER (updated):**
1. **Cursor ghost** (direct SELECT PLAYER row-select AE=35) — unchanged; the dialog backdrop SAVE bakes the cursor in; needs the asm-verified cursor hide/erase bracketing the save (`2d6d`/`2e12` or the INT-33h hide counter) before touching (all 13 flows' final cursor pixels are bit-verified — high regression risk).
2. **OK path → deeper screens** — SELECT PLAYER OK `e9a6→e9bd (copy name to STRSEG:0xbf1, `[0x6dad]:=1`)→0f69:6a9c/6ade` is **player-profile FILE I/O**, not a text refresh: 6a9c OPENs+READs the selected `.FPL` (INT 21h AH=3D/3F, 0xe6 bytes to 0x77ce, checksum vs [0x78b2]) and **6ade CREATES+WRITES it (AH=3C/40)** — a real filesystem side effect (write to `armoredfist/`), so it is a determinism/hygiene hazard, not the clean menu-refresh the handoff implied. CAMPAIGNS OK → `[0x6dad]:=1; 6a9c; ec7e; 7088` = campaign MISSION-SELECT (a large new screen); BATTLES OK → `e891` mission-editor build. Each needs the cdc4-lesson dispatch-target enumeration first.
3. **Scroll residuals** — the UP arrow auto-repeat (`d0d8`) and the THUMB DRAG (ce37 drag path, now reconstructed) are dispatch-resolvable but not yet bit-verified as flows. And a REAL divergence found while flow-hunting: **releasing the down-arrow after the clamp resets the port's scroll state (a83a/a838 → 0) while DOSBox keeps the scrolled view** (port-vs-ref AE=561 released vs AE=0 mid-hold) — the release path (cd8f release branch / the a881 toggle / an event-order artifact) needs asm-verified root-causing before a released-state scroll flow can be added.

## DONE (STAGE 3 — OK/CANCEL DIALOG CLOSE) — **THE DIALOG CLOSE PATH IS FIXED AND BIT-VERIFIED: clicking CANCEL in a list dialog closes it and restores the plain main menu, AE = 0/64000 vs genuine DOSBox references on BOTH native AND wasm, native↔wasm 0-diff, deterministic. Two new verify.sh flows → `bash tools/verify.sh both` = 11/11** (the 9 prior + **cancel** + **battles-cancel**). 1 new patch (143). `re_out/fist.c` pristine (`61453e42`); `make check` = 143 patches apply, native + wasm green. NO REGRESSION: all 9 prior flows PASS.

**THE TWO FLOWS (both exercise the modal-exit → return-to-menu path, one per list-dialog front-end):**
- **cancel** (`ref/cancel_native320.png`, md5 `b522abc1`): CAMPAIGNS (row 87) opens the SELECT PLAYER `.FPL` roster via **cb74**, then a click on CANCEL (205,140) closes the dialog → the plain menu is restored (final cursor rest = the CANCEL position, so no ghost).
- **battles-cancel** (`ref/battles_cancel_native320.png`, md5 `e06b7483`): BATTLES (row 100) opens the SELECT BATTLE `.FSG` list (47 entries) via **cb7c**, then CANCEL → menu. Same destination frame (both refs are pixel-identical to the port frame `f147108c`), but a fully distinct enumerate/dialog/front-end path.
Determinism: 5/5 serial single-md5 on the normal build AND 3/3 on the `-ftrivial-auto-var-init=pattern` build (byte-identical output), rc=0 throughout.

**PATCH 143 — the CANCEL return-to-menu SIGSEGV, root-caused with the patch-127 discipline (pattern-init deterministic repro + `FIST_SEGV_BT`; fault-addr `0x3776`, EIP in cb74).** The prior handoff's premise ("the close path re-runs the boot reloc appliers `FUN_1000_5214→f842/f7c3`") was a MISDIAGNOSIS — 5214 fires once at BOOT (from `FUN_0000_0007`/app_entry), never on the close path; the crash was in **the shared cb74/cb7c tail itself** (cb74 and cb7c are two entry points into ONE body from cb82, differing only by the `[0xe82e]` flag). asm-verified vs `re_out/fist_dat_image.bin`:
- `cba1 jb 0xcbcb` — after the modal loop `cc33` returns, **CF=1 (CANCEL, a85d>=2) SKIPS the selected-name copy**. Ghidra dropped the branch (folded `bVar7 = false`) so the copy ran unconditionally on CANCEL.
- `cba7 mov es,[0x70]; cbab mov si,[0xe838]; shl si,4; add si,0x3776` — the copy source is **STRSEG:(a838*0x10+0x3776)**; Ghidra base-lost it to the raw host pointer `0x3776` → SIGSEGV. The dest `di=[0xe855]` is the caller's DGROUP dest (param_7, unchanged across the call).
- `cb82 push [0x452]` / `cbc0|cbcb pop [0x452]` — the click-timestamp DGROUP:0x452 is SAVED/RESTORED across the dialog; Ghidra invented `DAT_1000_c452 = unaff_CS` (uninitialised garbage — why the crash was flaky ~60% under real init and only deterministic under pattern-init; plain gdb masked it entirely).
Fix: thread cc33's carry (`g_fist_cf`, set by the patch-142 exit to `a85d>=2`), skip the copy on CANCEL, base the copy on STRSEG, save/restore c452 faithfully. Applied to both cb74 and cb7c. `cbd4` (mission-load path) carries the same idiom but is unreached by any covered flow — documented latent debt. NB the e714 dispatch cases ignore cb74's return (they set `b78e=10` = menu-idle regardless), so CANCEL lands back in the verified menu idle loop.

**FRONTIER (honest, updated):**
1. **Cursor ghost on above-panel clicks (SELECT PLAYER direct row-select = AE 35, bbox x159-166 y73-79 — measured this iteration vs a fresh genuine two-click DOSBox ref).** NEW FACTS: the erase is NOT generally broken — a move-only script on the plain menu leaves ZERO ghost; the ghost appears ONLY across the menu→dialog transition. FBTRAP on the ghost region shows the writers are the cursor draw/restore `0db8`/`0e3b` AND the dialog save/restore `0fbc`/`0fe4` — i.e. the dialog backdrop SAVE (`e38a(0xf790)` → 0fbc) captures the frame WITH the cursor arrow baked in at the click position, and the per-frame restore then repaints those pixels forever. The faithful fix = the original's cursor hide/erase bracketing the backdrop save (the `2d6d`(hide+clip)/`2e12`(show) pair the driver uses around blits, or the INT-33h hide counter `0x5d6..` path) — needs the exact engine sequence asm-verified before touching it, since every existing flow's final cursor pixel is bit-verified.
2. **OK path → the deeper screens.** CANCEL is verified; OK (a85d<2) proceeds to the caller's tail: SELECT PLAYER `e9a6: call e9bd (copy name); 0f69:6a9c (refresh CURRENT PLAYER); 0f69:6ade`; CAMPAIGNS `e78c tail: movb [0x6dad],1; lcall 6a9c; call ec7e; 7088` = the campaign MISSION-SELECT; BATTLES `e891` → the mission-editor build. Each is a new-screen reconstruction (enumerate + reconstruct every newly-reached dirty-walk/activate target first — the cdc4 lesson).
3. **Scroll arrows (cd8f→d103):** unchanged — `FUN_0000_d103` (timer-throttled scroll stepper) is not a recovered function and cc33's `a85b` init is a host-pointer trap-to-no-op; needs a seed/fmap entry (patch-121 0x19ae precedent) + faithful a85b dispatch + fixed-tick hold determinism.

## DONE (STAGE 3 — LIST-DIALOG INTERACTIONS = ROW-SELECT) — **CLICKING A ROW INSIDE A LIST DIALOG NOW MOVES THE RED SELECTION BAR, BIT-IDENTICAL: AE = 0/64000 vs genuine DOSBox references on BOTH native AND wasm, native↔wasm 0-diff, deterministic (normal + pattern-init byte-identical, crash-free). Two new verify.sh flows → `bash tools/verify.sh both` = 9/9** (mainmenu+about+settings+review+selplayer+battles+campaigns+campaigns-select+battles-select). 1 new patch (142) + a two-click DOSBox capture (`tools/refcapture_click2.sh`). `re_out/fist.c` pristine (`61453e42`); `make check` = 142 patches apply, native + wasm green. NO REGRESSION: the 7 prior flows PASS.

**THE TWO FLOWS (both the row-select interaction on a DIFFERENT dialog, non-circular refs via a two-click XTest DOSBox capture):**
- **campaigns-select** (`ref/campaigns_select_native320.png`, md5 `c2bdaf7b`): CAMPAIGNS (row 87) opens the SELECT PLAYER `.FPL` roster, then a click at row (130,119) selects **KKR** — the red bar jumps D→KKR.
- **battles-select** (`ref/battles_select_native320.png`, md5 `c0a23f4e`): BATTLES (row 100) opens the SELECT BATTLE `.FSG` list (47 entries), then a click at row (130,119) selects **AZER5**.
Both use CAMPAIGNS/BATTLES rather than SELECT PLAYER directly because their menu-item pixel (row 87/100) is INSIDE the dialog panel, so the dialog paint covers the menu-click cursor — see the cursor-ghost frontier below.

**PATCH 142 — the whole dialog-internal-click path, all asm-verified vs `re_out/fist_dat_image.bin`:**
- **THE LINCHPIN — `FUN_1000_3566` + the hit-test `FUN_0000_1eb4`.** The hit-test synthesizes LEAVE (0x200) / ENTER (0x400) hover events by writing them into the QUEUE NODE that `3566` allocates (asm 0x1f20/0x1f39 `mov [bx],0x200/0x400`, where **bx = the 3566 return**). Patch 128 wrote them into `oldfe0c` (the previous hover ELEMENT) instead — clobbering a dialog CONTAINER node's word0 (cls=4 → 0x400), so the hit-test never recursed into the container's buttons/rows (it treated the broken container as a leaf). `3566` now publishes its BX in the shim global `g_fist_ev_node`; the hit-test writes the event fields there → the recursion reaches the buttons/rows and dispatches their `[elem+0x4660]` activate methods.
- **`FUN_0000_ccde` (list-row activate):** base-lost coord reads rebased to the DGROUP model — si = the drained event node `word[DGROUP:0x18e2]` (Ghidra wrongly bound the si operand to param_3=base); `row = byteTab[0xe88f + ((clickY-listY0)>>1)] + a83a`; on change `a838 = row; d066()` (mark the list dirty).
- **`a87d` (macro width):** the scroll-range DENOMINATOR is a 32-bit DWORD (asm b6a4 `mov dword[0xe87d],0x10000`, cef4 `div DWORD PTR`); Ghidra typed it undefined2 (WORD) → `0x10000` truncated to 0 → cef4 **divide-by-zero (SIGFPE)** on the first selection. → uint32_t.
- **`a8ee`/`a8f4`/`a8f5` (macro width) — the "no repaint" root cause:** these are BYTE per-element dirty-masks (asm `mov BYTE PTR`; 209e shifts them each frame, on carry-out it repaints the element). Ghidra typed them undefined2 (WORD), so `a8ee = 3` wrote a WORD that **clobbered the adjacent child-list HEAD word** at `base+di+1` → the container child-walk found head==0 (no children) → the list never repainted on a selection change. → uint8_t.
- **`FUN_0000_209e` / `FUN_0000_208a` (paint base):** 208a read its walk base from the GLOBAL `DAT_1000_fafe`, which holds the MENU base (f7b8) while the modal dialog is up (the cursor redraw updates fafe) → the repaint walked the wrong element list. 209e publishes the live walk base `bp` in the shim global `g_fist_paintbp` (the asm uses the preserved bp register); 208a reads it.
- **`FUN_0000_cf6b` (OK/CANCEL activate), `FUN_0000_cc33` (modal-loop exit), `FUN_0000_20d6`/`FUN_0000_2043` (container paint child-setup walkers):** reconstructed for the button-click/close path (base-loss + dropped saved-handle + SIGFPE-adjacent). These fire on OK/CANCEL; the OK/CANCEL flow itself is NOT YET bit-verified — see the frontier.

**FRONTIER (honest):**
1. **Direct SELECT PLAYER row-select leaves a CURSOR GHOST (AE=35).** Clicking SELECT PLAYER directly (row 74, ABOVE the dialog panel at y80) then a row leaves a 35-px cursor arrow at (160,74): the cursor drawn during the menu click is baked/not-erased across the menu→dialog transition (its saved background is lost when the dialog opens). CAMPAIGNS/BATTLES (rows 87/100, inside the panel) avoid it because the dialog paint covers that pixel. Fixing it = a cursor hide/erase before the dialog backdrop save (benefits any above-panel interaction).
2. **OK / CANCEL close → the menu-rebuild applier cascade.** CANCEL stores a85d≥2, cc33 exits (CF=1), and the return-to-menu path re-runs the boot reloc appliers `FUN_1000_5214→f842/f7c3` (base-lost, patch-091/097/127 class) → SIGSEGV. Needs the same faithful-install-or-elide treatment as patch 127 on this path.
3. **Scroll arrows (cd8f→d103):** the arrow activate `cd8f` is already reconstructed (sets `a85b=0xd103` auto-scroll handler), but `FUN_0000_d103` (the timer-throttled scroll-down) was NOT recovered by Ghidra as a function and cc33's `a85b` init is a host-pointer (traps to no-op) — needs seeding d103/d135/d146 + a near-offset a85b init; plus the hold-duration determinism.

## DONE (STAGE 3 — SCREENS 6 & 7 = BATTLES + CAMPAIGNS) — **BOTH REMAINING MENU ITEMS ARE BIT-IDENTICAL: AE = 0/64000 vs genuine DOSBox references (`ref/battles_native320.png`, `ref/campaigns_native320.png`) on BOTH native AND wasm, native↔wasm 0-diff, deterministic (normal + pattern-init byte-identical, crash-free). Added as verify.sh flows 6 & 7 → `bash tools/verify.sh both` = 7/7 (mainmenu+about+settings+review+selplayer+battles+campaigns). ALL SEVEN main-menu items now open + render bit-verified.** 2 new patches (140/141); `re_out/fist.c` pristine (`61453e42`); `make check` = 141 patches apply, native + wasm green. NO REGRESSION: the 5 prior flows PASS (menu `3a6ff1c5`, selplayer `8c72a1f7`, …), default boot unaffected.

**RECON — the e714 tail jumptable (image 0xe77a, 8 word entries; the screen-state var `DGROUP:0xf78e` = a byte offset).** Dumped the 8 targets: state 0x0→0xe78c (CAMPAIGNS), 0x2→0xe87a (BATTLES), 0x4→0xe93c (REVIEW), 0x6→0xe94f (SETTINGS), 0x8→0xe78a (bare ret), 0xa→0xe753 (menu idle), 0xc→0xe969 (ABOUT), 0xe→0xe97c (SELECT PLAYER). BATTLES + CAMPAIGNS were the two unwired states; each dispatched into the SAME SELECT-list dialog machinery reconstructed for SELECT PLAYER (patches 137-139), so each needed ONLY a faithful dispatch case — no new paint methods (checked: the cc33 dirty-walk targets are all shared with selplayer's `.FPL` dialog, so no cdc4-class base-lost paint method to reconstruct).

**BATTLES (patch 140) — SELECT BATTLE `.FSG` list.** asm-verified vs `re_out/fist_dat_image.bin` (0xe87a = FUN_0000_e87a): `call e39f; mov si,0x4509 (STRSEG:0x4509="SELECT BATTLE"); mov di,0x79a9 (out DGROUP:0x79a9); mov eax,0x4753462e (".FSG"); call cb7c; jae 0xe891 (CF=0/OK → mission-editor build); jmp 0xe714 (CF=1/CANCEL → menu)`. `cb7c` is the SAME list-dialog front-end as `cb74` (patch 137) — it differs ONLY by `movw [DGROUP:0xe82e],0` (vs cb74's 0xffff) and reuses the entire enumerate/sort/cc33-modal/paint chain (b756 enumerate, b6dd sort, cc33 modal, ccca/b652 title bar, ccd8/b5a4 list box + red selection, cd1c/cd70/cf8e/cf44 buttons, cdc4 scrollbar, MGAVIDEO writers). The dispatch case = `e39f(); cb7c(0x4753462e, 0,0,0,0, 0x4509, g_mem+0x1c000+0x79a9)`. Renders 47 `.FSG` files (AZER1-7/CYPRUS1-7/INDIA1-7/SAUDI1-7/SYRIA1-7/TRAIN1-4/UKRAINE1-8, DOSBox 8.3-ascending) + scrollbar + a `SIDE: WEST` label + OK/CANCEL, over the M1-Abrams/Hind menu-art backdrop. Ref `ref/battles_native320.png` = genuine DOSBox click at row 100 (`tools/refcapture_click.sh 160 100 45 8`, md5 `bc67bd1c` ≠ the port md5 → non-circular).

**CAMPAIGNS (patch 141) — a player must be chosen first → SELECT PLAYER `.FPL` roster.** asm-verified vs 0xe78c (FUN_0000_e78c): `call e39f; cmpb [DGROUP:0x6dad],1; je 0xe7c6 (already have a player → straight to campaign mission-select); mov si,0x451a; mov di,0x77bc; mov eax,0x4c50462e (".FPL"); call cb74; ...`. On a fresh boot `[0x6dad]==0`, so CAMPAIGNS opens the SAME SELECT PLAYER roster dialog as clicking SELECT PLAYER directly (`cb74(".FPL", si=0x451a, di=0x77bc)`, identical to state 0xe/e97c) — cursor at row 87. The dispatch case reconstructs e78c's first portion faithfully: `e39f(); if(*(uint8_t*)(g_mem+0x1c000+0x6dad) != 1) cb74(0x4c50462e, 0,0,0,0, 0x451a, g_mem+0x1c000+0x77bc)`. Ref `ref/campaigns_native320.png` (md5 `2d3e1ca9`, DOSBox click at row 87). CAMPAIGNS's first frame IS the SELECT PLAYER dialog (title "SELECT PLAYER", roster D/GAMESWIN/JO/JOE/KKR/PP/TRT, NEW/OK/CANCEL) — this is the real game behavior (the campaign screen requires a selected player), and it independently exercises the state-0x0 dispatch path.

**Both dialogs are MODAL** (cc33 spins until OK/CANCEL), so the captured frame is the FIRST FRAME before any OK. The deeper flows are later, separate deliverables: **the BATTLES mission EDITOR** = e87a's tail after CF=0/OK (`FUN_0000_7088` + `e43f`; `[DGROUP:0xce] |= 0x22`; `[0x1594]/[0x1598]` device methods; `be0e/e459`; `FUN_0000_4754` = the display-list build; `459a`; `[0xe8]/[0x594]/[0x59c]` device dispatches; `1631/be86/616e/6220`) → the in-engine LEVEL/MISSION editor UI; **the CAMPAIGNS mission-select** = e78c's tail after a player is picked (`movb [0x6dad],1; lcall 0f69:0x6a9c (refresh CURRENT PLAYER); call ec7e; 7088`). Neither editor/mission-select surface is touched by these patches.

## DONE (STAGE 3 — SCREEN 5/N = SELECT PLAYER) — **THE SELECT PLAYER LIST DIALOG IS BIT-IDENTICAL: AE = 0/64000 vs the genuine DOSBox reference `ref/selplayer_native320.png` on BOTH native AND wasm, native↔wasm 0-diff, deterministic (serial runs single md5 `8c72a1f7`, normal + pattern-init builds). Added as the 5th verify.sh flow → `bash tools/verify.sh both` = 5/5 (mainmenu+about+settings+review+selplayer). BOTH prior blockers root-caused + fixed by ONE patch (139).** NO REGRESSION: 4 prior flows PASS, `re_out/fist.c` pristine (`61453e42`), `make check` = 139 patches apply, native + wasm green.

**HEADLINE — Blocker 1 (flat-olive, no bg composite) and Blocker 2 (flaky ~1/8 SEGV) were the SAME defect: `FUN_0000_cdc4`, the scrollbar-TRACK paint method, ran base-lost with all four MGAVIDEO method-vector args dropped.** Its `[0x60a]`=1091 fills executed with codegen-GARBAGE bx/al → a WILD inverted rect (rect[168,19,133,184] → rows underflow to 65501) filled colour 0x30 (olive) that WIPED the whole screen every frame (fb → 15 indices) — and because the wild write's target depended on the garbage bx, it SEGV'd whenever the garbage landed outside g_mem (~1/8; crash-free under `-ftrivial-auto-var-init=pattern` where the 0xAAAA pattern lands inside — the classic uninitialised-garbage tell of patch 127/128). The prior "only ONE 1091 fires" diagnosis was an instrumentation artifact. Found by instrumenting 1091 with rect+caller (`__builtin_return_address`) on the selplayer flow. Patch 139 (all asm-verified vs `re_out/fist_dat_image.bin` + `re_out/fist_mga_image.bin`):
- **cdc4 reconstructed** (asm 0xcdc4..0xce36, the 209e-dispatch/DGROUP:0x3e08-recover model of patch 138): box outline `[0x61a]`(elem+4, 2); scratch rect @DGROUP:0xe873 (a873=y0+1, a877=a873+[0xe86f] thumb-top, a875=x0+1, a879=x1-1, a87b=y1-1); fill-ABOVE-thumb `[0x60a]`(0xe873, colour 0); thumb sprite `[0x6b4]`(id 0x108+[0xe86d]); fill-BELOW `[0x60a]`(0xe873, 0). **NB a875 is a WORD store (asm `mov [0xe875],ax`); Ghidra's `iRam0002a875` int (4-byte) accessor ZEROED the adjacent a877** → a second wild fill (the black top-strip) — the store-width class of patches 104/105/123.
- **0f1a (MGAVIDEO save-buffer RESERVE, fist_mga.c) reconstructed** — why the menu-item bars persisted behind the dialog: the cc33 dialog restores the TERRAIN-ONLY menu backdrop via the e38a(0xf790) save / e39f restore pair (both already arg-threaded by patches 112/131), but 0f1a's allocator `lcall [DGROUP:0xcc]` is UNINSTALLED in our port (the patch-049/134 device-alloc gap class) → no save buffer → the descriptor flag sign-bit never set → 0f3a save-blit and 0fe4 restore-blit both no-op'd. Fix: compute the size faithfully (the asm byte-multiply `mul cl; add ah,ch`), resolve `FUN_1000_182a` directly (patch-049 pattern) to reserve the buffer into the save descriptor ({seg, flag 0xa0}); REUSE an already-reserved buffer (flag bit7) so the per-frame menu save (e9f0 → e38a) does not leak (the raw 182a free+realloc goes through the 184b bit7 path, an unimplemented MEMMGR follow-on in our port).
- **thunk_FUN_1000_5531 (DGROUP-string glyph loop, asm 0x1552b)** — b631's player names rendered as identical BLOCKS: the loop dropped the AL char arg to the per-glyph blitter `[0x684]`. Thread al (mirror of 5591/patch 115).
- **MGAVIDEO 0x10d1 (masked highlight fill, fist_mga.c) reconstructed** — the red selection bar on the selected row was missing: Ghidra dropped the whole body ("Subroutine does not return", the 1091/patch-060 class). asm 0x10d1..0x1113: per pixel `cmp ah,es:[di]; je skip; stos al` = write AL except where the pixel equals AH. b5a4's `[0x60e]` call threaded to pass the full colour word ax=0x000e (AL=0xe red fill, AH=0 preserves the black glyph pixels).

**Verification (honest, non-circular):** ref `ref/selplayer_native320.png` = the genuine DOSBox capture committed at 1164cb1 (md5 `7c6a1a16…`, predates patch 139). Native AE=0, wasm AE=0, native↔wasm 0 differing bytes; serial runs single-md5 stable; the pattern-init build is also AE=0 and 12/12 crash-free (the flake is fixed at root, not masked). NB: heavily parallel-loaded runs can capture a frame with the cursor 29px mid-settle — a wall-clock FBDUMP-watchdog CAPTURE-time artifact (the dump fires at wall-clock 22s regardless of engine progress), not engine nondeterminism; the serial verify regime is byte-stable.

**NEXT SCREEN:** CAMPAIGNS (row87, state 0x0) / BATTLES (row100, state 0x2 → the `.FSG` SELECT BATTLE list → the mission EDITOR), the SELECT PLAYER interactions (row click-select, scroll arrows, NEW/OK/CANCEL), and the SETTINGS toggle interactions.

<details><summary>SUPERSEDED (patch 138 — render chain reconstructed, 2 blockers open)</summary>

## SUPERSEDED (STAGE 3 — SCREEN 5/N = SELECT PLAYER, render chain) — **THE FULL DIALOG RENDER CHAIN IS RECONSTRUCTED + DISPATCHES CRASH-FREE (title bar, list box + 7 sorted names + selection, 5 buttons, 2 scroll arrows, the container full-paint walker); NOT yet AE=0 — the menu-art BACKGROUND does not composite, so SELECT PLAYER is NOT added to verify.sh.** 1 new patch (138). **NO REGRESSION: `bash tools/verify.sh both` = 4/4 (mainmenu+about+settings+review, native↔wasm 0-diff, AE=0), `re_out/fist.c` pristine (`61453e42`), `make check` = 138 patches apply, native + wasm green, default boot rc=124 (alive, not SEGV).**

**WHAT IS DONE (patch 138, all asm-verified vs `re_out/fist_dat_image.bin` + `re_out/fist_mga_image.bin`).**  The linchpin: every list-dialog element paint method is dispatched by the **209e dirty-walk via a NEAR call with the element node live in BX**, but the __allregs C dispatch (`fist_icall_near(...)()`) passes NO args → the paint methods read **codegen garbage**.  209e *does* publish the current element to **DGROUP:0x3e08** (as every working paint method reads it); recover it there.  Element paint methods reconstructed (all recover elem from 0x3e08 + thread the dropped MGAVIDEO method-vector args):
- **ccca → b652** (TITLE BAR): rect=elem+4=[y0,x0,y1,x1]; `[0x60a]`=1091 SOLID fill (colour 8, panel) + `[0x61a]`=11db box OUTLINE (colour 1) + `[0x61e]`=119f box-inset1 (colour 4) + 52d1 setpos (fixed the int*→int16 pointer-scaling) + 5591 title string.
- **ccd8 → b5a4** (LIST BOX): reconstruct the whole visible-row loop Ghidra lost (item index=BX, scroll/cap-at-8 count, name-ptr scaling, the `loop` terminator): fill(8)+border(2); per row the name (STRSEG:0x3776+idx*0x10 via b631, '.'→NUL) + the SELECTION highlight `[0x60e]` colour 0xe on the selected index.
- **cd1c / cd70** (buttons/scroll arrows): box outline (2) + icon sprite `[0x6b4]`=26a1 (id 0xf8+/0x118+[0xe881]).  **cf8e / cf44** (themed buttons): box outline (0) + a state-indexed sprite (cf8e via [0xe860]/[0xe866]/[0x6db4] tables; cf44 via the element's own data bp+di+1 → [0xe8a9]).
- **b631** (name draw): fix the STRSEG name-source base-loss.
- **MGAVIDEO box/rect-OUTLINE cluster `119f`/`11ba`/`11db` + the folded line primitives `1216`(horiz)/`124b`(vert)** (fist_mga.c): the rect arrives as a DGROUP near OFFSET in BX (typed int* by Ghidra → wild deref); rebase to `g_mem+0x1c000+bx` + the fb/rowtable/pitch bases from the DGROUP:word[0x724] descriptor (same class as 1091/patch 060, 1677/patch 136).  11db draws the 4 edges with the rect fields temporarily swapped.
- **1e73** (container FULL-PAINT child walker, the `[+0x4872]` method-slot analog of 208a): reached when cc33's modal loop dispatches a paint event → 1e4b → 1e88 → the container's `[+0x4872]`=1e73.  Reconstructed in the DGROUP-offset model (the child `[+0x4872]` leaf methods are all no-ops for a paint event: 0x2104=`stc;ret`, cffc returns for a paint event) — the crash was 1e73's base-lost host-pointer walk, not a leaf method.

**THE FRONTIER — TWO DEEP cc33-LIST-DIALOG BLOCKERS (why AE≠0 ≈ 61811, NOT a verify flow):**
1. **The menu-art BACKGROUND does not composite behind the dialog.** In the plain menu, ABOUT/SETTINGS/REVIEW the M1-Abrams/Hind title scene shows (210/57/88 fb indices); clicking SELECT PLAYER, the final frame is **flat olive (15 indices)** — the tank art AND the (correctly-painted, colour-8) dialog panel are both absent, only the buttons/scroll-arrows survive.  Ruled out (diagnosed): it is **NOT** a fullscreen 1091 fill (only ONE fires, the harmless initial cursor-erase clear on an empty fb — same count in every flow); **NOT** a palette collapse (the raw fb genuinely has 15 indices, confirmed with FIST_PALNOW); **NOT** a corrupted menu-bg descriptor (`DGROUP:0xf6cc.word0 = 0x55ac`, identical in plain-menu and selplayer, so the 0340 art-blit decodes from the same valid resource).  The three-layer per-frame composite (cursor bp=0x0 → menu bp=0xf7b8 with 0340 art + e9f0 box/items + 7 items → dialog bp=0xe8e9) all runs, and the panel colour-8 fill *lands* (7016/7854 px after b652), yet the final capture is flat — pointing at a **paint-composite / page / e38a-save-restore-timing** interaction specific to the cc33 modal loop (7088 modals do NOT hit this).  Root cause not yet isolated — needs tracing the cc33 modal loop's paint ordering + the e38a/e39f save-restore (e38a's `[0x5fa]`/`[0x5fe]` args are still dropped) vs. the 0340 bg blit's target page.
2. **A flaky (~1/8, garbage-dependent) SEGV in the SELECT PLAYER click path under real init** (crash-free under `-ftrivial-auto-var-init=pattern`, so it is an uninitialised-value read in the newly-reached cc33 machinery — likely a state-derived sprite id in cf8e/cf44 → 26a1/260c/26de OOB, or the modal-loop event dispatch; a hard bt was not caught in 25+ runs this iteration).  It is **isolated to the selplayer click** — the 4 verify flows + the default boot are deterministic and unaffected.

NEXT: isolate the background-composite root (cc33 paint ordering / e38a save-restore / page) → drive the menu art + dialog to a stable complete frame → resolve the flaky uninitialised-value SEGV → AE→0 vs `ref/selplayer_native320.png` → add the `selplayer` flow → 5/5.

</details>

<details><summary>SUPERSEDED (patch 137 — dialog opens + enumerates)</summary>

## IN PROGRESS (STAGE 3 — SCREEN 5/N = SELECT PLAYER, the first file-enumerated LIST dialog) — **THE DIALOG OPENS + ENUMERATES CORRECTLY; the RENDER chain is the frontier (NOT yet AE=0, NOT a verify.sh flow).** 1 new patch (137) + the shim FILEMGR find-first/find-next.  Clicking SELECT PLAYER (menu row 74 → screen-state 0xe → e714 dispatch `_tgt 0xe97c = e39f(); cb74(".FPL",…)`) now opens the modal list dialog and **enumerates the seven `*.FPL` player files in the exact DOSBox order — D, GAMESWIN, JO, JOE, KKR, PP, TRT** (verified vs the genuine reference `ref/selplayer_native320.png`). **NO REGRESSION: `bash tools/verify.sh both` = 4/4 (mainmenu+about+settings+review, native↔wasm 0-diff, AE=0), default boot rc=0, `re_out/fist.c` pristine (`61453e42`), `make check` = 137 patches apply, native + wasm green.**

**WHAT IS DONE (patch 137, all asm-verified vs `re_out/fist_dat_image.bin`).**
- **The order source (recon):** SELECT PLAYER (state 0xe → handler 0xe97c) calls `cb74(".FPL")`; BATTLES (state 0x2 → 0xe87a) calls `cb7c(".FSG")` and leads into the mission EDITOR (huge) — SELECT PLAYER is the smaller/simpler list (7 `.FPL` vs 47 `.FSG`). `cb74/cb7c → b756` enumerate via INT 21h AH=4E/4F, then `cc33` runs the modal list dialog. The engine's OWN bubble-sort `b6dd` (asm 0x1b6dd) sorts the names ascending, so the displayed order is independent of the shim's find-first/next order.
- **Shim FILEMGR find-first/find-next** (`re_out/fist_dos.c`, INT 21h AH=4E/4F — not a patch): enumerates the real `armoredfist/(FISTDATA)` files matching the DS:DX 8.3 wildcard, **sorted ascending by 8.3 name — reproducing DOSBox's `DOS_Drive_Cache` sorted enumeration** (deterministic, identical native↔wasm, verified vs the reference). Fills the DTA at 0x0080:0 (name at +0x1e, size at +0x1a).
- **Engine reconstructions (patch 137):** `cb74/cb7c` (the `.FPL`/`.FSG` pattern is a 32-bit store `mov [0xe845],eax` that Ghidra truncated to a WORD ".F" → 0 matches; fixed); `b756` (the enum loop's three base-losses — DTA seg from the reg-file ES, list dest STRSEG:0x3776+cnt*0x10, name src DTA:+0x1e); `b6dd` (the bubble sort — a83e/a840 were host pointers → non-terminating); `cc33` (the modal loop — g_fist_cf threading + cooperative pump, was mis-exiting to the menu); `2006`/`208a` (the recursive child-list walkers that fall into the 201a/209e walks — same base-loss class as 201a/209e patch 056/057, reached for the first time by this dialog's container element); `1d84`/`1d8e` (the dialog-position + mark-dirty walker — whole walk was host-pointer-based + WORD/int mis-scaled).

**FRONTIER — THE LIST-DIALOG RENDER (the remaining work, comparable to the whole SETTINGS reconstruction).** The dialog OPENS + ENUMERATES + reaches the display-list PAINT phase, but the render chain is still base-lost/unthreaded and NOT yet reconstructed: the **title-bar drawer `ccca`→`b652`** (drops the rect arg into the driver box/border methods `[0x60a]`/`[0x61a]`/`[0x61e]`) + the **MGAVIDEO rect/line writers `119f`/`11ba`/`11db`/`1216`/`124b`** (bx=DGROUP rect offset, needs the `g_mem+0x1c000+bx` rebase like `1091`/patch 060), then the **list-box text rows + the red selection highlight, the scrollbar (up/down arrows + thumb), and the NEW/OK/CANCEL buttons** (each a base-lost element paint method + driver blit).  Clicking SELECT PLAYER therefore reaches this unreconstructed render and does not yet produce the DOSBox frame — so it is **NOT added to verify.sh**, and no verify flow / the default boot clicks it (4/4 stay PASS, deterministic).  NEXT: reconstruct the title-bar drivers, then the list-item glyph rows + selection, scrollbar, buttons → drive AE→0 vs `ref/selplayer_native320.png` → add the `selplayer` flow → 5/5.  BATTLES (state 0x2, `.FSG`) leads into the mission EDITOR and is a larger, later target.

</details>

## DONE (STAGE 3 — SCREEN CRAWL, SCREEN 4/N = REVIEW) — **THE REVIEW SCREEN (vehicle encyclopedia) IS BIT-IDENTICAL: AE = 0/64000 vs the genuine DOSBox reference `ref/review_native320.png` on BOTH native AND wasm, native↔wasm 0-diff (identical md5 `da9a850a`), deterministic (5/5 stable, normal + pattern-init build). Added as the 4th passing verify.sh flow → `bash tools/verify.sh both` = 4/4 (mainmenu + about + settings + review).** 1 new patch (136). NO REGRESSION: menu md5 `3a6ff1c5`, about AE=0, settings AE=0, `re_out/fist.c` pristine (`61453e42`), `make check` = 136 patches apply, native + wasm green.

**WHAT REVIEW IS.** Clicking REVIEW (menu row 113 → screen-state 0x4 → e714 tail dispatch `_tgt 0xe93c = e39f(); mov ax,2; 7088(2); jmp e714`) opens the **vehicle ENCYCLOPEDIA**: an orange wireframe of the M1A2 Abrams (2 views) + a spec column (ENGINE / MAX SPEED / ARMOR / SIGHTS / ORDNANCE) + a vehicle-select button row (M1A2 / M3 / AH-64 / T80 / EMP / HIND / OK).  It is a **modal 7088 dialog** (SAME machinery as ABOUT, id=2 vs id=4) — NO file enumeration, NO new file opened on the click (only the shared FLDCOMP.MRL panel; the wireframes are sprites in MSPRITE8/9.BIN pre-loaded at boot), so it is deterministic static data (vehicle index 0 = M1A2), the most self-contained of the 4 unstarted items (SELECT PLAYER / CAMPAIGNS / BATTLES all open dynamic file-enumerated LIST dialogs — a bit-verification hazard deferred).  The item→state map (verified by instrumenting the e714 dispatch on scripted clicks): SELECT PLAYER row74→0xe, CAMPAIGNS row87→0x0, BATTLES row100→0x2 (all `.FPL`/`.FSG` enumerate → SELECT PLAYER / SELECT BATTLE list dialogs), **REVIEW row113→0x4**, SETTINGS row126→0x6, ABOUT row139→0xc, QUIT row152.

**THE 7088 DIALOG DISPATCH IS ALREADY GENERIC (patch 131) — id selects the content.** 7088 always builds the SAME display-list template (0x2c56 → nd=d4 text element, nd=d6 button elements); the dialog `id` (=[0x8be5]) indexes two per-dialog FAR method tables: the text method `[id*2-0x739f]` and the button method `[id*2-0x7393]` (offset:seg WORD pairs in the static image).  For REVIEW (id=2): text = 0f69:0x75bc = FUN_1000_6c4c, button = 0f69:0x76d9 = FUN_1000_6d69.  The 71f8/7203 dispatch (patch 131) already far-dispatches through these tables, so id=2 auto-routes once 6c4c/6d69 are reconstructed.

**RECONSTRUCTED (patch 136, all asm-verified vs the images):**
- **e714 dispatch** — added the `_tgt==0xe93c` case (mirror of ABOUT 0xe969): `e39f(); 7088(2); b78e=10`.
- **6c4c (text/wireframe method)** — Ghidra dropped all three far-method-call register args + the drawstring's DGROUP base.  Thread: `[0x60a]`=MGAVIDEO 0x1091 box-fill (bx=0x8c45 rect, al=0xa1); `[0x6c8]`=MGAVIDEO 0x294d wireframe-sprite blit (ax=sprite id [veh*2-0x7403], bx=pos [veh*2-0x73f7]); `[0x648]`=MGAVIDEO 0x161c clip/line-draw (al=1, bx=0x8c4d rect); then font/pos/drawstring for the spec text `si=word[veh*2-0x735d]`.
- **6d69 (vehicle button)** — mirror of 6d3d (ABOUT button) + the SELECTED-vehicle highlight: `if(button index == word[0x8be7]) sprite+=8 (highlighted) else sprite+=[0x8be9] (base)`; far-blit via [0x6b4]=26a1.  Base-loss + dropped args (redeclared 5-param like 6d3d so 7203's rect lands in param_4).
- **MGAVIDEO 294d + 298a (fist_mga.c)** — the wireframe = a TRANSPARENCY sprite blit (294d dispatcher, structurally identical to the working 26a1 but blitting via 298a — per-pixel `al!=0?` transparency — instead of 26de).  294d: use the 260c shim statics (m_260c_cf/zf/recseg/recoff), Ghidra used in_CF/in_ZF=0.  298a: 26de-class base-loss — rebuild the fb-dest ES base + rowtable, the row stride, the source seg (m_260c_recseg).
- **MGAVIDEO 161c (fist_mga.c)** — the clip-window/line-draw entry.  Base-loss: `_DAT_1000_c724` (the DGROUP:0x724 NEAR offset to the fb/clip descriptor, typed by Ghidra as a host int** → wild 0xa000:off deref) + pointer-scaling of the 4-WORD rect (read as int[]).  NB **2d6d does NOT set the clip window** — it is the CURSOR coord-transform (d5ce..d5d4); the clip window (desc[3..6]) is the driver-init full-screen region.
- **MGAVIDEO 1677 (fist_mga.c)** — the Cohen-Sutherland clipped rect/line FILLER (338 instrs; draws the REVIEW title separator line).  The control flow is the pristine decompile (faithful); the fixes: (a) base-loss `_DAT_1000_c724`→desc, `_DAT_1000_c794`→rowtab, dropped ES=fb-seg base → `fb + rowtab[y] + x`; (b) param_2 4-word rect scaling → int16 word reads; (c) `puVar15`/`puVar2` `uint*`(4B)→`uint16_t*` (the fills stosw/stosb 16-bit); (d) **`in_AF` uninitialised flag-artifact → init 0** (the flaky-garbage class: `in_AF<<4` pollutes bit7 of the outcode byte → the clip wrongly REJECTS the line; deterministic 0xAA break under pattern-init); (e) **`DAT_0000_161b` (the solid/XOR fill-mode flag) WORD→BYTE** — offset 0x161c is the first opcode of the FUN_0000_161c code (0x8b), so the WORD read = 0x8b00 != 0 → the solid-fill check took the XOR path (box 0xa1 ^ colour 1 = 0xa0, a near-black, instead of the SOLID orange index 1) → THE last-pixel-delta root cause (AE 80→0).

**NEXT SCREEN:** the file-enumerated LIST dialogs — SELECT PLAYER (row74, state 0xe) / CAMPAIGNS (row87, state 0) → the player roster; BATTLES (row100, state 2) → SELECT BATTLE — each a list-box + scrollbar + NEW/OK/CANCEL (bit-verification needs the FILEMGR findfirst/findnext to enumerate `armoredfist/` identically to DOSBox, and the scrollbar/selection state); plus the SETTINGS toggle interactions and the in-engine editor.

## DONE (STAGE 3 — SCREEN CRAWL, SCREEN 3/N = SETTINGS) — **THE SETTINGS SCREEN IS NOW BIT-IDENTICAL: AE = 0/64000 vs the genuine DOSBox reference `ref/settings_native320.png` on BOTH native AND wasm, native↔wasm 0-diff (identical md5 `7e669259`), deterministic (8/8 stable, pattern-init + FIST_QCHK). Added as the 3rd passing verify.sh flow → `bash tools/verify.sh both` = 3/3 (mainmenu + about + settings).** 1 new patch (135) closes the last pixel. NO REGRESSION: menu md5 `3a6ff1c5`, about md5 `196e072c` (AE=0), `re_out/fist.c` pristine (`61453e42`), `make check` = 133 patches apply, native + wasm green.

**THE LAST PIXEL (patch 135) — the trailing `.` on the status line.** The sole AE=1 delta was framebuffer (88,192): port black `(0,0,0)`, ref red `(251,0,0)` — the missing red period on "NO JOYSTICK SELECTED**.**". The status line is built by the display-list message BUILDER `DGROUP:0x2ac = 0xf69:0x444f = FUN_1000_444f` (linear 0x13adf), a node-table subsystem (stride-10 nodes at str-seg:0x360; node-method dispatch `call *es:[si+0x35c/0x35e/0x5a]`; helpers 4507/0x13ebf/0x13f0e/0x13d93; strcat 0xf69:0x13f5c = `lods es:[si]; stos es:[di]`) whose reloc section (leading seg 0xf69, reloc-table linear 0x33720, si=0x200: installs DGROUP:0x2ac..0x2fc) is not installed in our port. **444f's NET EFFECT for a status line (asm-verified vs `re_out/fist_dat_image.bin`): assemble the (period-LESS, NUL-terminated) source status text + a trailing `.` into the builder's OWN str-seg scratch buffer str-seg:0x62f (di=0x62f in the 0x13f5c strcat) and return si = that buffer.** Source strings: str-seg:0x1672 "NO JOYSTICK SELECTED\0", 0x1687 "STANDARD JOYSTICK SELECTED\0", … (str-seg = word[DGROUP:0x70] = 0x2d74). **Patch 135 reconstructs that exact effect at the 6c38 call site** (copy the source status text into str-seg:0x62f, append `.`, NUL-terminate, pass si=0x62f to 700a) and renders through the unchanged 700a→6d65→5591 drawstring path → byte-identical output, the period lands as the single red pixel → AE=0. **A full FUN_1000_444f decompile+reloc-install was evaluated and judged not worth the collateral re-decompile risk to the 132-patch series for one pixel** — the inline effect reconstruction is bit-identical through the real render path and touches only the already-patched 6c38 status path (no re-decompile, pristine `fist.c` unchanged).

**HEADLINE — the frontier premise ("209e dispatches only the widget elements, not bg/text") was WRONG: gdb-tracing the `209e` dirty-walk showed the bg element (nd=0xda→6acb) and the text element (nd=0xe2→6d65) ARE dispatched on the first paint (carry=1 for all). The real bugs were five independent asm-verified defects (patch 134), each proven by fb-histogram tracing:**
- **6d65 c60a rect fill = THE panel-overwrite (biggest win, AE 63615→56251).** 6d65's label-background fill `mov al,0; lcall [0x60a]` (MGAVIDEO 0x1091 rect-fill) takes `bx=elem+4` (the element's `[y0,x0,y1,x1]` rect); patch 132 dropped bx and passed 0 → 1091 filled the rect at DGROUP:0 = a garbage FULL-SCREEN rect (index 0x19/0xa0) that WIPED the textured SETTINGS.MRL panel (painted correctly by 6acb) every single frame. The panel was there all along, immediately overwritten. Thread `bx=elem+4`.
- **0874 palette fade = the "green vs gray" +1 brightness (AE 56251→1538).** The fb had the settled SETTINGS content but rendered through a palette 1 level (6-bit) too bright everywhere. The DOSBox ref is the SETTLED post-fade frame (verified non-circular: refcapture at settle=8/40 both AE=0 to the committed ref). The settled fade is `present[i] = ramp[target[i]]` with `ramp[i]=(i*0xffff)>>16 = i-1` — a uniform 1-level dim. The port skipped it: the crossfade-target alloc `mov bx,0x788; mov ax,0x300; lcall [0xe4]` uses the UNINSTALLED device-alloc vector DGROUP:0xe4 (==0) → seg788=0 → 0874 took the present-only path (raw upload, +1 too bright). Fix: resolve 0xe4 directly to the real MEMMGR alloc `FUN_1000_182a` (allocate word[0x788], same pattern as the 0x782 alloc at 0009/patch 049), run the real fade, set the full `c5e8=<mga_seg>:0x946` fade-step far-vector, and have the shim retrace ISR (`fist_vga.c fist_vga_service_retrace`) invoke MGAVIDEO 0x0946 each frame (gated on the fade marker word[0x5e8]==0x946 so no other screen is touched) until it settles.
- **700a status-string ES = the bottom-row noise (AE 1538→360).** 6d65 draws the status string at `es:si = word[0x8b53]:word[0x8b51]`. 700a (`mov [0x8b53],es`) stores ES = the resource seg the caller's uninstalled far resolver ([0x2ac]/[0x2e0]/[0x2f0]) returns → garbage 0x819 → wild string. Thread `es = str-seg word[DGROUP:0x70]` (=0x2d74, where the UI strings live).
- **6ef5 config→widget store-width = SKY/SMOKE checkboxes empty (AE 360→262).** The boolean flags 0x8b45/46/50/4d/4e/4f are BYTE stores (`mov al`; asm-verified 0x6f10..0x6f7a) but Ghidra typed them WORD → the 0x8b4e word-write clobbered byte[0x8b4f] that 0x8b4f just set → SMOKE (and SKY) rendered empty. Restore the asm byte/word widths (config `0 N \0 Y 2 1 2 N Y N N` loaded byte-identical from FIST.SET).
- **6c38 joystick status = "NO JOYSTICK SELECTED" (AE 262→1).** `mov si,0x165c; lcall [0x2ac]; call 700a`: [0x2ac] modifies si (the status offset) before 700a but is uninstalled → si stayed 0x165c ("SETTINGS NOT CHANGED", wrong). The real DGROUP:0x2ac = 0xf69:0x444f = FUN_1000_444f is an UNDECOMPILED display-list status-message BUILDER (stride-10 node table @str-seg:0x360, linked-list ops @0xf69:0x482f, appends a trailing '.'); its reloc section (leading seg 0xf69 @reloc-table 0x33720) is not installed. PARTIAL reconstruction: resolve the status TEXT from the game's parallel status table at DGROUP:0x8b33 (`[0x1672 "NO JOYSTICK SELECTED", 0x1687 "STANDARD JOYSTICK SELECTED", …]`), `si=word[0x8b33+jstype*2]`. Correct text, but omits 444f's trailing '.' → the sole 1px delta.

**THE FRONTIER TO AE=0 — CLOSED (patch 135, above).** SETTINGS is AE=0 on both targets and IS in verify.sh (3/3). NEXT SCREEN: mission-select / briefing / controls-config, and the SETTINGS toggle interactions (detail LOW/MED/HIGH, sound SB/GUS, joystick radios) — a further follow-on now that the static SETTINGS frame is bit-identical.

<details><summary>SUPERSEDED (widget-sprite-sheet frontier — MSPRITE0.BIN)</summary>
**THE WIDGET-SPRITE-SHEET FRONTIER IS RESOLVED: MSPRITE0.BIN (the common widget/dialog sprite sheet) now LOADS and REGISTERS into the global sprite directory, so the SETTINGS checkbox/radio boxes RENDER and the runaway 26de blit (the reported hang) is GONE.** 1 patch (133) + the tools/oracle INT-21 open-trace (opentrace.c/trace_click.sh) + the genuine DOSBox reference `ref/settings_native320.png`. The prior "panel/palette/text render" claim (patch 132) was WRONG — SETTINGS was flat-green with only widget boxes; patch 134 (above) fixed the real render.</details>

**HEADLINE — the widget sheet is MSPRITE0.BIN, loaded at BOOT, not a per-screen `.BIN` (the prior premise was wrong).** DOSBox INT-21 open-trace (via `tools/oracle/opentrace.c` LD_PRELOAD'd on a scripted-click DOSBox run, `trace_click.sh`): the real game opens HIGH.DTL, **MSPRITE0.BIN**, DSOUNDS.BIN, INTRO.MS3 at boot, and a SETTINGS click opens **ONLY SETTINGS.MRL** — there is NO per-screen widget `.BIN`. Runtime directory dump (gdb) confirmed the global sprite directory `word[DGROUP:0x4f0]` (seg 0x4974) was ENTIRELY EMPTY — not even the menu box (which paints via a FILL, not this directory, so the empty directory went unnoticed until the SETTINGS widgets, its first real consumer). 260c resolves a sprite by `mov es,[ds:0x4f0]; les si,es:[si]` — sprite id = BYTE offset into a table of far-ptr records; each record is 8 bytes; the checkbox ids 0xe8/0xf0 = records 29/30 of a sheet registered at base 0. **Patch 069 already reconstructed 153c's registration loop correctly** (it DID register MSPRITE6 at base 0x13c8 — verified dir[0x13c8]!=0); the bug was purely that MSPRITE0 never LOADED: cae6 calls `153c(0x24,0x1c00,dx=0 base,bx=0xe816 desc)` (asm 0xcb07) but BEFORE FUN_0000_e714 installs the resource-open far vector DGROUP:0x388 (= FUN_1000_26fc; our port defers the si=0x174 install out of the inert-f842 CRT path to menu-enter, patch 098), so its `call [0x388]` open no-oped. Patch 133 loads MSPRITE0 at e714 immediately AFTER `fist_ensure_dlist_vecs()` (the same point MSPRITE6/MAINMENU.MRL load), guarded once on dir[0x4f0]:0xe8 being empty. **NB (dual-target regression caught by verify.sh):** installing si=0x174 EARLY in cae6 instead changes e2fc/e446's dispatch and TRAPS under wasm's `call_indirect` signature check (native tolerates it) → so we load MSPRITE0 at e714, we do NOT move the install.

**THE TRUE REMAINING FRONTIER — THE PANEL + TEXT + PALETTE DO NOT RENDER (contra the prior status).** Empirically (native FBDUMP + gdb): the SETTINGS background is FLAT index 0xf4 (62094/64000 identical px = the clear colour), palette `word[0x782]` holds INVALID values (>63) → the frame is bright green with only the widget boxes; native↔wasm also DIFFER (64585 B). gdb-proven root cause: **the SETTINGS bg-paint method 6acb and the text-label method 6d65 are NEVER invoked** during the 6a4a paint loop — `209e` dispatches the WIDGET elements (6c5f→6d54→26a1) but NOT the bg/text elements. SETTINGS.MRL DOES load (at the display-list build; descriptor 0x8a96 word0 = a valid seg with the "00 00 00 07 07 07" grayscale palette + PackBits image), but it is never BLITTED (0340/the bg paint is never reached for SETTINGS), so the panel stays the flat clear colour and the SETTINGS palette is never copied to `word[0x782]`. So patch 132's 6acb/6d65 reconstructions are present but UNREACHED — the next step is to find WHY 209e (the 0x423c dirty-walk) paints only the widget elements and drive it to also paint the bg (nd 0xda → 6acb) and text (nd → 6d65) elements (element dirty-flag / method-vector / walk-order), then the panel + text + the SETTINGS.MRL palette render → capture is already done (`ref/settings_native320.png`, genuine 1:1 DOSBox click at row 126, non-circular) → drive AE→0 on both targets → add the `settings` flow to verify.sh. (Toggle interactions — detail LOW/MED/HIGH, sound SB/GUS, joystick radios — are a further follow-on once the screen is static-bit-identical.)

**THE SETTINGS ACTIVATE PATH (all asm-verified vs `re_out/fist_dat_image.bin`).** A menu CLICK on the SETTINGS row activates via the same chain as ABOUT (patch 129): `ea5a` writes the item's target **screen state 0x6** to DGROUP:0xf78e; the e714 tail jumptable (image `0xe77a`, state = byte offset) dispatches state 0x6 → `_tgt 0xe94f = e39f(); call 6a4a; jmp e714`. Unlike ABOUT (a modal `7088` dialog), SETTINGS is a **full SCREEN**: `FUN_0000_6a4a` builds a display list from template seg `0x2c41` (size `0x148`) and runs the standard paint/event loop until the exit flag DAT_2000_4a92 (DGROUP:0x8a92). Item→state map (from `word[base+id+1]`, found by watching the state write on a scripted click): REVIEW→0x4, **SETTINGS→0x6**, ABOUT→0xc; the 7 menu-item rects are transformed at runtime (SELECT PLAYER≈row72 … ABOUT≈row139, ~13px pitch → SETTINGS click ≈ row 122–127).

**RECONSTRUCTED THIS ITERATION (patch 132):**
- **e714 dispatch** — added the `_tgt==0xe94f` case (mirror of 0xc/ABOUT): `e39f(); 6a4a(0,param_2); b78e=10; continue`.
- **6a4a screen loop** — CF threading (g_fist_cf from 3907 + 35a7) + the cooperative INT-8 pump in the spin-wait, identical to the e714 idle loop (117) and the 7088 modal loop (129).
- **6acb (SETTINGS BG element)** — threaded the dropped bg-loader args: `c560(ax=0x22 idx SETTINGS.MRL, cx=0x1c00, bx=0x8a96)`, `c564(paint)`, `c5b0(ax=0x20 palette fade)`. (idx 0x22 = MGAVIDEO name-table index 17; FLDCOMP=16=0x20.) SETTINGS.MRL now OPENS + the panel renders.
- **PALETTE FADE-IN (mga 0874/0859/0946)** — `c5b0`→`084e`→`0859`(`mov es,dx`, the loaded-palette seg = the descriptor word0, published via the new driver global `g_mga_fade_es`, set by the engine at the 6acb call site)→`0874`. 0874 (asm 0x874–0x945) had ALL three segment bases dropped (es=seg[0x782] present buf / ds=seg[0x788] crossfade target / entry-ES=loaded palette seg) → near-null copies (SEGV). Reconstructed faithfully in the flat seg<<4 model: copy1 present←loaded, `[0xe4]` alloc the target, copy2 target←present, copy3 zero present, register `0946` as the per-frame fade step; the wait is on **c786** (Ghidra mis-attributed it to c216), which the shim retrace service (`fist_vga_service_retrace`) clears. `0946` (asm 0x946–0x9bb) reconstructed too (segmented ramp remap `present[i]=ramp[target[i]]`, `ramp[i]=(i*acc)>>16`). **Fade-buffer alloc `[0xe4]` (c0e4) TRAPS in our port (→ seg788=0)**; 0874 therefore takes the present-only path (upload the loaded palette directly) when the target buffer is absent — the settled palette is the loaded SETTINGS.MRL palette either way. FULL FADE FIDELITY (the ~1-level ramp dim, and the fade transient) is a follow-on: it needs (a) `c0e4` to actually allocate seg[0x788] and (b) the retrace service to invoke the `c5e8=0x946` fade callback each frame (the driver's real ISR 0b1f does this; the shim replaces the ISR).
- **6d65 (text label) + 555f/thunk_555f (drawstring)** — 6d65 draws x/y from the element rect ([elem+4]/[elem+6], element node from DGROUP:0x3e08) + string at es:si=seg[0x8b53]:word[0x8b51]; reconstructed to pass the flat string pointer to the drawstring (mirror of the working 5591/patch 115; glyph blitter c684=driver 1d23 already fixed by 131).
- **6c5f + 11 state-renderers (6cb6/6cc6/6cd5/6ce4/6ce5/6cf4/6d05/6d16/6d27/6d36/6d45) + 6d54** — the SETTINGS checkbox/radio widgets. 6c5f computes the 8×8 box rect (DGROUP:0x8b55..0x8b5b) and dispatches via `cs:[si*+0x6c8e]` (si = `word[base+id+1]`, base = DAT_1000_fafe); each renderer picks the checked/unchecked sprite (0xe8/0xf0) from its DGROUP state byte and calls 6d54→26a1 (sprite blit). All were base-lost / dropped-arg. Threaded faithfully.

**FRONTIER — THE WIDGET SPRITE SHEET.** 26a1's clip/resolve (260c) looks up the sprite record in the directory at **seg[DGROUP:0x4f0]** (=0x4974 at runtime, the menu/MSPRITE6 directory) by BYTE offset. The menu box (0x13d8) resolves (record 0x00ca); the SETTINGS checkbox indices **0xe8/0xf0 resolve to record 0** (the directory has no entry there) → `26de` blits a garbage-sized sprite → runaway. So the SETTINGS **widget sprite sheet is not loaded into directory[0x4f0]** (no widget `.BIN` opens on the SETTINGS enter — only SETTINGS.MRL). NEXT: find + fix the (base-lost) widget sprite-sheet load in a SETTINGS element create/paint method (elements nd 0xda/0xdc/0xde/0xe0/0xe2/0xe4 → methvecs 2104/2112/6acb/6c5f/6d65) so directory[0x4f0] carries the checkbox sprites at 0xe8/0xf0; then the widgets render → capture the DOSBox SETTINGS reference (`tools/refcapture_click.sh` clicking the SETTINGS row) → drive AE→0 on both targets → add the `settings` flow to verify.sh. (A SETTINGS click currently HANGS at this frontier; no verify flow clicks SETTINGS, so mainmenu/about/default boot are unaffected + deterministic.)

## DONE (STAGE 3 — FIRST FULLY BIT-VERIFIED SUB-SCREEN) — **THE ABOUT DIALOG IS NOW BIT-IDENTICAL TO DOSBox ON BOTH TARGETS: credit TEXT + OK button render, the wasm OOB is fixed, native AE=0 AND wasm AE=0 vs `ref/about_native320.png`, native↔wasm 0-diff, deterministic.** 1 new patch (131). `bash tools/verify.sh both` = PASS mainmenu AND **about** (2/2). ABOUT md5 `196e072c` (native == wasm, 5/5 stable). Menu UNCHANGED (`3a6ff1c5`). `re_out/fist.c` pristine (`61453e42`); `make check` = 131 patches apply.

**HEADLINE — the credit text was a DROPPED FAR-CALL SEGMENT, not a missing paint walk.** The earlier frontier ("credits paint via the [+0x4872] 1e88 walk which patch 116 hard-stops") was a MISDIAGNOSIS: 1e88 is never invoked during the modal dialog (the modal loop 7088 only runs 209e dirty-paint + 35a7/1e4b on real input).  The credits are painted by the 209e (0x423c) dirty walk itself.  Two of the dialog's element paint methods are FAR calls through the per-dialog method table (offset:seg word pairs at DGROUP:0x8c61/0x8c6d, indexed `id*2`):
- **nd=d4 (element 3b24) `71f8`: `mov bx,[0x8be5]; shl bx,1; lcall [bx-0x739f]`** -> for ABOUT (id=4) = **0f69:0x7606 = FUN_1000_6c96 = the credit-TEXT drawer** (title "ARMORED FIST" via 52d1 setpos + 5591 drawstring @ color 6, then the 3 credit columns @ (0x1e,0x1a)/(0x1e,0x7c)/(0x1e,0xd9) and the copyright @ (0x9c,0x65); each 5591 call draws a whole multi-line COLUMN, the lines separated by the **0x0c (CR+LF) / 0x0e (advance) format control chars** interpreted by the driver text renderer).
- **nd=d6 (elements 3b30..3b78, x8) `7203`: `si=[bx+2]; ax=[bp+si+1]; bx+=4; lcall [id*2-0x7393]`** -> for ABOUT = **0f69:0x76ad = FUN_1000_6d3d = the button sprite draw** (per-state sprite table `[state*2-0x7387]` + `[0x8be9]` base -> `lcall [0x6b4]` = MGAVIDEO 26a1); draws the orange OK button (state 0 -> sprite 0x15f0) + the 7 empty slots.

**Patch 129 rendered BOTH as NEAR calls to the wrong seg-0 offsets** (0x7606/0x76ad are unrelated seg-0 functions), so neither drew.  Patch 131 restores the faithful FAR dispatch + threads 7203's per-element (bx=element from DGROUP:0x3e08, bp=base from DAT_1000_fafe) and 6d3d's (state,rect) args.

**THE CREDIT GLYPH BLITTER (driver `fist_mga.c`) reconstructed:** 6c96 -> 5591 -> `lcall [DGROUP:0x684]` -> driver **1d23** (text-line setup: `di=[0x736]; if word[di]==0xA000 { 2d6d([0x736]+0x1c); [0x794]=0x7c8;[0x796]=0x140; 1d5f; 2e12 }`) -> **1d5f** (the glyph blit + control-char handler).  Both were host-pointer base-losses on the text-state struct at DGROUP:[0x736] (Ghidra deref'd the VALUE at 0x736 as an int* and word-scaled its byte fields: fbseg[+0]/glyph-offtab[+6]/cellW[+0xa]/cellH[+0xc]/pos[+0xe/+0x12]/lineAdv[+0x14]/color[+0x16/+0x18]/startCol[+0x1a]/row[+0x1c]/col[+0x1e]).  1d5f: per-char glyph lookup in font seg [0x71a], 1bpp blit `fb[di] = (glyph^3)+color` to fb seg [bx+0]=0xA000, plus the 0x7/8/a/c/d/e/f control-char cursor moves.  Asm-verified vs `re_out/fist_mga_image.bin`.

**THE WASM OOB — ROOT-CAUSED + FIXED as a REAL defect (not papered over):** the modal path's `e39f` (asm `mov bx,[0xf6f2]; mov dx,0xf6ee; lcall [0x602]`) is the dialog SAVE/RESTORE blit (0fbc->0fe4); Ghidra DROPPED its args (dx=save-descriptor 0xf6ee, bx=rect DAT_2000_b6f2).  So 0fbc/0fe4 ran on whatever garbage sat in the arg slots -- and native vs wasm codegen leaves DIFFERENT garbage there: **native** got di_save=0x0004 (flag byte 0 -> the `jns` EARLY RETURN, harmless), **wasm** got di_save=0/rect=0 (flag 0xa0 -> a bogus `save_seg->fb` copy from seg 0 across a huge span) which wasm's linear-memory bounds check trapped as "memory access out of bounds".  It was an UNINITIALISED-ARG bug that native's 1 MB `g_mem` masked, NOT a timing divergence -- confirmed: native with the cooperative tick (`FIST_COOP_TICK=1`) takes the SAME harmless early return as native-SIGALRM (AE=0, no crash).  Threading e39f's args faithfully makes 0fe4 take its real early-return on BOTH targets -> native↔wasm identical, no OOB.  The cooperative-tick default was NOT changed (native↔wasm is already 0-diff for both flows without it).  New verify.sh flow `about` (name|hz|runms|FIST_MOUSE-script|ref); the FIST_MOUSE script settles the cursor to (160,138) after the click so it lands on DOSBox's cursor pixel -> AE=0.

## DONE (STAGE 3 — SUB-SCREEN OPEN) — **CLICKING "ABOUT FIST" NOW OPENS ITS DIALOG: the full event→activate→screen-state-machine→modal-loop chain works, and the ABOUT dialog PANEL (frame + button row) renders crash-free + DETERMINISTICALLY on native.** 2 new patches (129/130) + 1 shim fix (`re_out/fist_dos.c`). Menu UNCHANGED (`verify.sh both` = PASS mainmenu, native↔wasm 0-diff, md5 `3a6ff1c5`); ABOUT panel deterministic md5 `49310ace` on native. DOSBox reference `ref/about_native320.png` (via new `tools/refcapture_click.sh`). `re_out/fist.c` pristine (`61453e42`).

**THE FULL CLICK→SUB-SCREEN CHAIN (all asm-verified; the frontier docs' "sub-screen open").**
Feeding `FIST_MOUSE="200:160:139:0; 800:160:139:1; 1400:160:139:0"` (move to ABOUT FIST @ row 139, then
press+release, spaced so the cursor position settles into the queued node before the click — the queue/
present latency means a too-fast press enqueues STALE (0,0) node coords) drives:
`39d7 → 2f03 → 349b(enqueue) → 35a7(drain) → 1e4b → fist_hit1eb4` → the item's activate method
`[elem+0x4660]=ea5a` → writes screen-state `DGROUP:0xf78e = 0xc` → the e714 state jumptable
(`state 0xc → 0xe969 = e39f(); 7088(4)`) → the **modal dialog loop 7088** → loads the panel + spins.
- **Item identity (verified):** display-list elements 3b24(SELECT PLAYER, row 65-79) … 3b60(ABOUT FIST,
  row 130-144) … 3b6c(QUIT). Click y=139 → 3b60 = ABOUT FIST. DOSBox click at (160,139) shows the
  credits screen → confirms.
- **ea5a (activate):** `aa4e` returns the flags UNCHANGED (asm push ax/pop ax around the c626 side-effect
  call), so `(flags & 4)` gates activate-on-RELEASE (press flags 0x82 → no-op, release 0x84 → write).
  It writes `[base+[elem+2]+1]` = 0xc = the ABOUT screen state. param_2/param_3 were DGROUP offsets
  Ghidra deref'd as host ptrs (base-loss); the 3 `[elem+0x4660]` dispatches dropped `(flags,bb,base)`.
- **7088 modal loop:** the dialog-builder dispatch `word[DGROUP:(id-0x73ab)]` (id=4 → 0xc30e, a legit
  near-noop; the ABOUT list is the static template at `0x8cee` that `1cdb(0x2c56,0x8cee,…)` activates),
  the Ghidra-mangled CF flow, + the cooperative INT-8 pump in the modal spin-wait (70d5). Plus the
  id-indexed dispatch base-losses (71f8/7203/70fb).
- **71de (dialog BG element):** loads FLDCOMP.MRL (idx 0x20 = the GENERIC dialog panel; there is no
  ABOUT.MRL — the panel is shared, the credits are separate) into descriptor `0x8bce` via `[0x560]` +
  paints via `[0x564]` — args were dropped (same class as patch 110/094 e3f8/e3bc). Without it the panel
  resource never loaded → 0340 RLE-decoded zeros = full-screen garbage (distinct-indices 256→44 once
  fixed).
- **SHIM (`re_out/fist_dos.c` `open_ci`):** the engine builds DOS 8.3 names SPACE-PADDED — a 7-char base
  → "FLDCOMP .MRL"; real INT 21h treats the space as padding and opens "FLDCOMP.MRL". Strip spaces in
  the filename canonicalization (portable, native↔wasm). This was the actual load-failure root cause
  (the name built correctly, the host `fopen("FLDCOMP .MRL")` failed).
- **Driver blits (130, fist_mga.c):** 0fbc (`word[[0x724]]==0xA000` base-loss + dropped rowtable setup)
  and 0fe4 (RESTORE blit; `call 0xfe2`=pop ax;ret EARLY-RETURN idiom Ghidra mis-modeled → base-lost copy
  loop crash; same class as patch 113's 0f62). Plus 23bf (far-ptr `les di,ss:[0xea2c]` base-loss) and
  92ea (element event method DGROUP-offset base-loss) reached on the transition.

**FRONTIER — THE CREDIT TEXT + THE WASM DIALOG PATH.** The ABOUT panel (frame + buttons) renders, but the
credit-TEXT elements do NOT paint, so it is NOT yet bit-identical to DOSBox (which has "ARMORED FIST" +
~30 credit lines). Root-caused: the credits paint via the **[+0x4872] multi-element paint walk `1e88`**,
which patch 116 hard-stops after the FIRST element (the menu-enter behaviour; later menu elements are
painted by e9f0). The dialog needs ALL elements walked — but (a) the paint methods do NOT thread their CF
return (so the walk can't know when to stop), and (b) walking all reaches base-lost / un-threaded paint
methods (e.g. `ea7e`, which expects SI=element that `1e88`'s call doesn't set) → SIGSEGV. So the credits
need: reconstruct `1e88`'s full CF-threaded walk + thread each credit element paint method's register
args + reconstruct the base-lost ones. SEPARATELY: the **native↔wasm dialog parity** — the wasm click
path OOBs ("memory access out of bounds") where native tolerates it (bounds-checked wasm vs the 1 MB
g_mem), likely a modal-loop timing divergence (cooperative tick vs SIGALRM) reaching a different
paint/blit. Both are the next iteration; the MENU flow (verify.sh) is unaffected by either. New tool:
`tools/refcapture_click.sh` (scripted-click DOSBox reference via XTest) + `ref/about_native320.png`.

## DONE (STAGE 3 — PRIORITY REGRESSION FIX) — **THE FLAKY (nondeterministic) STATIC-MENU SIGSEGV FROM PATCHES 125/126 IS ROOT-CAUSED + FIXED: the menu is now DETERMINISTICALLY crash-free AND native↔wasm bit-identical.** 2 new patches (127/128). `bash tools/verify.sh both` = PASS mainmenu (native↔wasm 0 differing bytes + AE=0 vs DOSBox). `re_out/fist.c` pristine; shims portable.

**THE REGRESSION (why it was flaky, and how each instrumentation SUPPRESSED it).** Patches 125/126 (event-queue reconstruction) made the event queue actually deliver events, which advanced execution into (a) the main MENU LOOP `FUN_0000_00d0: while (5c5f(), !stop) { 5c3a(); cae6(); }` and (b) the display-list event HIT-TEST (`1e4b -> thunk_FUN_0000_1eb4`).  Both surfaces contained latent Ghidra **base-loss + dropped-register-arg** defects that read UNINITIALISED register/stack garbage; the garbage varies run-to-run, so the resulting wild-pointer deref SIGSEGV'd only ~30-40% of runs.  Every diagnostic that touches the stack layout (gdb, `FIST_SEGV_BT`, ASan) SUPPRESSED it — the tell of an uninitialised-garbage bug.  **The tool that cracked it: `-ftrivial-auto-var-init=pattern`** (build with `ASAN=-ftrivial-auto-var-init=pattern bash tools/build_native.sh`) makes uninitialised locals a fixed 0xAAAA pattern, turning the flaky crash DETERMINISTIC and catchable under `FIST_SEGV_BT`; plus a plain-binary **core-dump** loop (`ulimit -c unlimited`, run from repo root, `FIST_TICK_HZ=100000 FIST_RUNMS=8000`) caught the bare crash EIP = `FUN_0000_f842`.

**THE TWO CAUSES (both asm-verified):**
- **127 — the DGROUP:0x12 (far install `f842`) / DGROUP:0x16 (clear `f860->f869`) reloc-applier calls in the menu-loop `FUN_1000_2ebe`/`5c3a`/`5c5f`.** `xor bx,bx; mov si,0xNNN; lcall [DGROUP:0x12]` — Ghidra dropped BX/SI at the site AND the string-op segment bases inside f842, so the raw `(*[c012])()` ran the base-lost C on garbage: `if (param_1 != 0) uVar1 = *param_3;` derefs a wild pointer when the garbage BX is nonzero.  **Cannot be routed to a faithful install here:** the install has NO observable effect on the static menu (native AND wasm render the byte-exact menu, AE=0 vs DOSBox, without it) but it DIVERGES execution under the wasm cooperative time base — with the installs, wasm STARVES at a pre-menu frame while native (SIGALRM) reaches the menu → native≠wasm (verified: 127-install alone made wasm 7bd6… ≠ native 3a6ff1c5…).  So the deterministic, dual-target-identical behaviour is the inert one every non-crashing run already produced: **elide the base-lost applier calls** (same class as patch 053's elision of the bx==0 no-op reloc apply).  DEFERRED: the faithful vtable install + a cooperative pump for the spin-wait it enables is sub-screen frontier work.
- **128 — the recursive cursor HIT-TEST `thunk_FUN_0000_1eb4` / `FUN_0000_1eb4` (asm 0x1eab-0x1ff4).** base-loss + pointer-scaling: the element node `bx`, lower bound `dx`, child-bounds base `bp` and coord source `si` are DGROUP NEAR OFFSETS but were typed host `int*`, so `*(int*)(param_5+4)` (asm `mov cx,[si+4]`, the cursor X) dereferenced the raw 16-bit offset as a host pointer.  Element fields are WORD offsets `[bx+0/2/4/6/8/0xa]`, stride 0xc BYTES (`sub bx,0xc`); the int* typing scaled every index x4.  Reconstructed faithfully as a shared recursive helper `fist_hit1eb4` in the offset model (`HG(o)` = DGROUP word at offset o); coord source = `DAT_1000_d8e2`'s value directly (asm: `si==[0x18e2]` throughout, and 0x1f4c reloads `[0x18e2]`) so the cursor read is deterministic regardless of the dropped param_5.  On the static menu the cursor sits above every item rect, so the walk finds nothing and returns without posting → menu frame unchanged.  **This is also the sub-screen frontier: the element method dispatch `[HG(HG(bb)+0x4660)]` + the 3566 enter/leave/activate posts are reconstructed and only fire on a real hover/click.**

**PROOF (both targets):** `verify.sh both` = PASS (native rc=0, wasm rc=0, native↔wasm 0 differing bytes, AE=0 vs DOSBox `ref/main_menu_native320.png`).  Native 10/10 rc=0 with a single stable md5 `3a6ff1c5f0b0` @ FIST_TICK_HZ=25000/FIST_RUNMS=22000; wasm 10/10 rc=0 same md5.  New shim diagnostic `FIST_QCHK` (event-queue free-list/ready-list invariant validator, `tools/native_main.c`).

**FRONTIER (unchanged, now unblocked deterministically):** the sub-screen open — `fist_hit1eb4`'s reconstructed activate dispatch `[elem+0x4660]` + the 3566 posts fire on a real hover/click; thread the activate method args + open ABOUT FIST/SETTINGS + bit-verify.  Re-enabling the 127 vtable installs (needed once 5c5f's `[0x1696]/[0x169e]` handlers matter) requires a cooperative pump for the spin-wait they enable.

## DONE (STAGE 3) — **THE CLICK-ACTIVATE BLOCKER IS ROOT-CAUSED + FIXED, AND THE EVENT QUEUE IS RECONSTRUCTED: a mouse click now enqueues, drains, and DISPATCHES INTO THE DISPLAY-LIST EVENT HIT-TEST (1e4b -> 1eb4) instead of doing nothing.** 2 new patches (125/126), 1 shim debug seam. `bash tools/verify.sh both` = PASS mainmenu (native↔wasm bit-identical + AE=0 vs DOSBox); plain menu AE=0, NO REGRESSION.

**HEADLINE — the previously-inert click now drives the event pipeline end-to-end into the hit-test.** Feeding a press+release over an item used to leave the frame unchanged (docs' "CLICK does not yet ACTIVATE"); it now flows 2f03(button)->c3fe->349b(enqueue)->35a7(drain)->1e4b->thunk_FUN_0000_1eb4(cursor hit-test). The sub-screen does NOT open yet — the frontier is now 1eb4 (below).

**THE TWO ROOT CAUSES (both watchpoint/gdb-proven, both asm-verified):**
- **125 — `FUN_0000_139e` wrote `DAT_1000_c3fc` (Ghidra-typed `int**`) as a 4-BYTE host pointer, spilling into the adjacent enqueue vector c3fe and zeroing it.** The button sub-handler `FUN_0000_2f03` far-calls `[DGROUP:0x3fe]` (= c3fe) to POST a button event; c3fe is installed by reloc section **si=0x1d8** (the frame/event-scheduler method vectors, leading seg 0xf69) as `DGROUP:0x3fe = 0f69:0x3e0b = linear 0x1349b = FUN_1000_349b` (the enqueue). But at runtime c3fe read back `0f69:0000` — SEG word (0x400=0xf69) intact, OFFSET word (0x3fe) ZEROED — so the far-call hit `0f69:0000` and **no event was ever queued** (proven: 349b/34f5 never entered on a press). A hardware watchpoint on DGROUP:0x3fe caught the corruptor: after `f738->fist_install_dgroup` applies si=0x1d8 correctly, the boot idle path `FUN_0000_1384 -> FUN_0000_139e(0)` does `movw [0x3fc],0x712` (asm 0x13ad, a 2-byte WORD store) — but Ghidra's `int**` typing wrote 4 bytes `0x00000712` across 0x3fc..0x3ff → 0x3fe=0. Retyped c3fc to WORD + reconstructed its reader 13b8. **The section-si-per-DGROUP-offset decoder (`tools/`-less, in-head): section si=0x1d8 seg 0xf69 → 0x3fe=349b, 0x402=34f5, 0x40a=35a7, 0x416=0x423c… — the event-scheduler vtable.**
- **126 — the whole event queue was modeled with HOST POINTERS while the real code uses 16-bit DGROUP NODE OFFSETS.** With c3fe fixed, 2f03 (button) and 2f38 (cursor-move, asm `lcall *0x3fe` at 0x2f9e, AX=1) finally reached `FUN_1000_349b` and it SEGV'd. The queue is a free list of 63 18-byte (9-word) nodes at DGROUP:0x18ea; every link/handle is a 16-bit offset (empty=0xffff). `d8e0`/`d8e2` are ADJACENT DGROUP words, so Ghidra's 4-byte `int**` typing (a) overlapped them and (b) truncated every stored node link → the 2nd enqueue read a garbage host pointer. This path had NEVER executed (c3fe was corrupt), so the model was never exercised. Reconstructed `349b`/`34f5`/`3566` (enqueue) + the `d8e2` readers in the offset model — the init (3436/3446) and drain (35a7) were ALREADY offset-based from patches 043/073. Threaded the driver's `2f03`/`2f38` c3fe enqueue args (AX = button-transition bits `param_1 & 6/0x18/0x60`; AX=1 for cursor-move; DX = dd9e).

**Shim debug seam (`tools/native_main.c`): `FIST_COOP_TICK=1`** drives the native INT-8 time base COOPERATIVELY (one tick per pump, like wasm) so the engine can be traced under gdb without gdb drowning in 25000 SIGALRM/sec (each ptrace-intercepted). Diagnostics only; the shipped/verify run keeps the SIGALRM timer. Essential for this iteration's gdb watchpoint/backtrace work.

**FRONTIER — `thunk_FUN_0000_1eb4` (the recursive cursor hit-test) is the next blocker.** The click event (param_1=1) reaches 1e4b which, for a targeted event (`ax & 0x7ff != 0`), calls `thunk_FUN_0000_1eb4` — the recursive walker that hit-tests the cursor (x=`[d8e2+4]`, y=`[d8e2+6]`) against each element's rect (`[bx+4/8]`=x0/x1, `[bx+6/0xa]`=y0/y1, `[bx]`=type), descends into container elements (type==4, child bounds via `[bp+di+1/3]`+faf0), and on a leaf hit posts the enter/leave/activate events via `FUN_1000_3566` while tracking the hovered element in `fe0c`/`fe12`/`fe14`. Ghidra rendered it with BOTH base-loss (the element/coord DGROUP offsets bx/dx/si + fe0c/fe12/fe14 as host `int*`; crash = `*(int*)(param_5+4)` with param_5=16) AND pointer-scaling (`param_3[2]` = +8 bytes but asm `[bx+4]` = +4; element stride `param_3 -= 6` = -24 but asm `sub bx,0xc` = -12). Full asm mapped: 0x1eab-0x1f9x. Reconstructing it (mirror patch-116's 1e88 offset model: treat element/coord refs as 16-bit DGROUP offsets, coord source = DAT_1000_d8e2 directly since si==d8e2 throughout, `FUN_1000_3566` already offset-fixed by patch 126) opens the hovered-element event dispatch → the item's activate method [elem+0x4660] → the sub-screen `.MS3/.MRL` load. New env-gated diagnostic reused: `FIST_COOP_TICK`, gdb watchpoints on DGROUP:0x3fe / queue tail.

## SUPERSEDED (STAGE 3 BEGINS) — **THE MENU IS INTERACTIVE: scripted mouse input drives the cursor and the highlight follows it to the correct item — hover navigation works, native↔wasm BIT-IDENTICAL, no regression (static menu still AE=0 vs DOSBox).** 2 new patches (123/124), 1 patch revised (106), the MGAVIDEO cursor-draw function recovered (2f38), a leading-nop icall resolver, and a deterministic scripted-input mechanism in the shim.

**HEADLINE — the cursor now TRACKS the mouse and hovering HIGHLIGHTS the item under it.** Feeding a scripted move to (160,139) redraws the arrow there (old cursor at 0,0 erased) and highlights ABOUT FIST; a move to (160,72) highlights SELECT PLAYER instead (box-region diff: rows 68-77 vs 138-147 — exactly the two item rows). Native `/tmp/nfb_hover.ppm` md5 = wasm md5 (`abf975ca…`), identical; static menu (no input) still md5 `3a6ff1c5…` = AE 0 vs `ref/main_menu_native320.png` on both targets. `make check` = all patches apply; default boot rc=0, 0 SEGV; engine `re_out/fist.c` pristine (md5 `61453e42`).

**THE INPUT MODEL (recon, do NOT re-derive).** The main menu is EVENT-DRIVEN, not polled: at boot `FUN_1000_392a` resets the INT-33h mouse (fn 0), sets/reads the position (fn 4/3), and installs a movement+button EVENT HANDLER via fn 0x14 (ES:DX = `0f69:4348` = `FUN_1000_39d7`, call mask 0x1f). After that the idle loop never polls fn 3 — it waits for the driver (us) to far-call the handler. `FUN_1000_39d7`: on any event stores `dd9c`(buttons)/`dd9e`(y=DX)/`dda0`(x=CX>>1); on a button event (`AX & 0x7e`) far-calls the driver click sub-handler `d5ac` = `FUN_0000_2f03`. The per-frame present (`FUN_1000_31c3`) copies `dd9e/dda0`→`d5ca/d5cc` (`FUN_1000_3a00`) then calls the driver cursor-draw `d5b0` = MGAVIDEO `0x2f38`, which clamps + writes `d5ba=d5cc`/`d5b8=d5ca` (the cursor position the enqueue and hit-test read) and redraws (erase-old `0e3b` → coords `2fd7` → clip `2ed7` → blit `0db8`). Keyboard is INT-9 (vec 0x09 → `0f69:4ea7`, a mid-function ISR at 0x14537 that reads port 0x60 — not yet wired).

**SCRIPTED-INPUT MECHANISM (`tools/native_main.c`, deterministic + portable, DD2-`DD2_SCRIPT` analog).** `fist_dos.c` fn 0x14/0x0c now CAPTURE the handler (`fist_input_set_mouse_handler`); fn 3/4 return/accept the driver's virtual position. `FIST_MOUSE="t:x:y:b; …"` (t = pump-count-after-menu-ready, x,y = pixel pos, b = button mask) is delivered from `fist_input_pump()` (called each `fist_timer_pump`, re-entry-guarded) by SYNTHESISING the exact DOS mouse events and far-calling `FUN_1000_39d7` — fully faithful (we ARE the mouse driver). Deterministic: the menu idle loop is a fixed point, so injection during it is timing-independent → native==wasm. `g_menu_ready` is set at `fist_ensure_dlist_vecs`.

**THE FIVE FIXES (all asm-verified):**
- **123 — the per-frame PRESENT gate `c738` is a BYTE, not undefined2 (the linchpin).** `FUN_1000_31c3` gates its whole body (retrace/palette `0b1f`, cursor-pos copy `3a00`, cursor draw `d5b0`, blit `c2b0`) on `c738++; if (c738==0)`. Asm = `inc BYTE ds:0x738; jne` (0x131cc); resting value 0xff, so byte inc 0xff→0 RUNS the body. Ghidra typed it undefined2; the live word is `0x00ff`, so the C computes `0x00ff+1 = 0x0100 ≠ 0` → body ALWAYS SKIPPED. That is why the cursor never redrew from the mouse and the palette needed the shim retrace workaround (patch 117) — the ENTIRE present body was dead. Byte-typed → body runs → cursor tracks. Same class as 104/105/106. No regression (menu still AE=0 — the present body reproduces the identical static frame).
- **2f38 driver RECOVERY (cursor-draw `d5b0`).** MGAVIDEO `0x2f38` (the `d5ca/d5cc`→`d5ba/d5b8` clamp + cursor redraw) was an un-recovered mid-function (Ghidra ended `FUN_0000_2f03` at its `retf`), so `(*d5b0)()` trapped 174k×/frame. Seeded `0x2f38` in `SeedDriverVecs` (Makefile `FIST_DRIVER_SEED_OFFS`) + re-decompiled the driver (minimal drift: exactly one function added; its callees `2d52/0e3b/2fd7/2ed7/0db8` already recovered/patched).
- **106 revised — `_DAT_1000_d5b6` (+ the newly-reached `d5ca/d5cc`) are WORD.** With the present body live, `2f03` writes `_DAT_1000_d5b6 = buttons` and `2f38` compares/writes `d5ca/d5cc`; Ghidra's undefined4 typing wrote 4 bytes (`d5b6=btn` corrupted `d5b8`=cursor-y; the `d5ba=d5cc` copy corrupted `d5bc`). Extended patch 106's WORD block to cover `d5b6,d5ca,d5cc`.
- **124 — thread `FUN_1000_39d7 → d5ac` (=`2f03`) args.** The button sub-handler dispatch `(*d5ac)()` dropped both operands; asm (0x13a22) reaches it with `AX`=event flags, `BX`=`dd9c`(buttons). Threaded `(param_1, DAT_1000_dd9c)` so `2f03` sees the real flags/button state (per-patch indirect-vector threading, doctrine).
- **`fist_icall` LEADING-NOP mid-entry resolver.** `d5ac`=MGAVIDEO `0x2f04` = `FUN_0000_2f03`+1 (the 0x2f03 byte is a `nop`); same for the mouse handler `0f69:4348`=`FUN_1000_39d7`+1. On an fmap miss `fist_icall` now backscans ≤4 leading `0x90` bytes to the containing function (faithful: running from the start executes the harmless nop then the identical body). Applied to both the overlay and engine fmaps.

**FRONTIER — the CLICK does not yet ACTIVATE an item (open a sub-screen).** Hover works end-to-end; a click (press+release over an item) enqueues via `2f03 → c3fe` and is dispatched by the idle loop `e714 → 206f → FUN_0000_209e → (*[elem+0x423c])()` (the .MS3 element event method) WITHOUT trapping, but no item activates and no sub-screen `.MS3/.MRL` loads (final frame == the hover frame). Root-cause candidate found: **`DAT_1000_c73d` (the per-element event-type bit selector in `209e`, `c73d & 0x1f`) has NO writer in the engine decompile** — likely a byte write attributed to the overlapping `c73b/c73e` symbols, so the button-event bit isn't selected and the item's move-handler fires instead of its activate-handler. Resolving `c73d` (+ verifying the button bit reaches the element's event byte in the `349b/34f5` enqueue: flags = `df7e|d5b6|c305`) is the next step to open ABOUT FIST / SETTINGS. New env-gated shim diagnostics: `FIST_MOUSE` (the input script), `FIST_INDBG` (cursor/present-gate state dump at the watchdog).

## DONE (2026-07-09) — **DUAL-TARGET INVARIANT RESTORED: the WASM build is wired and the MENU is NATIVE↔WASM BIT-IDENTICAL (0 differing bytes, identical md5) — and thus AE = 0 vs the DOSBox reference on BOTH targets.**
- **`make wasm` is GREEN.** `tools/build.sh` (was missing — the pre-existing `make wasm` infra gap) now exists: emcc compiles the SAME units as `tools/build_native.sh` (`fist.c` + `native_main.c` + `fist_dos/vga/icall/modules` + the `mga/snd/ext` overlays) to a node target. `tools/native_main.c` is REUSED as the wasm main via `#ifdef __EMSCRIPTEN__` (no separate wasm main).
- **HEADLINE — native↔wasm menu bit-compare = 0/192000 bytes** (`cmp -l /tmp/nfb.ppm /tmp/wfb.ppm` → 0; identical md5 `3a6ff1c5…`). Both dumps: `mode=0x13 nonzero=61481/64000 distinct-indices=210`. WASM vs `ref/main_menu_native320.png`: **AE = 0**. Deterministic: 3 consecutive wasm runs → identical md5.
- **Repro.** Build: `make wasm` (or `bash tools/build.sh /tmp/fisttest/fistrun.js`). Run: `FIST_TICK_HZ=25000 FIST_RUNMS=22000 FIST_FBDUMP=/tmp/wfb.ppm node /tmp/fisttest/fistrun.js`. Compare: `cmp -l /tmp/nfb.ppm /tmp/wfb.ppm` (native side = the same command on `/tmp/fist_native`).
- **Portability seam (native_main.c, all `#ifdef __EMSCRIPTEN__`).** (a) The SIGALRM/setitimer host timer → a COOPERATIVE tick: `fist_wasm_tick()` advances the INT-8 time base once per `fist_timer_pump()` (pumps fire at the engine's spin-waits/port-I/O, so ticks accrue in lockstep with engine progress; the rendered frame is rate-independent → native parity). (b) The SIGSEGV/backtrace + `mprotect` FIST_FBTRAP diagnostics are compiled out (`fbtrap_arm_hook` is a no-op). setjmp/longjmp exit and the `gettimeofday` FIST_RUNMS watchdog are portable and shared. `fist_dos.c` already had its `<execinfo.h>`/backtrace guarded.
- **`call_indirect` signature mismatch (the one real WASM delta).** The engine's whole indirect-call surface is untyped K&R `code` (`typedef int code();`) resolved at runtime by `fist_icall` to any `__allregs` target; x86 calls it regardless of declared arity (cdecl), but wasm's `call_indirect` type-checks the table slot → "null function or function signature mismatch". Fixed at the BUILD layer (uniformly, no engine/call-site edit, output byte-identical) with `-sEMULATE_FUNCTION_POINTER_CASTS=1`, which emits adapter thunks restoring the native untyped-call ABI. One toolchain wrinkle: the widest `__allregs` fn has 19 params and binaryen's `fpcast-emu` universal thunk caps at 16 → raised via `-sBINARYEN_EXTRA_PASSES=pass-arg=max-func-params@64`. (DD2's doctrine-pure alternative — per-dispatch-family arity normalization — remains the long game; this restores the invariant at once, which is the point of doing it now at the small clean menu surface.)
- **NO REGRESSION.** `make native` rebuilt with the seam edits → menu byte-identical to the pre-edit native (0 diff). `make check` = all 122 patches apply cleanly. `re_out/*` engine + patches untouched (only `tools/native_main.c`, `tools/build.sh` [new], `tools/wasm_pre.js` [new] changed).
- **Env to node:** `tools/wasm_pre.js` (`--pre-js`) mirrors `process.env` → emscripten `ENV` so the `FIST_*` getenv knobs work exactly as native; `-sNODERAWFS=1` gives node's real fs so `re_out/*.bin` / `armoredfist/*` resolve from the repo root.

## DONE (2026-07-09) — **THE MAIN MENU IS GENUINELY BIT-IDENTICAL (STAGE 2 CLEARED FOR THE MENU): AE = 0/64000 vs a REAL native 320x200 DOSBox capture.** 4 render patches (119–122). `make check` = all patches apply; default boot crash-free (rc=0, no SEGV); intro plays → menu renders bit-identical to the DOSBox reference (bg + gray box at row 62 + all 7 items + version string "AF-FD 1U10" + white cursor + palette). **NO REGRESSION.**

**VERIFICATION REDONE HONESTLY.** The earlier "AE=0 vs `ref/main_menu.png`" claim on this line was FALSE — it compared the render against a byte-copy of the port's OWN output (circular). Re-verified against a **genuine native reference** `ref/main_menu_native320.png` (from `tools/refcapture_native.sh`, committed & reproducible): DOSBox rendered **1:1 UNSCALED** (`[render] aspect=false scaler=none`, `[sdl] output=surface`), windowed on xvfb; the game rect `-trim`s to **exactly 320x200** (asserted) so the X-grab IS the native VGA framebuffer, no resampling. 6→8-bit is VGA bit-replication `(v<<2)|(v>>4)` on BOTH sides.
- **HONEST AE = 0/64000** — via `compare -metric AE` (PNG and PPM) and a raw `cmp -l` of the two 192000-byte RGB buffers (0 differing bytes).
- **Non-circular / deterministic:** reference md5 `d14c96a0…` ≠ port `48767fb4…`; **three independent DOSBox captures** (`d14c96a0`/`eba6dc70`/`367551bb`) each AE=0 vs each other and vs the port.
- **Why the interim "~55k" was wrong:** that was a SCALED grab downscaled to 320x200 (resampling artifacts). Native 1:1 capture removes them → AE=0. No new patches were needed — the render was already correct; only the verification was broken.

**MEASUREMENT PROTOCOL.** Port side: the stable menu appears ~20 s in at `FIST_TICK_HZ=25000`; capture with the self-exit watchdog, NOT a mid-render SIGTERM:
`timeout 30 env FIST_TICK_HZ=25000 FIST_RUNMS=22000 FIST_FBDUMP=/tmp/fb.ppm /tmp/fist_native` then `convert /tmp/fb.ppm /tmp/fb.png`.
Reference side: `tools/refcapture_native.sh 40 ref/main_menu_native320.png`. Compare: `compare -metric AE /tmp/fb.png ref/main_menu_native320.png null:`. (The old 11685 figure was a SIGTERM dump taken mid-enter with the box half-drawn at row 31; NEVER compare a partial frame, and NEVER compare against a downscaled grab.)

**THE FOUR FIXES (all asm-verified):**
- **119 — the 0x7c8 rowtable pointer-scaling (builder + reader together).** `FUN_0000_0284`/`02c1` build the DGROUP:0x7c8 scanline table `rowtab[n]=n*320` with `stos ax` = 200 WORD entries, but Ghidra typed the store `int*` (4-byte stride) → readers indexing at word offset `row*2` saw `rowtab[row/2]` = HALF the row (box at 31 not 62). Fix builder → WORD stride + the reached reader `FUN_0000_2fd7` → WORD read. (`FUN_0000_1091` box fill and `FUN_0000_26de` already read uint16_t; the box-drawer on this path is actually the c684 text/glyph + the 1091 fill, not 26de — 26de/26a1 are clipped out for the box sprite. The rowtable is consumed by the reached readers only, so builder-alone regressed the mode-set blit 2fd7 exactly as the notes warned.) **AE 11685→204** (residual = version string + cursor).
- **120 — `FUN_1000_a370` version-string draw color/shadow.** Drew "AF-FD 1U10" with a black drop-shadow; Ghidra dropped the incoming colour `cl` (8/0xd), the `mov al,0` shadow colour, and the pop-restored pos2=(row-1,col-1)/2nd string arg. Added the colour as param_4 + reconstructed; threaded at all 3 call sites.
- **121 — the per-glyph blitter (c684 = driver 0x19ae → 0x19ea).** `0x19ae` is a mid-function entry Ghidra folded into `FUN_0000_198e` → absent from the driver fmap → c684 dispatched nowhere. Reconstruct `0x19ae` (wrapper: fb-check → 2d6d + rowtable setup → 19ea → 2e12) + register it in the fmap **(sorted before 0x19ca — binary-search order matters)**; reconstruct `FUN_0000_19ea` (font-seg [0x71a] / fb-seg [bx] / rowtable base-losses + the mangled 8-bit/row plot idiom `add ah,ah; je; inc di; adc cx,cx; sub di,cx; rep stos` — which is a plain 8px/row 1bpp blit, the je just skips trailing zeros). **AE 204→34** (residual = cursor colour).
- **122 — cursor colour white.** The at-rest arrow (default position (0,0), correct in both) drew in palette index **254 (red)** not **255 (white)**: `FUN_0000_0db8` blits `fb = sprite AND d5c2`; `FUN_0000_e714` sets the colour mask via `mov al,0xff; call [DGROUP:0x1598]` (→ 2d1f), but the __allregs vector site dropped al=0xff → 2d1f got a stale 0xfe → `0xff & 0xfe = 0xfe`. Thread al=0xff. **AE 34→0.** (The `[DGROUP:0x510]` cursor-colour vector in be0e/be67 is uninstalled (=0) and irrelevant on this path; the live setter is DGROUP:0x1598 in e714.)

New env-gated shim diagnostics (`tools/native_main.c`): `FIST_ROWDUMP` (0x7c8 table), `FIST_FBTRAP=<fb-offset>` (mprotect write-trap that prints the EIP of whoever writes a framebuffer page — the tool that finally located the drawers), `FIST_CURIDX` (cursor state + fb index).

## DONE (this iteration) — **THE FULL-COLOR MAIN MENU NOW RENDERS ON A NORMAL RUN (no FIST_PALNOW): frame completes -> palette auto-uploads, and the COMPLETE background paints (all 200 rows incl. the bottom foreground tank).** 2 new patches (117/118) + 2 shim fixes (`fist_vga.c`, `native_main.c`). **`make check` = all patches apply; `re_out/fist.c` byte-identical (md5 `61453e42`, 2269 fns); default boot crash-free (rc=124 alive, not SEGV); intro plays 395 frames -> menu; NO REGRESSION.** Proof: `scratch/menu_normalrun_autocolor.png` (a plain `timeout` run, NO FIST_PALNOW).

**FRONTIER 1 (frame completion + auto palette) — CLEARED.**
- **117 — `FUN_0000_e714` screen-enter tail jumptable + idle-loop CF + cooperative pump; `FUN_1000_3907` CF.** The screen-enter SEGV'd at `e714+0x1e5`: the tail dispatch `mov bx,[DGROUP:0xf78e]; mov ax,cs:[bx-0x1886]; jmp ax` is a jumptable Ghidra failed to recover AND mis-based (used the state var's VALUE as a host ptr -> deref ~0xffffe78a). Reconstructed: the table is at CS(=0)-relative -0x1886 == linear 0xe77a (8 word entries); the state var DGROUP:0xf78e holds a byte offset; **state 0xa -> 0xe753 = resume the idle paint loop** (steady-state menu). Modeled as an outer state loop (target 0xe753 continues, 0xe78a returns, else tail-dispatch). The idle loop exit `jb 0xe753` tests c40a (=`FUN_1000_35a7` event scheduler) CF; Ghidra dropped the flags so the C tested a stale local -> either SEGV'd at the jmp or spun forever. Threaded via `g_fist_cf` (3907 gates the idle housekeeping, 35a7 gates the loop). And **added `fist_timer_pump()` in the idle loop** (this loop IS the menu event/vblank spin-wait -> the cooperative INT-8 tick belongs here, same pattern as 017/074/086). The INT-8 ISR `FUN_1000_30f8` then runs the present `FUN_1000_31c3` each tick. `3907` (asm 0x13907) had its clc/stc.lret dropped -> restored g_fist_cf.
- **shim `fist_vga_service_retrace()` (`fist_vga.c`) driven from the timer pump (`native_main.c`).** The palette upload is the driver's ASYNC vertical-retrace ISR `FUN_0000_0b1f` (uploads word[DGROUP:0x782]->DAC when 0x786&1); our port has no retrace IRQ and only faked it on `in(0x3da)`, which the menu idle loop never polls. Refactored the 0x786 service into `fist_vga_service_retrace()` and call it from BOTH `in(0x3da)` AND `fist_timer_pump` (the "IRQs fire" heartbeat) -> the menu palette auto-uploads. **The uploaded palette matches MAINMENU.MRL byte-for-byte (verified via FIST_PALDUMP).**

**BONUS — full background + color accuracy.**
- **118 — `FUN_1000_2593` read the FULL resource (16-bit sign).** The loader read loop (asm 0x125ad: `int 21h AH=3F CX=0x8000 ; add 0x800,[2b62] ; or ax,ax ; js loop`) loops while the chunk was full (AX bit15 set). Ghidra rendered the back-edge `(int)uRam000f0000 < 0` but uRam000f0000 is undefined2 (16-bit) so `(int)` ZERO-extends 0x8000 to +32768 -> exit after ONE 32768-byte chunk. **MAINMENU.MRL is 53096 bytes -> the PackBits image was truncated to ~130/200 rows (bottom foreground tank black; nonzero 40311/64000).** `(short)` sign-extend -> reads 32768+20328=53096, full 200-row scene (nonzero 61357/64000).
- **shim `fist_vga.c` 6-bit->8-bit = VGA bit-replication `(v<<2)|(v>>4)`** (was `v*255/63` truncation, off-by-one on most values) so the dump is pixel-comparable to the DOSBox reference (0x33 -> 0xCF=207).

**RENDER vs `ref/main_menu.png` (320x200): AE = 11685/64000 (18%).** Sky/hills/terrain/foreground now MATCH the ref (pixel samples off by <=1 pre-conversion; byte-exact palette). The residual 18% is entirely: (a) the **menu box + 7 items are at HALF their row** (box top at row 31, should be row 62 per descriptor {row0=62} = the ref), and (b) the **version string "AF-FD 1U1Q" + mouse cursor are absent**.

**REMAINING BLOCKERS (all root-caused; all gated on ONE artifact) — the DGROUP:0x7c8 scanline-offset table int/word pointer-scaling.** The box, the version-string glyphs, and the cursor are ALL drawn by sprite/glyph blitters (`26de`, `c684->...`, `0db8` via `2fd7`) that index the scanline table `word[DGROUP:0x7c8 + row*2]`. That table is BUILT by `FUN_0000_0284` (asm 0x2a9: `xor ax,ax; mov bx,0x140; mov cx,0x64; L: stos ax; add ax,bx; stos ax; add ax,bx; loop L` = 200 WORD entries `rowtab[k]=k*320`), but Ghidra typed the table pointer `int*` (4 bytes under gcc), so it writes 4-byte entries at a 4-byte stride -> the WORD reader gets `rowtab[62]=31*320` -> **every sprite/glyph renders at HALF its row.** ATTEMPTED fixing the builder to true 16-bit words: it renders `rowtab[62]=62*320` (box would move to row 62) **but breaks the whole driver mode-set** (`FUN_0000_00e8` never reaches its INT 10h AX=13h) — the driver was tuned across many prior patches around the buggy half-resolution layout (multiple OTHER `0x7c8` accessors `2931/2946/3198/3201/5997/2fd7` are still Ghidra-typed `*(int*)(row*2+0x7c8)` = 4-byte reads at word offsets AND base-lost; boot-time renders assume the half-row coords). **The correct fix is a COORDINATED pointer-scaling correction of the 0x7c8 builder + ALL its readers in one patch** (the classic DD2 "pointer scaling" artifact class, here spread across the driver). Once coherent: the box lands at row 62 (pixel-exact bg+box), then reconstruct `c684`=MGAVIDEO 0x19ae glyph-blit subtree (version string) and drive the INT-33h cursor draw (`0db8`) — both then land at the right rows. NEW diagnostic: `FIST_PALDUMP` (dump word[0x782] vs the .MRL file palette).

## SUPERSEDED (prior iteration) — **THE FULL MAIN MENU RENDERS IN COLOR: title-art background + gray menu box + all 7 items (SELECT PLAYER / CAMPAIGNS / BATTLES / REVIEW / SETTINGS / ABOUT FIST / QUIT), matching `ref/main_menu.png`.** 5 new patches (112–116) + a `FIST_PALNOW` diagnostic (`re_out/fist_vga.c`). **`make check` = 114 patches apply clean (numbered to 116); default boot 3/3 crash-free; box+items blit to 0xA0000 (nonzero 40311/64000, 208 colors); the engine's OWN framebuffer + palette buffer (`word[DGROUP:0x782]`, the real `MAINMENU.MRL` palette) render the full-colour menu — proof `scratch/menu_render_FULL_color.png`. NO REGRESSION (bg byte-exact + intro + default boot all intact).**

**The chain, e9f0 (MAINMENU per-frame paint) = e3bc(bg,0340) -> e38a(2nd-elem SAVE) -> [0x6b4]=26a1(box+items sprite) -> 524e(0) -> a358/a370 (version string).** Fixes, all asm-verified:
- **112 — `e38a` 2nd-element SAVE arg-thread.** `mov [0xf6f2],bx; mov dx,0xf6ee; lcall [0x5fa]; lcall [0x5fe]`: bx=0xf790 (rect descr {row0=62,col0=111,row1=161,col1=209} = the box), dx=0xf6ee (save descr).  c5fa=MGAVIDEO 0f1a (save-buffer reserve), c5fe=0f3a (save blit->0f62).  The __allregs vector sites dropped bx/dx -> 0f62 di=0 SEGV.  Threaded both.
- **113 — `0f62` SAVE blit reconstruct.** param_1/param_2 are DGROUP near offsets (not host ptrs); the es(save seg)/ds(fb seg)/row-table bases were dropped; the `jns 0xf77; call 0xf60` (0f60=`pop ax;ret`) EARLY-RETURN idiom was mis-modeled as a normal call.  For the MAINMENU 2nd element the flag byte starts 0 => sign bit clear after `|0x20` => EARLY RETURN, no save buffer needed on first paint (the reserve method 0f1a's [DGROUP:0xcc] object-vtable slot is not resident this phase -- it is a per-object vtable slot 0xc4..0xd0 installed via reloc section si=0xec, cleared after driver load; only used when the element is marked dirty/sign-set).
- **114 — box+items SPRITE BLIT (260c clip + 26a1 dispatch + 26de blit) + `[0x6b4]` thread.** `e9f0` draws the box+items sprite `mov ax,0x13d8; mov bx,0xf790; lcall [0x6b4]=26a1`. ax=0x13d8 = the box sprite byte-offset into the MSPRITE6.BIN directory (DGROUP:0x4f0).  **260c** (clip): dropped the directory indirection `mov si,ax; mov es,[0x4f0]; les si,es:[si]; mov ax,es:[si-4]` (read ax as a resolved far-ptr), the DGROUP bases, ES:SI/CF/ZF returns, AND wrote the clip words DGROUP:0x1588..0x158e through the undefined4 `_DAT_1000_d58X` macros whose 4-byte stores OVERLAP (d588 zeroes d58a's high half -> column came out 0 -> box wrapped to the left edge) -> write uint16_t.  **26a1**: used in_CF/in_ZF=0, dropped 260c's ES:SI, read [0x1588] at the wrong base (0x1c588 vs 0x1d588=DGROUP:0x1588).  **26de**: masked 3-mode blit, dropped fb-dest/source segs + `_DAT_1000_c724` host-deref -> rebuilt (dest=word[[0x724]][0]:rowtab[row]+col, source=recseg:recoff).
- **115 — version-string text renderer seg (a358 width + 5591 glyph loop).** The string bytes are in the far string seg es=[DGROUP:0x70]=0x2d74 (5591 hardcodes 0x2d74); Ghidra dropped it (offset read as host ptr) and the c684(al) char arg.  Rebased a358 + 5591 + its 0x1558b thunk.
- **116 — `1e88` display-list element method-dispatch base.** cae6->e714->1e4b->1e88 walks the node list DGROUP:[0x3e04]..[0x3e06] calling each element method `di=word[DGROUP:node]; call [DGROUP:di+0x4872]`.  param_1/param_2 were host ptrs (deref 0x3b6c SEGV); rebased.

**FRONTIER (next iteration) — frame COMPLETION for the auto-DAC-upload + the version string glyphs.** The render (indices) AND the palette buffer (`word[0x782]` = the real MRL palette) are BOTH proven correct; the DAC is only black in a normal run because the frame never reaches the `in(0x3da)` retrace poll that uploads it (`FIST_PALNOW=1` forces the upload -> full colour, confirming the pipeline).  After 116, execution runs past 1e88 into more `FUN_0000_e714` base-losses (next: `e714+0x1e5`, a `*(DAT - 0x1886)` DGROUP negative-index + an `icall_near` -- another screen-enter dispatch base-loss).  Chasing e714's remaining base-losses to frame completion (then the retrace poll auto-uploads the palette) + the version-string glyph blitter (c684 = driver 0x19ae -> the deep glyph subtree) are the next steps.  New diagnostics: `FIST_PALNOW` (`fist_vga.c`, force the 0x782->DAC upload before an FB dump); the segv handler dumps the palette buffer + DGROUP vectors under `FIST_DGDUMP`.

## DONE (prior iteration) — **THE MENU BACKGROUND RENDERS BYTE-EXACT AND IN COLOR: `MSPRITE6.BIN` + `MAINMENU.MRL` now load on the engine's own path (both VERIFIED byte-for-byte against the real game via a DOSBox open-trace oracle), `FUN_0000_0340` paints the real title scene (two M1 Abrams + Mi-24 Hind + muzzle-flash + terrain; raw indices 62259/64000 nonzero, 177 colors — the docs' proven byte-exact render), and the DAC palette upload is wired (the engine's own `word[DGROUP:0x782]` palette produces the full-colour scene).** 4 new patches (108–111) + the palette-flush shim service in `fist_vga.c`. **Engine `re_out/fist.c` byte-identical (md5 `61453e42`, decompile pristine); 109 patches apply clean; default boot 5/5 crash-free; 2269 engine fns. NO REGRESSION.** Remaining before the FULL interactive menu: the `0f62` 2nd-element (gray menu box + 7 item strings) — its save-buffer (`word[DGROUP:0xf6ee]==0`) is not yet allocated, so the per-frame paint SEGVs there before a retrace poll fires the (correct) palette upload.

**THE FILENAME-INDEX MISDIAGNOSIS, CORRECTED BY THE ORACLE.** The prior notes' blocker #1 ("thread index 13 = `MAINMENU.MRL` into `FUN_0000_153c`→`c550`") was WRONG. Ground truth from a DOSBox host-`open()` trace (`LD_PRELOAD` hook; the DOSBox oracle reaches the menu): the menu opens, in order, `MSPRITE0.BIN, INTRO.MS3, TITLE.KDV, MAINMENU.MS3, MSPRITE6.BIN, MAINMENU.MRL`. So **`153c` faithfully builds `MSPRITE6.BIN`** (the menu-item sprite sheet), NOT `MAINMENU.MRL`; and **`MAINMENU.MRL` loads via a SEPARATE element** — the driver bg-loader `MGAVIDEO 0x310` (= `DGROUP:0x560`), called from `e3bc → e3f8` with `ax=0x1a` (name-table byte-offset 0x1a = 13-entry index 13). The driver 13-byte-stride name table + the runtime word-pointer table (`ds=[DGROUP:0x70a]`, `bx=0x1a → MAINMENU.MRL`) were dumped live to confirm.

**THE FIVE FIXES (all asm-verified; the two threaded call sites are per-patch indirect-vector arg threading, CLAUDE.md doctrine):**
- **108 — `FUN_0000_153c` element registrar (`MSPRITE6.BIN`).** loop `c550` threaded `ax=0x24` (asm 0x1524 → name-table[0x24]="MSPRITES.BIN"; 0x197's digit-patch → "MSPRITE0.BIN"; the 153c loop patches [nul-5] with the type digit 0x30→'6' → "MSPRITE6.BIN"); the post-loop `c550` threaded `ax=0xffff` (0x197 `cmp ax,0xffff; je skip` = NO-OP — Ghidra's dropped-arg garbage 0 had rebuilt "M1CON.MRL", clobbering it); the `c388`/26fc open threaded `cx=param_2`(descr seg), `dx=0x740`(filename), `bx=param_4`(descr off). Without `dx` the open used `DS:DX=1c00:1c00` (empty name).
- **109 — `FUN_1000_26fc` loader near-dispatch.** The `push si; mov dx,si; call cs:[di+0x2b82]` dispatch to the variant loader (`2508→250d`) dropped `cx`/`dx`(=si=filename)/`bx`; threaded (loader order `CX,DX,BX,BP,SI,DI`, per patch 100) so `250d` opens the real file, `184b` stores its segment into the descriptor word0, and `2593` reads it.
- **110 — `FUN_0000_e3f8` bg load.** asm 0xe3fd `mov bx,0xf6cc; mov cx,ds; mov ax,0x1a; call [DGROUP:0x560]` threaded `(ax=0x1a index, cx=0x1c00 seg, bx=0xf6cc bg descriptor)` — the far-vector site had dropped all three, so the bg loader built an empty name and 0340 painted segment-0 RLE noise.
- **111 — `MGAVIDEO FUN_0000_0310` bg-element loader.** Rebuilt the driver thunk (asm 0x310..0x327 = `or es:[bx+2],0x22; call 0x197(ax); mov dx,0x740; call [DGROUP:0x388]`): Ghidra dropped the descriptor's `cx` segment base (raw host deref of param_3) and the 26fc `dx/cx/bx` args. Reconstructed `descriptor = g_mem+(param_2<<4)+param_3`; threaded `cx=param_2, dx=0x740, bx=param_3`.
- **palette flush (`fist_vga.c`, shim) — blocker #5 SOLVED.** The driver's vertical-retrace ISR `FUN_0000_0b1f` (asm 0xb25 `shr BYTE ds:0x786,1; jae`) uploads the 768-byte 6-bit palette buffer at `word[DGROUP:0x782]` to the DAC (`out 0x3c8,0; 768× out 0x3c9`); our port runs no driver IRQ, so the `in(0x3da)` retrace-poll handler (which the engine busy-waits on) now services the `0x786` semaphore: consumes bit0 (shr) and uploads `word[0x782]` → `g_pal`. `0340` populates the buffer from the `.MRL` and sets `0x786=1`. Verified correct: forcing the upload (`FIST_PALNOW`) renders the full-colour tank scene; the 0b1f base-loss (`puVar10=0` instead of `word[0x782]<<4`) is the same defect, avoided by servicing in the shim.

**FRONTIER — the `0f62` 2nd element (gray menu box + 7 item strings SELECT PLAYER/CAMPAIGNS/BATTLES/REVIEW/SETTINGS/ABOUT FIST/QUIT).** Per-frame paint `2112/209e → e9f0 → e3bc(bg, OK) → e38a(2nd elem)`. `e38a` (asm 0xe38e) does `mov dx,0xf6ee; call [DGROUP:0x5fa]=0f3a; call [0x5fe]` — a background-SAVE (`0f62`: `es:di ← ds:si`, framebuffer rect {x0=62,y0=111,x1=161,y1=209} → the descriptor's save-buffer `word[0xf6ee]`). Two blockers, in order: (1) the `c5fa`/`c5fe` far-vector calls drop `dx=0xf6ee`/`bx=0xf790` (→ `0f62` param_1=0 → SEGV fault-addr 0x2); (2) even threaded, **`word[DGROUP:0xf6ee]==0`** (the save-buffer segment is NOT allocated — the 2nd element's CREATE/setup that allocs it is not yet threaded), and `0f62` itself has full base-losses (param_1/param_2 host-ptr, es=save-seg/ds=fb-seg dropped) so it needs reconstruction like `0340`/`0f3a`. Until the buffer is allocated + `0f62`/`c5fe` reconstructed, the frame SEGVs before a `0x3da` poll fires the (verified-correct) palette upload — hence the clean build shows the byte-exact INDEX render but a black DAC. New diagnostics (env-gated, `fist_dos.c`): `FIST_OPENBT` (backtrace a failed open), `FIST_NAMEDUMP` extended to dump the driver name-table; `FIST_PALNOW` (`0340`, build-only) forces the palette upload.

**ROOT CAUSE of the cursor SEGV (three interlocking store/load-width bugs, all asm-verified — NOT an init gap).** `FUN_0000_0db8` ran away because the cursor WIDTH `d5da` read 0 → `2fd7` computed `d5bc`(columns) = -1 = 0xffff → the blit's byte column-counter wrapped and walked off. The width IS loaded correctly by the cursor-shape select `FUN_0000_2ccd` (verified live: it reads descriptor `09 0b 01 01 28 00 8b 00` = w=9,h=11,hotspot(1,1),sprite@0x28,mask@0x8b into `d5da..d5e4`). But three Ghidra type-width defects then corrupted it:
- **Patch 104 — the four cursor show/hide counters `DGROUP:0x5d6/0x5d7/0x5d8/0x5d9` are BYTE, not WORD.** Asm is `mov/inc/dec/test/shr BYTE PTR ds:0x15dX` at every site, but Ghidra typed them `undefined2`, so `FUN_0000_2cc1`'s `DAT_1000_d5d9 = 0` (asm `mov BYTE PTR ds:0x15d9,0`) wrote a WORD, and byte `0x1d5da` overlaps its high byte → the cursor-hide zeroed the cursor WIDTH `d5da`. Retyped all four to `undefined1` (overlap cascade d5d6→d7→d8→d9→d5da).
- **Patch 105 — the screen-dim clip bounds `DGROUP:0x730`(height) `0x732`(width) are WORD, not undefined4.** The mode-set `FUN_0000_00e8` sets `c732=0x140`(320) then `c730=200`; because `_DAT_1000_c730` was `undefined4`, the DWORD store `c730=200` overwrote `c732` back to 0 → `2fd7`'s right-edge clip (asm 0x3055 `sub cx,c732`) clamped columns to -1. Retyped both to `undefined2`.
- **Patch 106 — the whole cursor render/clip state block `DGROUP:0x5b8..0x5e4` (19 vars) is 16-bit WORD, not undefined4.** This is 16-bit real-mode code; objdump shows 0 `DWORD PTR` refs across all 19. The dword typing made `_DAT_1000_d5da` read `width|height<<16` = 0x000b0009 in `2fd7`, so the clip `if (c732 <= (uint)(iVar2 + d5da))` saw a bogus huge extent and took the branch the asm skips (0x3059 `jb`). Retyped the contiguous block to `undefined2`.
- **Patch 107 — `FUN_0000_0e3b` (cursor background RESTORE) segment base-loss** (the inverse of 0db8, same class as patch 103): ES=0xA000 FB / DS=`c716` bg-save segment both dropped → SEGV at fault-addr 0xaa4. Rebased.
- **Patch 099 revision — `FUN_1000_26fc` dispatch index `iRam000f000a` is the DI reg-file slot (16-bit); the `int`-wide accessor aliased the adjacent BP slot (0x1c00) in its high word** → `idx*2` = 0x38000000 → SEGV in the near-dispatch table lookup. Masked to `(uint16_t)`.

**NEW FRONTIER (precisely scoped, runtime-confirmed) — the display-list ELEMENT resource-load + the `0f62` 2nd-element blocker + the DAC palette.** With the cursor closed, the per-frame render now: paints the bg (`0340`, 62245/64000 px, 256 colors — but from segment 0 = RLE NOISE, not the real `.MRL`, because the element resource never loads), draws+restores the cursor, then SEGVs at **`m_mga_FUN_0000_0f62 +0x1e` deref `[0x2]`** = the documented NULL 2nd-element descriptor (`DGROUP:0xf790.word0 == 0`, reached via `e38a → 0f3a → 0f62`). The framebuffer is BLACK on dump because the DAC palette (`0410` path) is still not uploaded (blocker #3). Three interlocking items, in order:
1. **The element filename is WRONG: `FUN_0000_153c` (the element registrar) → `c550` (driver 0x197 filename-builder) builds `"M1CON   .MRL"` (name-table index 0) instead of `"MAINMENU.MRL"` (index 13).** Root cause = the same indirect-vector arg-drop class: `c550` is a `call [DGROUP:0x550]` `__allregs` far vector, so the name-INDEX in `ax` (asm 0x1524 `mov ax,0x24` for the loop call; the input `ax` for the post-loop call) is DROPPED → `c550` reads name-table[garbage≈0] = M1CON. Fix: thread the correct name index to BOTH `c550` sites in 153c (asm 0x1527/0x1549), reconstructing where the index comes from (loop uses 0x24; determine the post-loop/menu-bg index — the `[DGROUP:0x70a]` word-pointer name table, MAINMENU is idx 13). Backtrace-proven: crashing 26fc caller = `FUN_0000_153c +0x105`.
2. **`153c → 26fc` (`call [DGROUP:0x388]`) drops `dx=0x740`** (asm 0x154f `mov dx,0x740; call [0x388]`) — the indirect method-vector site passes `param_2=0x3178` (garbage) instead of the filename buffer `0x740`. Thread `26fc(cx=descr_seg, dx=0x740, bx=descr_off, bp)` faithfully at the 153c call site (line ~7252, the `(*c388)()` after `(*c550)()`), so the loader `250d/2593` opens+allocs+reads the resource and stores its segment into the element descriptor's word0.
3. **The `0f62` blit + the `0410` DAC palette flush** — once the 2nd element's descriptor word0 is populated, `0f62` no longer null-derefs; then push `word[DGROUP:0x782]` (the loaded `.MRL` palette) to ports 0x3c8/0x3c9 (base-loss/timing). The `0340` PackBits render is already proven byte-exact vs `ref/main_menu.png`.

**NB — WASM build is broken by a PRE-EXISTING infra gap** (`tools/build.sh` referenced by `make wasm` does not exist; unrelated to this iteration — all 105 patches stage cleanly for wasm and the changes are portable C type-width fixes). New env-gated diagnostics used (throwaway, not committed): `FIST_CURDBG` (0db8/2ccd cursor state), `FIST_R26DBG` (26fc caller/filename).

## SUPERSEDED — **THE MENU IS `.MS3`-DRIVEN AFTER ALL: `MAINMENU.MS3` NOW OPENS + FULLY READS on the engine's own path, and execution dispatches INTO the driver's per-frame screen-render subsystem. The prior "the shipping menu is NOT `.MS3`" model correction is itself RETRACTED — runtime proves the screen-enter loads a per-screen `.MS3` script.** 4 new patches (100–103) + a general `fist_icall` far-thunk resolver. **Engine `re_out/fist.c` byte-identical (md5 `61453e42`, decompile pristine); 101 patches apply clean; default boot 5/5 crash-free (rc=124, alive in intro); intro plays 395 frames → menu. NO REGRESSION.**

**THE REAL MENU-LOAD PATH (backtrace-proven, supersedes the whole 153c/c550/26fc `.MRL` model for screen-enter):** `cae6 → e714 → FUN_0000_be0e(4) → (*c378)() = FUN_1000_250d` opens **`MAINMENU.MS3`**, not the `.MRL` directly. `be0e(param_1)` is the SCREEN-ENTER handler: `param_1` = the screen id (4=MAINMENU, 8=LOADING, …); asm 0xbe2a `mov dx,[bx+0x9f2c]` indexes the **4-byte-stride screen→`.MS3` table at DGROUP:0x9f2c** (`{name_off:word, seg:word}`; the offsets point at ASCIIZ names in DGROUP — idx4=0x9f30→`"MAINMENU.MS3"`, idx8=0x9f34→`"LOADING.MS3"`, `INTRO/WWIN/WLOSE/EWIN/ELOSE/MSN1-3.MS3`). So the earlier "`.MS3` is a red herring" retraction was WRONG — the screen system IS `.MS3`-script-driven; the `.MS3` interpreter (in the driver) is what then loads `MAINMENU.MRL` + builds the display list + drives `0340`.

**Patch 100 — `be0e` c378 open thread.** The `lcall [DGROUP:0x378]` (installed by reloc section si=0x174; stably resolves to 250d, the direct open/size/alloc/read loader) passed `dx=word[DGROUP:param_1+0x9f2c]` (filename off), `bx=0x9f1c` (screen descriptor), `cx=ds` — the `__allregs` indirect method-vector site dropped all three → `250d` opened an empty DX offset (`3D open '' -> FAIL`). Threaded faithfully (250d param map: param_1=CX descr-seg, param_2=DX filename-off, param_3=BX descr-off). → `3D open 'MAINMENU.MS3' -> ok`.

**Patch 101 — `250d`/`2593` read lands (the file is actually READ).** Two Ghidra defects left the screen resource allocated-but-never-read: (a) a spurious `if (!bVar8)` gate SKIPPED the `2593` read (bVar8=`(uVar2&4)` is a mis-decode of the 33-bit rcr/shr paragraph-count carry chain, asm 0x1255e; the asm calls 2593 UNCONDITIONALLY after a successful alloc — gate on the real alloc CF `g_mm_cf`); (b) `DAT_1000_2b5e = *DAT_1000_2b66` dereferenced the descriptor far-ptr (typed `int**`) as a raw host pointer → SEGV — the load segment is the descriptor word0 that `184b` wrote (read it via the g_mem accessor). And `2593` itself reads the file into ES:0 where ES=the freshly-allocated segment, but Ghidra dropped ES (`unaff_ES`) → threaded via `DAT_1000_2b5e` (both callers publish the alloc there). **→ `3F read h=8 n=32768 -> 3561` — the full 3561-byte `MAINMENU.MS3` reads into the descriptor's segment.**

**`fist_icall` FAR-THUNK RESOLVER (general, faithful, `re_out/fist_icall.c`).** Driver method vectors point at 4-byte **`E8 rel16 CB` = `call near <inner>; lret`** far-entry thunks that Ghidra recovers the inner near function of but not the thunk itself → the thunk offset misses the driver fmap (`TRAP overlay -> MGAVIDEO.DVR+0x2d1b/0x2d4e`). On an fmap miss the dispatcher now detects the thunk bytes and resolves to the inner near function (executing the thunk IS a near-call + far-return, which the inner `__allregs` C already models). Unblocks the whole driver method-dispatch surface (`c510→0x2d1f` state-set, `0x2d4e→FUN_0000_2d52` screen-processing, …). `FIST_NO_THUNK=1` disables it for A/B.

**Patches 102/103 — first two driver screen-render base-losses (reached via the resolver).** 102: `FUN_0000_2fd7` (blit clip/coord setup) `add ax,[bx+0x7c8]` = a DGROUP row-offset table (DS=DGROUP), was a raw host deref. 103: `FUN_0000_0db8` (masked mouse-cursor blit with background-save) — ES=0xA000 framebuffer, DS=`word[DGROUP:0x716]`=the sprite/mask/backup segment; di=d5c8 FB, si=d5c4 sprite, bx=d5c6 mask, bp=0xaa4 backup — Ghidra kept the loop logic but dropped all four segment bases.

**FRONTIER (precisely scoped) — the driver PER-FRAME SCREEN-RENDER subsystem.** With 100–103 + the resolver, execution now runs `cae6 → …(d755)… → FUN_0000_2d52 → 2ed7(clip)/2fd7(coords)/0db8(cursor)`. The wall is **`0db8` runs away / SEGVs because its sprite/mask/backup segment `c716` (word[DGROUP:0x716]) is NOT INITIALIZED** — an upstream **cursor-sprite-init gap** (the mask has no terminator → the inner draw loop walks off), NOT a base-loss. (The parallel display-list ELEMENT-load path via `26fc` also SEGVs when reached — `FIST_NO_THUNK=1` takes that branch instead of the cursor branch; both are the same driver-render frontier.) **Remaining chain to the menu:** (1) the cursor-sprite/`c716`-segment init (find + drive the driver's cursor-load); (2) the `.MS3` interpreter → it opens `MAINMENU.MRL`, builds the display list, and drives `0340` (proven byte-exact) + the `0410` DAC palette flush. Multi-patch, each an asm-verified driver reconstruction. **New env-gated diagnostics** (`tools/native_main.c` / `fist_dos.c`): `FIST_NAMEDUMP` (dump DGROUP:0x740/0xff4 + tseg on a failed open), `FIST_DESCRDUMP` (dump the screen descriptor word0 + `.MS3` seg bytes at the watchdog), `FIST_NO_THUNK` (disable the far-thunk resolver).

## SUPERSEDED — **SCREEN-ENTER/LOAD HANDLER FOUND + WIRED: the screen-resource OPEN is `FUN_1000_26fc` at `DGROUP:0x388 = 0f69:0x306c`; it never fired because reloc section `si=0x174` (the display-list ELEMENT method vectors `DGROUP:0x344..0x394`) is never applied. Installed it at menu-enter, seeded + decompiled the whole load cluster, and fixed 26fc's base-loss. The resource-open now EXECUTES on the engine's own path.** Engine re-decompiled (2245→2269 fns, reproducible via the updated `SeedServiceVecs`), md5 `re_out/fist.c` = `61453e42…` (supersedes `6cd1f94a`). **97 patches apply; default boot 0/8 SEGV (crash-free); intro plays 395 frames; fast-forward reaches the menu 3/3 — NO REGRESSION.**

**THE ROOT CAUSE (airtight, runtime-confirmed).** The driver's element-LOAD thunk (MGAVIDEO `0x310` = the bg element's `c560`, and the resource registrar `FUN_0000_153c`) builds a filename at `DGROUP:0x740` (via `c550` = MGAVIDEO `0x197`, which indexes the driver name-table at `[DGROUP:0x70a]`) and then does `lcall [DGROUP:0x388]` to OPEN it.  `DGROUP:0x388` was **`0000:0000`** (dumped live via the new `FIST_DGDUMP` seg-fault hook) → the open trapped to 0 → no `.MRL` ever loaded → the descriptor `word0` (resource segment) stayed 0 → `0340` painted garbage and `e38a→0f62` null-deref'd.  `DGROUP:0x388 = 0f69:0x306c = FUN_1000_26fc` is installed by **reloc section `si=0x174`** (a `(off,val)`-pair section at table-seg `0x3352:0x174`, leading seg `0f69`, that populates `DGROUP:0x344..0x394` = the screen-element class METHOD vectors, all far ptrs into the engine service seg `0f69`).  The original Doug-Huffman CRT applies it at load via the working far applier `f842` from **`FUN_1000_223c`** (asm `0x12291`: `xor bx,bx; mov si,0x174; lcall [ss:0x12]`); in our port `f842`'s C body is INERT (Ghidra dropped its string-op DS/ES bases) **and** applying `0x174` at that early-CRT point corrupts the not-yet-ready DGROUP (deterministically breaks `FUN_1000_5c3a` — the reason patch 091 neutered that exact site).  So it must be installed **later**, at menu-ready time.

**THE LOAD CHAIN (reversed end-to-end, asm-verified).**
- `153c(id,seg,out_off,struct_off)` marks the element, calls **`c550` = driver `0x197`** (copies name-table[ax] → `DGROUP:0x740`, uppercased), then `mov dx,0x740; lcall [DGROUP:0x388]` = **`26fc(dx=0x740)`**.
- **`26fc` (asm `0x126fc`, DGROUP:0x388):** saves the filename's ext byte `[0x740+0xa]` at `cs:0x2b7e`, walks the **extension-variant table** `cs:0x2b7e` (`' ','!','@',0` at cs=`0f69` → linear `0x1220e`), for each doing **INT 21h AH=4300 (get-attr probe)** on `DS:DX=DGROUP:0x740`; on the first existing variant it dispatches via the **near table** `cs:0x2b82` (`0f69:0x2e78/0x3230/0x365d/0x318e` = `FUN_1000_2508/28c0/2ced/281e`) to that variant's LOADER.
- **`2508 → 250d` (asm `0x1250d`):** INT 21h AH=3D open, AX=4202 lseek→size, MEMMGR alloc **`FUN_1000_184b`** (which stores the allocated segment into the display-list descriptor's `word0` via the far-ptr `cs:0x2b66` = `es:bx = DGROUP:descr`), then **`FUN_1000_2593`** reads the file into that segment.  So the descriptor `word0` (the `.MRL` resource seg `0340` consumes) is populated by `184b` on this path.

**WHAT WAS DONE (patches 097–099 + shim + decompile):**
- **Seeded the section-0x174 cluster** in `tools/ghidra/SeedServiceVecs.java` (the 16 element-method offsets `0x2cf0..0x3167` incl. `0x306c`=26fc, plus the 4 loader thunks `0x2e78/0x3230/0x365d/0x318e`) → `make decompile` promotes them to real functions (26fc/250d/2593/2508… now decompiled).  The re-decompile perturbed the `__allregs` fixpoint on the MEMMGR cluster (adding `26fc→250d→184b` changed 184b's caller set); the drifted patches (013/015/023/025/065) were re-fitted to the new pristine (023/087 cosmetic; 013/015/025/065 regenerated by splicing the known-good reconstructions from a prior build via the `/* ===== @ 1000:XXXX ===== */` function-header delimiter — see the recovery note).  fe47's chdir helper `23a5`/`23bf` became real functions (they double as element-method vectors) → patch 009 updated.
- **Shim `fist_ensure_dlist_vecs()`** (`tools/native_main.c`, loader role): applies section `0x174` once, at menu-enter.  **Patch 098** calls it at `FUN_0000_e714` entry (after boot+intro, before the first element paint).  Live: `DGROUP:0x388` is now `0f69:306c` and `26fc` executes.
- **Patch 097** — regression fix: the re-decompile's register perturbation exposed **`FUN_1000_5214`** (`xor bx,bx; mov si,0x2f0; lcall [DGROUP:0x12]`, `xor bx,bx` dropped) — a documented "bare `[DGROUP:0x12]` site" now reached on the driver-init path with garbage bx → deterministic `f842` crash before the menu.  Threaded `bx=0` (patch-091 class).  Without it the intro fast-forward crashed in `f842`; with it, the menu is reached again.
- **Patch 099** — `26fc` base-loss: Ghidra decoded 26fc under `CS=0x1000` and dropped the CS(=`0f69`)/DGROUP bases (ext-variant + near-dispatch tables `cs:0x2b7e/0x2b82` → host offsets; filename byte `[buf+0xa]` → host deref) → crash at the dispatch (+0x2d).  Rebased cs-data → `g_mem+0xf690`, filename → `g_mem+0x1c000+buf`, near dispatch → `fist_icall_near(0xf69,table[idx])`.  `26fc` now runs to completion.

**FRONTIER (scoped, the next unlock).** With `DGROUP:0x388` live and `26fc` fixed, the open EXECUTES but does not yet load `MAINMENU.MRL` because **the driver filename-builder `c550` = MGAVIDEO `0x197` has un-reconstructed base-losses** (name-table seg `[DGROUP:0x70a]`, dest `DGROUP:0x740`, the `153c` index/`dx=0x740` threading), so `DGROUP:0x740` isn't populated with `"MAINMENU.MRL"` (idx 13) → 26fc's AH=4300 probe fails → returns without loading → descriptor `word0` stays 0 → the same `e38a→0f62` null-deref (now reached via a *working* resource-open one filename-build away).  **Remaining, in order:** (1) reconstruct `0x197` (`c550`) name-table build + thread `153c`'s `ax`(index)/`dx=0x740`; (2) `250d`/`2593` base-losses so the open+alloc(`184b` word0-store)+read lands; (3) the `e38a/0f62` blit + the `0410` DAC palette flush (indices byte-perfect, palette still black — a base-loss/timing item).  The render side (`0340` PackBits) is already proven byte-exact vs `ref/main_menu.png`.

**RECOVERY NOTE (for reproducibility).** A `FIST_FRESH=1` re-decompile with added seeds perturbs the `__allregs` fixpoint across the dense MEMMGR function region (`096c/0a31/1467/184b/0d70/1345/1e27` …), breaking those patches' context (cosmetic re-rendering of the `DAT_1000_c004/c006/c008` union + new Ghidra `WARNING` comment lines + occasional structural variance).  Re-fit them by splicing the known-good reconstruction FUNCTION BLOCKS (delimited by the `/* ===== ((uint)(uintptr_t)&FUN_1000_XXXX) @ 1000:XXXX ===== */` headers — robust, no brace-counting) from a prior fully-patched build into the new pristine, then `diff -u`.  Do NOT hand-reconcile hunk-by-hunk in this region (weak `}`/blank trailing anchors cause runaway spans).

## SUPERSEDED (this iteration's predecessor) — **MODEL CORRECTION: THE SHIPPING MENU IS NOT DRIVEN BY `.MS3`. It is an embedded-template UI + a driver-indexed `.MRL` background. The `.MS3` "screen-state loader / state-table @ 0x25f6a" hypothesis in the notes below is UNSUPPORTED and is retracted.** No engine change (`re_out/fist.c` byte-identical md5 `6cd1f94a`, 94 patches apply, intro plays 395 frames, default boot crash-free — verified this iteration). Purely a reversal/correction + doc update; no new patches (the doctrine forbids a speculative patch, and the exact loader is not yet reversed — see NEXT).

**Why `.MS3` is a red herring (airtight, three independent proofs):** the `.MS3` filename block lives at engine image `0x25f64` (`INTRO.MS3 MAINMENU.MS3 LOADING.MS3 WWIN/WLOSE/EWIN/ELOSE.MS3 MSN1/2/3.MS3`), preceded by a 5-entry far-pointer table at `0x25f40` and a header (`1c 00 20 00 24 00 …`). **Nothing in FIST.DAT references any of it.** Every apparent reference is a false-positive byte match against an instruction immediate: the decompile's `s_INTRO_MS3_2000_5f64` at `FUN_0000_9f0f` is Ghidra mislabelling the *constant* `0x5f64` inside a garbage-decoded block (`in(0x9f)`); the "table@5f40 ref @0x4d59" is the bytes `40 5f` of `add ax,0x4000; pop di`; the "MSN1.MS3 refs @0x1a7c4…" are `pop di; retf` (`5f cb`) tails. The 5 far-pointers at `0x25f40` point into a **zero BSS region** (`0x29f9a…`, all zero in the static image) → a runtime-filled table, i.e. **editor / mission-script data loaded on demand**, not shipping-menu boot data. (`MS3-KGF'91` is real, but it is the mission/editor script format — `MSN1-3.MS3` are the campaign missions — not the menu renderer.)

**What the menu ACTUALLY is (verified from the binary):**
- **The shipping screen system is the EMBEDDED-TEMPLATE UI interpreter.** `FUN_0000_cae6` (the MAINMENU screen, called from the main loop `…5c3a → cae6`) hard-codes template seg **`0x2d21`** and hands it to `FUN_0000_e714`; siblings `FUN_0000_e4bb` (`0x2d1d`) and the `0x2d2a` screen. `e714`: `1e27(0x2d21→DGROUP:0xf7b8)` copies the template, `1cdb(0x2d21,0xf7b8,0x88,…)` builds the display list, then the `206f`/`1e4b` loop paints it.
- **The MAINMENU template (`image 0x2d210`, size `0x88`) decodes cleanly** into a header (`size=0x88, self=0x2d21, ptr=0x2d6f, count/flags, …`) + 12-byte element records: **type `0x30` id 3 rect (0,0,200,320) = the FULL-SCREEN BACKGROUND**; type `0x0e`, type `0x34`; then **seven type-`0x32` records id 5/8/0b/0e/11/14/17, x0 41→8f step 0xd, y 0x73..0xcd = the 7 clickable menu items** (SELECT PLAYER / CAMPAIGNS / BATTLES / REVIEW / SETTINGS / ABOUT FIST / QUIT). So the layout is in the template; only the pixels/resource are missing.
- **The background `.MRL` is named by the MGAVIDEO driver, indexed.** MGAVIDEO.DVR holds a 13-byte-stride resource-name table at driver image `0x47ec`: `M1CON…BMPCON.MRL`(consoles) `CCV CSTAT FULLSCRN WWIN EWIN WLOSE ELOSE WCAMP ECAMP` **`MAINMENU`=index 13** `LOADING PRESENTS FLDCOMP SETTINGS.MRL` then `MSPRITES.BIN`/`M1_x.MXX` models. The `".MRL"` extension string lives ONLY here (image `0x47f4`). The driver does **no INT-21 opens itself** — it supplies the name; the engine's FILEMGR opens it. (`MAINMENU.MRL` = `[768-B 6-bit VGA palette][PackBits 320×200]`, no magic header, exactly what patch-093's `0340` consumes.)
- **The two on-screen element descriptors are fixed DGROUP structs with NO static word0 writer.** `e9f0` (the per-frame paint) does `e3bc(→ 0d2e(0xf6cc,…))` for the **background** (descr `DGROUP:0xf6cc` = linear `0x2b6cc`) and `e38a(0xf790)` for the **2nd element** (descr `DGROUP:0xf790`; `e40e` uses the sibling `0xf6c4`). `0xf6cc`/`0xf790` are referenced ONLY at those render sites — their `word0` (the loaded-resource segment) is written **through a pointer by the screen-enter LOAD path**, which Ghidra cannot attribute to a literal, so it is invisible to a text grep. On our boot that LOAD never populates them → both stay 0.

**CURRENT CRASH (runtime-confirmed, `FIST_TICK_HZ=25000 FIST_SEGV_BT=1`) = the doc's blocker #2, and it is UPSTREAM of any faithful `.MRL` decode:** after "proceeding to main menu", `e9f0 → e3bc/0340` paints the background from **segment 0** (word0=0, harmless noise, no crash) then **`e38a(0xf790) → (*c5fa)/(*c5fe) → m_mga_FUN_0000_0f3a(+0x92) → m_mga_FUN_0000_0f62(+0x1e)` dereferences a NULL descriptor** (`param_1==0`, fault addr `0x2`). `0f62` is a rect blit reading `*(int*)(*param_2*2 + _DAT_1000_c794)+param_2[1]`; with the 2nd element's descriptor unpopulated it walks off 0. So the real blocker is not "decode the MRL" but **"the screen-enter code that CREATES + populates the element descriptors (and loads their `.MRL`/title resources) is not running / not firing its resource load."**

**NEXT (the actual unlock, scoped):** find the type-`0x30` (and type-`0x34`/title) **element-CREATE handler** invoked during `1cdb` display-list build — it is the site that (a) reads the driver name-table (MAINMENU=idx 13), (b) INT-21 opens `MAINMENU.MRL`, (c) allocs a ~53 KB segment, (d) reads the file, (e) stores the segment into `descr(0xf6cc).word0` via the descriptor pointer. Determine whether it's (i) NOT invoked because `cae6` is a degenerate/redraw entry and the real load-enter (analogous to the vector-only `FUN_0000_e87a`, which DOES carry a full load block `4754/e006/d755/9d26/459a/e4bb`) is skipped, or (ii) invoked but its INT-21-open / `word0`-store was dropped by a Ghidra base-loss/arg-drop (the recurring `__allregs` indirect-vector class). Either is a bounded, asm-verifiable patch once the create handler is located; the `FIST_MENUBG_TEST` diagnostic already proves that once `word0` is set, `0340` paints `ref/main_menu.png` byte-exactly, so the render side is done. (The QEMU oracle faults before the menu; a DOSBox file-open trace to confirm the exact open sequence needs a DOSBox instrument — not yet built.)

## SUPERSEDED (retained for history) — **THE MENU BACKGROUND IS `MAINMENU.MRL`, AND THE RENDER CHAIN IS PROVEN END-TO-END: injecting the REAL file bytes makes `FUN_0000_0340` paint the EXACT `ref/main_menu.png` tank+helicopter+terrain scene (62259/64000 px, 177 colors, remap-verified).** The content-load frontier is now precisely scoped. NO engine change (`re_out/fist.c` byte-identical md5 `6cd1f94a`, 94 patches, default boot crash-free, intro plays); only env-gated diagnostics added to `tools/native_main.c`.
**What the menu background is (VERIFIED, not hypothesised):**
- **`armoredfist/FISTDATA/MAINMENU.MRL` (53096 B) = the menu background.** Layout = **[768-byte 6-bit VGA palette][PackBits RLE 320×200 image]** — i.e. exactly what `0340` consumes (palette at `descr.word0:0`, image at `descr.word0:0x300`). Decoding it with the patch-093 PackBits decoder (signed count: `n>=0`→copy n+1 literals, `n<0`→fill |n|+1) consumes the file EXACTLY (53096 B → 64000 px) and reproduces the ref scene byte-for-byte (two M1 Abrams, a Mi-24 Hind + 2nd heli, muzzle-flash, green terrain/hills). **So the patch-093 `0340` reconstruction is byte-perfect.** The parallel screen `.MRL`s are `SETTINGS.MRL`, `LOADING.MRL`, `INTR_.MRL`, `WWIN/WLOSE/EWIN/ELOSE.MRL`, plus the cockpit-console `.MRL`s (`M1CON/M2CON/T80CON/BMPCON.MRL`, whose 8.3 names are a table in **MGAVIDEO.DVR @ image 0x47f4** — the only place the `".MRL"` extension string lives).
- **NOT the intro's last frame.** Ruled out: the final `TITLE.KDV` frame is the **"ARMORED FIST" camo logo** (dumped via the new `FIST_DUMP_INTROEND`), not the tank scene. The intro FMV (KDV, PCX-style 0xC0-run interframe codec) and the menu bg (standalone `.MRL`, PackBits) are different resources with different codecs.
- **The screen system is data-driven by `.MS3` scripts.** Image 0x25f6a holds a state-table of screen scripts `INTRO.MS3 / MAINMENU.MS3 / LOADING.MS3 / WWIN.MS3 / WLOSE.MS3 / EWIN.MS3 / ELOSE.MS3 / MSN1-3.MS3` (all present on disk; header `1c 00 20 00 24 00 04 00 04 00`). `MAINMENU.MS3` (3561 B) is a compiled **"MS3-KGF'91"** (Kyle G. Freeman) binary script; it references `MAINMENU.MRL` by index (no literal filename inside). So the intended flow is: enter the MAINMENU screen-state → load+run `MAINMENU.MS3` → it opens `MAINMENU.MRL` → decode → assign the loaded segment to the on-screen descriptor `DGROUP:0xf6cc.word0` → `0340` paints it.

**Why it's not loading (root cause):** the on-screen descriptor `DGROUP:0xf6cc.word0` (linear `0x2b6cc`) = the `.MRL` resource segment, and it stays **0** because **the engine never opens `MAINMENU.MRL`** (INT-21 3D-open trace: only `SOUND.CFG`, `FIST.CD`(fail), the 2 `.DVR`s, `FIST.SET`, `TITLE.KDV` — no `.MS3`/`.MRL` ever). `0xf6cc.word0` has NO immediate writer in the code (only `e3bc` references `0xf6cc`, as the descriptor pointer it hands to `0340`); it is meant to be written via a pointer by the `.MS3`/`.MRL` screen-load path — which our boot (`cae6 → e714`, screen setup from EMBEDDED templates `0x2d1d/0x2d21/0x2d2a` inside FIST.DAT) never enters. The prior "only 5 files opened" was because execution halts at the menu boundary before that load runs.

**Proven end-to-end (the render chain is correct; it just needs the bytes):** a gated diagnostic (`FIST_MENUBG_TEST`, `tools/native_main.c`) that, at intro-complete, reads `MAINMENU.MRL` into a free segment (0x8000) and writes `word[0x2b6cc]=0x8000` makes `e9f0 → e3bc → (*c564)()=0340` paint the framebuffer to **62259/64000 nonzero, 177 distinct indices**; dumping the raw indices (`FIST_FBRAW`) and remapping them through the file's own palette yields the **exact `ref/main_menu.png` scene** (`scratch`/`/tmp/fbremap.png`). This confirms: (1) `MAINMENU.MRL` IS the menu bg; (2) the `0340` PackBits chain is byte-perfect; (3) the ONLY missing piece is driving the real file-open + `word0` assignment (plus the two follow-ons below).

**NEW FRONTIER — three scoped items to a real, faithful menu:**
1. **The faithful `.MRL` open (the real fix).** Find/drive the engine's own `MAINMENU.MRL` open + `descr.word0` assignment. Candidate path: the `.MS3` screen-state loader (state-table @ 0x25f6a) that our hardcoded `cae6/e714` boot bypasses — locate where a screen-state opens its `.MS3` and, via the MS3 interpreter, its `.MRL`. (The `.MRL` open likely routes through MGAVIDEO.DVR, which owns the `".MRL"` extension.) This is the next major work item; the shim FILEMGR already serves `.MRL` opens (it serves `.DVR`/`FIST.SET`/`TITLE.KDV`).
2. **The `e38a → 0f3a → 0f62` crash (null descriptor `0xf790`).** With `word0` set, `e9f0` gets PAST `e3bc/0340` and crashes at `m_mga_FUN_0000_0f62 +0x1e` (deref `[0x2]`) — the SECOND screen element `e38a(0xf790)` (the gray menu box / title text) whose descriptor is ALSO unloaded (same class). Call chain confirmed via the new `FIST_SEGV_BT` handler: `e9f0 → e38a(+0x64) → …0f3a(+0x92) → 0f62`. Needs its resource loaded too, and the base-loss threaded.
3. **The DAC palette upload (`0410` path).** The `0340` INDICES are byte-perfect but the DAC (`g_pal`) is black at crash time — `0410`/the vblank palette flush isn't pushing `word[DGROUP:0x782]` (the MRL palette `0340` copied) to ports 0x3c8/0x3c9. Remap-through-file-palette confirms the pixels; the engine palette-upload is a separate base-loss/timing item.

**New diagnostics in `tools/native_main.c` (all env-gated, default OFF — no behavior change):** `FIST_SEGV_BT` (SIGSEGV→EIP+stack-candidate dump, maps via `nm`, since gdb can't keep up with the fast tick), `FIST_DUMP_INTROEND` (dump FB at intro-complete), `FIST_MENUBG_TEST` (inject real `MAINMENU.MRL` + set `word0` — a DIAGNOSTIC proving the chain, NOT the fix), `FIST_MENUBG_PAL` (also push the MRL palette to the DAC), `FIST_FBRAW` (dump raw 64000-byte index buffer for external palette remap).

## DONE (prior iteration) — **THE MENU-BACKGROUND RENDER CHAIN `FUN_0000_0340` IS FIXED: it no longer crashes and now FILLS the 320×200 framebuffer end-to-end (palette copy + RLE image blit + palette merge, all executing). Root-caused the "0x782 palette-buffer gap" as a store-width bug (NOT an alloc gap). Execution advances past 0340 → 0410 → 0f3a into 0f62.** 5 new patches (092–096), engine `re_out/fist.c` byte-identical (md5 `6cd1f94a`), 94 patches apply clean, default boot 8/8 crash-free, intro still plays 395 frames.
**What changed (all asm-verified):**
- **092 — the ROOT of the "word[DGROUP:0x782]==0" symptom = a store-width bug, not an alloc gap.** The palette buffer IS allocated by driver-init 0009 (patch 049 → 182a writes word[DGROUP:0x782] = a valid heap seg, e.g. 0x4944). But `_DAT_1000_c780` (DGROUP:0x780) was macro-typed `undefined4`; every `_DAT_1000_c780 = 0x782` (8 driver sites, incl. 0340/043e) wrote a **dword** 0x00000782 whose high word landed on 0x1c782 and zeroed the buffer segment. The asm is unambiguously `mov WORD PTR ds:0x780` at every site; changed the macro to `undefined2`. (Confirmed by watchpoint: 043e's `_DAT_1000_c780=0x782` was the writer that zeroed 0x782 right after mode-set.)
- **093 — `FUN_0000_0340` reconstructed (asm 0x340..0x387).** The menu-background paint: (1) copies a 768-B palette from the descriptor's resource segment (`*param_2` = word[descr:0]) into word[DGROUP:0x782]; (2) RLE-decodes the descriptor's background image into 0xA0000 via `[DGROUP:0x336]` = **far** 0xf69:0x57cb (a MID-function entry into FUN_1000_57be that Ghidra mis-decoded to garbage `in/out/4a2c`) — reconstructed the PackBits decoder INLINE (0x14e5b..0x14e97); (3) `FUN_0000_0410`. Ghidra had dropped BOTH rep-movs bases (NULL → SIGSEGV), the `ss:0x786` byte base (wrote wrong `DAT_2000_c216`), and mis-rendered the far fb-blit vector as a near module call.
- **094 — `FUN_0000_e3bc` descriptor thread.** asm 0xe3e2: `bx=0xf6cc; cx=ds` → the on-screen descriptor at DGROUP:0xf6cc = g_mem+0x2b6cc; the `(*c564)()=0340` `__allregs` vector site had dropped cx:bx, so 0340 got a garbage descriptor (~g_mem-0xEC). Threaded per-patch (same class as the indirect-call arg threading doctrine).
- **095 — `FUN_0000_0410` palette-window merge (asm 0x410..0x43d).** Ghidra mis-based the three `ss:`-prefixed DGROUP reads (0x78c/0x78e/0x790 → wrong `_DAT_2000_c21c/c21e/c220`) and dropped the rep-movs es(=word[0x782]):di / ds(=word[0x790]):si bases. es is set by both callers to the palette buffer.
- **096 — `FUN_0000_0f3a` DGROUP near-pointer base-loss (asm 0xf3a..0xf5f).** `di=word[DGROUP:0x724]; [di]==0xA000` — Ghidra typed `_DAT_1000_c724` as a host `int**` and deref'd it, AND marked `FUN_0000_2d6d` "does not return" (dropping the ==0xA000 tail). Reconstructed.

**FRONTIER — THE MENU CONTENT IS NEVER LOADED FROM DISK (the real blocker for a *recognizable* menu).** With 092–096 the render chain is mechanically correct and the framebuffer FILLS (62264/64000 nonzero, 256 colors) — but it is **RLE noise**, not the menu art, because the screen descriptor's resource segment `word[DGROUP:0xf6cc] == 0`: 0340 RLE-decodes from segment 0 (the engine's own low memory). **Root cause: no menu-background/title PCX is ever opened** — the ONLY files opened on the whole boot are SOUND.CFG, the two `.DVR`s, FIST.SET, TITLE.KDV (verified via the INT-21 3D-open trace). So the menu's title-art + gray-box resources are simply not loaded; every display-list paint node (e9f0 → e3bc/e38a → 0340/0f3a/0f62) points at an unloaded (word0=0 / NULL) descriptor. The immediate next crash is **`FUN_0000_0f62` param_1==0**, which traces to `FUN_0000_e38a` (asm 0xe38a: `dx=0xf6ee`) dropping the descriptor near-offset through its `[DGROUP:0x5fa]`/`[DGROUP:0x5fe]` `__allregs` method-vector calls — a threadable base-loss, but its descriptor is also unloaded. **Next step is UPSTREAM: find + drive the menu-content resource load (the file open + descriptor.word0 assignment) so 0340 has real image data; the render chain that consumes it is now proven correct.** NB the ~1/5 fast-forward boot flake at `FUN_1000_5c3a→f842` (the patch-091 residual "~12 other bare [DGROUP:0x12] dropped-xor-bx" sites, exposed by the 25000-Hz register perturbation) is unrelated to this path and untouched by 092–096 (default 200-Hz boot is 8/8 clean).

## DONE (prior iteration) — **INTRO PLAYS END-TO-END ON THE DEFAULT BOOT PATH (395 frames → EOF → advances), and execution now runs THROUGH the post-intro fade / delay / cursor-sprite draw / KDV-close all the way to the MENU-BACKGROUND RENDER (driver `FUN_0000_0340`). The default boot is crash-free (0/70 stress runs) — the ~1/8 f842 boot flake exposed by the longer intro is fixed.** 8 new patches (084–091), engine `re_out/fist.c` byte-identical (md5 `6cd1f94a`), 89 patches apply clean.
**What changed (all asm-verified):**
- **Intro EOF/loop (patch 084 + shim `fist_extender_gate`).** The KDV read-chunk `FUN_0000_708b` signals end-of-stream via the x86 **carry** (asm `stc@0x709e` when `read_off+8 > filesize`), consumed by `FUN_0000_6f3e` (`jae@0x6f43` → else `return -1`); Ghidra dropped the flag (`in_CF` never set) so at EOF the player re-parsed the *previous* frame's magic forever. 084 threads 708b's CF-out to 6f3e via the shim global `g_ext_eof`. TITLE.KDV = **1,845,827 B / 309 header frame-count / 395 present ops @ 320×200**; at EOF the shim gate closes the stream (`FUN_0000_6f17`) and returns **nonzero = "animation complete"** so `FUN_0000_e584` exits — the intro plays **once** then advances (the faithful contract; no loop).
- **Default-path drive.** `fist_extender_gate` now drives the KDV player **by default** (`FIST_KDV=0` disables); a normal `/tmp/fist_native` plays the NovaLogic logo (`nonzero=64000/64000`, 36 indices) then proceeds. NB at the default 200 Hz tick the 395-frame intro takes ~10 min wall-clock, so a bounded run stays in the intro; **`FIST_TICK_HZ=20000-30000` fast-forwards it** for driving the post-intro path (the tick source is the documented deterministic-replaceable seam).
- **Post-intro chain reconstructed (patches 085–090).** After the intro `FUN_0000_e446` does `(*c594)()=04cd palette-fade → e584(0x4404) 2nd anim → (*c59c)()`; execution then advances through `cae6`:
  - **085** MGAVIDEO `FUN_0000_04f1` palette-**fade**: Ghidra invented `bVar6=param_2<param_4` and folded the CF==0 fade body (asm 0x533–0x586: 1-tick wait + DAC save into `[c788]` + install the per-vblank stepper `c5e8=0x5b8`) to `do{}while(true)` → post-intro **hang**. Reconstructed both branches; the `call [DGROUP:0xe4]` (uninstalled=0, patch-049 gap class) traps → CF=0 → the fade path (high-detail intent), threaded via `g_fist_cf`.
  - **086** `FUN_0000_0249` INT-8 tick-wait (`ss:0x452`, SS-base dropped → folded `while(true)`) — pump the cooperative timer (017/072 class); `FUN_0000_0240(20)` = delay 20 ticks.
  - **087** `FUN_0000_1e27` resource copy (`rep movs DGROUP:di ← [param_1:4]:0`, count `[param_1:8]`): Ghidra assumed ds=DGROUP → src NULL / dst raw-host-di / count from DGROUP:8 — rebased.
  - **088/089** `FUN_0000_20ea` / `FUN_0000_2057` sprite/cursor **draw node-walkers** (dispatch bases 0x444e / 0x402a) — the 056/057 (201a/209e) base-loss class: DGROUP near-offset node ptrs used as host pointers.
  - **090** MEMMGR **free** `FUN_0000_3376` (KDV close via 6f17): used the caller param as the block index instead of 3661's ESI index (`[0x2f60]`, patch-082 class) → compacted the wrong block → SIGSEGV.
- **091 — the boot FLAKE fix.** `FUN_1000_223c` / `FUN_1000_449e` (CRT-init) do `xor bx,bx; mov si,X; lcall [DGROUP:0x12]`; Ghidra dropped the `xor bx,bx`, so the inert engine-C `f842` ran with a **stale bx** — ~1/8 of boots bx held nonzero host-pointer garbage that skipped f842's `or bx,bx; je` bail and dereferenced a stack address as the reloc section → SIGSEGV (only surfaced once the KDV intro perturbed the register state; the base build with the intro opt-in looked clean). Thread `param_1=0` so f842 takes the safe f7c3 bail. **A faithful full apply (`fist_apply_reloc_section(0x174,1)`) is WRONG here — it corrupts the not-yet-ready DGROUP and deterministically breaks `FUN_1000_5c3a`; the boot sections are already shim-seeded, so bx==0 is faithful.** Result: **0/70** boot-stress crashes (was ~1/8). ~12 other bare `[DGROUP:0x12]` sites carry the same dropped-`xor bx,bx` (a reloc-subsystem cleanup; only a risk once reached with garbage bx).

**FRONTIER — the MAIN MENU BACKGROUND render, driver `FUN_0000_0340` (reached via `e9f0→e3bc→(*c564)()`).** 0340 = palette-copy (768 B) + **framebuffer clear** (`es=0xA000, cx=0xfa00, call [DGROUP:0x336]=0x57cb`) + palette upload (`FUN_0410`) = the menu background. It crashes (with `FIST_TICK_HZ` fast-forward) because (a) the **screen descriptor** `cx:bx` (e3bc sets `bx=0xf6cc, cx=ds` at asm 0xe3e2–0xe3e7, dropped by the `(*c564)()` indirect-call arg-threading) arrives as garbage, (b) its NULL `rep movs` src/dst (base-loss), and (c) the dest palette buffer `word[DGROUP:0x782]==0` (an allocation gap in this menu phase). Fixing 0340 (thread the c564 descriptor + rebase src/dst/`[0x336]` + resolve the 0x782 buffer) is the next step to the first menu pixels. At the DEFAULT tick the intro never reaches 0340 within a bounded run, so the default boot is crash-free.

## DONE (prior iteration) — **FIRST LIGHT: NONZERO PIXELS. The NovaLogic intro-FMV logo renders.** The extender FILEMGR + a 32-bit-flat INT-21 are implemented, `TITLE.KDV` OPENS, and the real decompiled KDV player decodes+presents to 0xA0000 → `nonzero=64000/64000`. With `FIST_KDV=1 FIST_RUNMS=20000` the framebuffer shows the **NovaLogic company logo** ("NOVA"/"LOGIC" green text + the copper spherical emblem + ™), i.e. the intro FMV that plays before the main menu (ref/main_menu.png is the later menu screen). rc=0, no crash. NO REGRESSION (engine `re_out/fist.c` byte-identical md5 6cd1f94a; 81 patches apply clean; default boot KDV-off crash-free; ext/drivers 184/217/45 fns).
**How it was reached (6 new patches 078–083 + shim/assembler edits):**
- **(1) Reg-file aliased (assembler).** The FLAT32 assembler had mapped the extender's INT reg-file
  symbols (`uRam000f00XX`) into the per-module scratch window (`fist_ext_base+0xbfXX`), DISCONNECTED
  from the shim reg-file at 0xF0000 → the extender's marshalled `int n` registers never reached
  `fist_int_dispatch()`. Fix: `_remap` now passes the 0xF0000..0xF00FF range through absolute so the
  ext reg-file aliases the shim's (same layout as the 16-bit engine).
- **(2) 32-bit-flat INT-21 (fist_dos.c `dos_int_ext`).** The extender's real `int 0x21` is 32-bit flat:
  EDX=buffer (a small MODULE OFFSET like the path buf 0x5898, or a full flat HOST heap pointer like
  `[0x6e80]`), ECX=count, (E)BX=handle. Ghidra's 16-bit InstallIntFixup truncated EDX via `(short)`.
  Fix: the FLAT32 assembler now also emits a FULL-WIDTH shadow of each reg-file input store (DX/CX/BX/SI
  slots) at 0xF0020+, and `dos_int_ext` (gated by `g_fist_ext_int`, set by native_main around the KDV
  gate) services AH=1A/4E/4F/3D/3F/3E/42 with `ext_addr(EDX)` = `(v<0x10000)? g_mem+fist_ext_base+v :
  (host)v`. find-first stats the file + fills the DTA (size @ +0x1a); read uses the full EDX32/ECX32.
- **(3) FILEMGR path tables + a DEDICATED extender TCB (native_main).** Seeded `[0x6234]`=empty path
  root, `[0x622c]`/`[0x6238]`=0, `[0x927]`=host DTA buffer. And `[0xc93]` (extender current-TCB) is now
  a DEDICATED block (asset name copied from the engine intro task +0xBA) — NOT the engine intro task —
  so the player's MEMMGR error path 0f64 (`*[0xc93]=0xffff`) can't poison the abort flag e339 checks.
- **(4) FLAT32 ptr-DAT = BYTE pointer (assembler).** Pointer-valued DATs (TCB `[0xc93]`, chunk buffer
  `[0x6e80]`, `[0x623c]`) were typed `int **` → every `DAT + off` scaled ×4 (e.g. `[0xc93]+0xba` read at
  +0x2e8, so the filename read empty). Now `undefined1 **` (byte offsets), matching the flat byte model.
- **(5) The KDV player + MEMMGR base-loss/typing/threading patches (078–083, all asm-verified):**
  **078** 3661 block-lookup table base (`0x2bcc`/`0x2a3c` byte offsets); **079** 6f3e chunk MAGIC/width/
  height/frame-count are DWORD fields on the byte pointer (`*(int*)(DAT_6e80+8/0xc/0x10)`) — without it
  the magic read 1 byte, no chunk type matched; **080** the parallel MEMMGR block-info tables
  `0x2a3c/2bd0/2d5c/2d60` byte-scaled in the realloc/compaction (3439/345c/3584/385c); **081** the
  per-block FLAGS array `0x2eec/2eed` is a BYTE array (was `undefined4`); **082** 3661 returns the found
  block INDEX in **ESI** (the __allregs model dropped it → 345c/3439 compacted the WRONG block; now 3661
  publishes it to `[0x2f60]` and the callers read it); **083** the compaction shift do-while loops are
  guarded so growing the LAST block (0 blocks to relocate — the chunk buffer is the last-allocated block)
  is an in-place no-op instead of walking the tables out of bounds. With all six, the chunk buffer's
  8-byte header survives each resize, 6f3e dispatches `xVDK`(header)/`pVDK`(palette→DAC 746b)/`iVDK`
  (RLE 7135 → present 7120 blit 320×200 to 0xA0000), and the intro plays frame-by-frame.
**FRONTIER (post-first-light):** the intro is per-frame decoding under `FIST_KDV=1`; next is EOF/loop
handling + driving it in the DEFAULT boot (currently opt-in behind FIST_KDV), then bit-verify the intro
FB vs a DOSBox capture, then WASM. The MEMMGR compaction is now functional for the KDV allocation
pattern; a broader stress (many resizes / non-last-block grows) may surface further 3584/385c edges.

## DONE (prior iteration) — ROUTE 1 PLUMBING COMPLETE: the extender KDV intro-FMV player is now a compiled+linked+wired 3rd MODULE (32-bit-flat), execution runs FROM the engine's intro loop INTO the real decompiled KDV player. New frontier = the extender's own FILEMGR (path-root init + a 32-bit-flat INT-21) — the file-open subsystem the KDV player calls
The 5-step plan from the prior iteration is BUILT (steps 1-4). `fist_ext.c` (the extender kernel decompile) is
assembled by a NEW 32-bit-flat MODULE mode, compiles + links as a third unit, is registered like the .DVR
overlays, and the extender gate drives the REAL KDV OPEN/DECODE+PRESENT handlers. **NO REGRESSION** (engine
`re_out/fist.c` byte-identical md5 `6cd1f94a…`; `make check` = 75 patches clean; default boot rc=0 with every
prior milestone — EXT+both `.DVR` register/load, mode-set INT 10h AX=13h, mouse init, create-task, banner).
NONZERO PIXELS not yet reached: the KDV OPEN enters the extender's FILEMGR, which needs its path tables +
a flat INT-21 (below). The KDV drive is gated behind **`FIST_KDV=1`** (default OFF) so the default run stays
crash-free; with it on, execution reaches the real player end-to-end and stops at the FILEMGR frontier.

**(A) NEW 32-bit-flat MODULE mode in `tools/assemble_fist.py`** (`FIST_MODULE=ext FIST_FLAT32=1
FIST_MODULE_SIZE=0xbf90`, gated so engine + 16-bit-driver output is byte-identical). The extender kernel is
32-bit flat, linked base 0; its Ghidra decompile names every symbol by its SINGLE flat linear
(`DAT_00006e80`, `FUN_00006f3e @ 00006f3e`) not the engine's `SEG_OFF`. Since the whole image is < 64 KB
(0xbf90), the mode NORMALIZES each single address to the `0000:OFF` `SEG_OFF` shape the rest of the assembler
already handles, then reuses 100% of the machinery. Genuine 32-bit deltas handled: scalar DAT default width
4 B (not 2); `sRam`/`_uRam` (signed / 32-bit reg-file slots); `PTR_DAT/PTR_FUN/PTR_LAB/PTR_s_` pointer-typed
data (`undefined1**`); `code`-return/`(code)`-cast → undefined4; `switchD_ADDR::caseD_N` → `caseD_N`; a
switch-on-pointer → `switch((int)…)` + stripped case casts; `BADSPACEBASE`; bare-LAB SMC byte stores; a
compact scratch window for off-image Ghidra abs-immediate "symbols". New intrinsics in `ghidra_compat.h`:
`CONCAT24/26`, `segment(s,o)`, `LocalDescriptorTableRegister`. Result: `re_out/fist_ext.c` (183 fns) compiles
+ links clean.

**(B) MEMORY MODEL — host-pointers-in-slots (verified against the decompiled code).** The extender is a
flat BUMP allocator: `FUN_00003772` hands out running offsets from `[0x90b]` (heap base) up to `[0x90f]`
(top), stored into flat POINTER slots and dereferenced directly (present `FUN_00007120`:
`edi=[0x917]; rep movsd`; TCB reads `*(short*)([0xc93]+off)`; buffers `*DAT_00006e80`). Seeding those slots
(module-relative, rebased to `FIST_EXT_BASE`) with **HOST pointers into g_mem** makes the REAL allocator /
RLE decoder (`7135`) / present blit (`7120`) / DAC upload (`746b`) run with native derefs, NO per-deref
rewriting. Seeds (`ext_module_init`, native_main): `[0x807]=0` identity, `[0x917]=&g_mem[0xA0000]` (the
engine framebuffer fist_vga dumps), `[0xc93]=&g_mem[0x90000]` (the 16-bit create-task TCB that e584 fills
with "TITLE.KDV"), `[0x90b]/[0x90f]` = heap base/top. g_mem grown to `0x180000`: engine 0..1 MB unchanged;
ext image `0x100000..0x110000`; ext heap `0x110000..0x160000`. (`re_out/ghidra_compat.h`.)

**(C) BUILD / LINK / REGISTER / GATE wiring.** `Makefile assemble` emits `re_out/fist_ext.c`;
`tools/build_native.sh` `$m` loop adds `ext`; `re_out/fist_modules.c` adds the `"EXT"` known-module row;
`native_main.c ext_module_init()` loads `re_out/fist_image.bin` at `FIST_EXT_BASE`, calls
`fist_ovl_register("EXT", …)` (wires `fist_ext_fmap`/`fist_ext_base` so `fist_icall` resolves ext-internal
indirect calls), and seeds the slots. `fist_extender_gate` op 0x64/0x78 → `m_ext_FUN_0000_11cb` (OPEN
TITLE.KDV via TCB+0xBA) then `m_ext_FUN_0000_11dd` (DECODE+PRESENT). Live: `EXT registered base 0x100000
fmap=wired(183 fns)`; intro loop posts ops; `KDV OPEN (TITLE.KDV via TCB+0xBA)` fires; execution descends
`11cb → 6e95 → 701e → FILEMGR 5cc2 → 5d50`.

**(D) NEW FRONTIER — the extender's FILEMGR (`FUN_00005cc2`/`FUN_00005d50`) + a 32-bit-flat INT-21.** The KDV
OPEN calls the extender's OWN resource FILEMGR, which (asm-verified from the decompile) builds a path by
copying a **path-root** `PTR_DAT_00006234` + a resource-dir string `PTR_s_RESOURCE_RES_0000622c` into a
buffer, appending the uppercased filename, and issuing INT 21h. Two blockers, both the extender-file
subsystem (analogous to how the 16-bit engine's FILEMGR was brought up patch-by-patch):
  1. **FILEMGR path tables uninitialized** — `PTR_DAT_00006234` / `PTR_s_RESOURCE_RES_0000622c` /
     `DAT_00006238` (drive) / the DTA `DAT_00000927` hold raw link-time image bytes (not host pointers), so
     `FUN_00005d50`'s `for(pbVar4=PTR_DAT_00006230; *pbVar4; …)` walks a wild pointer → SIGSEGV. They are set
     by the extender's FILEMGR init (which the port has not run). Init/seed them faithfully (extender role).
  2. **INT-21 is 16-bit; the extender is flat.** The shared `InstallIntFixup` reg-file is 16-bit (`DS:DX`).
     `fist_dos.c` reads the filename/buffer at `g_mem+(DS<<4)+DX`, but the extender's static path buffers
     live at MODULE offsets (e.g. `0x5898`, which it passes as `DX=0x5898` with `DS=0x1c00`) → mis-read as
     engine space `0x21898`; and its heap read buffers are >16-bit host pointers the decompile truncates via
     `(short)DAT_00006e80`. Faithful fix = a **32-bit reg-file kernel re-decompile** (an
     `InstallIntFixup` variant with 4-byte EAX/EBX/ECX/EDX slots so the extender's real `mov [reg_edx],edx`
     is preserved) + an **extender INT-21** that addresses `g_mem+EDX` flat (`[0x807]=0` identity) and
     implements AH=4E/4F (find-first/next, currently stubbed). This is the road to intro pixels; the whole
     player (decode/RLE/palette/present) is already compiled and one working file-open away.

## DONE (prior iteration) — ROUTE 1 UNBLOCKED: the KDV intro-FMV player + op-service dispatcher are LOCATED, decompiled, and ABI-reversed in the extender kernel image (`re_out/fist_image.bin`). The per-frame renderer the prior iteration declared "absent / must be traced from DOSBox" IS the FIST.RUN Doug-Huffman-extender 32-bit-PM code we already extract via `make kernel-image` — now recovered as a fuller decompile (33 → **183 fns**, KDV cluster included) via a new reproducible `make decompile-kernel` target. This is the faithful path to intro pixels (REAL extender code, no DOSBox trace needed). Module integration (assemble as a 3rd, 32-bit, unit + wire the gate) is DESIGNED below; not yet built. No regression (Makefile-only + a reference-file regen; the engine build is untouched — `make check` green, 75 patches, 2251 engine fns pristine).

**(A) THE KDV PLAYER — located + ABI (`re_out/fist_image.bin`, 32-bit flat base 0; all offsets = app-off = linear).** The intro is FMV: `armoredfist/FISTDATA/TITLE.KDV` (1,845,827 B). The player is a self-contained cluster near the "Invalid Digital Video Data Stream" string (image 0x6fcf):
- **`0x6f3e` = decode-next-frame** (the per-frame engine). Reads an 8-byte-header chunk from the file via `0x708b`, then dispatches on the chunk MAGIC (dword at the chunk buffer `[0x6e80]`):
  - **`0x4b445678` `'xVDK'` = KDV file HEADER** → stores width `[0x5578]`, height `[0x557c]`, frame-count `[0x6e84]`; loops to next chunk.
  - **`0x4b445670` `'pVDK'` = PALETTE** → `rep movsd` 0xC0 dwords (768 B = 256×RGB) chunk+8 → palette buffer `[0x6e90]`; sets dirty flag `[0x6e94]=1`; loops.
  - **`0x4b445669` `'iVDK'` = IMAGE FRAME** → RLE-decode (`0x7135`) chunk+8 → decode buffer `[0x6e88]`; if palette dirty, upload (`0x746b`); present (`0x7120`); return 0.
- **`0x7120` = PRESENT**: `mov edi,[0x917]; mov ecx,0x3e80; rep movsd` — blits **16000 dwords = 64000 B = 320×200** from the decode buffer `[0x6e88]` to the framebuffer whose linear address is in var **`[0x917]`**.
- **`0x746b` = PALETTE→DAC**: `out 0x3c8,0` then 768× `lodsb; shr al,2; out 0x3c9,al` — direct VGA-DAC upload (6-bit), **already trapped by `fist_vga.c`**.
- **`0x7135` = RLE frame decoder** (classic `lodsb; cmp al,0xc0; jae run; stosb` run-length, into `[0x6e88]`, dims from `[0x5578]`/`[0x557c]`).
- **`0x701e` = OPEN** (FILEMGR `0x5cc2` op 0x4e00 find/prepare → op 0x3d00 open-read; file size `[FILEMGR_info 0x927]+0x1a → [0x937]`; handle → `[0x701c]`). **`0x708b` = read-chunk** (INT 21h AH=3F via `[0x701c]`, MEMMGR-reallocs `[0x6e80]`). **`0x6f17` = close/free**. **`0x6e95` = init** (MEMMGR-allocs the three buffers via `0x36bf`: 64000 B decode `[0x6e88]`, 768 B palette `[0x6e90]`, chunk `[0x6e80]`).
- File I/O is 100% INT 21h ⇒ our shim's `fist_dos.c`/`fist_filio.c` already serve it; palette is DAC ports ⇒ `fist_vga.c`; present is a linear write to `[0x917]`.

**(B) THE OP-SERVICE DISPATCH — how the engine's command reaches the player.** The player's two op-handlers are:
- **`0x11cb` (KDV OPEN wrapper)**: `esi = [0xc93] + 0xBA; call 0x6e95` — `DAT_00000c93` is the extender's **current-TCB pointer**; **TCB+0xBA = the asset-name field** the engine's `e584` fills with `"TITLE.KDV"` (matches the reversed TCB layout: `+0xba` = current asset name). So OPEN reads the filename straight out of the engine-built TCB.
- **`0x11dd` (KDV DECODE wrapper)**: `call 0x6f3e`.
These are entries in an op/method-vector table in the extender low data (image `0xcef..0xd2b`: dword ptrs → `0x11f4,0x11a6,0x11cb,0x11dd,…`). The real→PM gate (real-mode stub `0x0762:0x1179` = engine-linear 0x8799; the reversed `fist_extender_gate` seam) enters PM and dispatches an op to these handlers. So the prior iteration's "op 0x64/0x78 has no handler" was true *of FIST.DAT* — the handler lives here, in the extender.

**(C) THE 16→32-bit / addressing model (the intricate integration part — SOLVED on paper).** The extender is 32-bit **flat, linked base 0**; DOS low memory is mapped into its flat space at `[0x807]` (identity ⇒ `[0x807]=0`): at image `0xa3a`, `[0x917] = 0xA0000 + [0x807]` (= the VGA framebuffer), and MEMMGR heap `[0x90b]..[0x90f]` is DPMI-allocated (INT 21h AX=0x350C at image `0x9eb`). Faithful hosting in our single `g_mem`:
  - The extender's OWN image (disp32 abs refs `< 0xbf90`, e.g. `mov ds:0x917`, `[0x6e88]`) **collides** with the 16-bit engine's low memory ⇒ must be **rebased to a separate region** `g_mem + fist_ext_base` (module accessor, exactly like the .DVR `fist_<mod>_base`), keyed on base-0 module offsets in a `fist_ext_fmap`.
  - The extender's DOS-window / heap pointers are **runtime linear values** used register-indirect (`[edi]`, `[esi]`): framebuffer `0xA0000`, TCB `0x90000`, MEMMGR buffers. With `[0x807]=0` these are absolute g_mem linears ⇒ 32-bit register-indirect `[reg]` accessors resolve to **`g_mem[reg]`** directly (0xA0000 → engine framebuffer; 0x90000 → the TCB the create-task gate already allocs). So: **module self-data ⇒ +fist_ext_base ; register-indirect flat pointers ⇒ g_mem[reg] ; the constant 0xA0000/TCB seeds ⇒ absolute.**
  - Shim seeds (extender-loader role): `[0x807]=0`, `[0x917]=0xA0000`, `[0xc93]=TCB(0x90000)`, and INT 21h AX=0x350C → hand the extender its MEMMGR heap (a reserved `g_mem` region above the 1 MB engine window; `g_mem` grows to a few MB) so `[0x90b]/[0x90f]` frame the KDV buffers.

**(D) REMAINING WORK to intro pixels (scoped, each piece understood).** (1) **`assemble_fist.py` 32-bit-flat MODULE mode** (`FIST_MODULE=ext`, x86:LE:32): disp32 abs `< module_size` → `g_mem[fist_ext_base+off]`; register-indirect `[reg{+disp}]` → `g_mem[reg{+disp}]`; namespaced `m_ext_*` symbols; `fist_ext_fmap` keyed by base-0 offset. (2) **`fist_ext.c`** compiled + linked (extend `build_native.sh`'s `$m` loop to `ext`) + registered in `fist_modules.c`/`fist_icall.c` (a non-overlay module: base = `fist_ext_base`, fmap = `fist_ext_fmap`). (3) **grow `g_mem`** + a shim extender-heap allocator for INT 21h AX=0x350C. (4) **wire `fist_extender_gate`**: op 0 (create) already builds the TCB — additionally seed `[0x807]/[0x917]/[0xc93]` and run the extender's video/heap init (or seed its results); then map the intro's OPEN op → `m_ext_FUN_000011cb` (reads TITLE.KDV from TCB+0xBA), and each present/frame op → `m_ext_FUN_000011dd` (decode+present → `g_mem[0xA0000]`). (5) per-function **base-loss patches** as execution enters the player (recurring class). Loop: `make decompile-kernel` (done) → assemble ext → build → `FIST_RUNMS FIST_FBDUMP` → count nonzero at 0xA0000. **STOPPED HERE this iteration** at the module-plumbing boundary (a new 32-bit assemble mode is a large, build-risky capability; not rushed under the no-regression bar). New this iteration: `make decompile-kernel` target (Makefile) + `re_out/fist_kernel_decomp.c` regenerated (183 fns; reference-only, not in the native/wasm build). The KDV renderer is REAL, present, and decompiled — first light is now a plumbing task, not a research one.

## DONE (prior iteration) — EXTENDER TASK MODEL FULLY REVERSED + a hard architectural finding: the per-frame RENDERER is Doug-Huffman-extender 32-bit-PM code that is NOT in FIST.DAT (proven: the command inbox task+0x3f2 is written by ~40 posters but READ by nothing in the engine or either .DVR). So op 0x64/0x70/0x78 CANNOT be implemented by "calling an engine handler" — there is none. Also FIXED the intro-era `'\t>'` garbage open (it was FIST.SET with a host-pointer-truncated DX, patch 077). 75 patches, 2251 fns pristine, no regression. Intro FMV pixels remain blocked on acquiring the extender PM renderer; the reversible first-light target is the MAIN MENU (see recommendations).
75 patches apply (`make check` green), engine pristine (2251 fns), drivers 216/44, all prior milestones
intact (both `.DVR` load+wire, mode-set 0x13, mouse init, create-task, banner, intro frame loop all run;
rc=0 via FIST_RUNMS). NONZERO PIXELS not reached — root cause is architectural, detailed below.

**(A) THE EXTENDER TASK MODEL — reversed slot-for-slot from asm (`docs` = ground truth, no re-derive).**
- **Create (op 0):** e339 pushes args (word[ea1a]=0x0f30 size, word[ea1c]=0, word[ea1e]=0x0526) + a dword 0
  and `lcall [DGROUP:0xea16]` = the extender's real→PM gate (oracle: 0x0762:0x1179 = linear 0x8799).
  Returns AX = the TCB segment.  Shim `fist_extender_gate` op 0 allocates a 0xf30-byte zeroed paragraph-
  aligned TCB in the extender pool (seg 0x9000).  (Unchanged — was already correct.)
- **TCB layout (asm 0xd99b + the ~40 posters 0xd94e..0xe37b):** `+0x06:+0x08` far-ptr → DGROUP data
  (0x1c00:0x1548); `+0x26/+0x28/+0x2a` far-ptr(s) → DGROUP data (0x1c00:0x6d3c / 0xdfbc) — these are the
  intro framebuffer/palette/context data pointers, NOT code; `+0x3f2` = the **command inbox** (EBX, a
  dword, written by every poster); `+0x490/+0x492/+0x494` = command params (set by FUN_0000_e2fc from
  DGROUP:0x4f4/0x4f6/0x4f8); `+0x7a/+0x8a/+0x9a/+0xaa` = the 4 asset-name templates D02.PCX/C02.PCX/
  502.PAL/5.SKY; `+0xba` = the current asset name (e584 copies "TITLE.KDV" here from STRSEG:0x43fa);
  `+0xcc/+0xcd/+0xcf/+0xd0/+0xd2` = per-command state bytes/words; **`[0]` (task[0]) = an ERROR/ABORT flag,
  NOT a "started" flag.**
- **e339 tail (asm 0xe34c..0xe36c):** after the gate returns, `if (task[0]!=0 && op!=0) ljmp [DGROUP:0x58]`.
  Decoded the reloc table: **DGROUP:0x58 = 0xf69:0x314 = linear 0xf9a4**, which is the extender **ABORT/
  RESTART** handler (`mov ss,0x2ba9; mov sp,0x400; … ljmp 0:0xe0` = re-enter the program) — the "Interrupt
  divide by zero" reporter family lives right after it (0xf9fa/0xfa06).  So the ljmp is the FATAL-error
  path; task[0] stays 0 on the normal path and the shim's no-op-return-0 (never setting task[0]) is the
  **faithful non-error behavior**.  d99b's `(*[0x354])()/(*[0x358])()` (linear 0x123ba/0x123bf) are the
  keyboard/status handlers (read/xchg cs:0x2d59, store to task+0x496), not task registration.
- **The command ops (asm-verified posters):** 0x00 create; 0x04/0x20/0x44/0x68/0x6c/0x70 = display-list
  setup/attr commands (FUN_0000_d94e..e2fc family, each `EBX→task+0x3f2; word→ea10; call e339`); **0x64
  (FUN_0000_e2c2) = post frame DATA**; **0x78 (FUN_0000_d97e) = PRESENT frame N** (its return gates the
  intro loop: e584 does `iVar5 = d97e(cmd); if (iVar5!=0) goto exit`, so op 0x78 returns nonzero at
  end-of-animation).  Live trace confirms the intro loop runs: op 0x78 inbox = 0x4973_0000, _0001, _0005…
  (high word 0x4973 = present opcode, low word = the frame counter b6e4 advancing).

**(B) THE HARD FINDING — the frame RENDERER is extender PM code, absent from FIST.DAT (so directive #2's
premise is unsupported).** Exhaustive image scan (every encoding: `mov ebx,gs:0x3f2(si)` writes at 34
sites; **zero reads of offset 0x3f2** in FIST.DAT or MGAVIDEO.DVR/SOUNDDVR.DVR — checked disp16, disp32,
and `mov r16,0x3f2` immediate-index forms).  The engine only WRITES the command inbox; the CONSUMER that
reads task+0x3f2, decodes the display-list command, streams/decodes TITLE.KDV, and blits to 0xA0000 is the
**Doug-Huffman extender's 32-bit-PM service** invoked via `lcall [ea16]` (linear 0x8799 → PM).  That code
is **not in FIST.DAT** (which is 16-bit engine only) nor in the `.DVR`s, and the QEMU oracle **faults
before this point** so it cannot be captured live there.  Conclusion: **op 0x64/0x70/0x78 have no
engine/driver handler to call** — implementing them "by running the task's registered entry via fist_icall"
(the directive) is not possible because no such entry exists.  This is a load-bearing correction to the
project model: the intro is FULL-MOTION VIDEO (TITLE.KDV, 1.8 MB) played entirely by the extender PM
renderer; the 16-bit engine only feeds it a command stream.

**(C) FIXED — the `'\t>'` garbage-filename open = FIST.SET with a truncated-host-pointer DX (patch 077).**
The `'\t>'` open (traced to DS:DX=0x1c00:0x0b7c) is NOT the intro art — it is **FUN_0000_6e0e opening
"..\FIST.SET"** (the settings file; sibling FUN_0000_6e8d saves it).  asm: `mov $0x8a9c,%dx; AH=3D` then
`mov $0x8aa8,%dx; AH=3F`.  Ghidra's InstallIntFixup typed the DX reg-file slot as a POINTER and stored the
HOST address of the symbol (`pcRam000f0006 = (char*)s____FIST_SET_2000_4a9c` = g_mem+0x24a9c), so R_DX =
low16(host ptr) = 0x0b7c garbage.  Patch 077 writes the true DOS offset (0x8a9c / 0x8aa8) into the DX slot.
Verified: the open is now `'..\FIST.SET' -> ok`.  **Class: host-pointer-into-DOS-register INT-reg-file
artifact — recurring** (grep `pcRam000f0006 = (char *)` / `puRam000f0006 = (…*)&` finds ~10 more sites:
17765/17779 (fixed), 18335, 32199/32269/32545/32749/32782/32796/32806, 43237, 53573/53587/53637/53650,
62024/62650; each is a file/dir op with a symbol-offset register that will mis-open until similarly fixed).

**(D) RECOMMENDATIONS — the two faithful routes to first light (a strategy call).**
1. **Acquire the extender PM renderer.** DOSBox reaches the menu (`ref/main_menu.png`), i.e. under DOSBox
   the extender op-service DOES render — so instrument DOSBox's DPMI/PM dispatch to trace what the op-0x64/
   0x78 service does (its VGA writes + the TITLE.KDV stream), then reconstruct it faithfully in the shim
   (extender role).  This is the only route that renders the intro FMV itself.  (The QEMU oracle can't help
   — it faults earlier.)
2. **Model the intro as complete → drive to the reversible MAIN MENU.** The intro FMV can't render without
   (1), but the main menu is drawn by ENGINE + MGAVIDEO driver code (fully decompilable/reversible).  Make
   op 0x78 return "animation complete" (nonzero) so e584 exits its loop and execution proceeds to the menu,
   then reconstruct the menu render + the driver present method → NONZERO PIXELS via reversible code.  This
   is the faster path to *a* first-light frame but does not render the intro.  Needs a human call on whether
   "op 0x78 → complete" is acceptable as a faithful degenerate (intro-finished) contract vs. an approximation.

New shim edits this iteration: `tools/native_main.c` `fist_extender_gate` trace/comment corrected to the
reversed model (task[0]=abort flag; inbox word logged; renderer-absent note); `re_out/fist_dos.c` INT-21
3D-open trace now prints DS:DX+first bytes (diagnostic).  New patch: **077** (6e0e/6e8d FIST.SET DX offset).

## DONE (prior iteration) — VSYNC HANG RESOLVED: the frame/event SCHEDULER is installed + reconstructed, FUN_1000_38cc no longer spins, and the INTRO ANIMATION NOW PLAYS end-to-end (frame loop + script interpreter + timer + key poll all run; frames advance 0→573). New frontier = the EXTENDER TASK-EXECUTION service (op 0x64) — the intro frames are POSTED to it but it no-ops, so 0xA0000 stays black
74 patches apply cleanly (`make check` green), engine decompile pristine (**2251 fns** — seeded the frame-
scheduler vectors), driver 216 fns, NO regression (both `.DVR` load+wire, mode-set 0x13, mouse init,
create-task, banner all intact). The "driver vsync-poll c40a" frontier the prior notes flagged is **SOLVED**
— but the premise was wrong: **c40a is NOT a driver method, it is the ENGINE's frame/event SCHEDULER poll**
(`DGROUP:0x40a = 0xf69:0x3f17 = FUN_1000_35a7`, a seg-0xf69 service function).  The whole documented
"driver present/vsync method-vector installation" reframes to: install the engine's scheduler method-vector
reloc section + reconstruct the scheduler pump + thread its x86 CF.  Execution advanced dramatically:
`… → FUN_0000_e584 intro player → FUN_1000_38cc frame-wait (NOW RETURNS) → intro frame loop → per-frame
FUN_0000_e2c2 → extender op 0x64 (frame command POSTED) → frame-delay spin → next frame …` — the intro plays
steadily in real time (b6e4 climbs 0→573; verified frame 3 @6s → 16 @26s).

**(A) THE VSYNC POLL IS THE ENGINE FRAME/EVENT SCHEDULER (not a driver method).**  Decoding the DGROUP
reloc table (`re_out/fist_dat_image.bin` @ seg 0x3352) shows **reloc section si=0x1d8 (leading seg 0xf69)**
installs the frame/event-scheduler method vectors `DGROUP:0x3fe=0xf69:0x3e0b, 0x402=0x3e65, 0x406=0x3e05,
0x412=0x3ed6, 0x40a=0x3f17 (=FUN_1000_35a7, the POLL), 0x40e=0x422c, 0x416=0x423c (=FUN_1000_38cc itself),
0x41a=0x424e, 0x41e=0x4265`.  These target the engine's OWN service segment 0xf69 — the SAME KIND as the 12
service vectors 0x0a..0x36 — so the real Doug-Huffman CRT reloc walk installs them at LOAD.  The engine's own
0x1d8 sites do NOT re-install them: **FUN_1000_3446 (scheduler INIT) calls f842 (DGROUP:0x12) with bx==0,
which f842 BAILS on (`or bx,bx; je`); FUN_1000_3485 (scheduler SHUTDOWN) calls f860 (DGROUP:0x16), the
CLEAR applier**.  So the install is a boot/loader-role job: `native_main.c fist_install_dgroup ->
fist_apply_reloc_section(0x1d8, 1)`.  The vector targets were unrecovered (only reachable via the
`call far [DGROUP:0x40a]` family, which is data), so they were seeded into `tools/ghidra/SeedServiceVecs.java`
(OFFS += the nine 0x1d8 offsets) and the engine re-decompiled (2249→2251; RecoverAll's fixpoint pulled in the
cluster 0x349b/0x35a7/0x38f5/0x3920).

**(B) FUN_1000_35a7 frame/event SCHEDULER pump reconstructed (patch 073).**  asm 0x135a7..0x1363f: pumps
the cooperative event queue (DGROUP near-offset linked list at 0x18e0/0x18e2/0x18e4/0x18e8) and returns
status in the **x86 CF via stc/clc+lret** — CF=1 when the frame semaphore [0x3e00] is set OR the queue is
empty ([0x18e4]==0xffff).  Ghidra typed the queue node NEAR OFFSETS as host int*/int** (base-loss + pointer
scaling), compared the 0xffff sentinel to a host address, and dropped CF.  Reconstructed faithfully with
16-bit DGROUP near offsets (base g_mem+0x1c000) and CF threaded through the shim global **`g_fist_cf`**
(native_main.c).  On the intro path FUN_1000_3446 built an EMPTY queue ([0x18e4]=0xffff) so the fast path
returns CF=1 at once; the event-dispatch block is reconstructed but unexercised until events are queued.
Also FUN_1000_3920 (the cooperative-yield trampoline `push [0x310]; ljmp far [DGROUP:0x48]` → 0000:0199 pops
into [0x3a], `call far [0x3a]`, `ljmp 0xf69:0`=bare `ret` → returns to 35a7) reconstructed as its net effect
`[0x3a].low=[0x310]; call far [0x3c]:[0x310]; return` (Ghidra lost the SS→DGROUP base).

**(C) The CF consumers threaded — FUN_1000_38cc (074), FUN_1000_38bc (076).**  38cc (`xor al,al; xchg
al,[0x3e00]; L: lcall [0x40a]; jae L`) and 38bc (`L: lcall [0x40a]; jb ret; test ax,mask; je L`) both spin on
the poll's CF; Ghidra folded both to hardcoded-false loops → infinite spin.  Loop until `g_fist_cf` (set by
35a7).  On the empty-queue intro path the poll returns CF=1 immediately, so both terminate at once.

**(D) FUN_0000_e584 intro animation-SCRIPT interpreter reconstructed (patch 075).**  Patch 071 did only the
header (up to 38cc).  The frame loop (asm 0xe60b..0xe6db) walks a table of 6-byte script records at
**ES:[b6ec] where ES = STRSEG = word[DGROUP:0x70]** (the 0x2d74 resource seg).  Ghidra dropped ES=STRSEG (so
`*b6ec` / the key-record deref bare offsets → SIGSEGV), pointer-scaled the +6 cursor advance (uint*+3 = +12
vs the asm's +6 bytes), and dropped 38bc's CF (its key-event read became always-taken → deref of a stale far
ptr).  Reconstructed faithfully: STRSEG-based record reads, +6 cursor advance, and the key-event block gated
on `g_fist_cf` (skipped when CF=1 — faithful: no queued event on the empty-queue intro).  Each frame command
is posted to the intro TASK via FUN_0000_e2c2 (writes EBX→task+0x3f2, sets aa10=0x64, calls the extender gate
e339).

**NEW FRONTIER — the EXTENDER TASK-EXECUTION service (op 0x64) is the real per-frame render.**  The intro
does NOT blit directly: every frame FUN_0000_e2c2 posts a draw command to the intro task and invokes the
extender service **op 0x64** (aa10=100).  Our `fist_extender_gate` (native_main.c) currently no-ops all
task-control ops while task[0]==0 (`[ext] service op 0x64 … no-op`), so the frames are posted but never drawn
→ 0xA0000 stays 0/64000.  **Reaching NONZERO PIXELS = implementing the extender op 0x64 = running the intro
TASK's frame handler** (the task entry seg:off + the four asset templates D02.PCX/C02.PCX/502.PAL/5.SKY that
d99b installed at task+0x7a/0x8a/0x9a/0xaa) — i.e. the cooperative TASK-EXECUTION subsystem (load the PCX/PAL,
blit to 0xA0000).  This is the same class as the create-task gate (docs section A of the prior iteration) and
is the next major work item.  Secondary open item: FUN_1000_3920's far-call target [0x3a]=0x018a:0000
(linear 0x1a40, the cooperative-yield handler) is not a recovered function → traps→0 once (benign here, the
intro plays regardless); recover it alongside the task subsystem.  New shim edits this iteration:
`native_main.c` `g_fist_cf` global + `fist_apply_reloc_section(0x1d8,1)` in fist_install_dgroup;
`tools/ghidra/SeedServiceVecs.java` OFFS += the nine 0x1d8 scheduler-vector offsets.

## DONE (prior iteration) — EXTENDER CREATE-TASK GATE IMPLEMENTED: the task is now a REAL segment; banner prints; the task-struct base-loss family + 153c/04a3/e584 reconstructed; execution reaches the INTRO-ANIMATION PLAYER (FUN_0000_e584). New frontier = the driver vsync-poll method vector c40a (DGROUP:0x40a) — uninstalled, so FUN_1000_38cc spins
70 patches apply cleanly (`make check` green), engine decompile pristine (2249 fns), driver 216 fns, NO
regression (both `.DVR` load+wire, mode-set 0x13, mouse init, cb45 memory gate, env decode all intact).
The extender-gate frontier the prior notes flagged is **SOLVED** and execution advanced dramatically:
`… → d99b (env decode) → EXTENDER CREATE-TASK GATE (real task seg 0x9000) → db3f detail-select →
FUN_0000_153c resource-dir registrar → driver 0x4a3 palette clear+upload → FUN_0000_e584 INTRO-ANIMATION
PLAYER` (current spin). The banner `Armored Fist / (C) Copyright 1994 by NovaLogic, Inc.` now prints.
**NONZERO PIXELS not yet reached** (0xA0000 still 0/64000): the animation player hangs in the driver
vsync wait before any frame blit — the driver present/vsync method vectors are not installed yet.

**(A) THE EXTENDER CREATE-TASK GATE — implemented in the shim (extender role).** e339 does
`lcall [DGROUP:0xea16]` on the pointer d99b decoded from the extender config blob (oracle: 0x0762:0x1179
= linear 0x8799), pushing the arg block (word[ea1a],[ea1c],[ea1e]) and RETURNING the gate's AX. Ground
truth (both asm + kernel): (1) asm-verified 0xe339 — the gate return AX becomes the task segment (d99b
`mov 0xea2e,ax`); the far-jmp tail `ljmp [DGROUP:0x58]` (task scheduler) is guarded by `task[0]!=0 &&
aa10!=0` and task[0] stays 0 on this init path, so it is never taken. (2) The service selector is
DGROUP:0xea10 (aa10), set before each e339 call (0=create, 4/0x20/0x44/0x68/0x6c=task-control). (3) The
Doug-Huffman kernel decompile (`re_out/fist_kernel_decomp.c`) is the ~4 KB CRT/FILEMGR/MEMMGR loader; the
PM create-task service itself lives in the extender's paged PM image (linear 0x10000000+) which the
oracle cannot reach (it faults earlier), so the service is modelled faithfully from the observable
contract: **op 0 = allocate a paragraph-aligned, ZEROED, real-mode-addressable task-control block of
word[ea1a] BYTES (0x0f30 = 3888; the TCB is used at offsets up to ~0x492/0x3f2, so ea1a is a byte size),
return its segment in AX.** In the original the extender carves the TCB from its OWN reserved low DOS
memory (below the child engine, e.g. seg 0x0762); our base-0 model has none, so the shim extender pool is
the free gap ABOVE the engine's MEMMGR heap-top (0x9000) and below the PSP (0x9800) — 32 KB, real-mode
addressable. `fist_icall` routes linear FIST_EXTGATE_LIN (0x8799) → **`fist_extender_gate()`**
(`tools/native_main.c`, extender role, like the DGROUP/PSP/env seeds). op!=0 = task-control (no scheduler
needed while task[0]==0); logged, not silently swallowed. Live: `[ext] create-task: 3888 bytes -> task
seg 0x9000`.

**(B) The task-struct base-loss family — fixed ONCE at the macro (patch 068).** The intro TCB is
addressed via the DGROUP far pointer aa2c(offset,=0)/aa2e(segment,=task seg). Ghidra typed the symbol
`DAT_2000_aa2c` as a host `int **` reading the raw dword at 0x2aa2c (= aa2e<<16|aa2c) and dereferencing
THAT (base-loss), AND as an `int *` it pointer-scaled every `+off` by 4. Rebased the macro ONCE to a
BYTE pointer `g_mem + (word[aa2e]<<4) + word[aa2c]` → fixes all ~88 task-struct accesses (db3f/dab0/db11/
dde2/ddff/e21c/6de2/…) uniformly and faithfully (byte offsets match the asm; the `iVar=aa2c;*(iVar+off)`
idiom then loads the right base too). No pre-db3f function uses it, so zero regression. The `_DAT_2000_aa2c`
(raw far-ptr value) reads are a separate macro and untouched.

**(C) FUN_0000_153c resource-directory registrar (patch 069).** `153c(id, seg, out_off, struct_off)`:
the object struct is at param_2:param_4 (seg:near-offset, DGROUP), NOT a host pointer — Ghidra dropped
ES=param_2 (deref of bare 0xe816 = SIGSEGV) + folded the descriptor loop's walking DS (=resource segment
struct[0]) into fixed DAT_1000_c004 + dropped the output segment word[DGROUP:0x4f0]. Faithfully rebuilt
(asm 0x1541..0x15be): mark the object (flags |= 0x22/0xd20; far-call stub 0x15c2:CS), then parse the
resource chain in R=struct[0] into a 4-word-per-entry descriptor table at (word[DGROUP:0x4f0]):out_off,
with the +0x800 segment-wrap and R:2 = entry count.

**(D) MGAVIDEO driver 0x4a3 palette-clear + vblank service (patch 070).** Clears the palette buffer
(es = word[DGROUP:0x782] dropped -> host-0 SIGSEGV) then spins on the retrace semaphore DGROUP:0x786
(folded to while(true)). Rebased the ES; and since our port runs no driver retrace IRQ, serviced the
request as the ISR FUN_0000_0b1f's `if (0x786 & 1)` branch would (out 0x3c8,0; upload the 0x300-byte
buffer to 0x3c9) and cleared 0x786. (The full FUN_0000_0b1f retrace ISR + descriptor-chain DAC uploader
is a separate reconstruction; only its 0x786 upload branch is on this path.)

**(E) FUN_0000_e584 intro-animation player header (patch 071) + timer pump (patch 072).** e584(far_ptr)
plays the intro sequence. Its resource descriptor is at STRSEG:bx where STRSEG = word[DGROUP:0x70]
(0x2d74) and bx = (uint16_t)param_1; Ghidra dropped ES=STRSEG on the field reads and DS=STRSEG on the
16-byte asset-name copy → rebased (patch 071, asm 0xe584..0xe5d5). Its frame-timing waits
`while ((c452 - b6e6) < b6e0)` on the INT-8 tick counter DGROUP:0x452, and driver frame-sync
FUN_1000_38cc `do{(*c40a)();}while(CF==0)`, were folded to bare spins → pump the cooperative INT-8 ISR
(patch 072, same class as 017).

**NEW FRONTIER — the driver VSYNC-POLL method vector c40a (DGROUP:0x40a) is not installed.** FUN_1000_38cc
(the driver frame/vsync wait, asm 0x138cc `xchg;lcall [DGROUP:0x40a];jae`) loops until the driver
vsync-poll method c40a returns CF=1. That method vector is 0 (traps → CF=0) so the loop does not
terminate → the intro animation cannot advance to its frame blit → 0xA0000 stays black. This is the
**driver present/vsync method-vector installation** subsystem the earlier notes flagged (SeedDriverVecs
must be extended with the reloc-section method-vector offsets incl. 0x40a + the driver re-decompiled so
`fist_icall` resolves them; each may then need its own base-loss patch). Installing c40a (the retrace
poll — our fist_vga already models the retrace bit on in(0x3da)) + the present/blit method that copies
the offscreen buffer to 0xA0000 is the remaining path to NONZERO PIXELS. Also still open: the `'\t>'`
garbage-filename open (a resource-name build that mis-forms one name; non-fatal, the open FAILs and the
player continues) and the full FUN_0000_0b1f retrace-ISR reconstruction.
New shim edits this iteration: `ghidra_compat.h` FIST_EXTGATE_LIN + decl; `fist_icall.c` gate route;
`tools/native_main.c` fist_extender_gate() + the 0x9000..0x9800 extender task pool.

## DONE (prior iteration) — ENV-CONFIG HANDSHAKE RESOLVED: the real nibble blob captured from the oracle, decoded to the Doug-Huffman EXTENDER's real-mode→PM callback gate; d99b reconstructed + e339 base-loss fixed; cb45 memory-gate seeded. Execution advances past cb45 + d99b into FUN_0000_db3f. New frontier = the EXTENDER-SERVICE handler (the gate must allocate the intro task) + the task-struct base-loss family
65 patches apply cleanly (`make check` green), engine decompile pristine (2249 fns), driver 216 fns, NO
regression (both `.DVR` load+wire 44/216, `[vga] set video mode 0x13`, `[mouse] init`, and now the
cb45 memory-gate all intact). **The d99b "LOADGAME handshake" the prior notes flagged is RESOLVED — but
the premise was wrong: it is not a LOADGAME env var, it is an EXTENDER (FIST.RUN) callback vector.**

**(A) The `IBM=`/`ATCODE=` premise is DEBUNKED (LZEXE artifacts, not env vars).** Unpacked LOADGAME.EXE
(LZEXE 0.91, from-scratch unpacker) → `loadgame_image.bin`: it contains **no** `IBM=`/`ATCODE=`/`GnP=`
string (those `strings` hits are literal-run fragments of the compressed stream: `ATCODE=` is the tail of
`CHEATCODE=`, etc.). **LOADGAME sets NO environment variable** — the child (FIST.RUN) inherits the plain
shell env (PATH/COMSPEC/BLASTER). LOADGAME passes hardware config a *different* way (a fixed command-tail
token `016401D4` + an in-RAM ASCII-hex hardware/OS blob `>H C4934,0034 / V0056 / A0220 I0007 D0001 ... >M
X0300,3B40 / E0400,3B40 / >O D0500,0279 ...`; `-K400,0,1000`/`-X5000` are LOADGAME's own extender
memory-sizing switches and never appear in that blob). Artifacts in scratch: `unlzexe.py`,
`loadgame_image.bin`, `dosrun/`.

**(B) The REAL nibble blob — captured from the running original under QEMU.** The nibble-encoded string
`FUN_0000_d99b` decodes (chars 0x30..0x3f only) is synthesized by the **Doug-Huffman EXTENDER**
(FIST.RUN), which rewrites `PSP:0x2c` to point at a crafted block before entering FIST.DAT. Boot the
oracle in run mode, `pmemsave` low RAM, and grep for a `[\x30-\x3f]{12,}` run → **three identical copies
of `06=90762117900000?300526`** (24 chars, at e.g. linear 0xe080). Decode (`byte=((c0-0x30)<<4)|(c1-0x30)`,
MSB byte first per word) → ea14=0x06d9 ea16=0x0762 ea18=0x1179 ea1a=0 ea1c=0x0f30 ea1e=0x0526; after the
d99b swaps (ea16↔ea18, ea1a↔ea1c): **far ptr [ea16] = 0x0762:0x1179 = linear 0x8799**, args ea1a=0x0f30
ea1c=0 ea1e=0x0526. **Disassembling 0x0762:0x1179 in the oracle's own memory shows the extender's
real-mode→32-bit-PM callback GATE** (`movzwl %sp,%esp; push ds/es/fs/gs; rep movsl` the 9-dword arg block;
stack-switch `xchg ebx,esp` / `pop ss`; far-jmp into the PM extender at `cs:0x1014/0x1016`). So d99b's
`e339` far-call is an **EXTENDER SERVICE CALL, not an engine routine** — which is exactly why it is not a
recovered FUN_, and why the ONLY reader of the dispatch flag `aa10` in the whole image is `e339` itself
(scanned every `mov/cmp [0xea10]` opcode). In our port the extender role is the shim, so the gate routes
through `fist_icall_far(0x8799)` — an honest logged trap→0 today.

**(C) Seeds (loader/extender role, `tools/native_main.c`).**
- **Env blob** — `setup_dos_env` now writes the oracle-captured `06=90762117900000?300526\0` as the FIRST
  env string at `PSP:0x2c` (the extender's crafted block), so d99b decodes the TRUE vector, not garbage.
- **cb45 memory gate** — `FUN_0000_cb45` (called from cae6 just before d99b) requires DGROUP:0x260 (conv
  free KB) ≥485, DGROUP:0x258 (XMS free KB) ≥2500, DGROUP:0x26c (disk free) ≥50, else it far-calls the
  "NOT ENOUGH … MEMORY" reporter (traps 0xf9fa/0x314). None of the three has ANY writer in the engine
  (grep + full store-opcode image scan) — they come from LOADGAME's hw-blob detection. Seeded to a 32MB
  reference machine (XMS 0x7b40 ⇒ HIGH detail per db3f's <5000/5000-11000/>11000 select; conv 0x279 from
  the captured `D0500,0279`; disk 0x64). **cb45 now passes cleanly — the 0xf9fa/0x314 traps are GONE.**

**(D) Faithful reconstructions (patches 066/067).**
- **066 (d99b, asm 0x0d99b..0x0daaf)** — Ghidra read the env from host NULL (`pcVar10=(char*)0x0` →
  SIGSEGV) and dropped every task-struct dynamic-segment base. Rebuilt: ES=word[word[DGROUP:0x68]:0x2c]
  env seg; decode nibble pairs into DGROUP:0xea14; swaps; e339 create-task; build the intro task struct
  (DGROUP-rel near offsets 0x1548/0x6d3c/0xdfbc); (*c354)()/(*c358)(); copy the four 16-byte intro
  asset-name templates **D02.PCX/C02.PCX/502.PAL/5.SKY** (DGROUP:0xea5e/0xea66/0xea74/0xea6e) into the
  task at +0x7a/+0x8a/+0x9a/+0xaa — i.e. **d99b builds the intro/title-screen task** (this IS the first-
  render path). Env decode + the far-call now resolve correctly to the extender gate (linear 0x8799).
- **067 (e339, asm 0x0e339..0x0e36c)** — Ghidra typed the task pointer `aa2c` as a host `int**`
  (`*DAT_2000_aa2c` → NULL deref) and rendered the far `ljmp [DGROUP:0x58]` as a near call. Rebased the
  `word[aa2e:aa2c]` task test to g_mem and the tail dispatch to a far call.

**NEW FRONTIER — the EXTENDER-SERVICE handler + the task-struct base-loss family.** The gate at linear
0x8799 honestly traps→0, so the intro task segment is 0 → `FUN_0000_db3f` (next in cae6, the detail-level
setup) SIGSEGVs at `*(word[aa2e:aa2c]+0x54)` — the head of a whole **task-struct base-loss family**
(db3f/dab0/db11/dde2/ddff/… all do `*(DAT_2000_aa2c + off)` which Ghidra typed as host-pointer arithmetic
and which needs the `aa2e:aa2c` far base). Fixing those is mechanical, but pointless until the **extender
gate returns a REAL task segment** — the task struct must be allocated by the gate (the create-task
service; args ea1a=0x0f30 may be the size). RESOLVING IT = implement the extender-service handler at the
0x8799 seam (reverse the Doug-Huffman kernel `re_out/fist_kernel_decomp.c` where it builds the PM service
dispatch, or capture the PM handler at linear 0x10000000+ under QEMU), then thread the arg block through
`e339`/`fist_icall_far` and clear the task-struct base-loss family. That is the next step toward the intro
present. New shim edits this iteration: `native_main.c` env-blob + cb45 memory-field seeds (documented,
loader role). Oracle artifacts (env capture): the blob string above, reproducible via run-mode pmemsave +
`[\x30-\x3f]{12,}` grep.

## DONE (prior iteration) — CURSOR CASCADE CLEARED (1c6a RETURNS); the driver rect-fill pixel-writer 1091 reconstructed; execution runs through filename-init + MEMMGR-search into the MAIN function FUN_0000_00d0. New frontier = FUN_0000_d99b (a DOS-environment CONFIG-BLOB decoder — needs the LOADGAME handshake, not a base-loss fix)
63 patches apply cleanly (`make check` green), engine decompile pristine (2249 fns), driver 216 fns, NO
regression (both `.DVR` load+wire 44/216, `[vga] set video mode 0x13`, `[mouse] init` all intact). The
whole documented cursor-DRAW cascade inside INT-33h mouse init is now reconstructed and **FUN_0000_1c6a
RETURNS** — the gate the prior notes flagged. Execution advances materially further:
```
… → INT 33h mouse init → cursor tree walkers → cursor-DRAW handlers (2108 erase + 2112/211d/2123/212a) →
1c6a RETURNS → 51a6 → 2144 (filename/config init) → 21bd filename base-name builder + driver 0197
string-table copy → 18d1/1345/135f MEMMGR object-search → into **FUN_0000_00d0** (THE main init/render
function) → cae6 → **FUN_0000_d99b** (frontier).
```
NONZERO PIXELS not yet reached (gdb-verified: 0xA0000 is still 0/64000): the cursor erase fills colour 0,
and the real first render is deeper inside 00d0 (past the d99b env-config blocker).

**(A) THE PIXEL WRITER — driver rect-fill FUN_0000_1091 reconstructed (patch 060/061).** Ghidra dropped
its entire fill body ("Subroutine does not return", leaving only the 2d6d call). Rebuilt instruction-for-
instruction from the driver image (objdump 0x1091..0x10d0): after 2d6d transforms the dirty rect, it
computes `di = word[DGROUP:0x7c8 + word[bx]*2] + word[bx+2]` (rowtab[y0]+x0), sets `ES=0xA000` (→ linear
0xA0000), and per row does `rep stosw`(cols>>1)/`rep stosb`(cols&1) of the colour word, advancing di by
`0x140-cols`. bx (rect near-offset in DGROUP) + al (fill colour) are the register inputs; **1091 was given
an explicit 2nd colour param** and the erase handler 2108 threads colour 0 (patch 061, asm `xor al,al`).
This is THE literal VGA pixel writer; it now writes real bytes into g_mem's 0xA0000 aperture (colour 0 for
the erase, real colours for the eventual render's fills).

**(B) Cursor-DRAW handlers 2112/211d/2123/212a (patch 062).** Dispatched by REGISTER (bx=node ptr,
bp=base) from FUN_0000_209e/201a — which the `__allregs` indirect-call model cannot thread, so param_1/
param_2 arrived as garbage host pointers (SIGSEGV in 212a). Each recovers **bx = word[DGROUP:0x3e08]** (the
node offset the dispatcher stores) + **bp = word[DGROUP:0x3afe]** (node-data base) and rebases all node
reads to DGROUP: 2112 = `xchg node[bp+di+1] ↔ DGROUP:0x18de`; 211d = near tail-jmp `[bp+di+1]`; 2123 =
far-call `[bp+di+1]`; **212a = RMW** `ax=node[bp+di+1]; ax=(*DGROUP:0x584)(ax); node[bp+di+1]=ax` (Ghidra
also dropped the INPUT ax feeding the driver method). All asm-verified 0x2112/0x211d/0x2123/0x212a.

**(C) Filename-init subsystem (patches 063/064).** 2144→21bd builds the resource 8.3 base name.
- **064 (21bd)** threads the dropped `ax=0x148` into the c550 driver-string call and rebases param_2 (a
  DGROUP near offset, 0x740) to g_mem+DGROUP; the '\\'/'.'-scan pointer arithmetic is difference-based so
  the uniform rebase preserves it. Now parses a REAL filename ("BACKLAND.BIN" observed).
- **063 (driver 0197)** the driver string-table copy: `ES=DGROUP`, `DS=word[DGROUP:0x70a]` (a template seg);
  bx=0x148 indexes a near-offset table in that seg, the string it points to is copied into DGROUP:0x740.
  Ghidra dropped the DS(template)/ES(DGROUP) bases + returned a bogus 0x2ba9.

**(D) MEMMGR object search/release FUN_1000_135f/1345/18d1 (patch 065).** 21bd→18d1 releases a named MEMMGR
object. **135f** is a segment-linked list search (head=word[header+0xa]; walk node.next=word[nodeseg:0xc];
match node[6]==cx && node[8]==bx) whose walking DS was FOLDED to the fixed DAT_1000_c006/c00c → constant
re-read → **infinite loop**; reconstructed the real chain walk. **1345** tries the 3 lists (DGROUP:0x16d4/
0x16f6/0x1718) — Ghidra's dead `in_CF` collapsed it to one list with the result discarded; restored via
`found ⇔ 135f return != 0`. **18d1** only unlinks (0d70) on found; at this early init the lists are empty →
not-found → 0d70 (a documented MEMMGR free-path follow-on) is **NOT reached** (verified with a breakpoint).

**NEW FRONTIER — FUN_0000_d99b is a DOS-ENVIRONMENT CONFIG-BLOB DECODER (needs the LOADGAME handshake, NOT
a base-loss patch).** app_entry→00d0→cae6→d99b. asm 0xd99b: `ES = word[word[DGROUP:0x68(PSP)]:0x2c]` = the
DOS environment segment; it then decodes the env as **pairs of ASCII chars** — `byte = ((c0-0x30)<<4) |
(c1-0x30)` — into the engine's service/config table at DGROUP:0xea14 (aa14…) until a NUL, then swaps
aa16↔aa18 / aa1a↔aa1c and allocs a task struct via **e339**. **e339 FAR-CALLS the method pointer stored at
aa16** (`call far [ds:0xea16]`, with aa1a/aa1c/aa1e as args) — i.e. the env blob supplies engine service
FAR-POINTERS. gdb-verified: our env is only the shim's `PATH=C:\` and the aa14 table is **all-zeros before
d99b**, so the decode reads garbage → aa16 = a garbage far pointer → e339 traps. This is the NovaLogic
**LOADGAME.EXE → engine environment-config handshake** (LOADGAME is launched `LOADGAME -K400,0,1000 -X5000
FIST.RUN` and builds an env blob the engine decodes here). RESOLVING IT is a recon task on LOADGAME.EXE +
seeding the correct env blob in the shim (loader role, like the PSP/DGROUP/BDA seeds) — NOT a faithful
per-function base-loss patch. That is the next step to get deeper into 00d0 toward the first present.
New shim/tool edits this iteration: none (all 6 patches are engine/driver source patches; re_out pristine).

## DONE (prior iteration) — ALL DRIVER METHOD VECTORS RECOVERED; full mode-set→palette→driver-dispatch→cursor-init path reconstructed; frontier = the cursor-DRAW handler cascade inside INT-33h mouse init
57 patches apply cleanly (`make check` green), engine decompile pristine (**2249 fns**), MGAVIDEO driver
re-decompiled with **all 94 method-vector offsets seeded** (216 exported fns), no regression (SOUND.CFG +
both `.DVR` loads + `[vga] set video mode 0x13` + `[mouse] init` all intact, boot trace runs further than
ever). **The three documented driver-method traps (MGAVIDEO+0x3e0/0x26e/0x1963) are GONE** — the driver now
truly dispatches its own methods instead of returning 0. Execution advances:
```
CRT → … → both .DVR load → mode-set (INT 10h AX=13h) → driver palette upload → engine 0x5200 driver
dispatch (5228/524e) → post-driver free list (3436/3446) → INT 33h mouse init → cursor-init tree walkers
(1c6a→1cdb→1d59 mark / 1ff5→201a draw-dispatch / 206f→209e mask-shift) → cursor-DRAW handlers (2108→driver
rect-fill 1091, 2d6d rect-coord xform) → **frontier: FUN_0000_212a** (next cursor-draw handler).
```
NONZERO PIXELS not yet reached: framebuffer at 0xA0000 = 0 (the cursor "erase" handler fills colour 0, and
the driver rect-fill FUN_0000_1091's fill body was dropped by Ghidra — see frontier).

**(A) Driver method-vector recovery (the headline unblock).** Runtime-dumped the engine DGROUP for every
far vector (off:seg) pointing into the loaded MGAVIDEO range (new gated diagnostic `FIST_DUMP_VECS=1` in
`re_out/fist_icall.c` → `dump_module_vecs`, one-shot per module) → **94 method offsets**. Fed the complete
list to `FIST_DRIVER_SEED_OFFS` in the Makefile `decompile-drivers` target and re-ran the MGAVIDEO driver
decompile (`SeedDriverVecs.java` consumes it) → the offsets 0x18e/0x26e/0x328/…/0x1963/…/0x410a are now
recovered driver functions in `fist_mga`'s fmap, so `fist_icall` resolves the engine's driver-method
dispatches (no more `TRAP overlay call -> MGAVIDEO.DVR+0xNNN`). NB the driver fn count moved 236→216 (18
of the aggressively-seeded targets fail Ghidra export — incl. two method vectors 0x14b1/0x2441 not yet on
the executed path; a future re-seed refinement).

**(B) Mode-set palette path (patches 046–049).** The video mode-set FUN_0000_00e8 dispatches 03e0→043e→0a02
to build+upload the default VGA palette. Root cause the palette buffer was unallocated:
- **049 (driver init 0009 → device-alloc DGROUP:0xcc)** — the device-object allocator method vectors
  DGROUP:0xcc/0xd0/0xd4 are **NEVER installed** (verified live: == 0 when the init runs; patch 031's
  si=0xec install does not populate 0x78..0xf8, and FUN_1000_1917 for 0xd4 is not even a recovered engine
  fn — the "0x78..0xf8 targets need seeding" gap). So the init's `mov ax,0x300; mov bx,0x782; call [0xcc]`
  palette-buffer alloc silently no-op'd → word[DGROUP:0x782]==0 → the upload wrote to segment 0 (SIGSEGV).
  Resolve the KNOWN target FUN_1000_182a (linear 0x1182a) directly via `fist_icall`, per patch-033 doctrine.
  (0x3e4 workspace via 0xd4=1917 stays a harmless trap until 1917 is recovered.)
- **046 (03e0)** thread AX=0 into the c580 palette-set call (dropped AX → 043e wrongly took its copy branch
  from a garbage segment). **047 (043e)** ES/DS base of the 0x180-word palette copy (via FUN_0000_0467).
  **048 (0a02)** ES=word[0x782]/DS=word[0x70a]-template bases + the persistent DI folded across the 7 0a50
  ramp calls (0x60 + 7×0x60 = 0x300 DAC palette; 0a50 itself is faithful given an absolute pointer).

**(C) Post-mode-set driver dispatch (patches 050–053).** FUN_1000_5228→524e drive the video-driver object.
- **051 (5228)** thread bx=0x264c into the c558 method call; **050 (026e)** the driver-object do-once init
  (sibling of 024f): DGROUP near-offset base, store the OFFSET (not a host ptr) into DGROUP:0x736.
  **052 (1963)** dispatch-vector selector: DGROUP byte read + two CS-relative (=load_seg) far-ptr tables at
  module 0x198e/0x199e → far vectors DGROUP:0x684/0x688 with segment = load_seg.
- **053 (3436/3446)** elide the bx==0 (no-op) `lcall [DGROUP:0x12]` — patch 043 kept it as an indirect
  call to the INERT C f842 decompile, which derefs stale-register garbage and SIGSEGVs once the (now
  working) driver-dispatch path leaves a bad pointer in registers. f842 bails on bx==0 → faithful no-op.

**(D) INT-33h mouse-cursor init walkers (patches 054–059).** FUN_0000_1c6a builds the software cursor via a
family of node-list walkers, ALL with dropped segment bases + mis-scaled int* strides (real stride 0xc
bytes, 16-bit word fields):
- **055 (1cdb)** the cursor node-list near offset fe04 was computed from the HOST symbol address
  `&DAT_1000_faf0` (truncated to garbage 0xabe0) instead of the near offset 0x3af0+param_4.
- **054 (1d59)** the mark walker runs with **DS=word[DGROUP:0x3e02] (=0x1faf, the sprite-list segment)**
  reloaded by the caller (asm 0x1d40) — dropped; `[bp+di]` byte marks are SS(=DGROUP). **056 (201a)** the
  draw-dispatch walker (DS=DGROUP): `di=word[bx]; call near [DGROUP:di+0x3e18]`. **057 (209e)** the
  mask-shift walker: shift the mask byte, on carry dispatch near [DGROUP:[bx]+0x423c].
- **059 (2108)** cursor-node erase handler: thread bx=word[DGROUP:0x3e08]+4 into the c60a driver rect-fill
  call. **058 (2d6d)** the driver rect-coord transform: DGROUP near-offset base of the 4-word rect.

**FRONTIER — the cursor-DRAW handler cascade + the dropped FUN_0000_1091 rect-fill body.** 209e/201a
dispatch a SET of small per-node handlers (2108 done; **2112/211d/2123/212a** remain), each reached via the
near vector `[DGROUP:[bx]+0x3e18|0x423c]` and each needing (a) bx=word[DGROUP:0x3e08] + bp=word[DGROUP:0x3afe]
threaded and (b) DGROUP base-loss fixes. Current crash: **FUN_0000_212a** (`ax=word[bp+di+1]; ax=(*[0x584])
(); word[bp+di+1]=ax` — a read-modify-write, DGROUP bases dropped). ALSO: **FUN_0000_1091** (the driver
framebuffer rect-fill, `mov ax,0xa000; mov es,ax; rep stos es:[di]` — the literal pixel writer) had its
fill body **dropped by Ghidra** (2d6d is marked "does not return"), so the cursor is not actually painted;
it needs a full asm reconstruction (rect fill at ES=0xA000 with a threaded colour arg — the erase handler's
colour is 0, so it alone paints nothing). Getting past all cursor handlers (→ 1c6a returns) is the gate to
the engine's first real screen render (logo / palette-fade / menu) = the true first light. New shim/tool
edits this iteration: `re_out/fist_icall.c` `FIST_DUMP_VECS` diagnostic (+ `dumped` field in
`ghidra_compat.h` struct `fist_ovl`), Makefile `FIST_DRIVER_SEED_OFFS` (94 MGAVIDEO offsets).

## DONE (this iteration) — **FIRST LIGHT (mode-set half): INT 10h AX=13h FIRES.** The whole driver-init template-copy cascade + the VGA mode-set FUN_0000_00e8 execute; the engine runs through its 0x5200 driver-processing into input init (mouse + joystick)
The driver-init cascade the previous notes mapped is now **fully threaded and executing end-to-end**, and
the video driver's VGA mode-set runs to its `INT 10h AX=0x13` → **`[vid] 10h set mode 0x13` / `[vga] set
video mode 0x13 (320x200x256 linear)` fires on the real boot path.** Execution then returns through the
engine's 0x5200 driver-processing subsystem, builds the post-driver free list, initialises the **INT 33h
mouse** (`[mouse] init`), and reaches the **joystick calibration** `FUN_1000_3a14` (the current frontier).
41 patches apply, engine decompile pristine (2249 fns), driver units 236/44, `make check`/`make native`
green, no regression (SOUND.CFG + both `.DVR` loads + mode-set all intact). NONZERO PIXELS not yet reached
(the game must run its full input/config init and reach its first render+present; the driver present/blit
method vectors currently TRAP — see frontier).

**The cascade (all asm-verified; the "path to FIRST LIGHT" the prior notes mapped, now built):**
- **034 (FUN_0000_f98b) — the DGROUP:0x36 template-copy service reconstructed.** `f98b(count,srcseg,slot,
  srcoff)`: alloc `count` bytes via FUN_1000_182a into DGROUP:[slot] (writes struct[0]=allocated seg), then
  `rep movsb` copies `count` bytes srcseg:srcoff → allocseg:0. Ghidra dropped the string-op ES:DI/DS:SI
  bases (dst started at host 0 → the observed NULL-write SIGSEGV) + the alloc args. srcseg is passed as an
  ABSOLUTE runtime segment (threaded by 035).
- **035 (MGAVIDEO FUN_0000_0009 + the 0x2 jmp thunk) — the FIRST driver-unit patch.** Threads the init's
  5 f98b template copies (into DGROUP:0x70a/0x70e/0x712/0x716/0x71a), the two workspace allocs (DGROUP:0xd4
  =FUN_1000_1917 / 0xcc=182a), and the driver-local method-vector reloc apply. The 5 template srcseg
  immediates (0x46a/0x52c/0x54b/0x54d/0x608) + the reloc section's leading seg word are MZ reloc sites →
  relocated to the driver load-seg at LOAD, modelled by adding `(fist_mga_base>>4)` at the f98b call sites.
- **Shim `fist_apply_reloc_at(seg,si,is_far)` (native_main.c)** — the driver-local reloc apply: applies a
  reloc section from an ARBITRARY segment (`mov bx,[DGROUP:0x70e]; lcall [DGROUP:0x12]` = the FAR applier
  f842, which the raw decompile renders inert). Section si=0 installs DGROUP:0x540 = 0xe8:load_seg = the
  VGA mode-set FUN_0000_00e8; si=8 installs the 98-entry method-vector table DGROUP:0x540..0x5ee; si=0x1c8
  installs 8 more. Used by 035 (init si=0), 037 (mode-set si=8), 038 (2c92 si=0x1c8).
- **036 (024f), 037 (00e8 mode-set), 038 (2c92), 039 (2ccd color LUT)** — the VGA mode-set path:
  FUN_0000_00e8 sets DGROUP:0x722=0xa000 / 0x732=320 / 0x730=200, calls 024f (DGROUP near-ptr + int/word
  scaling, patch 036), then **`INT 10h AX=0x13`**, then applies the si=8 method-vector section (037), then
  2c92 (which applies si=0x1c8, patch 038, and calls 2ccd the per-index color LUT, patch 039 — dynamic-ES
  base loss on the DGROUP:0x716 template buffer + overlapping undefined4 typing).
- **040/041/042 (engine FUN_1000_524e/52d1/530b/531b) — the 0x5200 driver-processing surface.** Reached
  from FUN_1000_5228 right AFTER the driver init completes (root-caused earlier as the video-DRIVER dispatch
  surface — it null-derefed only because the driver never loaded). All operate on the driver-object struct
  at DGROUP:[0x736] + the template segment DGROUP:0x71a; Ghidra dropped the DGROUP/ES bases and mis-typed
  the 16-bit fields as int/uint. Rebased faithfully.
- **043 (FUN_1000_3436/3446) — post-driver free-list builder.** Builds a 0x3f-node DGROUP near-offset free
  list from offset 0x18ea; Ghidra typed the near offsets as host `undefined2 *` (&DAT_...) with host-ptr
  arithmetic → wrong links + bad-ptr SIGSEGV. Reconstructed with 16-bit near offsets (both the 3436 entry
  and its 3446 fall-through copy).
- **Shim `FIST_RUNMS=<ms>` watchdog (native_main.c fist_timer_pump)** — dumps FIST_FBDUMP + exits after a
  wall-clock deadline, to capture a frame while the engine is in its (non-returning) main loop.

**CURRENT FRONTIER — `FUN_1000_3a14` (joystick calibration), then the road to actual PIXELS.**
- **3a14** is folded by Ghidra to `(*c012)(); do{}while(true);`. asm 0x13a14: reloc apply (bx=0, bails);
  then `mov ax,[0x44e]; L: cmp ax,[0x44e]; je L` = a spin on the INT-8-tick counter DGROUP:0x44e (advanced
  by the ISR — pump the cooperative timer, cf. patch 017); then a joystick-port (0x201) decay-timing loop;
  then `lcall [0x2d4]/[0x2d8]` methods, a table fill, and `FUN_1000_3ebf(cs,0x4c19,0x2ec)`. Needs a full
  faithful reconstruction. **Its callee FUN_1000_3ebf is ALSO base-loss-broken** (host-derefs param_3 =
  a far-ptr/SMC-thunk list walker) — a follow-on reconstruction.
- **Then, to reach NONZERO PIXELS:** the game must finish input/config init and reach its first render +
  present. The **driver present/blit/palette method vectors installed by the reloc sections currently TRAP**
  (`MGAVIDEO.DVR+0x3e0/0x26e/0x1963/0x2d4/0x2d8/...` — installed as DGROUP far vectors but the target
  offsets are NOT in the driver fmap, so `fist_icall` traps→0). They are setup/state methods (verified: not
  blits), but the eventual frame-present method that writes 0xA0000 is in the same class → **SeedDriverVecs
  must be extended with all reloc-section method-vector offsets + the driver re-decompiled** so they resolve
  and run (each may then need its own base-loss patch). That + reaching the first render is the remaining
  path to pixels. The mode-set (INT 10h AX=13h) — the documented FIRST-LIGHT headline half — is DONE.

## DONE (prior iteration) — TASK 2 COMPLETE: BOTH DRIVERS LOAD; execution dispatches INTO the video driver
The load_seg gate is solved. Both driver overlays now **LOAD on the real boot path** (not the self-test)
with a real allocated segment, and the engine now **dispatches INTO the video driver's init**. 31 patches
apply, engine pristine (2249 fns), video driver re-decompiled with its entry vector seeded (233 -> **236
fns**), `make check`/`make native` green, no regression (SOUND.CFG/FIST.CD/both .DVR opens still work,
allocator still valid). Live boot trace:
```
4B03 loaded 'SOUNDDVR.DVR' at seg 3a06 ... registered ... fmap=wired(44 fns)     [was REJECT load_seg=0]
4B03 loaded 'MGAVIDEO.DVR' at seg 3e3a ... registered ... fmap=wired(236 fns)    [was REJECT load_seg=0]
```

**TASK 2 — the driver-object method vector + load_seg (patches 031/032/033 + the 030 fix):**
- **030 fix (5214, si=0x2f0) — is_far 0->1 (was a latent bug).** DGROUP:0x12 = FUN_0000_f842 is the FAR
  reloc applier (verified: caller BX is IGNORED — f842 calls f7c3 which loads BX from the table segment
  DGROUP:0x74=0x3352 and loops till nonzero; the applier is picked by the DGROUP SLOT: 0x0a/0x0e near
  f7ef/f81d, 0x12/0x16 FAR f842/f860). f842 reads a leading SEG word then (off,val) pairs -> DGROUP[off]=
  val:seg. Patch 030 passed is_far=0 -> the seg word was misread as an off, so DGROUP:0x468..0x4ec were
  NEVER installed and DGROUP:0xf69/0x5bbe got garbage (gdb-verified). Now is_far=1 installs them correctly.
- **031 (06e3) — install si=0xec (DGROUP:0x12 far).** asm 0x106e6 `xor bx,bx; mov si,0xec; lcall [DGROUP:
  0x12]` installs the driver-object METHOD VECTORS DGROUP:0x78..0xf8 (far off:seg into service seg 0xf69),
  incl. the allocator method **DGROUP:0xd0 = 0xf69:0x21bb = linear 0x1184b = FUN_1000_184b**, DGROUP:0xcc
  = 0x1182a = FUN_1000_182a, DGROUP:0xd8, 0xd4, .... Installed faithfully via fist_apply_reloc_section(0xec,
  1) at the engine's own site (app_entry->06bc->06e3, before the driver loads).
- **032 (50c8) — restore the dropped file SIZE.** asm 0x150c8 is a near helper: open .DVR (AH=3D) / seek-
  to-end (AH=42 -> DX:AX = size) / close, returns DX:AX = size. Ghidra's __allregs model returned param_8
  and DROPPED the DX:AX size. Capture the size low word after the seek (high=0 for these <64KB MZ overlay
  modules) and return it.
- **033 (5051) — thread the c0d0 allocator call that populates load_seg.** asm 0x15071: `add ax,0xf;adc
  dx,0; >>4` = paragraphs; `mov bx,cs:0x5957(struct 0x3e4/0x4fa); mov cx,0x1c00(DGROUP); lcall [DGROUP:
  0xd0]` -> 184b allocates `paras` paragraphs via the faithful 0a31 free-list and writes the allocated
  segment into the driver struct's load-seg field struct[0]. The __allregs indirect call dropped AX/BX/CX
  (and the paragraph math), so struct[0] stayed 0. Thread it: DAT_5955 = the real size (patch 032), compute
  paras, call FUN_1000_184b directly with (paras, DGROUP, struct near ptr). Verified live: SOUNDDVR
  load_seg=0x3a06, MGAVIDEO load_seg=0x3e3a (both >= heap base 0x3400).

**The 0x5200 "cascade" ROOT-CAUSED — it is the VIDEO-DRIVER dispatch surface, not a standalone base-loss.**
FUN_1000_5228/524e/52d1/530b operate on a struct pointed to by **DGROUP:0x736** and a segment **DGROUP:0x71a**
+ far vectors **DGROUP:0x558/0x680**. None of these are in the engine reloc table (grep-verified) and none
are written by the engine — they are set by the **video driver's own init** (`_DAT_1000_c736 = param_1` at
fist_mga_decomp.c:173, etc.). So 524e null-derefed only because MGAVIDEO.DVR never loaded/initialised. With
the drivers now loading, the engine reaches the driver-init dispatch BEFORE 5228 (5228's subsystem gets
populated by the driver init, once it runs).

**Driver-init dispatch WIRED (SeedDriverVecs.java, new).** FUN_1000_5051 far-calls the driver INIT via
`call far load_seg:0x0002` = the `jmp <init>` thunk at module offset 0x2. Ghidra never saw this runtime
far-call target (nor the init body 0x9 nor the methods 0x93/0xa3/0xb3/0xe8/mode-set 0xe8), so they were
un-promoted bytes. New driver-overlay seeder **tools/ghidra/SeedDriverVecs.java** (wired into decompile.sh
+ Makefile decompile-drivers via `FIST_DRIVER_SEED_OFFS`, gated behind FIST_ENGINE_SEEDERS=0) follows the
0x0..0x8 jmp thunks + seeds explicit method offsets -> the init (thunk_FUN_0000_0009 @ 0x2 -> FUN_0000_0009
@ 0x9), the entry methods, and the **VGA mode-set FUN_0000_00e8** (sets DGROUP:0x722=0xa000, 0x732=320,
0x730=200, then **INT 10h AX=0x13 = SET MODE 13h**, then palette/blit vectors 0x538/0x53c) are now clean C.
Result: the engine's overlay dispatch now RESOLVES driver+0x2 (no trap) and runs the driver init.

**CURRENT FRONTIER — the driver-init template-copy cascade (fully mapped; the path to FIRST LIGHT).** The
driver init (FUN_0000_0009, asm 0x9..0x91) does, in order: **(1) five template-copies** via the engine
service DGROUP:0x36 = **FUN_0000_f98b** — each `f98b(count, srcseg, slot)` calls c0cc(=182a->184b) to
allocate `count` bytes into DGROUP:[slot], then `rep movsb` copies a template from the driver's own data
(driver-relative segments 0x46a/0x52c/0x54b/0x54d/0x608) into it; **(2)** two more allocations via
DGROUP:0xd4 (=0x11917) and DGROUP:0xcc (=182a) for the driver workspace (struct 0x3e4) + a 0x300-byte
buffer; **(3)** a **driver-LOCAL reloc apply** `mov bx,[0x70e]; si=0; lcall [DGROUP:0x12]` that runs f842
over the reloc section sitting in the copied buffer at [0x70e] -> installs the driver's DGROUP method
vectors (incl. the mode-set) so the engine can dispatch them. Execution currently SIGSEGVs at the FIRST
f98b: it has (a) the classic `rep movsb` dynamic-seg base-loss (ds:si src / es:di dst dropped -> host 0),
(b) un-threaded __allregs args (the driver's `(*DAT_c036)()` drops ax=count/bx=slot/dx=srcseg), and (c) a
module-base subtlety — the copy SOURCE segment must be the module-relocated `(fist_mga_base>>4)+srcseg`
(the driver decompile is base-0; the loaded module's segment immediates were MZ-relocated at load).
**Remaining work to FIRST LIGHT (each piece asm-verified & understood; est. several focused patches):**
  1. Reconstruct FUN_0000_f98b faithfully (alloc via 182a into DGROUP:[slot] + module-based memcpy).
  2. Thread the driver init's 5 f98b calls + the c0d4/c0cc allocs (driver-unit patch on fist_mga.c — a NEW
     patch category; the driver can call the engine's FUN_0000_f98b/182a via extern).
  3. Add a general driver-local reloc shim `fist_apply_reloc_at(seg, si, is_far)` (reads from g_mem+(seg<<4)
     +si instead of the fixed engine table) for the `[0x70e]` apply.
  4. **Seed the si=0xec method-vector TARGETS that RecoverAll missed** — e.g. DGROUP:0xd4 = 0xf69:0x2287 =
     linear 0x11917 is NOT a recovered function (add to SeedServiceVecs + re-decompile the ENGINE; verify
     2249 -> 2249+N). Several 0x78..0xf8 targets need this.
  5. Then the engine dispatches the driver mode-set (FUN_0000_00e8: INT 10h AX=13h) + blit to 0xA0000.
Also: re-decompile SOUNDDVR.DVR with SeedDriverVecs (its +0x2 init still traps; sound is secondary).

## DONE
- **INT register threading** (`tools/ghidra/InstallIntFixup.java`): `int n` → reg-file at g_mem 0xF0000 +
  dispatch to `fist_int_dispatch()` (re_out/fist_dos.c). `swi()` count = 0. Registers (incl. CF, DX:AX)
  thread in/out.
- **Near-pointer DS=CS segment-basing** (`tools/ghidra/SegmentFixup.java`): functions that run with DS=CS
  (e.g. `mov ax,cs; mov ds,ax`) had near data accesses lose their base → segfault. Fixed by tracking DS
  and overriding the DS context to CS for those instructions (130 insns / 13 fns). Skips explicit
  segment-override prefixes so SS:/ES:/CS: accesses are preserved.
- First real execution: CRT init, banner via INT 21h AH=09, installs INT 21/23/24 vectors.

## DGROUP low-area cached segment slots — "[0x70] family" (SS-relative dynamic-segment scratch)
The engine caches runtime **segment values** in a few DGROUP low-offset slots (SS=DS=DGROUP=0x1c00 in the
base-0 layout, so `SS:0xNN` == `DGROUP:0xNN`). These are set at CRT init from the load base, never by a
plain engine store, and Ghidra drops the segment base on every access through them → the near offset is
dereferenced as a host pointer → SIGSEGV. **Oracle-confirmed values (base_seg 0x1519 → subtract for the
base-0 image paragraph):**
- **DGROUP:0x68 = PSP segment** (already handled: native_main seeds FIST_PSP_SEG; patch 005).
- **DGROUP:0x70 = far string/resource DATA segment**. Oracle: 0x428d = 0x1519+0x2d74 → **image para 0x2d74
  (linear 0x2d740)**, holds the engine's filename templates: `CONFIG.DVR`@0x7e, `SOUND.CFG`@0x89,
  `FIST.CD`@0x93, `\FISTDATA`@0x9b, `FISTDATA`@0x9c, a keyword table @0x212, etc. Read in **60+ sites** as
  ES/DS. **Seeded in native_main `fist_install_dgroup()` to 0x2d74** (the CRT/loader role).
- **DGROUP:0x74 = 0x3352 (RESOLVED, patch 010).** NOT an oracle probe — the engine's own CRT init
  `FUN_0000_f738` sets it with a genuine `movw [0x74],0x3352` (asm-verified), and the decompile already
  renders that store correctly at the true slot 0x1c074. 0x3352 is a segment → linear 0x33520 (image tail),
  a 0x3e0-byte double-buffer template. FUN_0000_f77f copies `[0x74]:0` → `(DGROUP+0x7d):0` (= 0x1c7d0),
  then `xchg`-installs 0x1c7d as the new DGROUP:0x74. The only bug was Ghidra dropping the `rep movsb`
  segment bases AND aliasing the one slot to two host addresses (DAT_1000_c074 @0x1c074 for the DS-ctx read,
  DAT_2000_bb04 @0x2bb04 for the post-`push ss;pop ds` SS-ctx xchg). Patch 010 reconstructs both operands
  via the true slot 0x1c074.

**Why the SegmentFixup context-register approach does NOT fully solve this:** it works for ordinary
`es:[mem]` operands (setting the ES context folds them), but Ghidra **ignores the segment context register
for x86 string ops (LODS/STOS/MOVS)** — and the dominant [0x70] idiom in this hand-asm engine is exactly
`mov es,[0x70]; lodsb/stosb/movsb`. So those must be fixed by **faithful per-function patches** that
reconstruct `g_mem + (seg<<4) + off`. (An ES-context extension of SegmentFixup was prototyped and reverted:
it only fixed the non-string minority and caused decompiler-rendering churn.)

## DONE (this iteration) — reached SOUND.CFG load; broke out of the entire sound-config subtree (fd1c)
Execution now: CRT → DGROUP install (incl. **DGROUP:0x70 = 0x2d74** seed) → INT 21/23/24 hook → **open+read
SOUND.CFG (real 10 bytes, parsed)** → FIST.CD check (correctly absent → HD data path) → **fd1c sound-config
returns** → next stage. Crashes in **FUN_0000_f77f** (DGROUP:0x74 double-buffer, above). Not yet at video.
New patches (all asm-verified, 2240 fns preserved, `make check` green):
- **006 (4c9a)** — SS:0x70 dynamic-segment source restore: `mov ax,ss:[0x70]; mov ds,ax; lodsb ds:si` →
  read src segment from DGROUP:0x70, base `g_mem+(seg<<4)+SI`; dest SS:0x740. (Ghidra mis-resolved
  ss:[0x70] to a fixed DAT_2000_bb00 via the static SS ctx 0x2ba9 and dropped `mov ds,ax`.)
- **007 (fd79, SOUND.CFG parser)** — four converging losses: dropped SI=0x89 template offset (threaded via
  fefb param_5), dropped open CF (threaded from INT reg-file slot uRam000f0012 — the `jb` was a dead
  `in_CF=0` local), [0x70] keyword-table base loss, and `int*`-vs-word **pointer scaling** (int is 4 B in
  the flat model; the table is 16-bit words).
- **008 (fdf7, FIST.CD check)** — same class: SI=0x93, CF thread, `es:[0x158]` STRSEG store base.
- **009 (fe47, FISTDATA data-path detector)** — DS=DGROUP base loss on `movb [0x16c6]` and the
  `lcall [0x5c]/[0x60]` vector reads; ES=[0x70] base on `es:[0x158]`. chdir helper 0x23a5 stays a no-op
  `func_0x` stub (unresolved-near-call class) — harmless (our INT-21 chdir accepts, open_ci searches dirs).
- Shim: **native_main** seeds DGROUP:0x70=0x2d74; **fist_dos.c open_ci** now rejects empty/dir names
  (real DOS returns "not found") — an empty name was opening the data DIRECTORY on Linux.

**Sub-classes surfaced (recurring, expect more before video):** (1) **[0x70]-family dropped segment base**
on string ops — per-function patch. (2) **CF/flag threading** — a `call fn; jb` where fn ends in an INT
reads back via the reg-file CF slot uRam000f0012; a general `in_CF=0` dead-local pattern. (3) **dropped
`mov si,imm` template offsets** not threaded to a callee's register param (__allregs param-propagation
gap). (4) **unresolved near-calls** rendered as no-op `func_0x…` stubs (0x23a5 etc.) — decompile-
completeness gap.

## DONE (this iteration) — past f77f, through the SMC callback subsystem, into MEMMGR (patches 010–014)
Execution now advances: CRT → DGROUP install → INT 21/23/24 hook → SOUND.CFG → FIST.CD absent → INT 9/5/2
hooks → **f77f double-buffer swap** → **06bc: SMC callback-list register (4cb9/4cd0/4ce7) + INT 21h AH=48
memory alloc (→ seg 0x3400) + MEMMGR block init (1467) + header copy (140e)** → crashes in **FUN_1000_096c**
(the MEMMGR free-list walker). 6 DOS calls now serviced (2× AH=48 alloc). 2240 fns preserved, all 14
patches apply clean, build green, decompile pristine, DGROUP:0x12 service vector verified intact (no
regression). New patches (all asm-verified):
- **010 (f77f)** — DGROUP:0x74 double-buffer swap; `rep movsb` seg-base restore (see DGROUP:0x74 above).
- **011 (4cb9 + 06bc)** — SMC far-JMP thunk install: `pop es` loads ES=caller CS and LEAVES it set;
  reconstruct dst=g_mem+(ES<<4)+DI; thread 06bc's CS (0x1000) and base its `movb es:[0x19bb],0x9a` SMC
  store at linear 0x119bb. Class: unaff_CS/ES segment register.
- **012 (4cd0 walk + 4ce7 insert)** — SMC callback-list linked-list: `lds si,[si+1]` walk to the 0xdb8b
  terminator, append the new node; every far-pointer segment base was dropped. Reconstruct all list
  accesses as g_mem+(seg<<4)+off (seg 0x1000, the caller CS). Nodes = 5-byte far-CALL/JMP thunks in an
  un-decompiled SMC scratch region ~0x1000:0x19bb; **inert for our C control flow** (we never execute
  g_mem as code) but memory-faithful.
- **013 (1467)** — MEMMGR block/pool init runs entirely with **DS=ES=param_1** (the allocated segment);
  Ghidra applied the static DS ctx 0x1c00 and mislabeled every access as DGROUP `DAT_1000_c0XX` (which
  would have clobbered the real DGROUP service table). Rebased the whole function to ds=g_mem+(param_1<<4).
  **New class: dynamic-segment DS** (SegmentFixup only handles DS=CS, not DS=<register>).
- **014 (140e)** — `rep movsw` copies 0x10 words from the new block `[DAT_1000_d758:0]` into DGROUP:0x16d4;
  Ghidra kept ES:DI=DGROUP:0x16d4 but dropped the DS source base (→ `*0x0`). Reconstruct src=g_mem+(seg<<4).
- **Shim (native_main):** seeds the **SMC callback-list terminator sentinel** — word 0xdb8b (`mov bx,bx`)
  at seg 0x1000:0x19c0 (linear 0x119c0). The engine's empty-list init establishes this in the un-decompiled
  SMC scratch region (a decompile-coverage gap); without it the 4cd0 walk spins forever. VALIDATED by gdb
  poke: seeding it makes the walk terminate and 06bc proceeds. NOT observable via the oracle — the original
  faults under QEMU **before** 06bc (verified: 0x119bb/0x119c0 are pristine 0x71.../0x554f in the live crash
  dump, i.e. our native port now runs materially FURTHER than the QEMU oracle reaches). TODO: locate the
  exact engine init and demote the seed to a patch.

## DONE (this iteration) — PAST MEMMGR + vsync; into the timer-tick stage (patches 013-rewrite, 015; seeder; BDA)
Execution now advances materially further: CRT → DGROUP install → INT hooks → SOUND.CFG → 2× AH=48 alloc
→ **MEMMGR pool build (1467) + free-list walk (096c) both complete** → past the DGROUP:0x2e service
dispatch (no trap) → **vsync wait (30de) clears** → **INT 8 (PIT timer) hook installed** → spins in
**FUN_1000_3346** (timer-tick wait). Deterministic (2/2 runs stop at 3346). 2248 fns, build green, all
patches apply, no service-routine dispatch traps. Changes (all asm-verified / loader-role):

- **(A) DGROUP service-routine recovery gap — SOLVED systematically.** New reproducible Ghidra postScript
  `tools/ghidra/SeedServiceVecs.java` (wired into `decompile.sh` POST **before** RecoverAll) creates
  functions at the 12 DGROUP service-table targets (seg 0xf69 offsets 0x15f,0x18d,0x1b2,…,0x2fb =
  linear 0xf7ef,0xf81d,0xf842,…,0xf98b) — they are reached ONLY via the runtime-installed
  `call far [DGROUP:0xNN]` so static discovery never seeded them. Fresh decompile: 2240 → **2248 fns**
  (the 8 previously-unrecovered service routines). All 12 now in the assemble fmap → the dispatcher
  resolves every service (the `TRAP … 0x0f932` for DGROUP:0x2e is **gone**; f932's guard runs and
  correctly skips its inner SMC loop since ss:[0x242] < 0x33). **Bonus:** seeding gave f7ef/f842 clean
  function boundaries, so the raw decompile now already renders their guarded loops correctly — patches
  **002/003 became redundant and were removed** (this is the "revert 002/003" the oracle note called for,
  achieved by fixing the root-cause boundary loss rather than by hand-patching the mangled loop).
- **(B) 1467 rewritten faithfully (patch 013) — the actual root cause of the 096c crash.** 1467 is the
  MEMMGR pool/free-list builder: ES pinned = block base, but **DS WALKS a node-segment chain**
  (block, block+2, block+4, … via `mov ds,ax; add ax,2` + a `loop`), threading a doubly-linked circular
  free-list. The prior patch 013 pinned DS=param_1 for the whole function → every node write clobbered
  the block header → the free list was never built and `block[0xa]` (the walker's head) stayed 0.
  Rewritten instruction-by-instruction from asm (0x11467..0x115f3). **Verified: block[0xa] = block+2 =
  0x3402** (valid head) at runtime.
- **(B) 096c + 0dc2 reconstructed faithfully (patch 015).** 096c = the segment-chained free-list walker
  (DS from SS:0x16de, reloaded per-iteration from node[0xc]); Ghidra had mangled it to `*0x0 < *0x0`.
  The near-called helper 0dc2 receives the current node via a shared MEMMGR DS cursor (globals
  `g_mm_ds`/`g_mm_cf`/`g_mm_zf` — DS persists across the near calls in the asm) and returns CF/ZF the
  walker branches on. Its **reached** path on the boot pool is entry-guard + shared tail 0x10d9e (every
  node has `[2]&0x50` set → deep block-relocating allocator at 0x10dcd never entered; verified by gdb).
  The deep 0dc2 allocator (+ callees 10f91/11019/110dc/11117/10cce/10f80) and 0d70 (the flag&2 unlink
  branch, unreached — no node has bit1 set) are documented follow-ons; 0dc2's deep path logs loudly if
  ever reached (no silent skip).
- **Loader role (native_main `setup_bda`):** seed the **BIOS Data Area** — `0040:0063 = 0x03D4` (color
  CRTC I/O base) and `0040:0049 = 0x03` (video mode). FUN_1000_30de is a vsync wait that reads
  `0040:0063`, `+6 → 0x3DA` Input Status Reg 1, and polls the retrace bit; with the BDA uninitialized it
  spun on a bogus port (0x99c8). Our `in(0x3DA)` handler toggles the retrace bits, so the wait now clears.
  A real BIOS sets these at power-on; TODO: consider setting `0040:0063` from the INT 10h mode-set path.

## DONE (this iteration) — TIMER-DRIVEN EXECUTION MODEL; 3346 spin cleared; into overlay load (patches 016-020)
Execution advances materially past the timer wall: CRT → DGROUP → INT hooks → SOUND.CFG → 2× AH=48 alloc →
MEMMGR → vsync → **INT 8 (PIT) ISR installed + DRIVEN** → **FUN_1000_3346 tick-spin CLEARS (ss:[0x452]
advances; verified c452=1)** → out of FUN_1000_2ebe → FUN_1000_5051 (patch 020) → into the engine's
**overlay/driver-load phase** (INT 21h AH=4B load-overlay + file opens). Deterministic (3/3 stop at the same
next crash). 2248 → **2249 fns** (+1 seeded ISR entry), **18 patches** (016-020 new), `make check` green, no regressions.

**Timer model (safe ISR invocation + rate knob):**
- Engine installs its INT-8 ISR via set-vector 0x08 (DS:DX = 0xf69:0x3a68 → linear **0x130f8**). RecoverAll
  never seeded it (reached only via the runtime IRQ vector), so **`tools/ghidra/SeedRuntimeVecs.java`** (new,
  wired into `decompile.sh` before RecoverAll like SeedServiceVecs) creates a function there → decompiles as
  **FUN_1000_30f8**, lands in the FUN-map.
- **Shim (`tools/native_main.c`):** SIGALRM is async-signal-safe — only bumps BIOS tick 0x46C + raises
  `g_tick_pending`. **`fist_timer_pump()`** drains pending ticks by invoking the ISR via `fist_icall(0x130f8)`
  (re-entry-guarded, 4-tick budget). Pump sites: the reconstructed spins (patch 017) + `in()`/`out()` port
  shims. Handler captured in `fist_dos.c` set-vector (AL=8 → `fist_set_int8_handler`); INT-8 dispatch case
  added (chained BIOS tick). **RATE KNOB `FIST_TICK_HZ`** (default 200) = SIGALRM freq; the documented seam
  for a later deterministic instruction-counted tick source.
- **Patch 017 (3346 + 3352):** reconstruct the two ISR busy-waits as genuine `volatile` re-read spins that
  pump the timer (Ghidra had folded 3346's two ss:[0x452] reads into `do{}while(true)`).

**Three asm-verified fixes were also required before ss:[0x452] could advance (the real substance):**
- **016 — timer calibration mis-based (DS=SS).** `FUN_1000_2fd3`+`FUN_1000_3064` compute the ISR's fractional
  STEP (d8b8) and PIT divisor (d8c6) but run `mov ds,ss` (DS=SS=DGROUP=0x1c00 at runtime); Ghidra used the
  load-time SS placeholder 0x2ba9 → results landed at dead 0x2bXXX instead of the DGROUP timer block →
  d8b8=d8c6=0 → accumulator never carried. Rebased 10 symbols to DGROUP. New class: **dynamic-DS-from-SS**
  (SS ctx 0x2ba9 is only correct pre-CRT; engine code runs SS=DGROUP=0x1c00). Verified d8b8=0x34b, d8c6=0xf8.
- **018 — ISR tick-gate semaphore (CS-base + byte).** `cmp byte cs:0x2d58,0xff` runs with the ISR's CS=0xf69
  → real enable byte at **0xf69:0x2d58 = 0x123e8 = 0xff**. Ghidra recovered the ISR subtree in the overlapping
  CS=0x1000 window → based it at 0x12d58 (an opcode byte of an unrelated decompressor) AND typed the byte
  compare as a word. Class: **CS-absolute mis-base for the 0xf69 cluster recovered under CS=0x1000**.
- **019 — reentrancy guard `c446==-1` typed as word.** `inc byte [0x446]; jne` (resting 0xff) → rendered
  `undefined2 == -1` ⇒ word 0x00ff ≠ 0xffff → the `d8b6+=d8b8; if(carry&&sema) c452++` body skipped every
  tick. Byte-corrected (also c2a8). Class: **byte variable typed as word** (family of 018).

## DONE (this iteration) — THE GATE FALLS: FUN_1000_0a31 MEMMGR allocator rewritten faithfully; execution runs deep into driver init
The 0a31 best-fit allocator (the ONE subsystem that gated FIRST LIGHT) is faithfully reconstructed and
**verified correct**; execution no longer hangs/crashes in MEMMGR and now runs materially further — through
the object-pool init (14da/197b) and into the overlay/driver-init subsystem (0x5200 range). 30 patches
apply, `make check` green, 2249 fns (decompile pristine), no regression (SOUND.CFG/FIST.CD/both .DVR opens
still work). New patches 025–030 + rev of 023 (all asm-verified):

- **025 (0a31 + 161d + 0bd1/0bef/0c21/0c7d/0c7f + 0d70 + 1040 + 1110/1114 + 17e5) — the allocator rewrite.**
  Best-fit over a **segment-linked doubly-linked circular free list** (control block BP=DGROUP:0x16d4; node
  paragraph struct [0]=base [2]=flags [4]=size [6]/[8]=owner [0xc]=next [0xe]=prev). DS/ES walk node
  paragraphs, advancing via node.next/prev each iteration; SS=DGROUP=0x1c00 so the heap bookkeeping
  (free-count 0x16dc, free-descriptor pool head 0x16e2, refill flag 0x1744) is DGROUP-absolute. Ghidra had
  folded the walking DS/ES node fields into fixed DGROUP globals (DAT_1000_c004/c00c/c00e) → constant
  re-read → infinite spin, and folded the split/allocate path (0b1a/0b58/0b70) to `*0x0` null-derefs.
  Rewritten instruction-by-instruction; the cross-call node cursor + returned CF/ES/DS are shared through
  the g_mm_* statics (extends patch 015's g_mm_ds/cf/zf with g_mm_es/di/bp/ctl). **CRITICAL asm-reading
  correction:** opcode `0x3b` (`cmp r16,r/m16`) computes `ax - mem` (req - size), so the objdump
  `cmp %es:0x4,%ax` is `req - size`, NOT `size - req` — every unsigned best-fit/base comparison had to be
  the reverse of the naive AT&T reading (a first wrong pass picked the 0xffff sentinel and spun in the
  base-sorted insert). 161d = the descriptor-pool REFILL (bumps free-count so the recursive 184b→0a31 alloc
  doesn't re-refill, wraps the returned block in a free node, carves it into descriptors). 17e5 fixed to
  return the descriptor seg (AX) + propagate CF, and to pass DGROUP=0x1c00 (not the static SS 0x2ba9) to
  184b. The cascade helpers 0c21 (1c68/1fcc deep block-mover) and the FREE-path coalescers 1110/1114 are
  faithfully transcribed but NOT on the boot alloc path (log loudly if reached, per patch 015 precedent).
- **023 (184b, rev) — read the allocated segment from 0a31's descriptor.** After 0a31, ES = the descriptor
  node it carved (g_mm_es); struct[0] = 0a31's return = descriptor.base = the allocated segment (= load_seg).
  The prior 023 mis-read it from DGROUP:0 and checked a dead `uVar4` instead of the shared g_mm_cf.
- **026 (140e) — pool size = the DOS AH=48 max-alloc result, not a phantom param.** 140e grabs all free
  conventional memory (INT 21h AH=48, ~23552 paras here) and passes BX (that size) to 1467; __allregs
  couldn't thread BX through the swi() so it treated 1467's size as an input register param (garbage 99) →
  a 99-paragraph pool inside a 23552-paragraph block → the first >85-para request overflowed → 0a31 CF=1 →
  184b panic (13e4). Pass the saved AH=48 result (uStack0002). **Pool now spans the full block.**
- **027 (14da zero-fill) / 028 (197b) / 029 (14da counter) — the object-pool init cascade past the
  allocator.** 027: the `rep stosw` that zeroes the just-allocated buffer at ES=struct[0] lost its ES base
  (wrote host 0). 028: 197b's DGROUP table init lost the DS/ES=DGROUP base + int-vs-word scaling. 029: the
  14da pool-init loop counter DGROUP:0x2674 was typed `int**` so `- 2` scaled to `-8 bytes` → underflowed
  past 0 → infinite loop (word arithmetic). Class: pointer-scaling on an int**-typed DGROUP slot.
- **030 (5214) — install the near reloc/method-vector section si=0x2f0.** asm `xor bx,bx; mov si,0x2f0;
  lcall [DGROUP:0x12]` → the reloc service f842 (BX=0 ⇒ near applier f7c3). The __allregs indirect service
  call dropped BX/SI (f842 ran on garbage → SIGSEGV) and f7c3/f842 are inert (dropped string-op bases);
  installed faithfully via the shim `fist_apply_reloc_section(0x2f0,0)` at the engine's own site (as 022).

**VERIFIED allocator correctness (gdb at the 5214 checkpoint, one allocation outstanding):** control block
`base=0x3400 size=0x5c00 free=0x5496 head[0xa]=0x3402 tail[0xc]=0x3a04 alloc#=1`; free-count=0x2fb;
free list **base-sorted ascending** `3402(base 3404,sz 903) -> 39fc(base 3a06,sz 164) -> 39fe(base ffff,
sz 0) -> 0` — terminates at the 0xffff sentinel, **no cycle**, all bases ≥ heap base 0x3400, sizes
consistent, pool spans the full 23552-para block. Returned load segments (e.g. struct[0x4f0]=0x3a06) are
sane (≥ 0x3400). The GATE is solved.

**CURRENT FRONTIER (post-gate):** app_entry advances CRT→…→MEMMGR (works)→14da/197b object-pool init
(works)→5214 reloc install (works)→**FUN_1000_5228→FUN_1000_524e** (SIGSEGV) — the 0x5200 driver-init
subsystem, a run of the SAME dynamic-ES/DGROUP base-loss class (524e: `mov es,[0x71a]; lods es:(si)` with
the ES base dropped and a small-offset host deref). Both .DVRs still REJECT (`load_seg=0`) because the
driver-object method vector DGROUP:0xd0 (reloc section si=0xec at FUN_1000_06bc) is still not installed and
the `(*DAT_c0d0)()` method call's args are not yet threaded (TASK 2, per-patch) — that install populates
struct 0x3e4/0x4fa[0] via 184b (which now works) → real load_seg → the .DVRs load + dispatch. Next: work the
524e→52d1→530b→… base-loss cascade forward and install si=0xec at 06bc, toward the video mode-set + blit.

## DONE (prior iteration) — 184b crash CLEARED + both .DVR filenames byte-perfect; leg (b) narrowed to the 0a31 allocator
Execution advances past the documented terminal crash. Both driver overlays now build a **byte-perfect
filename that OPENS** on the real boot path — `..\SOUNDDVR.DVR` AND `..\MGAVIDEO.DVR` (`3D open ... -> ok`
for both). Both are now gated ONLY on `load_seg` (leg b). 22 patches apply, `make check` green, 2249 fns
(decompile pristine), overlay self-test WIRED(233) — no regression. New patches 023/024 + one shim seed:

- **TASK 1 — systematic indirect-arg-threading via Ghidra pointer-typing: ATTEMPTED, FALLS BACK (mechanism).**
  Typing a method-vector slot (`DAT_1000_c0cc/c0d0/c0d8`, the `(*DAT)()` sites) as a pointer-to-`__allregs`-
  FunctionDefinition CANNOT faithfully thread args, because Ghidra assigns a FunctionDefinition's parameter
  storage from the prototype **model's fixed pentry order** (AX,BX,CX,DX,SI,DI,BP), whereas `ApplyConv`
  commits each real target's params in **per-function varnode-iteration order** (a NON-canonical
  register→slot map — e.g. FUN_1000_184b has param_2=CX, param_4=BX, not the canonical param_2=BX,param_3=CX).
  A generic pointer type would therefore emit args in the WRONG positional order for the specific target,
  and in the flat-C ABI (register storage is erased at C emission → positional cdecl) that silently
  corrupts the call. Making it correct would require **either** a global convention change (commit ALL 7
  regs, canonical order, to every function — churns all 2249 fns + all patches: regression) **or** a
  per-slot FunctionDefinition matched to each target's exact committed storage (needs the runtime
  vector→target map and equals per-patch work but with fresh-decompile risk). Same class of Ghidra
  limitation as the string-op segment-context failure. **⇒ FALL BACK to per-patch faithful threading**,
  which the DIRECT-call path already does correctly (args ARE threaded there — the crash was base-loss,
  not dropped args); the indirect method-vector sites get threaded per-patch as they come onto the path.
- **023 (FUN_1000_184b) — MEMMGR object-allocator dynamic-segment base restore = the terminal crash FIXED.**
  184b builds a MEMMGR object at CX:BX (CX=DGROUP seg, BX=near offset). asm 0x1184b: `mov es,cx;
  or es:[bx+2],0x22; … call 0a31; mov ax,es:[0]; mov [bx],ax` (struct[0]=DGROUP:0=the allocated segment).
  Ghidra dropped the ES=CX/DS=CX bases (bare-offset host deref → SIGSEGV at `*0x4f2`) and mis-based the SS
  slot store (DGROUP:0x16f2 → static-SS 0x2d182). Rebased every struct access to g_mem+(param_2<<4)+off;
  struct[0] now receives DAT_1000_c000 → real load_seg once 0a31 works. **Verified: crash gone, execution
  reaches 0a31.**
- **024 (FUN_0000_14da) — alloc SIZE immediate mis-typed as a symbol address.** The scratch-alloc size
  (asm `mov ax,0x2c6; inc; shl x3` = **0x1638**) was rendered `(undefined2 *)&DAT_1000_d638` → the host
  pointer g_mem+0x1d638, so 182a's size math ran on garbage (req size 8485490). Pass the asm-verified
  constant 0x1638. Class: immediate-vs-symbol (pointer-vs-constant) mis-type.
- **Shim seed — video chipset `DAT_1000_c246` (DGROUP:0x246) = 0x56 (TASK 3, DONE).** FUN_0000_134e:
  `c246==0x56 → drive letter 'M'(0x4d)` → "..\MGAVIDEO.DVR" (patch 021 pokes ds:0x743). c246 is ONLY EVER
  READ — a VESA/chipset probe our port doesn't run populates it; the value the original detect yields on the
  supported Matrox (MGA) path is 0x56. Seeded in `native_main fist_install_dgroup()` (loader role, models
  the not-run detect result), documented. **Verified: video filename byte-perfect + `3D open ... -> ok`.**

**LEG (b) FRONTIER — now precisely the MEMMGR free-list allocator FUN_1000_0a31 (the "deep MEMMGR"
follow-on the docs flagged).** With 184b fixed + the size threaded, 0a31 is reached and either spins or
null-derefs on its FOLDED segment-walk. Reversed (asm 0x10a31..0x10bd0): 0a31 is a best-fit allocator over
a **segment-linked circular free list** (control block at SS/BP=0x16d4 = DGROUP:0x16d4; head segs at
[bp+0xa]=0x16de / [bp+0xc]=0x16e0; free-count SS:0x16dc). Node struct (paragraph-based): `[0]`=base,
`[2]`=flags, `[4]`=size, `[6]/[8]`=owner, `[0xc]`=next-seg, `[0xe]`=prev-seg. The walker
(`mov es,[bp+0xc]; mov cx,es:[0xc]; jcxz end; mov es,cx; cmp es:[4],ax; ja loop`) advances **ES through
node.next** each iteration — Ghidra FOLDED the walking-ES/DS node fields into fixed DGROUP globals
(`DAT_1000_c004`=es:[4]/size, `DAT_1000_c00c`=es:[0xc]/next, `DAT_1000_c00e`=prev) → the loop re-reads a
constant → infinite spin; and the allocate/split path (0b1a/0b58/0b70) is rendered as pure null-derefs
(`*(int*)0x0`, `*(uint*)0x0 < *(uint*)0x0`). "Type propagation not settling" is flagged on the function.
FAITHFUL FIX = a full asm rewrite of 0a31 (like patch 013 for 1467 / 015 for 096c) **plus** its interlocking
callees — the **161d refill** (taken first here: free-count=0 ≤ 3), the search helpers **0bd1/0bef/0c21/
0c7d/0c7f** (each an ES/DS node-walker, same fold), **0d70** unlink, **1040/11040**. This is the remaining
gate: once 0a31 returns a real segment at DGROUP:0 → 184b writes struct[0] → 5051 feeds it as load_seg →
BOTH .DVRs load at their alloc'd segment + dispatch. NB the driver-struct path also needs the method vector
`(*DAT_c0d0)` installed (reloc section si=0xec at FUN_1000_06bc) + its args threaded per-patch (per TASK 1).

## DONE (prior iteration) — BLOCKER #1 leg (a) SOLVED: driver-filename BUILDER works; overlay loads + dispatches
The AH=4B overlay filename is now **correct** ("..\SOUNDDVR.DVR" built byte-perfect; "..\?GAVIDEO.DVR" for
video, drive-letter pending — see below). The driver-filename builder call finally runs, and the overlay
**load → reloc → register → fmap-wire → dispatch** machinery is proven end-to-end on the REAL boot path
(not just the self-test). Root cause was NOT (only) a dropped DX — it was that the whole **DGROUP
indirect-call VECTOR TABLE was never installed**, so the builder slot itself was 0. Patches 021+022, shim
edits; `re_out/fist.c`/`fist_decomp.c` untouched (pristine), 20 patches apply clean, overlay self-test
(233 fns) still passes, no regression (terminal crash = the SAME documented next blocker, FUN_1000_184b).

**The mechanism (fully reversed — the big find):** the engine wires its DGROUP indirect-call vectors from
an **in-image relocation table at seg 0x3352:0 (linear 0x33520)** — a sequence of sections, each a
`(dgroup_off, code_off)` list, applied by two appliers: **FUN_0000_f7ef** (near/offset) and
**FUN_0000_f842** (far off:seg; f842 == the DGROUP:0x12 service). f738 applies the boot sections at CRT
init (`si=0,0x14,0x38`); every OTHER subsystem applies its own section on init via `mov si,<off>;
call far [DGROUP:0x0a|0x12]`. The table decoder was **validated slot-for-slot against the oracle sample**
(`tools/oracle/samples/dgroup_0x0_0x100.bin`): service vectors 0x0a/0x0e/0x12/…/0x36, STRSEG 0x70, and
the **BUILDER at DGROUP:0x31a = 0xf69:0x560a = linear 0x14c9a = FUN_1000_4c9a** all match. Ghidra dropped
the x86 string-op segment bases in the C f7ef/f842 (DS=table-seg 0x3352, ES=DGROUP) **and** the SI arg of
the indirect `(*DAT_1000_c012)()` service calls, so the sections never landed → DGROUP:0x31a stayed 0 →
`call [ds:0x31a]` trapped to linear 0 → uninitialised filename (the "'i6' garbage" from the prompt).

- **Builder = FUN_1000_4c9a (patch 006):** copies the template from `[STRSEG(DGROUP:0x70=0x2d74)]:SI` into
  **DGROUP:0x740**, returns **DX=0x740** (asm `mov ax,ss:0x70; mov ds,ax; mov di,0x740; mov dx,di; rep
  lodsb/stosb`). Templates: `..\XGAVIDEO.DVR`@STRSEG:0x6fa, `..\SOUNDDVR.DVR`@STRSEG:0x6ea.
- **Patch 021 (5177/515e):** thread the builder's dropped **SI** (0x6fa video / 0x6ea sound) into the
  indirect call and reconstruct its constant **DX=0x740** return → FUN_1000_5051 gets the real filename
  pointer. Video 5177 then patches drive-letter byte `ds:0x743` (=0x740+3, the template 'X').
- **Patch 022 (4b16) + shim `fist_apply_reloc_section`:** install the stream/overlay vector section
  (si=0x274 far, incl. the builder 0x31a; + si=0x258 near) at exactly the engine's own call site
  (FUN_1000_4b16, just before the driver EXEC-load, AFTER the SOUND.CFG parse). **Timing is load-bearing**:
  pre-installing at load fires not-yet-initialised method vectors and corrupts the SOUND.CFG parse
  (FUN_1000_001f base-loss) — validated by trying it. The shim applies the game's OWN reloc data (base-0
  ⇒ identity); the engine's f7ef/f842 stay inert (documented). native_main's `fist_apply_reloc_section`
  applies one section with correct near/far semantics.
- **Shim `fist_load_overlay`:** honest **load_seg validation** (`< 0x3400 heap base ⇒ reject, no corruption`)
  so the leg-(b) gap (load_seg=0) fails loudly instead of loading the .DVR over the engine at linear 0.

**Proven live:** `3D open '..\SOUNDDVR.DVR' -> ok`; `4B03 load-overlay name@0x1c740='..\SOUNDDVR.DVR'`
(when load_seg was faked: `overlay 'SOUNDDVR.DVR' registered ... fmap=wired(44 fns)` → `4B03 loaded ... 21
relocs applied` → dispatch `overlay call -> SOUNDDVR.DVR+0x2`). So the entire real load path works; only
the load SEGMENT and (for video) the chipset drive-letter remain.

**BLOCKER #1 leg (b) — driver-struct load SEGMENT (the remaining gate), now precisely mapped:**
`FUN_1000_5051` reads `load_seg = struct[0]` (struct at DGROUP:0x3e4 video / 0x4fa sound) and passes it in
the ES:BX param block; it is 0. The struct's segment/load fields are populated by the driver-object
allocator **FUN_1000_184b** (DGROUP:0xd0 method), reached from 5051 via `call [ds:0xd0]`, and by the
DGROUP:0xd8 alloc method (via FUN_1000_5195, `struct[0..1]=*(struct+5)`). Both are in the reloc section
**si=0xec** (DGROUP:0x78..0xd0, installed by **FUN_1000_06bc** just before the loads). That section is NOT
installed yet because its method vectors are invoked through indirect `(*DAT_c0cc/c0d0)()` __allregs calls
that **still drop AX/BX/CX** — so 182a/184b run on garbage. 184b itself is a base-loss + MEMMGR-allocator
(0a31/1345/0d70/13e4) cascade: the struct is at **CX:BX = DGROUP:offset** (CX=DS=DGROUP always, verified),
BX the near offset — Ghidra derefs the bare offset (`*(param_4+1)|=0x22`, the crash). The current terminal
crash is 184b via the DIRECT `FUN_0000_14da → FUN_1000_182a` path (struct 0x4f0, a scratch buffer; asm
0x14da: `mov bx,0x4f0; mov cx,ds; call 182a`), i.e. **the exact NEXT BLOCKER documented previously** — no
regression. NEXT: reconstruct 184b/182a/0a31 (base-loss + arg-thread the 0xcc/0xd0/0xd8 method calls +
install section si=0xec at 06bc) → real load_seg → the .DVR actually loads at its alloc'd segment.
Separately, VIDEO needs `DAT_1000_c246` (video chipset id; only read, never written — a hardware-detection
subsystem) = 0x56 so the drive letter is 'M' ("..\MGAVIDEO.DVR"); currently 0 ⇒ name truncates to "..\".

## DONE (prior iteration) — MULTI-MODULE OVERLAY LOADING + DRIVER DECOMPILE + DISPATCH (video path)
The engine reaches **INT 21h AH=4B AL=03 (load-overlay)** twice to load its driver overlays
(**MGAVIDEO.DVR** video, then **SOUNDDVR.DVR** sound). This iteration built the entire multi-module
machinery — overlay-load shim, both driver decompiles, and the runtime dispatch — and integrated it
regression-free (engine `re_out/fist.c` **byte-identical**, `make check` green, 2249-fn fmap intact,
boot trace to AH=4B unchanged). The video path is now gated on the engine-side filename corruption
(blocker #1, characterized precisely below), not on missing overlay infrastructure.

- **(1) Driver image extraction** — `tools/extract_dat_image.py` parameterized (`[SRC [OUT [--relocs]]]`);
  `make image-drivers` emits `re_out/fist_mga_image.bin` (31388 B, 9 relocs, entry off 0) +
  `fist_snd_image.bin` (16700 B, 21 relocs) as base-0 flat MZ load images + `.relocs` sidecars. FIST.DAT
  output stays **byte-identical** (LOAD_PARA=0, no reloc file). The .DVR MZ reloc table is applied at
  LOAD time by the shim (reloc factor = engine-chosen load_seg), not baked.
- **(2) Driver decompiles** — `make decompile-drivers` runs the SAME Ghidra pipeline per driver
  (`FIST_PROJ_NAME`/`FIST_ENTRY=0x0`/`FIST_ENGINE_SEEDERS=0` gate the engine-only seeders
  SeedServiceVecs/SeedRuntimeVecs, added to `decompile.sh`; `GHIDRA_FIST_OUT` redirects the export).
  → `re_out/fist_mga_decomp.c` (**233 fns**) + `fist_snd_decomp.c` (**44 fns**). Finding: a driver runs
  with **DS = the ENGINE's DGROUP**, so ~93% of its data refs are ENGINE memory (DGROUP 0x1c000+,
  resources 0x2xxxx) + VGA 0xA0000 + INT reg-file 0xF0000 — all ABSOLUTE and already correct at base-0
  in the shared g_mem; only the driver's OWN low data (module-relative linear < module_size) + its near
  indirect-call CS need the runtime load base.
- **(3) Driver assemble (`assemble_fist.py` MODULE mode)** — `FIST_MODULE=<mod> FIST_MODULE_SIZE=<hex>`
  emits `re_out/fist_mga.c`/`fist_snd.c`: module-relative accessors (< MODULE_SIZE) add a per-module
  `fist_<mod>_base` global (set at load); absolute refs stay base-0; near indirect calls add the base;
  the FUN map (`fist_<mod>_fmap`) is keyed by **base-0 module offsets**. Every function-like identifier
  is `#define`-namespaced (`m_<mod>_…`) so the driver unit's `app_entry`/`FUN_0000_*`/stubs don't
  collide with the engine unit's identical names at link. Engine mode emits **byte-identical** output
  (every branch guarded; verified md5). New generic rules exposed by driver code: **RULE 10** faithful
  duplicate-label dedup (Ghidra splits a `repne scasb` into two blocks with one code_r0x… name — forward
  goto → body, backward goto → loop top), **RULE 8** extended to `_param_N` dead-scratch pseudo-vars,
  `longdouble`/`ROUND` compat additions. Both units compile clean (`gcc -m32`).
- **(4) Overlay-load shim — `fist_dos.c` INT 21h AH=4B AL=03** (`fist_load_overlay`): opens the real
  .DVR (`open_ci`), parses its MZ header, copies the load-module to `load_seg<<4`, applies each MZ
  relocation (`word += reloc_factor`), and registers the module. Honest CF=1 on missing file / bad MZ /
  load overflow. AL≠03 (EXEC-and-run) fails honestly. Rich `FIST_TRACE_TRAPS` dump of DS:DX/ES:BX/params.
- **(5) Multi-module dispatch — `fist_icall.c` + `fist_modules.c`** — `fist_ovl_register` records each
  loaded overlay `[base, base+size)` and wires its decompiled fmap + base global from `fist_ovl_known[]`
  (weak driver symbols → engine-only builds still link; loaded-but-unwired overlays trap honestly as
  `NAME+0xOFF`). `fist_icall` maps a resolved target inside an overlay range to `linear-base = module
  offset → module fmap → &FUN`. Build (`build_native.sh`) auto-links `fist_{mga,snd}.c` when present.
- **VALIDATED end-to-end** (`FIST_OVL_SELFTEST=1`, gated diagnostic in native_main): loads the REAL
  MGAVIDEO.DVR at seg 0x3400 → 9 relocs applied (factor 0x3400) → registered → **fmap WIRED (233 fns)**
  → `fist_icall(base+0)` and `(base+0x110)` resolve to **distinct real driver functions** (not traps).
  So load → reloc → register → wire → dispatch all work; the engine simply can't feed AH=4B a good
  filename yet (blocker #1).

**BLOCKER #1 — engine-side AH=4B inputs are corrupt (the exact video-path stopper).** Traced the call
chain: `FUN_0000_134e` (video-load orchestrator: chipset `DAT_1000_c246==0x56` → drive letter
`0x4d='M'` → the `..\XGAVIDEO.DVR` template's X→M ⇒ "MGAVIDEO.DVR") → `FUN_1000_5177` →
`FUN_1000_5051(…, extraout_DX, 0x3e4, …, 0x6fa, …)` → `FUN_1000_5014` (the AH=4B site). Two converging
losses put garbage into AH=4B (observed: filename ptr DS:DX=1c00:0808 = `'i6'`; `load_seg=reloc=0`):
  - **filename pointer = `extraout_DX` from `(*DAT_1000_c31a)()`** (the vtable filename-builder). The
    `__allregs` C model returns only AX, so the builder's **DX return (the built-name buffer pointer) is
    dropped** → uninitialized. This is the "multi-register return" residual class, here load-bearing.
    (`DAT_1000_c31a` is a DGROUP method slot filled at runtime, not statically captured.)
  - **load_seg = `*(word at DGROUP:0x3e4)`** was 0 — the driver-object struct's segment field (the
    AH=48-allocated segment) wasn't populated (uninitialized/near-ptr base-loss, the same DGROUP
    stream/handle-struct family as patches 020/184b).
  Fixing these (thread the builder's DX return; populate the driver-struct load segment) yields a correct
  "MGAVIDEO.DVR" + load_seg to AH=4B → the shim loads it → dispatch runs the video driver → first light.

## NEXT BLOCKERS (scoped)
0. **BLOCKER #1 leg (a) — DONE** (patches 021/022). **184b crash + both driver filenames + chipset — DONE
   this iteration** (patches 023/024 + chipset seed; see the DONE section above): both `..\SOUNDDVR.DVR`
   and `..\MGAVIDEO.DVR` are byte-perfect and OPEN. **leg (b) is now narrowed to a single subsystem: the
   MEMMGR free-list allocator FUN_1000_0a31** (+ its callees). Reached (past 184b) but FOLDED by Ghidra —
   a segment-linked circular free-list walker whose walking-ES/DS node fields collapsed into fixed DGROUP
   globals (`DAT_1000_c004/c00c/c00e`) → infinite spin; allocate/split path (0b1a/0b58/0b70) rendered as
   null-derefs. FAITHFUL FIX = full asm rewrite of 0a31 (node struct + walk reversed, see DONE section)
   **plus** callees **161d** (refill, taken first: free-count=0), **0bd1/0bef/0c21/0c7d/0c7f** (ES/DS
   node-walk search helpers, same fold), **0d70** (unlink), **1040/11040**. Then `struct[0]` = the
   allocated segment ⇒ real load_seg ⇒ both .DVRs load + dispatch. The driver-struct path additionally
   needs method vector `(*DAT_c0d0)` installed (reloc section **si=0xec** at FUN_1000_06bc) + its args
   threaded per-patch (TASK 1 falls back to per-patch; see DONE). This is the last gate to FIRST LIGHT.
1. **DGROUP near-pointer base-loss family — overlay/stream-init subsystem (in progress).** `FUN_1000_5051`
   fixed (**patch 020**: param_3=0x4fa is a DGROUP near ptr; `ds:[bx+2]`/`ds:[bx]` lost their base; also a
   mis-based `call dword cs:0x5951` far vector rendered `DAT_2000_1951`). The SAME class now crashes in
   **FUN_1000_184b** (param_4=0x4f0, `*(byte*)(param_4+1)|=0x22`) via `FUN_0000_14da → FUN_1000_182a`. 184b's
   callees are the documented **MEMMGR walker family** (0a31, 0d70, 1345, 13e4) — dynamic-DS/near-ptr heavy;
   expect a short cascade of the same reconstruction. They initialise the DGROUP stream/handle structs the
   engine feeds to INT 21h AH=4B.
2. **INT 21h AH=4B (AL=03 load-overlay) — DONE this iteration** (faithful MZ overlay loader + module
   registry + multi-module dispatch; see the DONE section above). Now gated only on blocker #1 feeding it
   a correct filename + load segment.
3. **Benign `TRAP … linear 0x00000`/`0x00002`** — null/near-null indirect calls (as-yet-unset SMC/callback
   slots); return 0 and execution proceeds. Confirm legitimately unset at this point.
4. **MEMMGR deep follow-ons (not yet on the path):** 0dc2's block-relocating allocator, 0d70 free-node
   unlink, 0bd1 walker — reconstruct as free/realloc is exercised.

## RUNTIME ORACLE (tools/oracle/, built + working) — ground truth corrections
QEMU+FreeDOS+gdb oracle boots FreeDOS → LOADGAME → Huffman extender → EXEC FIST.DAT. gdb-stub
breakpoints/mem-dumps/`find`/`screendump`/`pmemsave` all work; README + helper scripts in `tools/oracle/`.
Runtime linear bases (re-derive per run from engine strings — the base moves ±0x100 segs): extender
**0x10000000**, engine load seg **0x1419** (linear 0x14190), engine **DGROUP runtime seg 0x3019**.
(DOSBox still the only path that reaches the menu; the original hits an engine early-fault under QEMU —
an oracle-environment issue, instrumentable, not our port's concern yet.)

**KEY CORRECTION — DGROUP:0x12 is an ENGINE-INTERNAL service, not a saved vector / extender ABI.** At
runtime DGROUP holds an **11-entry service-dispatch table** (offsets 0x0a,0x0e,0x12,…,0x36; 4-byte far-ptr
off:seg stride) that the engine installs at load, all pointing into its own service segment (seg 0xf69).
**DGROUP:0x12 = seg:off 0xf69:0x1b2 → static-base-0 linear 0xf842 = FUN_0000_f842** (a guarded
word-copy/relocation routine: `or bx,bx; je bail; mov ds,bx; lods/movsw/stos; lret`). The indirect-call
dispatcher's linear→&FUN map resolves it directly (seg*16+off); NO saved-vector magic for this one.
**⇒ patches 002/003 are WRONG:** they neutralized FUN_0000_f7ef / FUN_0000_f842 as "redundant reloc-apply",
but the oracle proves f842 is LIVE (dispatched via DGROUP:0x12). Revert them and fix the Ghidra
mis-recovered `or bx,bx; je -3` boundary-crossing loop faithfully (not a blanket `return;`).
Saved-vector magic still applies to the GENUINE saved INT21/23/24 handlers (DAT_1000_2b50/2b54/2b58 from
AH=35 get-vector) — those the engine far-calls to chain to DOS.

## NEXT CORE MECHANISM — indirect calls through g_mem-stored code pointers ("the biggest class")
Evidence (engine decompile, `FUN_1000_223c` + refs):
- The engine hooks interrupts: **INT 21h AH=3521 get-vector → saves the original DOS INT 21h handler
  seg:off to `DAT_1000_2b50/2b52`** (linear 0x12b50), then AH=2521 set-vector to its own handler; same for
  INT 23/24. Later it **chains** by far-calling the saved handler. So our shim's **get-vector (AH=35) must
  return a synthetic, CALLABLE seg:off** (a magic value our far-call dispatcher recognizes → routes back to
  `fist_int_dispatch` as "chain to INT n"). Returning NULL (current) → `call far [ptr]` on NULL → segfault.
- Engine-internal indirect dispatch: `DAT_1000_c012` (DGROUP:0x12, linear 0x1c012) is written
  (`*(u16*)&DAT_1000_c012 = 9`, line 38217) and called (`(*DAT_1000_c012)()`, lines 34051/34071/37370).
  Many such stored-pointer dispatchers exist (the RecoverAll jump tables promoted the *targets* to
  functions, but a `call [mem]` at runtime needs the stored value mapped to a C function).

**Systematic fix (design):** an indirect-call dispatcher + a **linear-address → C-function table** emitted
by `tools/assemble_fist.py` (every `FUN_SEG_OFF` → its linear `(SEG<<4)+OFF` → &FUN). A far/near `call`
through a g_mem-stored code pointer computes the linear target and looks it up:
  - if it matches a known FUN_ linear address → call that C function;
  - if it matches a **saved-INT-vector magic** (what get-vector handed out) → `fist_int_dispatch(n, ...)`;
  - else → honest trap (log the unmapped pointer; candidate patch).
This resolves BOTH the saved-vector chaining and the engine's own indirect dispatch, uniformly. The
runtime oracle (QEMU+gdb, `tools/oracle/`) confirms the exact seg:off values the real extender/DOS hands
back, but the MECHANISM above is derivable statically and does not block on the oracle.

## Then
Video mode (INT 10h) + first resource open (INT 21h AH=3D against FISTDATA) → pixels to 0xA0000 → FIRST
LIGHT. Compare qualitatively to `ref/main_menu.png`. After crash-free boot: Stage 2 bit-verify vs the
oracle. Flag/CF per-site threading and dynamic-ES far pointers remain documented classes.

## Indirect-call dispatcher — SIZED (inventory of re_out/fist_decomp.c)
- **759** near `(*DAT_...)()` stored-pointer dispatch sites, through **137 distinct DAT_ pointers**.
- **6** far-call-through-memory `(*(code*)*(u16*)&DAT)()` idioms (explicit off:seg pair).
- **164** `(*(code*)0xf0100)()` = the INT dispatch (already solved by InstallIntFixup).
- **~2220** FUN_ functions = entries in the `linear (SEG<<4)+OFF → &FUN` map the dispatcher looks up.
- Confirmed seg:off-pair store idiom: `DAT_1000_c684 = 0x330; DAT_1000_c686 = 0xf69` → code ptr
  segment 0xf69, offset 0x330 → linear 0xf9c0 (an engine code address). NEAR indirect calls carry only
  the offset (implied segment = the caller's CS); FAR ones carry both (off in DAT_x, seg in DAT_x+2).

**Implementation plan:** `tools/assemble_fist.py` emits a sorted `{linear → &FUN}` table + a
`fist_icall(linear)` dispatcher (binary search). Rewrite `(*DAT_...)()`:
  - near: `fist_icall(cur_cs*16 + (u16)DAT)` — cur_cs = the calling function's segment (known per function
    from its FUN_SEG_OFF name);
  - far: `fist_icall(((u16)DAT_seg)*16 + (u16)DAT_off)`.
  Dispatcher: linear in FUN-map → call it; linear in the saved-INT-vector magic range → `fist_int_dispatch`;
  else honest trap (log unmapped ptr → candidate patch). This uniformly resolves the 759 sites + the
  saved-vector chaining. The runtime oracle confirms exact stored values but the mechanism is static.
