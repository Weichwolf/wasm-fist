import sys
# apply held patches 439/441/442/443/444/445 to build/fist.c + build/fist_mga.c
f="build/fist.c"; s=open(f).read()
# 439 077e
note=open("patches/held/439-077e-faithful-atan2.note").read(); body=note.split("Body:\n\n",1)[1]
st=s.index("int __allregs FUN_0000_077e(uint param_1,uint param_2,uint param_3,uint param_4)\n\n{")
en=s.index("/* ===== ((uint)(uintptr_t)&FUN_0000_0840)"); s=s[:st]+body+"\n\n"+s[en:]
# 441 op54 e1a6 publish
s=s.replace("""  DAT_2000_aa10 = 0x54;
  uVar1 = FUN_0000_e339();
  *(undefined1 *)(param_2 + 0x18) = (char)uVar1;""",
"""  DAT_2000_aa10 = 0x54;
  { extern unsigned short g_fist_op54_proj; g_fist_op54_proj = (unsigned short)((intptr_t)param_2 - (intptr_t)((unsigned char*)g_mem + 0x1c000)); }
  uVar1 = FUN_0000_e339();
  *(undefined1 *)(param_2 + 0x18) = (char)uVar1;""",1)
# 0578 arg fix
s=s.replace("""    g_fist_a18e_bx = (unsigned short)FUN_0000_077e(*(uint *)(param_5 + 8) - *(uint *)(param_4 + 8),param_2,
                (*(int *)(param_5 + 10) - *(int *)(param_4 + 10)) -
                (uint)(*(uint *)(param_5 + 8) < *(uint *)(param_4 + 8)),iVar2); }""",
"""    { unsigned tz=*(uint *)(param_5 + 8), sz=*(uint *)(param_4 + 8); unsigned bor=(tz<sz);
      short dzhi=(short)(*(unsigned short *)(param_5 + 0xa) - *(unsigned short *)(param_4 + 0xa) - bor);
      g_fist_a18e_bx = (unsigned short)FUN_0000_077e(tz - sz, (uint)iVar2 >> 16, (uint)(int)dzhi, (uint)iVar2); } }""",1)
# 442 77cf
n=open("patches/held/442-77cf-b767-spawn-baseloss.note").read()
c77=n.split("--- FUN_1000_77cf ---\n",1)[1].split("\n--- FUN_0000_b767 ---\n")[0]
i1=s.index("void __allregs\nFUN_1000_77cf"); j1=s.index("\n}\n",i1)+3; s=s[:i1]+c77+s[j1:]
# 442 b767 (body anchor)
s=s.replace('''void __allregs FUN_0000_b767(undefined2 param_1,int param_2,int param_3,undefined2 param_4)

{
  *(undefined1 *)(param_2 + 0x2a) = 10;
  *(undefined2 *)(param_2 + 0x1b) = 0x355;
  FUN_1000_ace0(param_1,*(undefined2 *)(param_3 + 0x97),param_2,param_3,param_4);
  return;
}''','''void __allregs FUN_0000_b767(undefined2 param_1,int param_2,int param_3,undefined2 param_4)
{
  unsigned char *dg = (unsigned char *)g_mem + 0x1c000;
  dg[(unsigned short)(param_2 + 0x2a)] = 10;
  *(unsigned short *)(dg + (unsigned short)(param_2 + 0x1b)) = 0x355;
  FUN_1000_ace0(param_1,*(unsigned short *)(dg + (unsigned short)(param_3 + 0x97)),param_2,param_3,param_4);
  return;
}''',1)
# 443 b60f
nb=open("patches/held/443-b60f-b808-baseloss-splash-callers.note").read()
# 443 note doesn't have the C body; reconstruct b60f inline (same as applied before)
i=s.index("void __allregs\nFUN_0000_b60f(undefined2 param_1,undefined2 param_2,undefined4 param_3,int param_4,int *param_5)")
j=s.index("/* ===== ((uint)(uintptr_t)&FUN_0000_b6c9)",i)
b60f=open("/tmp/b60f_body.txt").read()
s=s[:i]+b60f+s[j:]
open(f,"w").write(s)
# 444/445 mga
g="build/fist_mga.c"; ms=open(g).read()
n4=open("patches/held/444-2b1e-fb-segment-wrap.note").read(); b4=n4.split("Body:\n\n",1)[1]
i=ms.index("void __allregs FUN_0000_2b1e(int *param_1,byte *param_2)"); j=ms.index("/* ===== ((uint)(uintptr_t)&FUN_0000_2baa)",i)
ms=ms[:i]+b4.rstrip()+"\n\n\n"+ms[j:]
n5=open("patches/held/445-26de-fb-segment-wrap.note").read(); b5=n5.split("Body:\n\n",1)[1]
i=ms.index("undefined2 __allregs FUN_0000_26de(uint param_1,int param_2,byte *param_3,int param_4)\n\n{"); j=ms.index("/* ===== ((uint)(uintptr_t)&FUN_0000_2758)",i)
ms=ms[:i]+b5.rstrip()+"\n\n\n"+ms[j:]
open(g,"w").write(ms)
print("chain applied: 077e",("PATCH 439" in s),"77cf",("PATCH 442: weapon" in s),"b60f",("PATCH 443" in s),"2b1e",("PATCH 444" in ms),"26de",("PATCH 445" in ms))
