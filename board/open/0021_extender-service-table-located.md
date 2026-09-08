Type: discovery
Status: open
Parent: 0018
Title: every extender service handler is located in re_out/fist_image.bin

The Doug-Huffman extender's PM service dispatch table is at **file offset 0xcb3** in
`re_out/fist_image.bin`, with 4-byte entries indexed by `op/4` (the engine's selector at
DGROUP:0xea10). Verified by three independent anchors:

- `op 0x58 -> 0x1103`, a stub that calls **0x8030** -- the LOS handler decoded from its TCB field
  accesses (`0xde/0xe2/0xe6` minus `0xd2/0xd6/0xda`, the +-0x40000 gate, the `h<<24` vs `Z<<16`
  occlusion test, `-1`/`0` returns);
- `op 0x54 -> 0x11a6`, the terrain probe the shim already implements;
- `op 0x18 -> 0x10ca` MAP-LOAD, `op 0x24 -> 0x82c0` render/camera, `op 0x2c -> 0x8460` -- all
  services the shim already implements, all present and in the right slots.

```
 op 0x00 -> 0x0f77   op 0x04 -> 0x10c9   op 0x08 -> 0x10e0   op 0x0c -> 0x10eb
 op 0x10 -> 0x10f7   op 0x14 -> 0x1089   op 0x18 -> 0x10ca   op 0x1c -> 0x1109
 op 0x20 -> 0x0f8b   op 0x24 -> 0x82c0   op 0x28 -> 0x8470   op 0x2c -> 0x8460
 op 0x30 -> 0x11a5   op 0x34 -> 0x11f2   op 0x38 -> 0x11f3   op 0x3c -> 0x11f4
 op 0x40 -> 0xad1e   op 0x44 -> 0x10da   op 0x48 -> 0x1265   op 0x4c -> 0x123f
 op 0x50 -> 0x1266   op 0x54 -> 0x11a6   op 0x58 -> 0x1103   op 0x5c -> 0x10fd
 op 0x60 -> 0x10f1   op 0x64 -> 0x786a   op 0x68 -> 0x76fd   op 0x6c -> 0x77e2
 op 0x70 -> 0x11cb   op 0x74 -> 0x6f17   op 0x78 -> 0x11dd   op 0x7c -> 0x77a4
 op 0x80 -> 0x7762   op 0x84 -> 0x108b
```

This removes the standing premise that the extender's services are "absent" or "paged out". They are
all in the tree, at known addresses, and can be read instead of guessed. The shim currently implements
only ops 0x0c, 0x18, 0x24, 0x2c, 0x4c, 0x54, 0x58, 0x64, 0x70, 0x78, 0x80; every other op the engine
posts (0x04, 0x08, 0x10, 0x14, **0x1c**, 0x20, 0x28, 0x3c, 0x40, 0x44, 0x50, 0x5c, 0x60, 0x74, 0x7c)
currently returns 0.

## op 0x1c IS the per-unit ground clamp (board:0018's missing writer)

