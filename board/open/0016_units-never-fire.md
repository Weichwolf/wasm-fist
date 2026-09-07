Type: bug
Area: sim
Tags: combat victory-condition board0012

Units engage: an AI unit that sees an enemy in range fires at it, so missions reach a resolved
victory/defeat state instead of running forever.

## The blocker, measured

AZER1 run to t=20000 (post-537, emulation removed), shim outcome counters:

    [outcome] a294=150 a296=11  loaded=1 min_a296=11   (mission loaded, NOT resolved)
    [op58]    LOS calls=17795  out-of-range=8591  occluded=2523  VISIBLE=6681
    [range]   min cross-unit |dx|+|dy| = 158670        (threshold 0x40000 = 262144)
    [chain]   a286-request=0   7e29-dispatch=0   7745-spawn=0
    [spawn]   b1df-total=0
    [7e29]    entered=0        gate-open=0

So the sim is alive and the perception layer WORKS -- 17795 line-of-sight queries, 6681 of them
returning VISIBLE, and the closest cross-side pair is well inside the engagement threshold.  But the
fire chain never starts: not one fire request, not one projectile, not one kill.  `a296` (live object
count) never moves off 11.

**That is why no mission reaches a resolved victory/defeat.**  It is not the win/lose evaluator --
FUN_0000_a5dc is correct (victory when word[0x978e] hits 0 with word[0x9790] != 0; defeat when the
friendly count word[0x6d38] hits 0).  Nothing ever dies, so neither condition can be reached.

## Where it stops

FUN_0000_a286 is the fire trigger ("mov BYTE [di+0x92],0x30", patch 427) and has exactly ONE caller,
the aim gate at 0xafa2..0xb008:

    afa2: bx = word[0x9796] ; cmpw [bx],3        ; je ret
    afab: si = word[di+0x97]                     ; the TARGET
    afaf: or si,si ; je ret                      ; <-- no target -> never fires
    afb3: testb [di+0x16],8 ; jne afc0
    afb9: testb [0x978c],3 ; jne ret
    afc0: cmpw [di+0x99],0xc8 ; jbe afde
    afc8: testw [di+0x40],0x80 ; jne afde
    afcf: bx = word[0x9796] ; bx = word[bx]
    afd5: al = byte[0x978c] ; cmp al,[bx-0x66ae] ; jae ret
    afde: testb [si+0x16],0x10 ; je afee
    afe4: cmpw [di],1 ; je 8711     afe9: cmpw [di],3 ; je 96c0
    afee: ax = word[di+0x8b] - word[di+0x89]     ; aim error = target bearing - current aim
    aff6: cmp ax,0xb6   ; jb  a286               ; FIRE if |error| < 182
    affb: cmp ax,0xff4a ; jb  ret
    b000: call a286

Six independent gates precede the shot.  The next step is to instrument them and find WHICH one
rejects -- most likely candidates, in order: word[di+0x97] (no target acquired), the [0x978c] mask, or
an aim error that never converges inside +/-0xb6.

## Do NOT

Do not "fix" this by forcing a target or widening the aim window.  Every gate above is a faithful
transcription of the asm; if the port never satisfies one, the defect is in what FEEDS it, and the
answer is upstream exactly as it was for the 9200 group (patch 537) and the palette overrun.

## Notes cleared while finding this

  * board:0001's "the port stays in render phase d549=0x1e, the original reaches 0x1c" is OUTDATED:
    FUN_1000_a84c ("movb $0x1c,[0x1549]", patch 302) is reached 41322 times in AZER1 to t=9000, and
    patch 308 already fixed the signed-byte read of d548 that the note blamed.
  * a gdb `watch -l` on g_mem+0x1c549 reported ZERO writes across that same run while the byte
    demonstrably changes -- the watchpoint was silently ineffective.  Do not trust a negative
    watchpoint result on g_mem without a positive control.