Found by oracle write-trace, not by reading: `FIST_MEMARM_BOOT=1 FIST_WATCHPHYS=<DGROUP+0xc079>` on the
first vehicle slot's `byte[obj+0x1d]` names its writer as `cs=002b eip=00001195 flatip=10001195` --
`cs=0x2b` is the extender's 32-bit flat CODE selector and the code segment is based at 0x10000000
(the dump's `ds=0033:10000000`), so the writer is at offset **0x1195** in the extender image. The
values written were 0x29..0x6a: terrain heights.

```
1109: mov 0xc99,%edi           ; the object list
110f: mov $0x20,%ecx           ; 32 entries
1114: push %ecx ; push %edi
1116: movzwl (%edi),%edi       ; object near offset (WORD)
1119: or %di,%di ; je next
111e: add 0xca1,%edi           ; + DGROUP base -> flat object pointer
1124: cmpw $0x3,(%edi) ; ja next   ; <-- ONLY object types 0..3
112b: mov 0x4(%edi),%ebx       ; X
...   call 0x7fa0              ; slope -> [edi+0x22], [edi+0x24]
1182: mov 0x4(%edi),%ebx ; 1185: mov 0x8(%edi),%edx
1188: shl $0xd,%edx ; 118b: shl $0xd,%ebx ; 118e: neg %edx   ; the fixed-10 tile scaling
1190: call 0x8480              ; terrain height lookup
1195: mov %al,0x1d(%edi)       ; byte[obj+0x1d] = terrain height
1199: add $0x2,%edi ; 119e: jne 0x1114
```

`cmpw $0x3` restricts it to types 0..3 -- **exactly the set the port gets wrong**. The census showed
types 0x1a/0x1b sitting exactly on the terrain (they use FUN_1000_adcd, which the port has and calls)
while types 00/01/02 were buried; types 0..3 are precisely the ones served by this op, and the port
never runs it.

That closes board:0018's chain: `op 0x1c` writes `byte[obj+0x1d]`, the type-00/01/02 step methods copy
it to the ground byte `[obj+0xd]` (asm `7c1d/87df/902c: mov 0x1d(%di),%al ; mov %al,0xd(%di)`), and the
object Z is `ground<<8`, which is the scale the op-0x58 LOS compares against.

NEXT: implement op 0x1c in the shim as a faithful transcription of 0x1109-0x119e -- the same way the
shim already transcribes op 0x54 and op 0x58 -- reading the object list at `[0xc99]`/`[0xca1]` and the
heightmap the op-0x54/0x58 handlers already use. Then re-measure the ground byte against the oracle
(type 00 should track 81 -> 64, not sit at 5) and delete board:0018's LOS stand-in, which at that point
has nothing left to mask.

## Implementation attempt: op 0x20 works, op 0x1c is blocked behind another latent defect

Transcribed both handlers into the shim and measured. Findings, all kept because they are the
prerequisites for the next attempt:

1. **op 0x20 must NOT carry the `g_fist_after_map` gate.** It is posted during MISSION INIT, right
   after the engine seeds the TCB (`t[0x26]=0x1c00` the DGROUP segment, `t[0x28]=0x6d3c` the object
   list, `t[0x2a]=0xdfbc` the display table). With the render services' after-map gate it never runs,
   and `[0xc99]`/`[0xca1]` stay ZERO -- measured: op 0x1c fired 19600 times and skipped every one for
   want of a list.
2. **op 0x1c is called constantly** once in mission (19600 times in 20000 ticks), so the engine really
   does expect this service every frame.
3. With op 0x20's gate corrected so the globals populate, the run SIGSEGVs -- **not in the new code**
   but in `FUN_0000_7da5`, reached `ad2f -> ad3b -> a19e -> 7da5`. That function is pristine:

   ```c
   void __allregs FUN_0000_7da5(undefined1 param_1,int param_2)
   { *(undefined1 *)(param_2 + 0x90) = param_1; *(undefined1 *)(param_2 + 0xd6) = 3; ... }
   ```

   `param_2` is stack residue (measured 32165) and the object arrives from `a19e`'s dispatch
   (patch 283: `mov bx,[di] ; shl bx,1 ; call [bx-0x69a2]`, DI live, one argument). asm 0x7da5 is
   `mov %al,0x90(%di) ; movb $3,0xd6(%di) ; call 70eb` -- it needs BOTH AL and DI, and only DI is
   passed. Exactly the class patch 540 fixed for a0c8, where AX was recovered from the word the
   dispatcher had just stored.

The shim transcription was REVERTED rather than shipped: it changes behaviour (a294 105 -> 66, and
AZER1's four kills went to zero) without yet fixing the ground byte, and in its corrected-gate form it
crashes. Shipping that would be worse than not having it.

ORDER OF WORK for the next attempt: fix `7da5` (and audit a19e's whole `-0x69a2` dispatch table the way
patch 542 audited the 9e2b table) FIRST, then re-apply op 0x20 + op 0x1c and verify the ground byte
against the oracle -- type 00 should track 81 -> 64 rather than sitting at 5.
