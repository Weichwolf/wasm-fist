
/* ===== app_entry @ 0000:0000 ===== */

void __allregs app_entry(void)

{
  return;
}


/* ===== thunk_FUN_0000_0009 @ 0000:0002 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs thunk_FUN_0000_0009(void)

{
  (*_DAT_1000_c036)();
  (*_DAT_1000_c036)();
  (*_DAT_1000_c036)();
  DAT_1000_c714 = DAT_1000_c714 | 0x60;
  (*_DAT_1000_c036)();
  DAT_1000_c718 = DAT_1000_c718 | 0x60;
  (*_DAT_1000_c036)();
  (*_DAT_1000_c0d4)();
  (*_DAT_1000_c0cc)();
  DAT_1000_c784 = DAT_1000_c784 | 0x60;
  (*_DAT_1000_c012)();
  return;
}


/* ===== thunk_FUN_0000_0092 @ 0000:0005 ===== */

void __allregs thunk_FUN_0000_0092(void)

{
  return;
}


/* ===== FUN_0000_0009 @ 0000:0009 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0009(void)

{
  (*_DAT_1000_c036)();
  (*_DAT_1000_c036)();
  (*_DAT_1000_c036)();
  DAT_1000_c714 = DAT_1000_c714 | 0x60;
  (*_DAT_1000_c036)();
  DAT_1000_c718 = DAT_1000_c718 | 0x60;
  (*_DAT_1000_c036)();
  (*_DAT_1000_c0d4)();
  (*_DAT_1000_c0cc)();
  DAT_1000_c784 = DAT_1000_c784 | 0x60;
  (*_DAT_1000_c012)();
  return;
}


/* ===== FUN_0000_0092 @ 0000:0092 ===== */

void __allregs FUN_0000_0092(void)

{
  return;
}


/* ===== FUN_0000_0093 @ 0000:0093 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_0093(void)

{
  undefined2 uVar1;
  undefined1 in_CF;
  
  uVar1 = FUN_0000_00b3(0x2ba9);
  if (!(bool)in_CF) {
    uVar1 = (*_DAT_2000_c174)();
  }
  return uVar1;
}


/* ===== FUN_0000_00a3 @ 0000:00a3 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_00a3(void)

{
  undefined2 uVar1;
  undefined1 in_CF;
  
  uVar1 = FUN_0000_00b3(0x2ba9);
  if (!(bool)in_CF) {
    uVar1 = (*_DAT_2000_c186)();
  }
  return uVar1;
}


/* ===== FUN_0000_00b3 @ 0000:00b3 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_00b3(undefined2 param_1)

{
  undefined1 in_CF;
  
  (*_DAT_1000_c316)();
  (*_DAT_1000_c388)();
  if (!(bool)in_CF) {
    _DAT_1000_c6e4 = 2;
    (*thunk_FUN_0000_0009)();
  }
  return param_1;
}


/* ===== FUN_0000_00e8 @ 0000:00e8 ===== */

/* WARNING: Removing unreachable block (ram,0x00000124) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_00e8(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined2 extraout_DX;
  undefined2 uVar1;
  undefined2 unaff_ES;
  
  _DAT_1000_c722 = 0xa000;
  DAT_1000_c73c = 3;
  DAT_1000_c73d = 2;
  FUN_0000_0284();
  _DAT_1000_c726 = 0;
  _DAT_1000_c72c = 0;
  _DAT_1000_c72e = 0;
  _DAT_1000_c732 = 0x140;
  _DAT_1000_c730 = 200;
  uVar1 = 0x726;
  FUN_0000_024f(0x726,param_3);
  (*_DAT_1000_c538)();
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  uRam000f0014 = 0x10;
  uRam000f0000 = 0x13;
  uRam000f000e = 0x1c00;
  uRam000f0002 = uVar1;
  uRam000f0004 = param_1;
  uRam000f0006 = extraout_DX;
  uRam000f0008 = param_3;
  uRam000f000a = param_4;
  uRam000f000c = param_2;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  DAT_1000_c738 = DAT_1000_c738 + -1;
  (*_DAT_1000_c53c)();
  (*_DAT_1000_c426)();
  (*_DAT_1000_c012)();
  if ((_DAT_1000_c706 != 0x5981) && (0x30 < _DAT_1000_c242)) {
    _DAT_1000_c5e4 = 0xbe2;
  }
  (*_DAT_1000_c02e)();
  (*_DAT_1000_c578)();
  FUN_0000_2c92();
  return;
}


/* ===== FUN_0000_0166 @ 0000:0166 ===== */

/* WARNING: Removing unreachable block (ram,0x00000183) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0166(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  undefined2 extraout_DX;
  undefined2 uVar1;
  undefined2 unaff_ES;
  
  FUN_0000_2cc1();
  (*_DAT_1000_c57c)();
  uVar1 = _DAT_1000_c70e;
  (*_DAT_1000_c016)();
  (*_DAT_1000_c538)();
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  uRam000f0014 = 0x10;
  uRam000f0000 = 3;
  uRam000f0008 = 8;
  uRam000f000e = 0x1c00;
  uRam000f0002 = uVar1;
  uRam000f0004 = param_1;
  uRam000f0006 = extraout_DX;
  uRam000f000a = param_3;
  uRam000f000c = param_2;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  DAT_1000_c738 = DAT_1000_c738 + -1;
  (*_DAT_1000_c53c)();
  return;
}


/* ===== FUN_0000_018e @ 0000:018e ===== */

void __allregs FUN_0000_018e(void)

{
  return;
}


/* ===== FUN_0000_0197 @ 0000:0197 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_0197(undefined2 *param_1)

{
  char *pcVar1;
  char cVar2;
  undefined2 uVar3;
  char *pcVar4;
  char *pcVar5;
  
  uVar3 = _DAT_1000_c70a;
  if (param_1 != (undefined2 *)0xffff) {
    pcVar4 = (char *)*param_1;
    pcVar5 = (char *)0x740;
    do {
      pcVar1 = pcVar4;
      pcVar4 = pcVar4 + 1;
      cVar2 = *pcVar1;
      pcVar1 = pcVar5;
      pcVar5 = pcVar5 + 1;
      *pcVar1 = cVar2;
    } while (cVar2 != '\0');
    return 0x2ba9;
  }
  return 0x1c00;
}


/* ===== FUN_0000_01c5 @ 0000:01c5 ===== */

undefined2 __allregs FUN_0000_01c5(uint param_1,int param_2)

{
  if ((0x23 < param_1) && (param_1 < 0x38)) {
    *(char *)(param_2 + 7) = (char)(param_1 - 0x24 >> 1) + '0';
  }
  return 0x1c00;
}


/* ===== FUN_0000_0208 @ 0000:0208 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0208(void)

{
  bool bVar1;
  
  bVar1 = _DAT_1000_c708 < 0xe263;
  if (_DAT_1000_c708 == 0xe263) {
    FUN_0000_022d();
    if (!bVar1) {
      (*_DAT_1000_d65e)();
      (*_DAT_1000_c3c0)();
    }
  }
  return;
}


/* ===== FUN_0000_022d @ 0000:022d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_022d(void)

{
  (*_DAT_1000_c316)();
  (*_DAT_1000_c3bc)();
  return;
}


/* ===== FUN_0000_024f @ 0000:024f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_024f(int *param_1,undefined2 param_2)

{
  while (*param_1 == 0) {
    *param_1 = -0x6000;
    param_1[2] = 0x140;
  }
  LOCK();
  UNLOCK();
  _DAT_1000_c724 = param_1;
  FUN_0000_02c1(param_1[2],param_2);
  return;
}


/* ===== FUN_0000_026e @ 0000:026e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_026e(int *param_1,undefined2 param_2)

{
  while (*param_1 == 0) {
    *param_1 = -0x6000;
    param_1[0x12] = 0x140;
  }
  LOCK();
  UNLOCK();
  _DAT_1000_c736 = param_1;
  FUN_0000_02c1(param_1[0x12],param_2);
  return;
}


/* ===== FUN_0000_0284 @ 0000:0284 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0284(void)

{
  int *piVar1;
  undefined2 *puVar2;
  int iVar3;
  int iVar4;
  undefined2 *puVar5;
  int *piVar6;
  
  iVar4 = 0x7c8;
  iVar3 = 8;
  puVar2 = (undefined2 *)&DAT_1000_c798;
  do {
    puVar5 = puVar2;
    *puVar5 = puVar5 + 3;
    puVar5[1] = 0xffff;
    puVar5[2] = iVar4;
    iVar4 = iVar4 + 400;
    iVar3 = iVar3 + -1;
    puVar2 = puVar5 + 3;
  } while (iVar3 != 0);
  *puVar5 = 0;
  iVar3 = 0;
  iVar4 = 100;
  piVar6 = (int *)&DAT_1000_c7c8;
  do {
    piVar1 = piVar6 + 1;
    *piVar6 = iVar3;
    piVar6 = piVar6 + 2;
    *piVar1 = iVar3 + 0x140;
    iVar3 = iVar3 + 0x280;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  _DAT_1000_c79a = 0x140;
  return;
}


/* ===== FUN_0000_02c1 @ 0000:02c1 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_02c1(int param_1,undefined2 *param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined2 *puVar4;
  undefined2 *puVar5;
  int *piVar6;
  
  _DAT_1000_c796 = param_1;
  if (param_1 == 0x140) {
    _DAT_1000_c794 = 0x7c8;
    return;
  }
  puVar5 = (undefined2 *)&DAT_1000_c798;
  while (puVar4 = puVar5, puVar5 = (undefined2 *)*puVar4, puVar5 != (undefined2 *)0x0) {
    param_2 = puVar4;
    if (param_1 == puVar5[1]) goto LAB_0000_02e2;
  }
  puVar4[1] = param_1;
  piVar6 = (int *)puVar4[2];
  iVar3 = 100;
  iVar2 = 0;
  do {
    piVar1 = piVar6 + 1;
    *piVar6 = iVar2;
    piVar6 = piVar6 + 2;
    *piVar1 = iVar2 + param_1;
    iVar2 = iVar2 + param_1 + param_1;
    iVar3 = iVar3 + -1;
    puVar5 = puVar4;
  } while (iVar3 != 0);
LAB_0000_02e2:
  *param_2 = *puVar5;
  LOCK();
  UNLOCK();
  puVar4 = puVar5;
  *puVar5 = _DAT_1000_c798;
  _DAT_1000_c798 = puVar4;
  _DAT_1000_c794 = puVar5[2];
  return;
}


/* ===== FUN_0000_0310 @ 0000:0310 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0310(undefined2 param_1,undefined2 param_2,int param_3)

{
  *(byte *)(param_3 + 2) = *(byte *)(param_3 + 2) | 0x22;
  FUN_0000_0197(param_1);
  (*_DAT_1000_c388)();
  return;
}


/* ===== FUN_0000_0328 @ 0000:0328 ===== */

void __allregs
FUN_0000_0328(int param_1,undefined2 param_2,int param_3,undefined2 *param_4,undefined1 *param_5)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  undefined2 uVar3;
  int iVar4;
  undefined1 *puVar5;
  undefined2 unaff_ES;
  
  uVar3 = *param_4;
  puVar5 = (undefined1 *)(param_3 * 3);
  for (iVar4 = param_1 * 3; iVar4 != 0; iVar4 = iVar4 + -1) {
    puVar2 = puVar5;
    puVar5 = puVar5 + 1;
    puVar1 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = *puVar1;
  }
  return;
}


/* ===== FUN_0000_0340 @ 0000:0340 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0340(undefined2 param_1,undefined2 *param_2)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  
  uVar3 = *param_2;
  FUN_0000_2d3d();
  uVar4 = _DAT_2000_c212;
  DAT_2000_c216 = 0;
  puVar6 = (undefined2 *)0x0;
  puVar7 = (undefined2 *)0x0;
  for (iVar5 = 0x180; iVar5 != 0; iVar5 = iVar5 + -1) {
    puVar2 = puVar7;
    puVar7 = puVar7 + 1;
    puVar1 = puVar6;
    puVar6 = puVar6 + 1;
    *puVar2 = *puVar1;
  }
  (*(code *)*(undefined2 *)0x336)();
  FUN_0000_0410();
  _DAT_1000_c780 = 0x782;
  DAT_1000_c786 = 1;
  FUN_0000_2d52();
  return;
}


/* ===== FUN_0000_0388 @ 0000:0388 ===== */

undefined2 __allregs FUN_0000_0388(undefined2 param_1,undefined2 param_2,int param_3)

{
  byte *pbVar1;
  undefined2 uVar2;
  undefined2 unaff_CS;
  
  pbVar1 = (byte *)(param_3 + 2);
  *pbVar1 = *pbVar1 | 0x20;
  if ((char)*pbVar1 < '\0') {
    if ((*(byte *)(param_3 + 2) & 0x10) != 0) {
      FUN_0000_2c8b();
    }
    FUN_0000_04a3();
    FUN_0000_03bd(unaff_CS,param_2);
    uVar2 = FUN_0000_084e(param_2);
    return uVar2;
  }
  return param_1;
}


/* ===== FUN_0000_03bd @ 0000:03bd ===== */

void __allregs FUN_0000_03bd(undefined2 param_1,undefined2 *param_2)

{
  undefined2 uVar1;
  
  uVar1 = *param_2;
  FUN_0000_2d3d();
  (*(code *)*(undefined2 *)0x336)();
  FUN_0000_2d52();
  return;
}


/* ===== FUN_0000_03e0 @ 0000:03e0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_03e0(void)

{
  _DAT_1000_c78c = 0x100;
  (*_DAT_1000_c580)();
  DAT_1000_c786 = 1;
  return;
}


/* ===== FUN_0000_03f2 @ 0000:03f2 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_03f2(void)

{
  DAT_1000_c786 = 0;
  _DAT_1000_c78c = 0x100;
  return;
}


/* ===== FUN_0000_03fe @ 0000:03fe ===== */

void __allregs FUN_0000_03fe(void)

{
  FUN_0000_0410();
  DAT_2000_c216 = 1;
  return;
}


/* ===== FUN_0000_0410 @ 0000:0410 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0410(void)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  int iVar3;
  int iVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  undefined2 unaff_ES;
  
  iVar3 = _DAT_2000_c220;
  if ((0x100 - _DAT_2000_c21c != 0) && (_DAT_2000_c220 != 0)) {
    puVar6 = (undefined1 *)(_DAT_2000_c21c * 3);
    puVar5 = puVar6 + _DAT_2000_c21e;
    for (iVar4 = (0x100 - _DAT_2000_c21c) * 3; iVar4 != 0; iVar4 = iVar4 + -1) {
      puVar2 = puVar6;
      puVar6 = puVar6 + 1;
      puVar1 = puVar5;
      puVar5 = puVar5 + 1;
      *puVar2 = *puVar1;
    }
  }
  return;
}


/* ===== FUN_0000_043e @ 0000:043e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_043e(int param_1,undefined2 param_2,undefined2 *param_3)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  undefined2 *puVar5;
  
  uVar3 = _DAT_1000_c782;
  DAT_1000_c786 = 0;
  if (param_1 != 0) {
    puVar5 = (undefined2 *)0x0;
    for (iVar4 = 0x180; iVar4 != 0; iVar4 = iVar4 + -1) {
      puVar2 = puVar5;
      puVar5 = puVar5 + 1;
      puVar1 = param_3;
      param_3 = param_3 + 1;
      *puVar2 = *puVar1;
    }
    _DAT_1000_c780 = 0x782;
    return;
  }
  FUN_0000_0a02(param_2);
  _DAT_1000_c780 = 0x782;
  return;
}


/* ===== FUN_0000_0467 @ 0000:0467 ===== */

void __allregs FUN_0000_0467(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  FUN_0000_043e(param_1,param_2,param_3);
  DAT_1000_c786 = 1;
  return;
}


/* ===== FUN_0000_0471 @ 0000:0471 ===== */

void __allregs FUN_0000_0471(void)

{
  DAT_2000_c216 = 1;
  return;
}


/* ===== FUN_0000_0478 @ 0000:0478 ===== */

void __allregs FUN_0000_0478(void)

{
  FUN_0000_0483(0);
  return;
}


/* ===== FUN_0000_0483 @ 0000:0483 ===== */

void __allregs FUN_0000_0483(undefined1 *param_1)

{
  undefined1 *puVar1;
  undefined1 uVar2;
  int iVar3;
  undefined2 unaff_ES;
  
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  out(0x3c7,0);
  iVar3 = 0x300;
  do {
    uVar2 = in(0x3c9);
    puVar1 = param_1;
    param_1 = param_1 + 1;
    *puVar1 = uVar2;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  DAT_1000_c738 = DAT_1000_c738 + -1;
  return;
}


/* ===== FUN_0000_04a3 @ 0000:04a3 ===== */

/* WARNING: Removing unreachable block (ram,0x000004cc) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_04a3(void)

{
  undefined2 *puVar1;
  int iVar2;
  undefined2 *puVar3;
  
  puVar3 = (undefined2 *)0x0;
  for (iVar2 = 0x180; iVar2 != 0; iVar2 = iVar2 + -1) {
    puVar1 = puVar3;
    puVar3 = puVar3 + 1;
    *puVar1 = 0;
  }
  do {
  } while( true );
}


/* ===== FUN_0000_04cd @ 0000:04cd ===== */

void __allregs FUN_0000_04cd(undefined2 param_1)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_04d8(param_1);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_04d8 @ 0000:04d8 ===== */

void __allregs FUN_0000_04d8(undefined2 param_1)

{
  FUN_0000_04f1(param_1,0xff,0xffff,0);
  return;
}


/* ===== FUN_0000_04e6 @ 0000:04e6 ===== */

void __allregs
FUN_0000_04e6(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_04f1(param_1,param_2,param_3,param_4);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_04f1 @ 0000:04f1 ===== */

/* WARNING: Removing unreachable block (ram,0x0000053c) */
/* WARNING: Removing unreachable block (ram,0x00000575) */
/* WARNING: Removing unreachable block (ram,0x0000057b) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_04f1(int param_1,uint param_2,undefined2 param_3,uint param_4)

{
  undefined2 *puVar1;
  undefined2 uVar2;
  uint uVar3;
  uint uVar4;
  undefined2 *puVar5;
  bool bVar6;
  
  DAT_1000_c786 = 0;
  _DAT_1000_c5e8 = 0;
  DAT_0000_0632 = 0;
  DAT_0000_0634 = (undefined2)(0x10000 / (ulong)(param_1 + (uint)(param_1 == 0)));
  bVar6 = param_2 < param_4;
  DAT_0000_0638 = (param_2 - param_4) + 1;
  DAT_0000_0636 = param_4;
  DAT_0000_063a = param_3;
  (*_DAT_1000_c0e4)(0);
  uVar2 = _DAT_1000_c782;
  if (!bVar6) {
    do {
    } while( true );
  }
  puVar5 = (undefined2 *)(DAT_0000_0636 * 3);
  uVar4 = DAT_0000_0638 * 3;
  for (uVar3 = uVar4 >> 1; uVar3 != 0; uVar3 = uVar3 - 1) {
    puVar1 = puVar5;
    puVar5 = puVar5 + 1;
    *puVar1 = 0;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar1 = puVar5;
    puVar5 = (undefined2 *)((int)puVar5 + 1);
    *(undefined1 *)puVar1 = 0;
  }
  _DAT_1000_c780 = 0x782;
  DAT_1000_c786 = 1;
  return;
}


/* ===== FUN_0000_05b8 @ 0000:05b8 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_05b8(void)

{
  byte *pbVar1;
  char *pcVar2;
  byte *pbVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  char cVar6;
  int iVar7;
  uint uVar8;
  byte *pbVar9;
  char *pcVar10;
  byte *pbVar11;
  bool bVar12;
  char local_42 [66];
  
  uVar5 = _DAT_1000_c788;
  uVar4 = _DAT_1000_c782;
  DAT_0000_0632 = -(uint)!CARRY2(DAT_0000_0634,DAT_0000_0632) & DAT_0000_0634 + DAT_0000_0632;
  cVar6 = '\0';
  uVar8 = 0;
  pcVar10 = local_42;
  iVar7 = 0x40;
  do {
    pcVar2 = pcVar10;
    pcVar10 = pcVar10 + 1;
    *pcVar2 = cVar6;
    bVar12 = CARRY2(uVar8,-DAT_0000_0632);
    uVar8 = uVar8 + -DAT_0000_0632;
    cVar6 = cVar6 + bVar12;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  iVar7 = DAT_0000_0638 - (int)DAT_0000_0636;
  pbVar9 = DAT_0000_0636;
  pbVar11 = DAT_0000_0636;
  do {
    pbVar1 = pbVar9;
    pbVar9 = pbVar9 + 1;
    pbVar3 = pbVar11;
    pbVar11 = pbVar11 + 1;
    *pbVar3 = *(byte *)ZEXT24(local_42 + *pbVar1);
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  DAT_1000_c786 = 1;
  if ((DAT_0000_0632 != 0) && (DAT_0000_063a = DAT_0000_063a + -1, DAT_0000_063a != 0)) {
    return;
  }
  _DAT_1000_c5e8 = 0;
  DAT_1000_c78a = DAT_1000_c78a | 0x22;
  return;
}


/* ===== FUN_0000_063c @ 0000:063c ===== */

void __allregs FUN_0000_063c(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_0647(param_1,param_2,param_3);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_0647 @ 0000:0647 ===== */

void __allregs FUN_0000_0647(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  FUN_0000_0660(param_1,0xff,0xffff,0,param_2,param_3);
  return;
}


/* ===== FUN_0000_0655 @ 0000:0655 ===== */

void __allregs
FUN_0000_0655(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5,undefined2 param_6)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_0660(param_1,param_2,param_3,param_4,param_5,param_6);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_0660 @ 0000:0660 ===== */

/* WARNING: Removing unreachable block (ram,0x000006d5) */
/* WARNING: Removing unreachable block (ram,0x000006ef) */
/* WARNING: Removing unreachable block (ram,0x000006f5) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_0660(int param_1,int param_2,undefined2 param_3,int param_4,int param_5,int param_6)

{
  undefined2 *puVar1;
  undefined2 uVar2;
  uint uVar3;
  uint uVar4;
  undefined2 *puVar5;
  bool bVar6;
  
  DAT_1000_c786 = 0;
  _DAT_1000_c5e8 = 0;
  DAT_0000_084a = (param_2 - param_4) + 1;
  DAT_0000_083c = 0;
  DAT_0000_083e = (undefined2)(0x10000 / (ulong)(param_1 + (uint)(param_1 == 0)));
  DAT_0000_0840 = 0;
  DAT_0000_0842 = (undefined2)(0x10000 / (ulong)(param_5 + (uint)(param_5 == 0)));
  bVar6 = false;
  DAT_0000_0844 = 0;
  DAT_0000_0846 = (undefined2)(0x10000 / (ulong)(param_6 + (uint)(param_6 == 0)));
  DAT_0000_0848 = param_4;
  DAT_0000_084c = param_3;
  (*_DAT_1000_c0e4)(0);
  uVar2 = _DAT_1000_c782;
  if (!bVar6) {
    do {
    } while( true );
  }
  puVar5 = (undefined2 *)(DAT_0000_0848 * 3);
  uVar4 = DAT_0000_084a * 3;
  for (uVar3 = uVar4 >> 1; uVar3 != 0; uVar3 = uVar3 - 1) {
    puVar1 = puVar5;
    puVar5 = puVar5 + 1;
    *puVar1 = 0;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar1 = puVar5;
    puVar5 = (undefined2 *)((int)puVar5 + 1);
    *(undefined1 *)puVar1 = 0;
  }
  _DAT_1000_c780 = 0x782;
  DAT_1000_c786 = 1;
  return;
}


/* ===== FUN_0000_0732 @ 0000:0732 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0732(void)

{
  byte *pbVar1;
  char *pcVar2;
  byte *pbVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  char cVar6;
  int iVar7;
  uint uVar8;
  byte *pbVar9;
  char *pcVar10;
  byte *pbVar11;
  bool bVar12;
  char local_c2 [64];
  char local_82 [64];
  char local_42 [66];
  
  uVar5 = _DAT_1000_c788;
  uVar4 = _DAT_1000_c782;
  bVar12 = CARRY2(DAT_0000_083c,DAT_0000_083e);
  DAT_0000_083c = DAT_0000_083c + DAT_0000_083e & -(uint)!bVar12;
  DAT_0000_083e = DAT_0000_083e & -(uint)!bVar12;
  cVar6 = '\0';
  uVar8 = 0;
  pcVar10 = local_c2;
  iVar7 = 0x40;
  do {
    pcVar2 = pcVar10;
    pcVar10 = pcVar10 + 1;
    *pcVar2 = cVar6;
    bVar12 = CARRY2(uVar8,-DAT_0000_083c);
    uVar8 = uVar8 + -DAT_0000_083c;
    cVar6 = cVar6 + bVar12;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  bVar12 = CARRY2(DAT_0000_0840,DAT_0000_0842);
  DAT_0000_0840 = DAT_0000_0840 + DAT_0000_0842 & -(uint)!bVar12;
  DAT_0000_0842 = DAT_0000_0842 & -(uint)!bVar12;
  cVar6 = '\0';
  uVar8 = 0;
  pcVar10 = local_82;
  iVar7 = 0x40;
  do {
    pcVar2 = pcVar10;
    pcVar10 = pcVar10 + 1;
    *pcVar2 = cVar6;
    bVar12 = CARRY2(uVar8,-DAT_0000_0840);
    uVar8 = uVar8 + -DAT_0000_0840;
    cVar6 = cVar6 + bVar12;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  DAT_0000_0844 = DAT_0000_0840 + DAT_0000_0842 & -(uint)!CARRY2(DAT_0000_0840,DAT_0000_0842);
  DAT_0000_0846 = DAT_0000_0842 & -(uint)!CARRY2(DAT_0000_0840,DAT_0000_0842);
  cVar6 = '\0';
  uVar8 = 0;
  pcVar10 = local_42;
  iVar7 = 0x40;
  do {
    pcVar2 = pcVar10;
    pcVar10 = pcVar10 + 1;
    *pcVar2 = cVar6;
    bVar12 = CARRY2(uVar8,-DAT_0000_0844);
    uVar8 = uVar8 + -DAT_0000_0844;
    cVar6 = cVar6 + bVar12;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  pbVar9 = (byte *)(DAT_0000_0848 * 3);
  pbVar11 = pbVar9;
  cVar6 = (char)DAT_0000_084a;
  do {
    *pbVar11 = *(byte *)ZEXT24(local_c2 + *pbVar9);
    pbVar1 = pbVar9 + 2;
    pbVar3 = pbVar11 + 2;
    pbVar11[1] = *(byte *)ZEXT24(local_82 + pbVar9[1]);
    pbVar9 = pbVar9 + 3;
    pbVar11 = pbVar11 + 3;
    *pbVar3 = *(byte *)ZEXT24(local_42 + *pbVar1);
    cVar6 = cVar6 + -1;
  } while (cVar6 != '\0');
  DAT_1000_c786 = 1;
  if (((DAT_0000_083c != 0 || DAT_0000_0840 != 0) || DAT_0000_0844 != 0) &&
     (DAT_0000_084c = DAT_0000_084c + -1, DAT_0000_084c != 0)) {
    return;
  }
  _DAT_1000_c5e8 = 0;
  DAT_1000_c78a = DAT_1000_c78a | 0x22;
  return;
}


/* ===== FUN_0000_084e @ 0000:084e ===== */

void __allregs FUN_0000_084e(undefined2 param_1)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_0859(param_1);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_0859 @ 0000:0859 ===== */

void __allregs FUN_0000_0859(undefined2 param_1)

{
  FUN_0000_0874(param_1,0xff,0xffff,0);
  return;
}


/* ===== FUN_0000_0869 @ 0000:0869 ===== */

void __allregs
FUN_0000_0869(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined1 in_CF;
  undefined1 uVar1;
  
  FUN_0000_0874(param_1,param_2,param_3,param_4);
  do {
    uVar1 = in_CF;
    FUN_0000_09c6();
    in_CF = 1;
  } while ((bool)uVar1);
  return;
}


/* ===== FUN_0000_0874 @ 0000:0874 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0874(int param_1,int param_2,undefined2 param_3,int param_4)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  uint uVar6;
  uint uVar7;
  undefined2 *puVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  bool bVar10;
  
  uVar3 = _DAT_1000_c782;
  DAT_1000_c786 = 0;
  _DAT_1000_c5e8 = 0;
  DAT_0000_09bc = 0;
  DAT_0000_09be = (undefined2)(0x10000 / (ulong)(param_1 + (uint)(param_1 == 0)));
  puVar8 = (undefined2 *)(param_4 * 3);
  DAT_0000_09c2 = ((param_2 - param_4) + 1) * 3;
  uVar7 = DAT_0000_09c2 & 1;
  puVar9 = puVar8;
  DAT_0000_09c0 = puVar8;
  DAT_0000_09c4 = param_3;
  for (uVar6 = DAT_0000_09c2 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
    puVar2 = puVar9;
    puVar9 = puVar9 + 1;
    puVar1 = puVar8;
    puVar8 = puVar8 + 1;
    *puVar2 = *puVar1;
  }
  bVar10 = false;
  for (uVar7 = (uint)(uVar7 != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = puVar9;
    puVar9 = (undefined2 *)((int)puVar9 + 1);
    puVar1 = puVar8;
    puVar8 = (undefined2 *)((int)puVar8 + 1);
    *(undefined1 *)puVar2 = *(undefined1 *)puVar1;
  }
  (*_DAT_1000_c0e4)(0);
  uVar5 = _DAT_1000_c788;
  uVar3 = _DAT_1000_c782;
  if (!bVar10) {
    uVar7 = DAT_0000_09c2 & 1;
    puVar8 = DAT_0000_09c0;
    puVar9 = DAT_0000_09c0;
    for (uVar6 = DAT_0000_09c2 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      puVar1 = puVar8;
      puVar8 = puVar8 + 1;
      *puVar2 = *puVar1;
    }
    for (uVar7 = (uint)(uVar7 != 0); uVar4 = _DAT_1000_c782, uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      puVar1 = puVar8;
      puVar8 = (undefined2 *)((int)puVar8 + 1);
      *(undefined1 *)puVar2 = *(undefined1 *)puVar1;
    }
    uVar7 = DAT_0000_09c2 & 1;
    puVar8 = DAT_0000_09c0;
    for (uVar6 = DAT_0000_09c2 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
      puVar1 = puVar8;
      puVar8 = puVar8 + 1;
      *puVar1 = 0;
    }
    for (uVar7 = (uint)(uVar7 != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar1 = puVar8;
      puVar8 = (undefined2 *)((int)puVar8 + 1);
      *(undefined1 *)puVar1 = 0;
    }
    _DAT_1000_c780 = 0x782;
    DAT_1000_c786 = 1;
    do {
    } while (DAT_2000_c216 != '\0');
    _DAT_1000_c5e8 = 0x946;
    return;
  }
  _DAT_1000_c780 = 0x782;
  DAT_1000_c786 = 1;
  do {
  } while (DAT_2000_c216 != '\0');
  return;
}


/* ===== FUN_0000_0946 @ 0000:0946 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0946(void)

{
  byte *pbVar1;
  char *pcVar2;
  byte *pbVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  char cVar6;
  int iVar7;
  uint uVar8;
  byte *pbVar9;
  char *pcVar10;
  byte *pbVar11;
  bool bVar12;
  char local_42 [66];
  
  uVar5 = _DAT_1000_c788;
  uVar4 = _DAT_1000_c782;
  DAT_0000_09bc = -(uint)CARRY2(DAT_0000_09be,DAT_0000_09bc) | DAT_0000_09be + DAT_0000_09bc;
  cVar6 = '\0';
  uVar8 = 0;
  pcVar10 = local_42;
  iVar7 = 0x40;
  do {
    pcVar2 = pcVar10;
    pcVar10 = pcVar10 + 1;
    *pcVar2 = cVar6;
    bVar12 = CARRY2(uVar8,DAT_0000_09bc);
    uVar8 = uVar8 + DAT_0000_09bc;
    cVar6 = cVar6 + bVar12;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  iVar7 = DAT_0000_09c2 - (int)DAT_0000_09c0;
  pbVar9 = DAT_0000_09c0;
  pbVar11 = DAT_0000_09c0;
  do {
    pbVar1 = pbVar9;
    pbVar9 = pbVar9 + 1;
    pbVar3 = pbVar11;
    pbVar11 = pbVar11 + 1;
    *pbVar3 = *(byte *)ZEXT24(local_42 + *pbVar1);
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  DAT_1000_c786 = 1;
  if ((DAT_0000_09bc != 0xffff) && (DAT_0000_09c4 = DAT_0000_09c4 + -1, DAT_0000_09c4 != 0)) {
    return;
  }
  _DAT_1000_c5e8 = 0;
  DAT_1000_c78a = DAT_1000_c78a | 0x22;
  return;
}


/* ===== FUN_0000_09c6 @ 0000:09c6 ===== */

void __allregs FUN_0000_09c6(void)

{
  return;
}


/* ===== FUN_0000_09cd @ 0000:09cd ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_09cd(void)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  undefined1 in_CF;
  
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  FUN_0000_09c6();
  uVar4 = _DAT_1000_c788;
  uVar3 = _DAT_1000_c782;
  if ((bool)in_CF) {
    puVar6 = (undefined2 *)0x0;
    puVar7 = (undefined2 *)0x0;
    for (iVar5 = 0x180; iVar5 != 0; iVar5 = iVar5 + -1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      puVar1 = puVar6;
      puVar6 = puVar6 + 1;
      *puVar2 = *puVar1;
    }
    _DAT_1000_c5e8 = 0;
    _DAT_1000_c780 = 0x782;
    DAT_1000_c786 = 1;
  }
  DAT_1000_c738 = DAT_1000_c738 + -1;
  return;
}


/* ===== FUN_0000_0a02 @ 0000:0a02 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0a02(undefined2 param_1)

{
  undefined2 *puVar1;
  byte *pbVar2;
  byte *pbVar3;
  undefined2 uVar4;
  int iVar5;
  undefined1 uVar6;
  undefined2 uVar7;
  undefined1 uVar8;
  undefined2 *puVar9;
  byte *pbVar10;
  byte *pbVar11;
  
  uVar4 = _DAT_2000_c19a;
  uVar7 = _DAT_1000_c782;
  pbVar11 = (byte *)0x0;
  puVar9 = (undefined2 *)0x420;
  for (iVar5 = 0x18; iVar5 != 0; iVar5 = iVar5 + -1) {
    pbVar3 = pbVar11;
    pbVar11 = pbVar11 + 2;
    puVar1 = puVar9;
    puVar9 = puVar9 + 1;
    *(undefined2 *)pbVar3 = *puVar1;
  }
  pbVar10 = (byte *)0x420;
  iVar5 = 0x30;
  do {
    pbVar3 = pbVar10;
    pbVar10 = pbVar10 + 1;
    pbVar2 = pbVar11;
    pbVar11 = pbVar11 + 1;
    *pbVar2 = *pbVar3 >> 1;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  uVar7 = 2;
  uVar6 = 0;
  FUN_0000_0a50(0,2,pbVar11);
  uVar7 = CONCAT11((char)uVar7,(char)((uint)uVar7 >> 8));
  FUN_0000_0a50(uVar6,uVar7,pbVar11);
  uVar8 = (undefined1)((uint)uVar7 >> 8);
  uVar7 = CONCAT11(uVar6,(char)uVar7);
  FUN_0000_0a50(uVar8,uVar7,pbVar11);
  uVar6 = (undefined1)uVar7;
  uVar7 = CONCAT11(uVar8,uVar8);
  FUN_0000_0a50(uVar6,uVar7,pbVar11);
  uVar8 = (undefined1)((uint)uVar7 >> 8);
  uVar7 = CONCAT11(uVar6,(char)uVar7);
  FUN_0000_0a50(uVar8,uVar7,pbVar11);
  uVar7 = CONCAT11((char)uVar7,(char)((uint)uVar7 >> 8));
  FUN_0000_0a50(uVar8,uVar7,pbVar11);
  FUN_0000_0a50(uVar8,CONCAT11((char)((uint)uVar7 >> 8),uVar8),pbVar11);
  return;
}


/* ===== FUN_0000_0a50 @ 0000:0a50 ===== */

void __allregs FUN_0000_0a50(char param_1,int param_2,int *param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  char cVar4;
  undefined2 unaff_ES;
  
  iVar2 = 0;
  cVar4 = '\0';
  iVar3 = 0x20;
  do {
    piVar1 = param_3 + 1;
    *param_3 = iVar2;
    param_3 = (int *)((int)param_3 + 3);
    *(char *)piVar1 = cVar4;
    iVar2 = iVar2 + param_2;
    cVar4 = cVar4 + param_1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}


/* ===== FUN_0000_0a64 @ 0000:0a64 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0a64(undefined2 param_1,int param_2)

{
  byte *pbVar1;
  byte *pbVar2;
  byte *pbVar3;
  char *pcVar4;
  undefined2 uVar5;
  byte bVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  char *pcVar10;
  undefined2 unaff_ES;
  bool bVar11;
  
  bVar6 = (byte)((uint)param_1 >> 8);
  if (!CARRY1(bVar6,bVar6)) {
    pbVar8 = (byte *)((uint)*(byte *)(param_2 + 5) * 3);
    iVar7 = (uint)*(byte *)(param_2 + 6) * 3;
    pcVar10 = (char *)(param_2 + 10 + iVar7);
    *(undefined2 *)(param_2 + 8) = pcVar10;
    pbVar9 = (byte *)(param_2 + 10);
    uVar5 = *_DAT_2000_c210;
    do {
      pbVar3 = pbVar9;
      pbVar9 = pbVar9 + 1;
      pbVar1 = pbVar8;
      bVar11 = *pbVar3 < *pbVar1;
      pbVar2 = pbVar8;
      pbVar8 = pbVar8 + 1;
      pcVar4 = pcVar10;
      pcVar10 = pcVar10 + 1;
      *pcVar4 = *pbVar2 + ((byte)((uint)(byte)((*pbVar3 - *pbVar1 ^ -bVar11) + bVar11) *
                                  (uint)(byte)(bVar6 * '\x02') >> 8) ^ -bVar11) + bVar11;
      iVar7 = iVar7 + -1;
    } while (iVar7 != 0);
    return;
  }
  *(int *)(param_2 + 8) = param_2 + 10;
  return;
}


/* ===== FUN_0000_0abc @ 0000:0abc ===== */

void __allregs FUN_0000_0abc(undefined2 param_1,int param_2)

{
  byte *pbVar1;
  byte *pbVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte bVar7;
  byte *pbVar5;
  byte *pbVar6;
  int iVar8;
  byte *pbVar9;
  undefined2 unaff_ES;
  bool bVar10;
  
  bVar7 = (byte)((uint)param_1 >> 8);
  if (CARRY1(bVar7,bVar7)) {
    *(int *)(param_2 + 8) = param_2 + 10 + (uint)*(byte *)(param_2 + 6) * 3;
    return;
  }
  if ((byte)(bVar7 * '\x02') != 0) {
    iVar8 = (uint)*(byte *)(param_2 + 6) * 3;
    pbVar9 = (byte *)(param_2 + 10);
    pbVar5 = pbVar9 + iVar8;
    pbVar6 = pbVar5 + iVar8;
    *(undefined2 *)(param_2 + 8) = pbVar6;
    do {
      pbVar3 = pbVar5;
      pbVar5 = pbVar5 + 1;
      pbVar1 = pbVar9;
      bVar10 = *pbVar3 < *pbVar1;
      pbVar2 = pbVar9;
      pbVar9 = pbVar9 + 1;
      pbVar4 = pbVar6;
      pbVar6 = pbVar6 + 1;
      *pbVar4 = *pbVar2 + ((byte)((uint)(byte)((*pbVar3 - *pbVar1 ^ -bVar10) + bVar10) *
                                  (uint)(byte)(bVar7 * '\x02') >> 8) ^ -bVar10) + bVar10;
      iVar8 = iVar8 + -1;
    } while (iVar8 != 0);
    return;
  }
  *(int *)(param_2 + 8) = param_2 + 10;
  return;
}


/* ===== FUN_0000_0b1f @ 0000:0b1f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: ram */

void __allregs FUN_0000_0b1f(undefined2 param_1,undefined2 param_2)

{
  char *pcVar1;
  undefined1 *puVar2;
  undefined2 *puVar3;
  int *piVar4;
  undefined2 uVar5;
  byte bVar6;
  int iVar7;
  uint uVar8;
  int *piVar9;
  undefined1 *puVar10;
  int *piVar11;
  undefined2 *puVar12;
  bool bVar13;
  
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  if (DAT_1000_c738 == '\0') {
    bVar13 = (bool)(DAT_1000_c786 & 1);
    DAT_1000_c786 = DAT_1000_c786 >> 1;
    if (bVar13) {
      out(0x3c8,0);
      puVar10 = (undefined1 *)0x0;
      iVar7 = 0x300;
      do {
        puVar2 = puVar10;
        puVar10 = puVar10 + 1;
        out(0x3c9,*puVar2);
        iVar7 = iVar7 + -1;
      } while (iVar7 != 0);
    }
    if (DAT_1000_c2a8 == '\0' && DAT_1000_c787 == '\0') {
      uVar5 = *_DAT_1000_c3fc;
      for (piVar9 = (int *)0x0; *piVar9 != 0; piVar9 = (int *)((int)piVar9 + *piVar9)) {
        bVar6 = *(byte *)(piVar9 + 1);
        if ((int)((uint)bVar6 << 8) < 0) {
          pcVar1 = (char *)((int)piVar9 + 3);
          *pcVar1 = *pcVar1 + -1;
          if (*pcVar1 == '\0') {
            out(0x3c8,*(undefined1 *)((int)piVar9 + 5));
            bVar6 = bVar6 & 7;
            if (bVar6 != 0) {
              if (bVar6 < 2) {
                uVar8 = (uint)(byte)((char)piVar9[3] - *(char *)((int)piVar9 + 7));
                puVar12 = (undefined2 *)piVar9[4];
                do {
                  puVar3 = puVar12 + 1;
                  out(0x3c9,(char)*puVar12);
                  out(0x3c9,(char)((uint)*puVar12 >> 8));
                  puVar12 = (undefined2 *)((int)puVar12 + 3);
                  out(0x3c9,*(undefined1 *)puVar3);
                  uVar8 = uVar8 - 1;
                } while (uVar8 != 0);
                uVar8 = (uint)*(byte *)((int)piVar9 + 7);
                if (uVar8 != 0) {
                  piVar11 = piVar9 + 5;
                  do {
                    piVar4 = piVar11 + 1;
                    out(0x3c9,(char)*piVar11);
                    out(0x3c9,(char)((uint)*piVar11 >> 8));
                    piVar11 = (int *)((int)piVar11 + 3);
                    out(0x3c9,(char)*piVar4);
                    uVar8 = uVar8 - 1;
                  } while (uVar8 != 0);
                }
              }
              else if ((bVar6 == 2) || (bVar6 < 5)) {
                uVar8 = (uint)*(byte *)(piVar9 + 3);
                puVar12 = (undefined2 *)piVar9[4];
                do {
                  puVar3 = puVar12 + 1;
                  out(0x3c9,(char)*puVar12);
                  out(0x3c9,(char)((uint)*puVar12 >> 8));
                  puVar12 = (undefined2 *)((int)puVar12 + 3);
                  out(0x3c9,*(undefined1 *)puVar3);
                  uVar8 = uVar8 - 1;
                } while (uVar8 != 0);
              }
            }
          }
        }
      }
      FUN_0000_0c89();
    }
    (*_DAT_1000_c5e8)();
  }
  DAT_1000_c738 = DAT_1000_c738 + -1;
  return;
}


/* ===== FUN_0000_0be2 @ 0000:0be2 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: ram */

void __allregs FUN_0000_0be2(undefined2 param_1)

{
  char *pcVar1;
  undefined1 *puVar2;
  int *piVar3;
  undefined2 uVar4;
  byte bVar5;
  int iVar6;
  int *piVar7;
  undefined1 *puVar8;
  int *piVar9;
  bool bVar10;
  
  DAT_1000_c738 = DAT_1000_c738 + '\x01';
  bVar10 = (bool)(DAT_1000_c786 & 1);
  DAT_1000_c786 = DAT_1000_c786 >> 1;
  if (bVar10) {
    out(0x3c8,0);
    puVar8 = (undefined1 *)0x0;
    iVar6 = 0x300;
    while (iVar6 != 0) {
      iVar6 = iVar6 + -1;
      puVar2 = puVar8;
      puVar8 = puVar8 + 1;
      out(*puVar2,0x3c9);
    }
  }
  if (DAT_1000_c2a8 == '\0' && DAT_1000_c787 == '\0') {
    uVar4 = *_DAT_1000_c3fc;
    for (piVar7 = (int *)0x0; *piVar7 != 0; piVar7 = (int *)((int)piVar7 + *piVar7)) {
      bVar5 = *(byte *)(piVar7 + 1);
      if ((int)((uint)bVar5 << 8) < 0) {
        pcVar1 = (char *)((int)piVar7 + 3);
        *pcVar1 = *pcVar1 + -1;
        if (*pcVar1 == '\0') {
          out(0x3c8,*(undefined1 *)((int)piVar7 + 5));
          bVar5 = bVar5 & 7;
          if (bVar5 != 0) {
            if (bVar5 < 2) {
              puVar8 = (undefined1 *)piVar7[4];
              iVar6 = (uint)(byte)((char)piVar7[3] - *(char *)((int)piVar7 + 7)) * 3;
              while (iVar6 != 0) {
                iVar6 = iVar6 + -1;
                puVar2 = puVar8;
                puVar8 = puVar8 + 1;
                out(*puVar2,0x3c9);
              }
              if (*(byte *)((int)piVar7 + 7) != 0) {
                piVar9 = piVar7 + 5;
                iVar6 = (uint)*(byte *)((int)piVar7 + 7) * 3;
                while (iVar6 != 0) {
                  iVar6 = iVar6 + -1;
                  piVar3 = piVar9;
                  piVar9 = (int *)((int)piVar9 + 1);
                  out((char)*piVar3,0x3c9);
                }
              }
            }
            else if ((bVar5 == 2) || (bVar5 < 5)) {
              puVar8 = (undefined1 *)piVar7[4];
              iVar6 = (uint)*(byte *)(piVar7 + 3) * 3;
              while (iVar6 != 0) {
                iVar6 = iVar6 + -1;
                puVar2 = puVar8;
                puVar8 = puVar8 + 1;
                out(*puVar2,0x3c9);
              }
            }
          }
        }
      }
    }
    FUN_0000_0c89();
  }
  (*_DAT_1000_c5e8)();
  DAT_1000_c738 = DAT_1000_c738 + -1;
  return;
}


/* ===== FUN_0000_0bee @ 0000:0bee ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_0bee(undefined1 param_1,int param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5)

{
  char *pcVar1;
  undefined1 *puVar2;
  int *piVar3;
  undefined2 uVar4;
  byte bVar5;
  int iVar6;
  int *piVar7;
  undefined1 *puVar8;
  int *piVar9;
  
  out(param_2,param_1);
  puVar8 = (undefined1 *)0x0;
  iVar6 = 0x300;
  while (iVar6 != 0) {
    iVar6 = iVar6 + -1;
    puVar2 = puVar8;
    puVar8 = puVar8 + 1;
    out(*puVar2,param_2 + 1);
  }
  if (*(char *)&DAT_1000_c2a8 == '\0' && *(char *)&DAT_1000_c787 == '\0') {
    uVar4 = *(undefined2 *)*(undefined2 *)&DAT_1000_c3fc;
    for (piVar7 = (int *)0x0; *piVar7 != 0; piVar7 = (int *)((int)piVar7 + *piVar7)) {
      bVar5 = *(byte *)(piVar7 + 1);
      if ((int)((uint)bVar5 << 8) < 0) {
        pcVar1 = (char *)((int)piVar7 + 3);
        *pcVar1 = *pcVar1 + -1;
        if (*pcVar1 == '\0') {
          out(0x3c8,*(undefined1 *)((int)piVar7 + 5));
          bVar5 = bVar5 & 7;
          if (bVar5 != 0) {
            if (bVar5 < 2) {
              puVar8 = (undefined1 *)piVar7[4];
              iVar6 = (uint)(byte)((char)piVar7[3] - *(char *)((int)piVar7 + 7)) * 3;
              while (iVar6 != 0) {
                iVar6 = iVar6 + -1;
                puVar2 = puVar8;
                puVar8 = puVar8 + 1;
                out(*puVar2,0x3c9);
              }
              if (*(byte *)((int)piVar7 + 7) != 0) {
                piVar9 = piVar7 + 5;
                iVar6 = (uint)*(byte *)((int)piVar7 + 7) * 3;
                while (iVar6 != 0) {
                  iVar6 = iVar6 + -1;
                  piVar3 = piVar9;
                  piVar9 = (int *)((int)piVar9 + 1);
                  out((char)*piVar3,0x3c9);
                }
              }
            }
            else if ((bVar5 == 2) || (bVar5 < 5)) {
              puVar8 = (undefined1 *)piVar7[4];
              iVar6 = (uint)*(byte *)(piVar7 + 3) * 3;
              while (iVar6 != 0) {
                iVar6 = iVar6 + -1;
                puVar2 = puVar8;
                puVar8 = puVar8 + 1;
                out(*puVar2,0x3c9);
              }
            }
          }
        }
      }
    }
    FUN_0000_0c89();
  }
  (*(code *)*(undefined2 *)&DAT_1000_c5e8)();
  *(char *)&DAT_1000_c738 = *(char *)&DAT_1000_c738 + -1;
  return;
}


/* ===== FUN_0000_0c89 @ 0000:0c89 ===== */

void __allregs FUN_0000_0c89(void)

{
  char *pcVar1;
  byte *pbVar2;
  char cVar3;
  byte bVar4;
  int iVar5;
  byte bVar6;
  int *piVar7;
  uint uVar8;
  int *piVar9;
  
  piVar9 = (int *)0x0;
  do {
    if (*piVar9 == 0) {
      return;
    }
    bVar4 = *(byte *)(piVar9 + 1);
    if (((int)((uint)bVar4 << 8) < 0) && (*(char *)((int)piVar9 + 3) == '\0')) {
      *(undefined1 *)((int)piVar9 + 3) = (char)piVar9[2];
      uVar8 = CONCAT11(bVar4,bVar4) & 0xff07;
      if ((bVar4 & 7) != 0) {
        bVar6 = (byte)uVar8;
        if ((bVar4 & 7) < 2) {
          uVar8 = (uint)(byte)((char)piVar9[3] - *(char *)((int)piVar9 + 7)) * 3 + piVar9[4];
          if ((bVar4 & 0x40) == 0) {
            piVar9[4] = piVar9[4] + 3;
            iVar5 = *piVar9;
            if (((uint)((int)piVar9 + iVar5) <= uVar8) &&
               (*(char *)((int)piVar9 + 7) = *(char *)((int)piVar9 + 7) + '\x01',
               (uint)((int)piVar9 + iVar5) <= (uint)piVar9[4])) {
              *(undefined1 *)((int)piVar9 + 7) = 0;
              piVar9[4] = (int)(piVar9 + 5);
            }
          }
          else {
            iVar5 = piVar9[4];
            piVar9[4] = iVar5 + -3;
            if ((int *)(iVar5 + -3) < piVar9 + 5) {
              *(char *)((int)piVar9 + 7) = (char)piVar9[3] + -1;
              piVar9[4] = (int)piVar9 + *piVar9 + -3;
            }
            else if ((uint)((int)piVar9 + *piVar9) <= uVar8) {
              pcVar1 = (char *)((int)piVar9 + 7);
              cVar3 = *pcVar1;
              *pcVar1 = *pcVar1 + -1;
              if (cVar3 == '\0') {
                *(undefined1 *)((int)piVar9 + 7) = 0;
              }
            }
          }
        }
        else if (bVar6 == 2) {
          if ((bVar4 & 0x40) == 0) {
            bVar4 = *(byte *)(piVar9 + 3);
            piVar7 = (int *)(piVar9[4] + (uint)bVar4 * -3);
            piVar9[4] = (int)piVar7;
            if (piVar7 < piVar9 + 5) {
              piVar9[4] = (int)piVar9 + (uint)bVar4 * -3 + *piVar9;
              bVar4 = *(byte *)(piVar9 + 1);
joined_r0x00000d88:
              if ((((bVar4 & 0x20) != 0) || ((bVar4 & 0x10) == 0)) &&
                 (*(byte *)(piVar9 + 1) = bVar4 & 0x5f, (bVar4 & 8) != 0)) {
                iVar5 = *piVar9;
                pbVar2 = (byte *)((int)piVar9 + iVar5 + 2);
                *pbVar2 = *pbVar2 | 0x80;
                *(undefined1 *)((int)piVar9 + iVar5 + 3) = (char)piVar9[2];
              }
            }
          }
          else {
            uVar8 = (uint)*(byte *)(piVar9 + 3) * 3 + piVar9[4];
            piVar9[4] = uVar8;
            if ((uint)((int)piVar9 + *piVar9) <= uVar8) {
              piVar9[4] = (int)(piVar9 + 5);
              bVar4 = *(byte *)(piVar9 + 1);
              goto joined_r0x00000d88;
            }
          }
        }
        else if (bVar6 < 5) {
          *(byte *)(piVar9 + 1) = (byte)(uVar8 >> 8) & 0x7f;
        }
      }
    }
    piVar9 = (int *)((int)piVar9 + *piVar9);
  } while( true );
}


/* ===== FUN_0000_0db4 @ 0000:0db4 ===== */

void __allregs FUN_0000_0db4(void)

{
  FUN_0000_0db8();
  return;
}


/* ===== FUN_0000_0db8 @ 0000:0db8 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0db8(void)

{
  char cVar1;
  undefined2 uVar2;
  byte bVar3;
  int iVar4;
  byte bVar5;
  char cVar6;
  byte bVar7;
  char *pcVar8;
  byte *pbVar9;
  byte *pbVar10;
  byte *pbVar11;
  
  bVar5 = DAT_1000_d5c2;
  iVar4 = _DAT_1000_d5be;
  bVar3 = DAT_1000_d5bc;
  uVar2 = _DAT_1000_c716;
  pbVar9 = (byte *)0xaa4;
  pcVar8 = _DAT_1000_d5c6;
  pbVar10 = _DAT_1000_d5c4;
  pbVar11 = _DAT_1000_d5c8;
  cVar6 = DAT_1000_d5c0;
  do {
    *pbVar9 = *pbVar11;
    cVar1 = *pcVar8;
    bVar7 = bVar3;
    while( true ) {
      while (pbVar9 = pbVar9 + 1, cVar1 != '\0') {
        if ((byte)(bVar7 - 1) == 0) goto LAB_0000_0e1c;
        *pbVar9 = pbVar11[1];
        cVar1 = pcVar8[1];
        pcVar8 = pcVar8 + 1;
        pbVar10 = pbVar10 + 1;
        pbVar11 = pbVar11 + 1;
        bVar7 = bVar7 - 1;
      }
      *pbVar11 = *pbVar10 & bVar5;
      bVar7 = bVar7 - 1;
      if (bVar7 == 0) break;
      *pbVar9 = pbVar11[1];
      cVar1 = pcVar8[1];
      pcVar8 = pcVar8 + 1;
      pbVar10 = pbVar10 + 1;
      pbVar11 = pbVar11 + 1;
    }
LAB_0000_0e1c:
    pbVar10 = pbVar10 + 1 + iVar4;
    pcVar8 = pcVar8 + 1 + iVar4;
    pbVar11 = pbVar11 + 1 + (0x140 - (uint)bVar3);
    cVar6 = cVar6 + -1;
    if (cVar6 == '\0') {
      DAT_1000_d5b5 = DAT_1000_d5b4;
      return;
    }
  } while( true );
}


/* ===== FUN_0000_0e37 @ 0000:0e37 ===== */

void __allregs FUN_0000_0e37(void)

{
  FUN_0000_0e3b();
  return;
}


/* ===== FUN_0000_0e3b @ 0000:0e3b ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0e3b(void)

{
  undefined1 *puVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  
  iVar3 = _DAT_1000_d5bc;
  uVar2 = _DAT_1000_c716;
  puVar7 = (undefined1 *)0xaa4;
  iVar6 = 0x140 - _DAT_1000_d5bc;
  iVar4 = iVar3;
  puVar8 = _DAT_1000_d5c8;
  iVar5 = _DAT_1000_d5c0;
  do {
    for (; iVar4 != 0; iVar4 = iVar4 + -1) {
      puVar1 = puVar7;
      puVar7 = puVar7 + 1;
      *puVar8 = *puVar1;
      puVar8 = puVar8 + 1;
    }
    puVar8 = puVar8 + iVar6;
    iVar5 = iVar5 + -1;
    iVar4 = iVar3;
  } while (iVar5 != 0);
  return;
}


/* ===== FUN_0000_0e63 @ 0000:0e63 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0e63(void)

{
  char cVar1;
  byte bVar2;
  undefined2 uVar3;
  byte bVar4;
  int iVar5;
  byte bVar6;
  char cVar7;
  byte bVar8;
  char *pcVar9;
  byte *pbVar10;
  byte *pbVar11;
  byte *pbVar12;
  
  bVar6 = DAT_1000_d5c2;
  iVar5 = _DAT_1000_d5be;
  bVar4 = DAT_1000_d5bc;
  uVar3 = _DAT_1000_c716;
  pbVar10 = (byte *)0xaa4;
  pcVar9 = _DAT_1000_d5c6;
  pbVar11 = _DAT_1000_d5c4;
  pbVar12 = _DAT_1000_d5c8;
  cVar7 = DAT_1000_d5c0;
  do {
    cVar1 = *pcVar9;
    bVar8 = bVar4;
    while( true ) {
      while (cVar1 != '\0') {
        *pbVar10 = *pbVar12;
        if ((byte)(bVar8 - 1) == 0) goto LAB_0000_0ec7;
        cVar1 = pcVar9[1];
        pcVar9 = pcVar9 + 1;
        pbVar10 = pbVar10 + 1;
        pbVar11 = pbVar11 + 1;
        pbVar12 = pbVar12 + 1;
        bVar8 = bVar8 - 1;
      }
      bVar2 = *pbVar12;
      if ((bVar2 != bVar6) && (bVar2 != 0)) {
        *pbVar10 = bVar2;
      }
      *pbVar12 = *pbVar11 & bVar6;
      bVar8 = bVar8 - 1;
      if (bVar8 == 0) break;
      cVar1 = pcVar9[1];
      pcVar9 = pcVar9 + 1;
      pbVar10 = pbVar10 + 1;
      pbVar11 = pbVar11 + 1;
      pbVar12 = pbVar12 + 1;
    }
LAB_0000_0ec7:
    pbVar10 = pbVar10 + 1;
    pbVar11 = pbVar11 + 1 + iVar5;
    pcVar9 = pcVar9 + 1 + iVar5;
    pbVar12 = pbVar12 + 1 + (0x140 - (uint)bVar4);
    cVar7 = cVar7 + -1;
    if (cVar7 == '\0') {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_0edc @ 0000:0edc ===== */

void __allregs FUN_0000_0edc(undefined1 param_1,int *param_2)

{
  FUN_0000_2dbd(param_2);
  *(undefined1 *)(*(int *)(*param_2 * 2 + 0x7c8) + param_2[1]) = param_1;
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_0efa @ 0000:0efa ===== */

undefined2 __allregs FUN_0000_0efa(int *param_1)

{
  undefined1 uVar1;
  undefined1 extraout_AH;
  
  FUN_0000_2dbd(param_1);
  uVar1 = *(undefined1 *)(*(int *)(*param_1 * 2 + 0x7c8) + param_1[1]);
  FUN_0000_2e12();
  return CONCAT11(extraout_AH,uVar1);
}


/* ===== FUN_0000_0f1a @ 0000:0f1a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0f1a(void)

{
  (*_DAT_1000_c0cc)();
  return;
}


/* ===== FUN_0000_0f3a @ 0000:0f3a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0f3a(undefined2 param_1,undefined2 param_2)

{
  if (*_DAT_1000_c724 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(param_2);
  }
  FUN_0000_0f62(param_1,param_2);
  return;
}


/* ===== FUN_0000_0f4c @ 0000:0f4c ===== */

void __allregs FUN_0000_0f4c(int param_1,int param_2,undefined2 *param_3,undefined2 param_4)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  int aiStack_1a [6];
  undefined1 *puStack_e;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  puStack_2 = param_3;
  cVar1 = '\x05';
  do {
    param_3 = param_3 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_3;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  puStack_e = (undefined1 *)&puStack_2;
  *(int *)((int)aiStack_1a + param_1) = *(int *)((int)aiStack_1a + param_1) + param_2;
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_0f60 @ 0000:0f60 ===== */

undefined2 __allregs FUN_0000_0f60(undefined2 param_1)

{
  return param_1;
}


/* ===== FUN_0000_0f62 @ 0000:0f62 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0f62(undefined2 *param_1,int *param_2)

{
  byte *pbVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  
  pbVar1 = (byte *)(param_1 + 1);
  *pbVar1 = *pbVar1 | 0x20;
  if ((char)*pbVar1 < '\0') {
    if ((*(byte *)(param_1 + 1) & 0x10) != 0) {
      FUN_0000_2c8b();
    }
  }
  else {
    FUN_0000_0f60(0xf7a);
  }
  uVar4 = *param_1;
  puVar11 = (undefined2 *)0x0;
  puVar10 = (undefined2 *)(*(int *)(*param_2 * 2 + _DAT_1000_c794) + param_2[1]);
  iVar8 = param_2[2] - *param_2;
  uVar6 = param_2[3] - param_2[1];
  uVar5 = *_DAT_1000_c724;
  iVar9 = _DAT_1000_c796 - uVar6;
  do {
    for (uVar7 = uVar6 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar3 = puVar11;
      puVar11 = puVar11 + 1;
      puVar2 = puVar10;
      puVar10 = puVar10 + 1;
      *puVar3 = *puVar2;
    }
    for (uVar7 = (uint)((uVar6 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar3 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      puVar2 = puVar10;
      puVar10 = (undefined2 *)((int)puVar10 + 1);
      *(undefined1 *)puVar3 = *(undefined1 *)puVar2;
    }
    puVar10 = (undefined2 *)((int)puVar10 + iVar9);
    iVar8 = iVar8 + -1;
  } while (iVar8 != 0);
  return;
}


/* ===== FUN_0000_0fbc @ 0000:0fbc ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0fbc(undefined2 param_1,undefined2 param_2)

{
  if (*_DAT_1000_c724 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(param_2);
  }
  FUN_0000_0fe4(param_1,param_2);
  return;
}


/* ===== FUN_0000_0fce @ 0000:0fce ===== */

void __allregs FUN_0000_0fce(int param_1,int param_2,undefined2 *param_3,undefined2 param_4)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  int aiStack_1a [6];
  undefined1 *puStack_e;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  puStack_2 = param_3;
  cVar1 = '\x05';
  do {
    param_3 = param_3 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_3;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  puStack_e = (undefined1 *)&puStack_2;
  *(int *)((int)aiStack_1a + param_1) = *(int *)((int)aiStack_1a + param_1) + param_2;
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_0fe2 @ 0000:0fe2 ===== */

undefined2 __allregs FUN_0000_0fe2(undefined2 param_1)

{
  return param_1;
}


/* ===== FUN_0000_0fe4 @ 0000:0fe4 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_0fe4(undefined2 *param_1,int *param_2)

{
  byte *pbVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  
  pbVar1 = (byte *)(param_1 + 1);
  *pbVar1 = *pbVar1 | 0x20;
  if ((char)*pbVar1 < '\0') {
    if ((*(byte *)(param_1 + 1) & 0x10) != 0) {
      FUN_0000_2c8b();
    }
  }
  else {
    FUN_0000_0fe2(0xffc);
  }
  uVar4 = *param_1;
  uVar5 = *_DAT_1000_c724;
  puVar11 = (undefined2 *)(*(int *)(*param_2 * 2 + _DAT_1000_c794) + param_2[1]);
  iVar8 = param_2[2] - *param_2;
  uVar6 = param_2[3] - param_2[1];
  iVar9 = _DAT_1000_c796 - uVar6;
  puVar10 = (undefined2 *)0x0;
  do {
    for (uVar7 = uVar6 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar3 = puVar11;
      puVar11 = puVar11 + 1;
      puVar2 = puVar10;
      puVar10 = puVar10 + 1;
      *puVar3 = *puVar2;
    }
    for (uVar7 = (uint)((uVar6 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar3 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      puVar2 = puVar10;
      puVar10 = (undefined2 *)((int)puVar10 + 1);
      *(undefined1 *)puVar3 = *(undefined1 *)puVar2;
    }
    puVar11 = (undefined2 *)((int)puVar11 + iVar9);
    iVar8 = iVar8 + -1;
  } while (iVar8 != 0);
  return;
}


/* ===== FUN_0000_103e @ 0000:103e ===== */

void __allregs FUN_0000_103e(int *param_1,int *param_2)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  
  FUN_0000_2d3d();
  puVar7 = (undefined2 *)(*(int *)(*param_1 * 2 + 0x7c8) + param_1[1]);
  iVar5 = param_2[2] - *param_2;
  uVar3 = param_2[3] - param_2[1];
  puVar6 = (undefined2 *)(*(int *)(*param_2 * 2 + 0x7c8) + param_2[1]);
  do {
    for (uVar4 = uVar3 >> 1; uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      puVar1 = puVar6;
      puVar6 = puVar6 + 1;
      *puVar2 = *puVar1;
    }
    for (uVar4 = (uint)((uVar3 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = (undefined2 *)((int)puVar7 + 1);
      puVar1 = puVar6;
      puVar6 = (undefined2 *)((int)puVar6 + 1);
      *(undefined1 *)puVar2 = *(undefined1 *)puVar1;
    }
    puVar6 = (undefined2 *)((int)puVar6 + (0x140 - uVar3));
    puVar7 = (undefined2 *)((int)puVar7 + (0x140 - uVar3));
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  FUN_0000_2d52();
  return;
}


/* ===== FUN_0000_1091 @ 0000:1091 ===== */

void __allregs FUN_0000_1091(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_109d @ 0000:109d ===== */

void __allregs
FUN_0000_109d(undefined2 param_1,int param_2,undefined2 *param_3,int param_4,undefined2 *param_5,
             undefined1 param_6)

{
  undefined2 *puVar1;
  char cVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  undefined2 **ppuVar7;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar7 = &puStack_2;
  cVar2 = '\x1e';
  puStack_2 = param_3;
  do {
    param_3 = param_3 + -1;
    ppuVar7 = ppuVar7 + -1;
    *ppuVar7 = (undefined2 *)*param_3;
    cVar2 = cVar2 + -1;
  } while ('\0' < cVar2);
  piVar6 = (int *)CONCAT11((char)((uint)param_2 >> 8) + *(char *)(param_2 + param_4 + -0x6000),
                           (char)param_2);
  iVar5 = piVar6[2] - *piVar6;
  uVar3 = piVar6[3] - piVar6[1];
  do {
    for (uVar4 = uVar3 >> 1; uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar1 = param_5;
      param_5 = param_5 + 1;
      *puVar1 = CONCAT11(param_6,param_6);
    }
    for (uVar4 = (uint)((uVar3 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar1 = param_5;
      param_5 = (undefined2 *)((int)param_5 + 1);
      *(undefined1 *)puVar1 = param_6;
    }
    param_5 = (undefined2 *)((int)param_5 + (0x140 - uVar3));
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_10d1 @ 0000:10d1 ===== */

void __allregs FUN_0000_10d1(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_10dd @ 0000:10dd ===== */

void __allregs
FUN_0000_10dd(undefined2 param_1,int param_2,undefined2 *param_3,int param_4,char *param_5,
             undefined2 param_6)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined2 **ppuVar6;
  char *pcVar7;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar6 = &puStack_2;
  cVar1 = '\x1e';
  puStack_2 = param_3;
  do {
    param_3 = param_3 + -1;
    ppuVar6 = ppuVar6 + -1;
    *ppuVar6 = (undefined2 *)*param_3;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  piVar5 = (int *)CONCAT11((char)((uint)param_2 >> 8) + *(char *)(param_2 + param_4 + -0x6000),
                           (char)param_2);
  iVar4 = piVar5[2] - *piVar5;
  iVar2 = piVar5[3] - piVar5[1];
  iVar3 = iVar2;
  do {
    do {
      pcVar7 = param_5;
      if ((char)((uint)param_6 >> 8) != *pcVar7) {
        *pcVar7 = (char)param_6;
      }
      iVar3 = iVar3 + -1;
      param_5 = pcVar7 + 1;
    } while (iVar3 != 0);
    iVar4 = iVar4 + -1;
    iVar3 = iVar2;
    param_5 = pcVar7 + (0x141 - iVar2);
  } while (iVar4 != 0);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1114 @ 0000:1114 ===== */

void __allregs FUN_0000_1114(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_1120 @ 0000:1120 ===== */

void __allregs
FUN_0000_1120(undefined2 param_1,int param_2,undefined2 *param_3,int param_4,char *param_5,
             undefined2 param_6)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined2 **ppuVar6;
  char *pcVar7;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar6 = &puStack_2;
  cVar1 = '\x1e';
  puStack_2 = param_3;
  do {
    param_3 = param_3 + -1;
    ppuVar6 = ppuVar6 + -1;
    *ppuVar6 = (undefined2 *)*param_3;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  piVar5 = (int *)CONCAT11((char)((uint)param_2 >> 8) + *(char *)(param_2 + param_4 + -0x6000),
                           (char)param_2);
  iVar4 = piVar5[2] - *piVar5;
  iVar2 = piVar5[3] - piVar5[1];
  iVar3 = iVar2;
  do {
    do {
      pcVar7 = param_5;
      if ((char)((uint)param_6 >> 8) == *pcVar7) {
        *pcVar7 = (char)param_6;
      }
      iVar3 = iVar3 + -1;
      param_5 = pcVar7 + 1;
    } while (iVar3 != 0);
    iVar4 = iVar4 + -1;
    iVar3 = iVar2;
    param_5 = pcVar7 + (0x141 - iVar2);
  } while (iVar4 != 0);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1157 @ 0000:1157 ===== */

void __allregs FUN_0000_1157(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_1162 @ 0000:1162 ===== */

void __allregs
FUN_0000_1162(int param_1,int *param_2,undefined2 *param_3,uint *param_4,byte param_5)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined2 **ppuVar4;
  uint uVar5;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar4 = &puStack_2;
  cVar1 = '\x1e';
  puStack_2 = param_3;
  do {
    param_3 = param_3 + -1;
    ppuVar4 = ppuVar4 + -1;
    *ppuVar4 = (undefined2 *)*param_3;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  iVar3 = param_1 - *param_2;
  uVar5 = param_2[3] - param_2[1];
  do {
    uVar2 = uVar5 >> 1;
    if ((uVar5 & 1) != 0) {
      *(byte *)param_4 = (byte)*param_4 ^ param_5;
      param_4 = (uint *)((int)param_4 + 1);
    }
    for (; uVar2 != 0; uVar2 = uVar2 - 1) {
      *param_4 = *param_4 ^ CONCAT11(param_5,param_5);
      param_4 = param_4 + 1;
    }
    param_4 = (uint *)((int)param_4 + (0x140 - uVar5));
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_119f @ 0000:119f ===== */

void __allregs FUN_0000_119f(int *param_1)

{
  *param_1 = *param_1 + 1;
  param_1[1] = param_1[1] + 1;
  param_1[2] = param_1[2] + -1;
  param_1[3] = param_1[3] + -1;
  FUN_0000_11db(param_1);
  *param_1 = *param_1 + -1;
  param_1[1] = param_1[1] + -1;
  param_1[2] = param_1[2] + 1;
  param_1[3] = param_1[3] + 1;
  return;
}


/* ===== FUN_0000_11ba @ 0000:11ba ===== */

void __allregs FUN_0000_11ba(int *param_1)

{
  *param_1 = *param_1 + 2;
  param_1[1] = param_1[1] + 2;
  param_1[2] = param_1[2] + -2;
  param_1[3] = param_1[3] + -2;
  FUN_0000_11db(param_1);
  *param_1 = *param_1 + -2;
  param_1[1] = param_1[1] + -2;
  param_1[2] = param_1[2] + 2;
  param_1[3] = param_1[3] + 2;
  return;
}


/* ===== FUN_0000_11db @ 0000:11db ===== */

void __allregs FUN_0000_11db(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_1290 @ 0000:1290 ===== */

void __allregs FUN_0000_1290(undefined2 param_1)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_1);
}


/* ===== FUN_0000_1335 @ 0000:1335 ===== */

void __allregs FUN_0000_1335(int *param_1)

{
  *param_1 = *param_1 + 1;
  param_1[1] = param_1[1] + 1;
  param_1[2] = param_1[2] + -1;
  param_1[3] = param_1[3] + -1;
  FUN_0000_1290(param_1);
  *param_1 = *param_1 + -1;
  param_1[1] = param_1[1] + -1;
  param_1[2] = param_1[2] + 1;
  param_1[3] = param_1[3] + 1;
  return;
}


/* ===== FUN_0000_1350 @ 0000:1350 ===== */

void __allregs FUN_0000_1350(int *param_1)

{
  *param_1 = *param_1 + 2;
  param_1[1] = param_1[1] + 2;
  param_1[2] = param_1[2] + -2;
  param_1[3] = param_1[3] + -2;
  FUN_0000_1290(param_1);
  *param_1 = *param_1 + -2;
  param_1[1] = param_1[1] + -2;
  param_1[2] = param_1[2] + 2;
  param_1[3] = param_1[3] + 2;
  return;
}


/* ===== FUN_0000_1376 @ 0000:1376 ===== */

void __allregs
FUN_0000_1376(uint param_1,uint param_2,int param_3,int *param_4,uint param_5,int *param_6,
             undefined1 *param_7)

{
  int iVar1;
  undefined1 *puVar2;
  uint uVar3;
  byte bVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  undefined1 *puVar8;
  int iVar9;
  byte bVar10;
  int iVar11;
  int iVar12;
  byte bVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  undefined2 unaff_ES;
  bool bVar16;
  int local_1a;
  
  iVar1 = *param_4;
  bVar4 = (byte)param_5;
  if (((char)(param_5 >> 8) == '\0') && (bVar4 != 0)) {
    iVar5 = *param_6 - param_6[2];
    if (iVar5 != 0) {
      local_1a = -iVar5;
      uVar7 = *param_6 -
              ((uint)((int)(CONCAT12((char)((uint)local_1a >> 8),iVar5 * -0x100) / (uint3)param_5) +
                     iVar5) >> 1);
      iVar5 = param_6[1] - param_6[3];
      if (iVar5 != 0) {
        for (iVar6 = param_6[1] -
                     ((uint)((int)(CONCAT12((char)((uint)-iVar5 >> 8),iVar5 * -0x100) /
                                  (uint3)param_5) + iVar5) >> 1); iVar6 < 0; iVar6 = iVar6 + param_1
            ) {
        }
        for (; (int)uVar7 < 0; uVar7 = uVar7 + param_2) {
        }
        puVar2 = (undefined1 *)((ulong)uVar7 * (ulong)param_1);
        puVar8 = (undefined1 *)puVar2;
        uVar3 = (uint)((ulong)puVar2 >> 0x11);
        bVar13 = 0xff;
        iVar9 = ((((uint)(((ulong)puVar2 & 0x10000) != 0) << 0xe | (uint)((uVar3 & 1) != 0) << 0xf)
                  >> 1 | (uint)((uVar3 & 2) != 0) << 0xf) >> 1 | (uint)((uVar3 & 4) != 0) << 0xf) +
                iVar1;
        do {
          while( true ) {
            while( true ) {
              if ((int)puVar8 < 0) {
                puVar8 = puVar8 + -0x8000;
                iVar9 = iVar9 + 0x800;
              }
              if (uVar7 < param_2) break;
              uVar7 = 0;
              puVar8 = (undefined1 *)0x0;
              iVar9 = iVar1;
            }
            bVar16 = CARRY1(bVar13,bVar4);
            bVar13 = bVar13 + bVar4;
            if (bVar16) break;
            uVar7 = uVar7 + 1;
            puVar8 = puVar8 + param_1;
          }
          bVar10 = 0xff;
          iVar12 = -iVar5;
          puVar14 = puVar8;
LAB_0000_146a:
          do {
            iVar11 = -(iVar6 - param_1);
            puVar15 = puVar14 + iVar6;
            do {
              while (puVar14 = puVar15, bVar16 = CARRY1(bVar10,bVar4), bVar10 = bVar10 + bVar4,
                    !bVar16) {
                iVar11 = iVar11 + -1;
                puVar15 = puVar14 + 1;
                if (iVar11 == 0) goto LAB_0000_147f;
              }
              puVar2 = param_7;
              param_7 = param_7 + 1;
              *puVar2 = *puVar14;
              iVar12 = iVar12 + -1;
              iVar11 = iVar11 + -1;
              puVar15 = puVar14 + 1;
            } while (iVar11 != 0 && iVar12 != 0);
            if (iVar12 == 0) goto LAB_0000_149b;
LAB_0000_147f:
            puVar14 = puVar14 + (1 - param_1);
            iVar11 = iVar6;
          } while (iVar6 == 0);
          do {
            while (puVar15 = puVar14, bVar16 = CARRY1(bVar10,bVar4), bVar10 = bVar10 + bVar4,
                  !bVar16) {
              iVar11 = iVar11 + -1;
              puVar14 = puVar15 + 1;
              if (iVar11 == 0) goto LAB_0000_1496;
            }
            puVar2 = param_7;
            param_7 = param_7 + 1;
            *puVar2 = *puVar15;
            iVar12 = iVar12 + -1;
            iVar11 = iVar11 + -1;
            puVar14 = puVar15 + 1;
          } while (iVar11 != 0 && iVar12 != 0);
          if (iVar12 != 0) {
LAB_0000_1496:
            puVar14 = puVar15 + (1 - iVar6);
            goto LAB_0000_146a;
          }
LAB_0000_149b:
          puVar8 = puVar8 + param_1;
          param_7 = param_7 + param_3 + iVar5;
          uVar7 = uVar7 + 1;
          local_1a = local_1a + -1;
          if (local_1a == 0) {
            return;
          }
        } while( true );
      }
    }
  }
  return;
}


/* ===== FUN_0000_14b2 @ 0000:14b2 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_14b2(undefined2 param_1)

{
  undefined2 uVar1;
  undefined1 in_CF;
  
  (*_DAT_1000_c316)();
  uVar1 = (*_DAT_1000_c388)();
  if ((bool)in_CF) {
    return uVar1;
  }
  _DAT_1000_c636 = 2;
  (*thunk_FUN_0000_0009)();
  if (!(bool)in_CF) {
                    /* WARNING: Could not recover jumptable at 0x000014e7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar1 = (*(code *)(ulong)_DAT_1000_c636)();
    return uVar1;
  }
  return param_1;
}


/* ===== FUN_0000_14fb @ 0000:14fb ===== */

uint __allregs FUN_0000_14fb(int param_1,int param_2,int *param_3,int *param_4,char *param_5)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  undefined1 uVar10;
  int iVar8;
  uint uVar9;
  int iVar11;
  uint uVar12;
  int iVar13;
  int iVar14;
  char *pcVar15;
  char *pcVar16;
  undefined2 unaff_ES;
  bool bVar17;
  int local_10;
  
  iVar11 = *param_4;
  local_10 = -(iVar11 - param_4[2]);
  if (local_10 != 0) {
    uVar12 = param_4[1];
    iVar4 = uVar12 - param_4[3];
    uVar5 = -iVar4;
    if (uVar5 != 0) {
      iVar6 = (param_1 - param_4[3]) + uVar12;
      iVar14 = *param_3;
      iVar7 = 0;
      pcVar15 = (char *)0x300;
joined_r0x00001540:
      if (iVar11 != 0) {
        for (; iVar13 = param_1, (int)pcVar15 < 0; pcVar15 = pcVar15 + -0x8000) {
          iVar14 = iVar14 + 0x800;
        }
        do {
          while( true ) {
            cVar1 = *pcVar15;
            uVar10 = (undefined1)((uint)iVar7 >> 8);
            iVar8 = CONCAT11(uVar10,cVar1);
            if (cVar1 < '\0') break;
            iVar7 = iVar8 + 1;
            pcVar15 = pcVar15 + iVar8 + 2;
            iVar13 = iVar13 - iVar7;
            if (iVar13 == 0) {
              iVar11 = iVar11 + -1;
              goto joined_r0x00001540;
            }
          }
          pcVar15 = pcVar15 + 2;
          iVar7 = CONCAT11(uVar10,-cVar1) + 1;
          iVar13 = iVar13 - iVar7;
        } while (iVar13 != 0);
        iVar11 = iVar11 + -1;
        goto joined_r0x00001540;
      }
LAB_0000_1582:
      for (; (int)pcVar15 < 0; pcVar15 = pcVar15 + -0x8000) {
        iVar14 = iVar14 + 0x800;
      }
      uVar9 = 0;
LAB_0000_158b:
      do {
        pcVar16 = pcVar15;
        cVar1 = *pcVar16;
        uVar10 = (undefined1)(uVar9 >> 8);
        iVar11 = CONCAT11(uVar10,cVar1);
        if (-1 < cVar1) goto LAB_0000_15ae;
        pcVar15 = pcVar16 + 2;
        uVar9 = CONCAT11(uVar10,-cVar1) + 1;
        bVar17 = uVar9 <= uVar12;
        uVar12 = uVar12 - uVar9;
      } while (bVar17 && uVar12 != 0);
      uVar12 = -uVar12;
      iVar7 = (uVar12 + iVar4 & -(uint)(uVar12 < uVar5)) + uVar5;
      iVar13 = uVar12 - iVar7;
      cVar1 = pcVar16[1];
      for (iVar11 = iVar7; iVar11 != 0; iVar11 = iVar11 + -1) {
        pcVar2 = param_5;
        param_5 = param_5 + 1;
        *pcVar2 = cVar1;
      }
      goto LAB_0000_15cb;
    }
  }
  return 0;
LAB_0000_15ae:
  uVar9 = iVar11 + 1;
  bVar17 = uVar12 < uVar9;
  uVar12 = uVar12 - uVar9;
  pcVar15 = pcVar16 + iVar11 + 2;
  if (bVar17 || uVar12 == 0) goto code_r0x000015b5;
  goto LAB_0000_158b;
code_r0x000015b5:
  uVar9 = -uVar12;
  pcVar15 = pcVar16 + iVar11 + 2 + uVar12;
  iVar7 = (uVar9 + iVar4 & -(uint)(uVar9 < uVar5)) + uVar5;
  iVar13 = uVar9 - iVar7;
  for (iVar11 = iVar7; iVar11 != 0; iVar11 = iVar11 + -1) {
    pcVar3 = param_5;
    param_5 = param_5 + 1;
    pcVar2 = pcVar15;
    pcVar15 = pcVar15 + 1;
    *pcVar3 = *pcVar2;
  }
  pcVar15 = pcVar15 + iVar13;
LAB_0000_15cb:
  uVar12 = uVar5 - iVar7;
  uVar9 = 0;
  while (uVar12 != 0) {
    pcVar16 = pcVar15 + 1;
    cVar1 = *pcVar15;
    uVar10 = (undefined1)(uVar9 >> 8);
    if (cVar1 < '\0') {
      uVar9 = CONCAT11(uVar10,-cVar1) + 1;
      iVar11 = (uVar9 - uVar12 & -(uint)(uVar9 < uVar12)) + uVar12;
      iVar13 = uVar9 - iVar11;
      uVar12 = uVar12 - iVar11;
      pcVar15 = pcVar15 + 2;
      cVar1 = *pcVar16;
      uVar9 = CONCAT11((char)(uVar9 >> 8),cVar1);
      for (; iVar11 != 0; iVar11 = iVar11 + -1) {
        pcVar2 = param_5;
        param_5 = param_5 + 1;
        *pcVar2 = cVar1;
      }
    }
    else {
      uVar9 = CONCAT11(uVar10,cVar1) + 1;
      iVar11 = (uVar9 - uVar12 & -(uint)(uVar9 < uVar12)) + uVar12;
      iVar13 = uVar9 - iVar11;
      uVar12 = uVar12 - iVar11;
      for (; iVar11 != 0; iVar11 = iVar11 + -1) {
        pcVar3 = param_5;
        param_5 = param_5 + 1;
        pcVar2 = pcVar16;
        pcVar16 = pcVar16 + 1;
        *pcVar3 = *pcVar2;
      }
      pcVar15 = pcVar16 + iVar13;
    }
  }
  uVar12 = iVar6 - iVar13;
  param_5 = param_5 + param_2 + iVar4;
  local_10 = local_10 + -1;
  if (local_10 == 0) {
    return uVar9;
  }
  goto LAB_0000_1582;
}


/* ===== FUN_0000_161c @ 0000:161c ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_161c(undefined1 param_1,int *param_2,undefined2 param_3)

{
  int iVar1;
  int local_a;
  int local_8;
  int local_6;
  int local_4;
  undefined2 uStack_2;
  
  if (*_DAT_1000_c724 == -0x6000) {
    iVar1 = *param_2;
    local_6 = param_2[2];
    local_a = iVar1;
    if (local_6 < iVar1) {
      local_a = local_6;
      local_6 = iVar1;
    }
    iVar1 = param_2[1];
    local_4 = param_2[3];
    local_8 = iVar1;
    if (local_4 < iVar1) {
      local_8 = local_4;
      local_4 = iVar1;
    }
    uStack_2 = param_3;
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(&local_a);
  }
  uStack_2 = 0x1671;
  FUN_0000_1677(param_1,param_2,param_3);
  return;
}


/* ===== FUN_0000_1659 @ 0000:1659 ===== */

void __allregs
FUN_0000_1659(int param_1,undefined2 *param_2,undefined2 param_3,int param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *(int *)(&stack0x0056 + param_4) = *(int *)(&stack0x0056 + param_4) + param_1;
  FUN_0000_1677((char)param_3 + '\x01',param_1,param_5);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1677 @ 0000:1677 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1677(byte param_1,uint *param_2,undefined2 param_3)

{
  byte *pbVar1;
  uint *puVar2;
  undefined2 uVar3;
  byte bVar4;
  byte bVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  byte *pbVar14;
  uint *puVar15;
  char in_AF;
  bool bVar16;
  byte local_12;
  uint local_a;
  uint local_8;
  uint local_6;
  uint local_4;
  undefined2 uStack_2;
  
  uStack_2 = param_3;
  uVar3 = *_DAT_1000_c724;
  iVar13 = _DAT_1000_c724[2];
  local_a = *param_2;
  local_8 = param_2[1];
  local_6 = param_2[2];
  local_4 = param_2[3];
  local_12 = 0xff;
  do {
    uVar8 = _DAT_1000_c724[3];
    uVar12 = _DAT_1000_c724[5] - 1;
    uVar9 = local_a - uVar8;
    uVar11 = uVar12 - local_a;
    bVar4 = ((int)((uint)(byte)(((int)uVar9 < 0) << 7 | (uVar9 == 0) << 6 | in_AF << 4 |
                                ((POPCOUNT(uVar9 & 0xff) & 1U) == 0) << 2 | 2U | local_a < uVar8) <<
                  8) < 0) << 1 |
            (char)(((int)uVar11 < 0) << 7 | (uVar11 == 0) << 6 | in_AF << 4 |
                   ((POPCOUNT(uVar11 & 0xff) & 1U) == 0) << 2 | 2U | uVar12 < local_a) < '\0';
    uVar10 = (uint)bVar4;
    uVar9 = local_6 - uVar8;
    uVar11 = uVar12 - local_6;
    bVar5 = ((int)((uint)(byte)(((int)uVar9 < 0) << 7 | (uVar9 == 0) << 6 | in_AF << 4 |
                                ((POPCOUNT(uVar9 & 0xff) & 1U) == 0) << 2 | 2U | local_6 < uVar8) <<
                  8) < 0) << 1 |
            (char)(((int)uVar11 < 0) << 7 | (uVar11 == 0) << 6 | in_AF << 4 |
                   ((POPCOUNT(uVar11 & 0xff) & 1U) == 0) << 2 | 2U | uVar12 < local_6) < '\0';
    bVar16 = (bVar5 & bVar4) == 0;
    do {
      do {
        if (!bVar16) {
          return;
        }
        uVar8 = _DAT_1000_c724[4];
        uVar12 = _DAT_1000_c724[6] - 1;
        uVar9 = local_8 - uVar8;
        uVar11 = uVar12 - local_8;
        bVar4 = ((char)uVar10 << 1 |
                (char)(((int)uVar9 < 0) << 7 | (uVar9 == 0) << 6 | in_AF << 4 |
                       ((POPCOUNT(uVar9 & 0xff) & 1U) == 0) << 2 | 2U | local_8 < uVar8) < '\0') <<
                1 | (char)(((int)uVar11 < 0) << 7 | (uVar11 == 0) << 6 | in_AF << 4 |
                           ((POPCOUNT(uVar11 & 0xff) & 1U) == 0) << 2 | 2U | uVar12 < local_8) <
                    '\0';
        uVar10 = (uint)bVar4;
        uVar9 = local_4 - uVar8;
        uVar11 = uVar12 - local_4;
        bVar5 = (bVar5 << 1 |
                (char)(((int)uVar9 < 0) << 7 | (uVar9 == 0) << 6 | in_AF << 4 |
                       ((POPCOUNT(uVar9 & 0xff) & 1U) == 0) << 2 | 2U | local_4 < uVar8) < '\0') <<
                1 | (char)(((int)uVar11 < 0) << 7 | (uVar11 == 0) << 6 | in_AF << 4 |
                           ((POPCOUNT(uVar11 & 0xff) & 1U) == 0) << 2 | 2U | uVar12 < local_4) <
                    '\0';
        bVar16 = (bVar5 & bVar4) == 0;
      } while (!bVar16);
      uVar10 = CONCAT11(bVar5,bVar4);
      if (uVar10 == 0) {
        if (local_a == local_6) {
          puVar15 = (uint *)(*(int *)(local_a * 2 + _DAT_1000_c794) + local_8);
          if (DAT_0000_161b == '\0') {
            uVar8 = local_4 - local_8;
            if (uVar8 != 0) {
              if (local_4 < local_8) {
                puVar15 = (uint *)((int)puVar15 + uVar8);
                uVar8 = -uVar8;
              }
              for (uVar9 = uVar8 >> 1; uVar9 != 0; uVar9 = uVar9 - 1) {
                puVar2 = puVar15;
                puVar15 = puVar15 + 1;
                *puVar2 = CONCAT11(param_1,param_1);
              }
              for (uVar8 = (uint)((uVar8 & 1) != 0); uVar8 != 0; uVar8 = uVar8 - 1) {
                puVar2 = puVar15;
                puVar15 = (uint *)((int)puVar15 + 1);
                *(byte *)puVar2 = param_1;
              }
            }
            return;
          }
          uVar8 = local_4 - local_8;
          if (uVar8 != 0) {
            if (local_4 < local_8) {
              puVar15 = (uint *)((int)puVar15 + uVar8);
              uVar8 = -uVar8;
            }
            uVar9 = uVar8 >> 1;
            if ((uVar8 & 1) != 0) {
              *(byte *)puVar15 = (byte)*puVar15 ^ param_1;
              puVar15 = (uint *)((int)puVar15 + 1);
            }
            for (; uVar9 != 0; uVar9 = uVar9 - 1) {
              *puVar15 = *puVar15 ^ CONCAT11(param_1,param_1);
              puVar15 = puVar15 + 1;
            }
          }
          return;
        }
        uVar8 = local_4 - local_8;
        if (local_4 < local_8) {
          pbVar14 = (byte *)(*(int *)(local_6 * 2 + _DAT_1000_c794) + local_4);
          uVar8 = -uVar8;
          uVar9 = local_6 - local_a;
          if (local_a <= local_6) goto LAB_0000_180e;
        }
        else {
          if (uVar8 == 0) {
            uVar8 = local_a - local_6;
            if (uVar8 != 0) {
              if (local_a < local_6) {
                uVar8 = -uVar8;
                local_6 = local_a;
              }
              pbVar14 = (byte *)(*(int *)(local_6 * 2 + _DAT_1000_c794) + local_8);
              if (DAT_0000_161b != '\0') {
                uVar11 = uVar8 >> 1;
                uVar9 = uVar11 + 1;
                if ((uVar8 & 1) != 0) goto LAB_0000_1946;
                do {
                  *pbVar14 = *pbVar14 ^ param_1;
                  pbVar14 = pbVar14 + iVar13;
                  uVar9 = uVar11;
LAB_0000_1946:
                  *pbVar14 = *pbVar14 ^ param_1;
                  pbVar14 = pbVar14 + iVar13;
                  uVar11 = uVar9 - 1;
                } while (uVar11 != 0);
                return;
              }
              uVar11 = uVar8 >> 1;
              uVar9 = uVar11 + 1;
              if ((uVar8 & 1) != 0) goto LAB_0000_1927;
              do {
                *pbVar14 = param_1;
                pbVar14 = pbVar14 + iVar13;
                uVar9 = uVar11;
LAB_0000_1927:
                *pbVar14 = param_1;
                pbVar14 = pbVar14 + iVar13;
                uVar11 = uVar9 - 1;
              } while (uVar11 != 0);
            }
            return;
          }
          pbVar14 = (byte *)(*(int *)(local_a * 2 + _DAT_1000_c794) + local_8);
          uVar9 = local_a - local_6;
          if (local_6 <= local_a) goto LAB_0000_180e;
        }
        uVar9 = -uVar9;
        iVar13 = -iVar13;
LAB_0000_180e:
        if (uVar8 <= uVar9) {
          iVar7 = -((int)uVar9 >> 1);
          iVar6 = uVar9 + 1;
          if (DAT_0000_161b == '\0') {
            while( true ) {
              *pbVar14 = param_1;
              pbVar14 = pbVar14 + (1 - (iVar13 + 1));
              iVar6 = iVar6 + -1;
              if (iVar6 == 0) break;
              iVar7 = iVar7 + uVar8;
              if (-1 < iVar7) {
                pbVar14 = pbVar14 + 1;
                iVar7 = iVar7 - uVar9;
              }
            }
            return;
          }
          while( true ) {
            *pbVar14 = *pbVar14 ^ param_1;
            pbVar14 = pbVar14 + -iVar13;
            iVar6 = iVar6 + -1;
            if (iVar6 == 0) break;
            iVar7 = iVar7 + uVar8;
            if (-1 < iVar7) {
              pbVar14 = pbVar14 + 1;
              iVar7 = iVar7 - uVar9;
            }
          }
          return;
        }
        iVar7 = -((int)uVar8 >> 1);
        iVar6 = uVar8 + 1;
        if (DAT_0000_161b == '\0') {
          while( true ) {
            pbVar1 = pbVar14;
            pbVar14 = pbVar14 + 1;
            *pbVar1 = param_1;
            iVar6 = iVar6 + -1;
            if (iVar6 == 0) break;
            iVar7 = iVar7 + uVar9;
            if (-1 < iVar7) {
              pbVar14 = pbVar14 + -iVar13;
              iVar7 = iVar7 - uVar8;
            }
          }
          return;
        }
        while( true ) {
          *pbVar14 = *pbVar14 ^ param_1;
          pbVar14 = pbVar14 + 1;
          iVar6 = iVar6 + -1;
          if (iVar6 == 0) break;
          iVar7 = iVar7 + uVar9;
          if (-1 < iVar7) {
            pbVar14 = pbVar14 + -iVar13;
            iVar7 = iVar7 - uVar8;
          }
        }
        return;
      }
      local_12 = local_12 ^ 0xff;
      bVar16 = local_12 == 0;
    } while (!bVar16);
    iVar6 = local_4 - local_8;
    iVar7 = local_6 - local_a;
    puVar15 = &local_a;
    do {
      if ((char)uVar10 != '\0') {
        uVar8 = puVar15[1];
        uVar9 = *puVar15;
        if ((uVar10 & 8) != 0) {
          uVar10 = uVar10 ^ 8;
          uVar9 = _DAT_1000_c724[3];
          uVar8 = (int)(((long)(int)(uVar9 - *puVar15) * (long)iVar6) / (long)iVar7) + puVar15[1];
        }
        if ((uVar10 & 2) != 0) {
          uVar10 = uVar10 ^ 2;
          uVar11 = _DAT_1000_c724[4];
          if ((int)uVar8 < (int)uVar11) {
            uVar9 = (int)(((long)(int)(uVar11 - puVar15[1]) * (long)iVar7) / (long)iVar6) + *puVar15
            ;
            uVar8 = uVar11;
          }
        }
        if ((uVar10 & 4) != 0) {
          uVar10 = uVar10 ^ 4;
          if ((int)_DAT_1000_c724[5] <= (int)uVar9) {
            uVar9 = _DAT_1000_c724[5] - 1;
            uVar8 = (int)(((long)(int)(uVar9 - *puVar15) * (long)iVar6) / (long)iVar7) + puVar15[1];
          }
        }
        if ((uVar10 & 1) != 0) {
          uVar10 = uVar10 ^ 1;
          if ((int)_DAT_1000_c724[6] <= (int)uVar8) {
            uVar8 = _DAT_1000_c724[6] - 1;
            uVar9 = (int)(((long)(int)(uVar8 - puVar15[1]) * (long)iVar7) / (long)iVar6) + *puVar15;
          }
        }
        *puVar15 = uVar9;
        puVar15[1] = uVar8;
      }
      puVar15 = &local_6;
      uVar10 = CONCAT11((char)uVar10,(char)(uVar10 >> 8));
    } while (uVar10 != 0);
  } while( true );
}


/* ===== FUN_0000_1952 @ 0000:1952 ===== */

void __allregs FUN_0000_1952(undefined1 param_1,undefined2 param_2,undefined2 param_3)

{
  DAT_0000_161b = 0xff;
  FUN_0000_161c(param_1,param_2,param_3);
  DAT_0000_161b = 0;
  return;
}


/* ===== FUN_0000_1963 @ 0000:1963 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1963(void)

{
  uint uVar1;
  
  uVar1 = (uint)*(byte *)(_DAT_1000_c736 + 8) * 2;
  if (uVar1 < 0x10) {
    _DAT_1000_c684 = *(undefined2 *)(uVar1 + 0x198e);
    _DAT_1000_c686 = 0;
    _DAT_1000_c688 = *(undefined2 *)(uVar1 + 0x199e);
    _DAT_1000_c68a = 0;
  }
  return;
}


/* ===== FUN_0000_198e @ 0000:198e ===== */

/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_198e(undefined2 param_1,int param_2,uint param_3,uint *param_4)

{
  uint uVar1;
  uint uVar2;
  undefined2 unaff_ES;
  
  uVar1 = *param_4;
  uVar2 = *param_4;
  *(uint *)(byte *)((int)param_4 + -0x72e1) =
       (*(int *)(byte *)((int)param_4 + -0x72e1) - param_2) - (uint)((byte)param_1 < (byte)*param_4)
  ;
  if (*_DAT_1000_c736 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    _DAT_2000_d68a = 0x19c6;
    _DAT_2000_d68c = (param_3 & uVar1 & uVar2) + (int)param_4 * 10 + 5;
    _DAT_2000_d68e = param_1;
    FUN_0000_2d6d(_DAT_1000_c736 + 0xe);
  }
  _DAT_2000_d68e = 0x19e0;
  FUN_0000_19ea((byte)param_1);
  return;
}


/* ===== FUN_0000_19ca @ 0000:19ca ===== */

void __allregs FUN_0000_19ca(int param_1,undefined2 *param_2,undefined2 param_3,int param_4)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *(int *)(&stack0x0056 + param_4) = *(int *)(&stack0x0056 + param_4) + param_1;
  FUN_0000_19ea((char)param_3 + '\x01');
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_19ea @ 0000:19ea ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_19ea(byte param_1)

{
  undefined1 *puVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  byte bVar4;
  undefined2 uVar5;
  undefined2 *puVar6;
  byte bVar7;
  uint uVar8;
  int iVar9;
  byte *pbVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  bool bVar13;
  
  puVar6 = _DAT_1000_c736;
  uVar5 = _DAT_1000_c71a;
  uVar2 = *(undefined1 *)(_DAT_1000_c736 + 0xc);
  if (!CARRY1(param_1,param_1)) {
    uVar2 = *(undefined1 *)(_DAT_1000_c736 + 0xb);
  }
  pbVar10 = (byte *)*(undefined2 *)(_DAT_1000_c736[3] + (uint)(byte)(param_1 * '\x02'));
  DAT_1000_c792 = uVar2;
  if (*pbVar10 == 0) {
    bVar7 = param_1 & 0x7f;
    if (bVar7 == 7) {
      FUN_0000_2c7b();
    }
    else if (bVar7 == 8) {
      iVar9 = _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] - iVar9;
      puVar6[0x10] = puVar6[0x10] - iVar9;
      iVar9 = puVar6[5] + puVar6[7] + puVar6[9];
      puVar6[0xf] = puVar6[0xf] - iVar9;
      puVar6[0x11] = puVar6[0x11] - iVar9;
    }
    else if (bVar7 == 10) {
      iVar9 = _DAT_1000_c736[6] + _DAT_1000_c736[8] + _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] + iVar9;
      puVar6[0x10] = puVar6[0x10] + iVar9;
    }
    else if (bVar7 == 0xd) {
      iVar9 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar9;
      puVar6[0x11] = iVar9 + puVar6[5];
    }
    else if (bVar7 == 0xc) {
      iVar9 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar9;
      puVar6[0x11] = iVar9 + puVar6[5];
      iVar9 = puVar6[6] + puVar6[8] + puVar6[10];
      puVar6[0xe] = puVar6[0xe] + iVar9;
      puVar6[0x10] = puVar6[0x10] + iVar9;
    }
    else if (bVar7 == 0xe) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + 1;
      puVar6[0x11] = puVar6[0x11] + 1;
    }
    else if (bVar7 == 0xf) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + -1;
      puVar6[0x11] = puVar6[0x11] + -1;
    }
  }
  else {
    puVar11 = (undefined1 *)
              (*(int *)(_DAT_1000_c736[0xe] * 2 + _DAT_1000_c794) + _DAT_1000_c736[0xf]);
    uVar3 = *_DAT_1000_c736;
    iVar9 = _DAT_1000_c736[6] + -1;
    do {
      pbVar10 = pbVar10 + 1;
      bVar7 = *pbVar10;
      bVar13 = CARRY1(bVar7,bVar7);
      bVar4 = bVar7 * '\x02';
      puVar12 = puVar11;
      if (bVar4 != 0) {
        uVar8 = (uint)bVar13;
        puVar12 = puVar11 + (1 - uVar8);
        for (; uVar8 != 0; uVar8 = uVar8 - 1) {
          puVar1 = puVar12;
          puVar12 = puVar12 + 1;
          *puVar1 = uVar2;
        }
        bVar13 = CARRY1(bVar4,bVar4);
        bVar4 = bVar7 * '\x04';
        if (bVar4 != 0) {
          uVar8 = (uint)bVar13;
          puVar12 = puVar12 + (1 - uVar8);
          for (; uVar8 != 0; uVar8 = uVar8 - 1) {
            puVar1 = puVar12;
            puVar12 = puVar12 + 1;
            *puVar1 = uVar2;
          }
          bVar13 = CARRY1(bVar4,bVar4);
          bVar4 = bVar7 * '\b';
          if (bVar4 != 0) {
            uVar8 = (uint)bVar13;
            puVar12 = puVar12 + (1 - uVar8);
            for (; uVar8 != 0; uVar8 = uVar8 - 1) {
              puVar1 = puVar12;
              puVar12 = puVar12 + 1;
              *puVar1 = uVar2;
            }
            bVar13 = CARRY1(bVar4,bVar4);
            bVar4 = bVar7 * '\x10';
            if (bVar4 != 0) {
              uVar8 = (uint)bVar13;
              puVar12 = puVar12 + (1 - uVar8);
              for (; uVar8 != 0; uVar8 = uVar8 - 1) {
                puVar1 = puVar12;
                puVar12 = puVar12 + 1;
                *puVar1 = uVar2;
              }
              bVar13 = CARRY1(bVar4,bVar4);
              bVar4 = bVar7 * ' ';
              if (bVar4 != 0) {
                uVar8 = (uint)bVar13;
                puVar12 = puVar12 + (1 - uVar8);
                for (; uVar8 != 0; uVar8 = uVar8 - 1) {
                  puVar1 = puVar12;
                  puVar12 = puVar12 + 1;
                  *puVar1 = uVar2;
                }
                bVar13 = CARRY1(bVar4,bVar4);
                bVar4 = bVar7 * '@';
                if (bVar4 != 0) {
                  uVar8 = (uint)bVar13;
                  puVar12 = puVar12 + (1 - uVar8);
                  for (; uVar8 != 0; uVar8 = uVar8 - 1) {
                    puVar1 = puVar12;
                    puVar12 = puVar12 + 1;
                    *puVar1 = uVar2;
                  }
                  bVar13 = CARRY1(bVar4,bVar4);
                  bVar7 = bVar7 << 7;
                  if (bVar7 != 0) {
                    uVar8 = (uint)bVar13;
                    puVar12 = puVar12 + (1 - uVar8);
                    for (; uVar8 != 0; uVar8 = uVar8 - 1) {
                      puVar1 = puVar12;
                      puVar12 = puVar12 + 1;
                      *puVar1 = uVar2;
                    }
                    bVar13 = CARRY1(bVar7,bVar7);
                  }
                }
              }
            }
          }
        }
      }
      uVar8 = (uint)bVar13;
      puVar12 = puVar12 + (1 - uVar8);
      for (; uVar8 != 0; uVar8 = uVar8 - 1) {
        puVar1 = puVar12;
        puVar12 = puVar12 + 1;
        *puVar1 = uVar2;
      }
      puVar11 = puVar11 + _DAT_2000_c226;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    iVar9 = puVar6[10];
    puVar6[0xe] = puVar6[0xe] + iVar9;
    puVar6[0x10] = puVar6[0x10] + iVar9;
    iVar9 = puVar6[5] + puVar6[7] + puVar6[9];
    puVar6[0xf] = puVar6[0xf] + iVar9;
    puVar6[0x11] = puVar6[0x11] + iVar9;
  }
  return;
}


/* ===== FUN_0000_1b5c @ 0000:1b5c ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1b5c(undefined1 param_1)

{
  if (*_DAT_1000_c736 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(_DAT_1000_c736 + 0xe);
  }
  FUN_0000_1b98(param_1);
  return;
}


/* ===== FUN_0000_1b78 @ 0000:1b78 ===== */

void __allregs FUN_0000_1b78(int param_1,undefined2 *param_2,undefined2 param_3,int param_4)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *(int *)(&stack0x0056 + param_4) = *(int *)(&stack0x0056 + param_4) + param_1;
  FUN_0000_1b98((char)param_3 + '\x01');
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1b98 @ 0000:1b98 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1b98(byte param_1)

{
  byte bVar1;
  byte *pbVar2;
  byte *pbVar3;
  undefined2 uVar4;
  uint uVar5;
  undefined2 uVar6;
  undefined2 *puVar7;
  int iVar8;
  byte bVar9;
  byte bVar10;
  char cVar11;
  uint uVar12;
  char cVar13;
  byte *pbVar14;
  byte *pbVar15;
  
  puVar7 = _DAT_1000_c736;
  uVar6 = _DAT_1000_c71a;
  _DAT_1000_c792 = _DAT_1000_c736[0xc];
  if (!CARRY1(param_1,param_1)) {
    _DAT_1000_c792 = _DAT_1000_c736[0xb];
  }
  pbVar3 = (byte *)*(undefined2 *)(_DAT_1000_c736[3] + (uint)(byte)(param_1 * '\x02'));
  if (*pbVar3 == 0) {
    bVar9 = param_1 & 0x7f;
    if (bVar9 == 7) {
      FUN_0000_2c7b();
    }
    else if (bVar9 == 8) {
      iVar8 = _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] - iVar8;
      puVar7[0x10] = puVar7[0x10] - iVar8;
      iVar8 = puVar7[5] + puVar7[7] + puVar7[9];
      puVar7[0xf] = puVar7[0xf] - iVar8;
      puVar7[0x11] = puVar7[0x11] - iVar8;
    }
    else if (bVar9 == 10) {
      iVar8 = _DAT_1000_c736[6] + _DAT_1000_c736[8] + _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] + iVar8;
      puVar7[0x10] = puVar7[0x10] + iVar8;
    }
    else if (bVar9 == 0xd) {
      iVar8 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar8;
      puVar7[0x11] = iVar8 + puVar7[5];
    }
    else if (bVar9 == 0xc) {
      iVar8 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar8;
      puVar7[0x11] = iVar8 + puVar7[5];
      iVar8 = puVar7[6] + puVar7[8] + puVar7[10];
      puVar7[0xe] = puVar7[0xe] + iVar8;
      puVar7[0x10] = puVar7[0x10] + iVar8;
    }
    else if (bVar9 == 0xe) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + 1;
      puVar7[0x11] = puVar7[0x11] + 1;
    }
    else if (bVar9 == 0xf) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + -1;
      puVar7[0x11] = puVar7[0x11] + -1;
    }
  }
  else {
    pbVar14 = (byte *)(*(int *)(_DAT_1000_c736[0xe] * 2 + _DAT_1000_c794) + _DAT_1000_c736[0xf]);
    uVar4 = *_DAT_1000_c736;
    bVar10 = (byte)((uint)_DAT_1000_c792 >> 8);
    bVar9 = (byte)_DAT_1000_c792 ^ bVar10;
    uVar12 = CONCAT11((char)(_DAT_1000_c736[6] + -1),(char)((uint)(_DAT_1000_c736[6] + -1) >> 8));
    uVar5 = _DAT_1000_c736[5];
    do {
      pbVar3 = pbVar3 + 1;
      bVar1 = *pbVar3;
      uVar12 = uVar12 | uVar5;
      pbVar15 = pbVar14 + 1;
      *pbVar14 = -CARRY1(bVar1,bVar1) & bVar9 ^ bVar10;
      cVar13 = (char)uVar12;
      cVar11 = '\0';
      if (cVar13 != '\x01') {
        pbVar2 = pbVar15;
        pbVar15 = pbVar14 + 2;
        *pbVar2 = -CARRY1(bVar1 * '\x02',bVar1 * '\x02') & bVar9 ^ bVar10;
        cVar11 = '\0';
        if (cVar13 != '\x02') {
          pbVar2 = pbVar15;
          pbVar15 = pbVar14 + 3;
          *pbVar2 = -CARRY1(bVar1 * '\x04',bVar1 * '\x04') & bVar9 ^ bVar10;
          cVar11 = '\0';
          if (cVar13 != '\x03') {
            pbVar2 = pbVar15;
            pbVar15 = pbVar14 + 4;
            *pbVar2 = -CARRY1(bVar1 * '\b',bVar1 * '\b') & bVar9 ^ bVar10;
            cVar11 = '\0';
            if (cVar13 != '\x04') {
              pbVar2 = pbVar15;
              pbVar15 = pbVar14 + 5;
              *pbVar2 = -CARRY1(bVar1 * '\x10',bVar1 * '\x10') & bVar9 ^ bVar10;
              cVar11 = '\0';
              if (cVar13 != '\x05') {
                pbVar2 = pbVar15;
                pbVar15 = pbVar14 + 6;
                *pbVar2 = -CARRY1(bVar1 * ' ',bVar1 * ' ') & bVar9 ^ bVar10;
                cVar11 = '\0';
                if (cVar13 != '\x06') {
                  pbVar2 = pbVar15;
                  pbVar15 = pbVar14 + 7;
                  *pbVar2 = -CARRY1(bVar1 * '@',bVar1 * '@') & bVar9 ^ bVar10;
                  cVar11 = '\0';
                  if (cVar13 != '\a') {
                    pbVar2 = pbVar15;
                    pbVar15 = pbVar14 + 8;
                    *pbVar2 = -CARRY1(bVar1 << 7,bVar1 << 7) & bVar9 ^ bVar10;
                    cVar11 = cVar13 + -8;
                  }
                }
              }
            }
          }
        }
      }
      pbVar14 = pbVar15 + (_DAT_2000_c226 - uVar5);
      cVar13 = (char)(uVar12 >> 8) + -1;
      uVar12 = CONCAT11(cVar13,cVar11);
    } while (cVar13 != '\0');
    iVar8 = puVar7[10];
    puVar7[0xe] = puVar7[0xe] + iVar8;
    puVar7[0x10] = puVar7[0x10] + iVar8;
    iVar8 = puVar7[5] + puVar7[7] + puVar7[9];
    puVar7[0xf] = puVar7[0xf] + iVar8;
    puVar7[0x11] = puVar7[0x11] + iVar8;
  }
  return;
}


/* ===== FUN_0000_1d23 @ 0000:1d23 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1d23(undefined1 param_1)

{
  if (*_DAT_1000_c736 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(_DAT_1000_c736 + 0xe);
  }
  FUN_0000_1d5f(param_1);
  return;
}


/* ===== FUN_0000_1d3f @ 0000:1d3f ===== */

void __allregs FUN_0000_1d3f(int param_1,undefined2 *param_2,undefined2 param_3,int param_4)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *(int *)(&stack0x0056 + param_4) = *(int *)(&stack0x0056 + param_4) + param_1;
  FUN_0000_1d5f((char)param_3 + '\x01');
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1d5f @ 0000:1d5f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1d5f(byte param_1)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 *puVar5;
  char *pcVar6;
  byte bVar7;
  uint uVar8;
  char cVar9;
  uint uVar10;
  int iVar11;
  byte *pbVar12;
  char *pcVar13;
  
  puVar5 = _DAT_1000_c736;
  uVar4 = _DAT_1000_c71a;
  cVar9 = *(char *)(_DAT_1000_c736 + 0xc);
  if (!CARRY1(param_1,param_1)) {
    cVar9 = *(char *)(_DAT_1000_c736 + 0xb);
  }
  pbVar2 = (byte *)*(undefined2 *)(_DAT_1000_c736[3] + (uint)(byte)(param_1 * '\x02'));
  pbVar12 = pbVar2 + 1;
  bVar7 = *pbVar2;
  DAT_1000_c792 = cVar9;
  if (bVar7 == 0) {
    bVar7 = param_1 & 0x7f;
    if (bVar7 == 7) {
      FUN_0000_2c7b();
    }
    else if (bVar7 == 8) {
      iVar11 = _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] - iVar11;
      puVar5[0x10] = puVar5[0x10] - iVar11;
      iVar11 = puVar5[5] + puVar5[7] + puVar5[9];
      puVar5[0xf] = puVar5[0xf] - iVar11;
      puVar5[0x11] = puVar5[0x11] - iVar11;
    }
    else if (bVar7 == 10) {
      iVar11 = _DAT_1000_c736[6] + _DAT_1000_c736[8] + _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] + iVar11;
      puVar5[0x10] = puVar5[0x10] + iVar11;
    }
    else if (bVar7 == 0xd) {
      iVar11 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar11;
      puVar5[0x11] = iVar11 + puVar5[5];
    }
    else if (bVar7 == 0xc) {
      iVar11 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar11;
      puVar5[0x11] = iVar11 + puVar5[5];
      iVar11 = puVar5[6] + puVar5[8] + puVar5[10];
      puVar5[0xe] = puVar5[0xe] + iVar11;
      puVar5[0x10] = puVar5[0x10] + iVar11;
    }
    else if (bVar7 == 0xe) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + 1;
      puVar5[0x11] = puVar5[0x11] + 1;
    }
    else if (bVar7 == 0xf) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + -1;
      puVar5[0x11] = puVar5[0x11] + -1;
    }
  }
  else {
    uVar8 = (uint)bVar7;
    uVar3 = *_DAT_1000_c736;
    iVar11 = _DAT_1000_c736[6] + -1;
    uVar10 = uVar8;
    pcVar6 = (char *)(*(int *)(_DAT_1000_c736[0xe] * 2 + _DAT_1000_c794) + _DAT_1000_c736[0xf]);
    do {
      do {
        pcVar13 = pcVar6;
        pbVar1 = pbVar12;
        pbVar12 = pbVar12 + 1;
        if (*pbVar1 != 0) {
          *pcVar13 = (*pbVar1 ^ 3) + cVar9;
        }
        uVar10 = uVar10 - 1;
        pcVar6 = pcVar13 + 1;
      } while (uVar10 != 0);
      iVar11 = iVar11 + -1;
      uVar10 = uVar8;
      pcVar6 = pcVar13 + (_DAT_2000_c226 - uVar8) + 1;
    } while (iVar11 != 0);
    iVar11 = puVar5[10];
    puVar5[0xe] = puVar5[0xe] + iVar11;
    puVar5[0x10] = puVar5[0x10] + iVar11;
    iVar11 = uVar8 + 1 + puVar5[7] + puVar5[9];
    puVar5[0xf] = puVar5[0xf] + iVar11;
    puVar5[0x11] = puVar5[0x11] + iVar11;
  }
  return;
}


/* ===== FUN_0000_1e8d @ 0000:1e8d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1e8d(undefined1 param_1)

{
  if (*_DAT_1000_c736 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(_DAT_1000_c736 + 0xe);
  }
  FUN_0000_1ec9(param_1);
  return;
}


/* ===== FUN_0000_1ea3 @ 0000:1ea3 ===== */

/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_1ea3(undefined2 *param_1,int param_2,undefined2 *param_3,undefined2 param_4,int param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  char cVar3;
  undefined2 **ppuVar4;
  undefined2 *puStack_2;
  undefined2 **ppuVar5;
  
  ppuVar4 = &puStack_2;
  ppuVar5 = &puStack_2;
  puStack_2 = param_3;
  cVar3 = '\x05';
  do {
    param_3 = param_3 + -1;
    ppuVar4 = ppuVar4 + -1;
    *ppuVar4 = (undefined2 *)*param_3;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  *param_1 = &puStack_2;
  cVar3 = '\x05';
  puVar2 = param_1;
  do {
    ppuVar5 = ppuVar5 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *ppuVar5;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  param_1[-6] = param_1;
  piVar1 = (int *)((int)param_1 + param_5 + 0x58);
  *piVar1 = *piVar1 + param_2;
  *(undefined2 *)((int)param_1 + 0x38ed) = 0x1eb6;
  FUN_0000_1ec9((char)param_4 + '\x01');
  *(undefined2 *)((int)param_1 + 0x38ed) = 0x1eb9;
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_1ec9 @ 0000:1ec9 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_1ec9(byte param_1)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 *puVar5;
  byte bVar6;
  uint uVar7;
  int iVar8;
  undefined2 uVar9;
  char cVar10;
  uint uVar11;
  undefined1 uVar12;
  char cVar13;
  byte *pbVar14;
  char *pcVar15;
  char *pcVar16;
  
  puVar5 = _DAT_1000_c736;
  uVar4 = _DAT_1000_c71a;
  uVar9 = _DAT_1000_c736[0xc];
  if (!CARRY1(param_1,param_1)) {
    uVar9 = _DAT_1000_c736[0xb];
  }
  pbVar2 = (byte *)*(undefined2 *)(_DAT_1000_c736[3] + (uint)(byte)(param_1 * '\x02'));
  pbVar14 = pbVar2 + 1;
  bVar6 = *pbVar2;
  _DAT_1000_c792 = uVar9;
  if (bVar6 == 0) {
    bVar6 = param_1 & 0x7f;
    if (bVar6 == 7) {
      FUN_0000_2c7b();
    }
    else if (bVar6 == 8) {
      iVar8 = _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] - iVar8;
      puVar5[0x10] = puVar5[0x10] - iVar8;
      iVar8 = puVar5[5] + puVar5[7] + puVar5[9];
      puVar5[0xf] = puVar5[0xf] - iVar8;
      puVar5[0x11] = puVar5[0x11] - iVar8;
    }
    else if (bVar6 == 10) {
      iVar8 = _DAT_1000_c736[6] + _DAT_1000_c736[8] + _DAT_1000_c736[10];
      _DAT_1000_c736[0xe] = _DAT_1000_c736[0xe] + iVar8;
      puVar5[0x10] = puVar5[0x10] + iVar8;
    }
    else if (bVar6 == 0xd) {
      iVar8 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar8;
      puVar5[0x11] = iVar8 + puVar5[5];
    }
    else if (bVar6 == 0xc) {
      iVar8 = _DAT_1000_c736[0xd];
      _DAT_1000_c736[0xf] = iVar8;
      puVar5[0x11] = iVar8 + puVar5[5];
      iVar8 = puVar5[6] + puVar5[8] + puVar5[10];
      puVar5[0xe] = puVar5[0xe] + iVar8;
      puVar5[0x10] = puVar5[0x10] + iVar8;
    }
    else if (bVar6 == 0xe) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + 1;
      puVar5[0x11] = puVar5[0x11] + 1;
    }
    else if (bVar6 == 0xf) {
      _DAT_1000_c736[0xf] = _DAT_1000_c736[0xf] + -1;
      puVar5[0x11] = puVar5[0x11] + -1;
    }
  }
  else {
    uVar7 = (uint)bVar6;
    pcVar15 = (char *)(*(int *)(_DAT_1000_c736[0xe] * 2 + _DAT_1000_c794) + _DAT_1000_c736[0xf]);
    uVar3 = *_DAT_1000_c736;
    uVar11 = CONCAT11((char)(_DAT_1000_c736[6] + -1),(char)((uint)(_DAT_1000_c736[6] + -1) >> 8));
    do {
      uVar11 = uVar11 | uVar7;
      do {
        while( true ) {
          pcVar16 = pcVar15;
          pbVar1 = pbVar14;
          pbVar14 = pbVar14 + 1;
          cVar13 = (char)((uint)uVar9 >> 8);
          uVar12 = (undefined1)(uVar11 >> 8);
          if (*pbVar1 == 0) break;
          *pcVar16 = (*pbVar1 ^ 3) + (char)uVar9;
          cVar10 = (char)uVar11 + -1;
          uVar11 = CONCAT11(uVar12,cVar10);
          pcVar15 = pcVar16 + 1;
          if (cVar10 == '\0') goto LAB_0000_1f33;
        }
        *pcVar16 = cVar13;
        cVar10 = (char)uVar11 + -1;
        uVar11 = CONCAT11(uVar12,cVar10);
        pcVar15 = pcVar16 + 1;
      } while (cVar10 != '\0');
LAB_0000_1f33:
      pcVar16[1] = cVar13;
      pcVar15 = pcVar16 + 1 + (_DAT_2000_c226 - uVar7);
      cVar13 = (char)(uVar11 >> 8) + -1;
      uVar11 = CONCAT11(cVar13,(char)uVar11);
    } while (cVar13 != '\0');
    iVar8 = puVar5[10];
    puVar5[0xe] = puVar5[0xe] + iVar8;
    puVar5[0x10] = puVar5[0x10] + iVar8;
    iVar8 = uVar7 + 1 + puVar5[7] + puVar5[9];
    puVar5[0xf] = puVar5[0xf] + iVar8;
    puVar5[0x11] = puVar5[0x11] + iVar8;
  }
  return;
}


/* ===== FUN_0000_2004 @ 0000:2004 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2004(char param_1,undefined2 *param_2,undefined2 *param_3)

{
  uint uVar1;
  char *pcVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  uint *puVar5;
  undefined2 uVar6;
  byte bVar7;
  byte bVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  int iVar13;
  uint uVar14;
  uint *puVar15;
  uint *puVar16;
  char *pcVar17;
  undefined2 uVar18;
  uint local_16;
  char *local_14;
  undefined2 local_12;
  uint local_10;
  char *local_e;
  uint local_c;
  char *local_a;
  
  (*_DAT_1000_c0cc)();
  (*_DAT_1000_c0cc)();
  uVar6 = _DAT_1000_d590;
  local_c = 0;
  local_a = (char *)0x0;
  local_10 = 0xffff;
  local_e = (char *)0xffff;
  uVar3 = *param_3;
  uVar4 = *param_2;
  puVar15 = (uint *)0x0;
  *(undefined2 *)0x0 = 0;
  local_16 = 0;
  iVar12 = 200;
LAB_0000_2060:
  uVar18 = 0x2073;
  (*(code *)*(undefined2 *)0x336)();
  pcVar17 = (char *)0x0;
  local_14 = (char *)0x0;
  iVar13 = 0x141;
  puVar16 = puVar15;
code_r0x00002083:
  if (iVar13 != 0) goto code_r0x00002083;
  goto code_r0x00002085;
code_r0x00002083:
  iVar13 = iVar13 + -1;
  pcVar2 = pcVar17;
  pcVar17 = pcVar17 + 1;
  if (param_1 != *pcVar2) goto code_r0x00002083;
code_r0x00002085:
  pcVar17 = pcVar17 + -1;
  LOCK();
  UNLOCK();
  *puVar16 = ((int)local_14 - (int)pcVar17) * -4 | 3;
  puVar16[1] = -((int)local_14 - (int)pcVar17);
  puVar15 = puVar16 + 2;
  if (iVar13 != 0) {
    iVar13 = iVar13 + 1;
    local_10 = (local_16 - local_10 & -(uint)(local_16 < local_10)) + local_10;
    local_e = pcVar17 + ((int)local_e - (int)pcVar17 & -(uint)(local_e < pcVar17));
    local_14 = pcVar17;
    do {
      if (iVar13 == 0) break;
      iVar13 = iVar13 + -1;
      pcVar2 = local_14;
      local_14 = local_14 + 1;
    } while (param_1 == *pcVar2);
    local_14 = local_14 + -1;
    LOCK();
    UNLOCK();
    iVar9 = (int)local_14 - (int)pcVar17;
    *puVar15 = iVar9 * 2;
    puVar15 = puVar16 + 3;
    for (; iVar9 != 0; iVar9 = iVar9 + -1) {
      pcVar2 = pcVar17;
      pcVar17 = pcVar17 + 1;
      *pcVar2 = '\0';
    }
    local_c = (local_16 - local_c & -(uint)(local_c <= local_16)) + local_c;
    local_a = pcVar17 + ((int)local_a - (int)pcVar17 & -(uint)(pcVar17 <= local_a));
    if (iVar13 == 0) goto LAB_0000_2106;
    iVar13 = iVar13 + 1;
    puVar16 = puVar15;
    goto code_r0x00002083;
  }
LAB_0000_2106:
  local_16 = local_16 + 1;
  iVar12 = iVar12 + -1;
  if (iVar12 == 0) {
    *puVar15 = 0;
    bVar7 = (byte)local_e;
    bVar8 = (byte)local_e & 0xfe;
    local_e = (char *)((uint)local_e & 0xfffe);
    uVar10 = (uint)(local_a + 1) & 0xfffe;
    local_12 = CONCAT11((char)local_a - (char)uVar10,bVar8 ^ bVar7);
    puVar15 = (uint *)0x0;
    for (uVar14 = local_10; uVar14 != 0; uVar14 = uVar14 - 1) {
      *puVar15 = 3;
      puVar15 = puVar15 + 2;
    }
    while (*puVar15 != 0) {
      uVar14 = *puVar15 >> 2;
      *puVar15 = (uVar14 - (int)local_e) * 4 | 3;
      puVar16 = puVar15 + 2;
      puVar5 = puVar15;
      for (; puVar15 = puVar16, uVar14 < 0x140; uVar14 = uVar14 + uVar11) {
        puVar16 = puVar15 + 1;
        uVar1 = *puVar15;
        uVar11 = uVar1 >> 1;
        if ((uVar1 & 1) != 0) {
          uVar11 = uVar1 >> 2;
          puVar16 = puVar15 + 2;
        }
        puVar5 = puVar15;
      }
      puVar15 = puVar5 + 1;
      if ((*puVar5 & 1) != 0) {
        *puVar5 = ((*puVar5 >> 2) - (0x140 - uVar10)) * 4 | 3;
        puVar15 = puVar5 + 2;
      }
    }
    param_2[2] = local_10;
    param_2[3] = local_e;
    param_2[4] = local_c + 1;
    param_2[5] = uVar10;
    param_2[10] = local_12;
    bRam0001d592 = bRam0001d592 | 0x22;
    FUN_0000_21d6(param_2,uVar18);
    return;
  }
  goto LAB_0000_2060;
}


/* ===== FUN_0000_21d6 @ 0000:21d6 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_21d6(undefined2 *param_1,uint *param_2)

{
  uint *puVar1;
  undefined2 uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint *puVar9;
  uint *puVar10;
  uint *puVar11;
  bool bVar12;
  int local_6;
  int local_4;
  
  uVar2 = *param_1;
  puVar9 = (uint *)0x0;
  puVar11 = (uint *)0x0;
  local_4 = 0;
  local_6 = 0;
  do {
    while( true ) {
      puVar10 = puVar9 + 1;
      uVar4 = *puVar9;
      if ((uVar4 & 1) == 0) break;
      uVar8 = uVar4 >> 2;
      if ((uVar4 & 2) != 0) {
        uVar8 = *puVar10;
        puVar10 = puVar9 + 2;
      }
      do {
        puVar9 = puVar10;
        if ((*puVar9 & 1) == 0) goto LAB_0000_2234;
        puVar10 = puVar9 + 1;
        uVar6 = *puVar9;
        uVar7 = uVar6 >> 2;
        if ((uVar6 & 2) != 0) {
          puVar1 = puVar10;
          puVar10 = puVar9 + 2;
          uVar7 = *puVar1;
        }
        uVar6 = uVar6 | 2;
        uVar3 = uVar4 & 0xfffc;
        uVar4 = uVar3 + uVar6;
        if (CARRY2(uVar3,uVar6)) goto LAB_0000_2228;
        bVar12 = CARRY2(uVar8,uVar7);
        uVar8 = uVar8 + uVar7;
      } while (!bVar12);
      uVar8 = uVar8 - uVar7;
LAB_0000_2228:
      puVar10[-1] = uVar7;
      uVar4 = uVar4 - uVar6;
      puVar9 = puVar10 + -2;
      *puVar9 = uVar6;
LAB_0000_2234:
      uVar4 = uVar4 >> 2;
      if (uVar4 == uVar8) {
        local_4 = local_4 + uVar4;
        local_6 = local_6 + uVar4;
        puVar1 = puVar11;
        puVar11 = puVar11 + 1;
        *puVar1 = uVar4 << 2 | 1;
      }
      else {
        local_4 = local_4 + uVar4;
        puVar10 = puVar11 + 1;
        *puVar11 = uVar4 << 2 | 3;
        local_6 = local_6 + uVar8;
        puVar11 = puVar11 + 2;
        *puVar10 = uVar8;
      }
    }
    uVar4 = uVar4 >> 1;
    if (uVar4 == 0) {
      *param_2 = 0;
      (*_DAT_1000_c0d4)();
      return;
    }
    do {
      puVar9 = puVar10;
      uVar8 = *puVar9 >> 1;
      if ((*puVar9 & 1) != 0 || uVar8 == 0) goto LAB_0000_226e;
      bVar12 = CARRY2(uVar4,uVar8);
      uVar4 = uVar4 + uVar8;
      puVar10 = puVar9 + 1;
    } while (!bVar12);
    uVar4 = uVar4 - uVar8;
LAB_0000_226e:
    uVar8 = ((byte)local_4 & (byte)local_6) & 1;
    if (((byte)local_4 & (byte)local_6 & 1) != 0) goto LAB_0000_227f;
    while( true ) {
      uVar8 = 0x7ffe;
LAB_0000_227f:
      iVar5 = (uVar8 - uVar4 & -(uint)(uVar8 < uVar4)) + uVar4;
      if (iVar5 == 0) break;
      uVar4 = uVar4 - iVar5;
      local_4 = local_4 + iVar5;
      local_6 = local_6 + iVar5;
      puVar1 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar1 = iVar5 * 2;
      param_2 = puVar11;
    }
  } while( true );
}


/* ===== FUN_0000_22bf @ 0000:22bf ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_22bf(undefined2 param_1,undefined2 *param_2,undefined1 *param_3,undefined2 param_4)

{
  byte *pbVar1;
  undefined2 *puVar2;
  undefined1 *puVar3;
  int *piVar4;
  uint *puVar5;
  undefined2 uVar6;
  undefined2 uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  int iVar12;
  uint uVar13;
  undefined2 *puVar14;
  undefined2 *puVar15;
  uint *puVar16;
  uint *puVar17;
  int *piVar18;
  int *piVar19;
  undefined2 uStack_2;
  
  puVar14 = &uStack_2;
  uVar6 = *param_2;
  uVar7 = param_2[1];
  puVar15 = param_2;
  for (iVar11 = 0x1c; iVar11 != 0; iVar11 = iVar11 + -1) {
    puVar3 = param_3;
    param_3 = param_3 + 1;
    puVar2 = puVar15;
    puVar15 = (undefined2 *)((int)puVar15 + 1);
    *puVar3 = *(undefined1 *)puVar2;
  }
  puVar15[-0xd] = uVar7;
  puVar15[-0xe] = uVar6;
  uStack_2 = param_1;
  (*_DAT_1000_c0cc)();
  pbVar1 = (byte *)(param_2 + 1);
  *pbVar1 = *pbVar1 | 0x20;
  if ((char)*pbVar1 < '\0') {
    puVar15 = &uStack_2;
    if ((*(byte *)(param_2 + 1) & 0x10) != 0) {
      FUN_0000_2c8b();
      puVar15 = puVar14;
    }
    uVar6 = *(undefined2 *)*(undefined2 *)((int)puVar15 + -6);
    uVar7 = *(undefined2 *)*(undefined2 *)((int)puVar15 + -4);
    piVar18 = (int *)0x0;
    uVar10 = 0;
    iVar11 = 0;
    puVar16 = (uint *)0x0;
    while( true ) {
      while( true ) {
        do {
          iVar12 = iVar11;
          uVar13 = uVar10;
          uVar10 = uVar13 - 0x140;
          iVar11 = iVar12 + 1;
        } while (0x13f < uVar13);
        puVar17 = puVar16 + 1;
        uVar10 = *puVar16;
        uVar9 = uVar10 >> 1;
        if ((uVar10 & 1) == 0) break;
        uVar10 = uVar10 >> 2;
        if ((uVar9 & 1) == 0) {
          piVar4 = piVar18;
          piVar18 = piVar18 + 1;
          *piVar4 = uVar10 * 4 + 1;
          uVar10 = uVar13 + uVar10;
          iVar11 = iVar12;
          puVar16 = puVar17;
        }
        else {
          puVar5 = (uint *)(piVar18 + 1);
          *piVar18 = (uVar10 * 2 + 1) * 2 + 1;
          uVar10 = *puVar17;
          piVar18 = piVar18 + 2;
          *puVar5 = uVar10;
          uVar10 = uVar13 + uVar10;
          iVar11 = iVar12;
          puVar16 = puVar16 + 2;
        }
      }
      if (uVar9 == 0) break;
      uVar9 = uVar9 + uVar13;
      if (0x13f < uVar9) {
        *puVar16 = (uVar9 - 0x140) * 2;
        uVar9 = 0x140;
        puVar17 = puVar16;
      }
      puVar16 = puVar17;
      if ((((iVar12 < *(int *)((int)puVar15 + -0x10)) || (*(int *)((int)puVar15 + -0xc) <= iVar12))
          || ((int)uVar9 < *(int *)((int)puVar15 + -0xe))) ||
         (*(int *)((int)puVar15 + -10) <= (int)uVar13)) {
        uVar10 = uVar13 + (uVar9 - uVar13);
        iVar8 = (uVar9 - uVar13) * 2;
        iVar11 = iVar12;
        if (iVar8 != 0) {
          piVar4 = piVar18;
          piVar18 = piVar18 + 1;
          *piVar4 = iVar8;
        }
      }
      else {
        iVar11 = uVar13 - *(int *)((int)puVar15 + -0xe);
        piVar19 = piVar18;
        if ((int)uVar13 < *(int *)((int)puVar15 + -0xe)) {
          piVar19 = piVar18 + 1;
          *piVar18 = iVar11 * -2;
          iVar11 = 0;
        }
        uVar13 = uVar9 - (iVar11 + *(int *)((int)puVar15 + -0xe));
        iVar11 = (iVar11 + *(int *)((int)puVar15 + -0xe)) - *(int *)((int)puVar15 + -10);
        uVar10 = -iVar11;
        if (uVar13 <= uVar10 && -uVar13 != iVar11) {
          uVar10 = uVar13;
        }
        *piVar19 = uVar10 * 4 + 1;
        piVar18 = piVar19 + 2;
        piVar19[1] = 1;
        iVar8 = (uVar13 - uVar10) * 2;
        uVar10 = uVar9;
        iVar11 = iVar12;
        if (iVar8 != 0) {
          piVar4 = piVar18;
          piVar18 = piVar19 + 3;
          *piVar4 = iVar8;
        }
      }
    }
    *piVar18 = 0;
    FUN_0000_21d6(*(undefined2 *)((int)puVar15 + -6),param_4);
  }
  return;
}


/* ===== FUN_0000_23d8 @ 0000:23d8 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_23d8(undefined2 param_1,undefined2 *param_2)

{
  uint *puVar1;
  undefined2 *puVar2;
  uint *puVar3;
  undefined2 *puVar4;
  undefined2 uVar5;
  undefined2 uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined2 *puVar10;
  uint *puVar11;
  undefined2 *puVar12;
  bool bVar13;
  
  _DAT_1000_d584 = _DAT_1000_d584 + 1;
  FUN_0000_2e3f(param_2 + 2);
  uVar6 = _DAT_1000_d56c;
  puVar12 = (undefined2 *)0x0;
  uVar5 = *param_2;
  puVar11 = (uint *)0x0;
  puVar10 = (undefined2 *)param_2[0xd];
  uVar7 = 0;
LAB_0000_23fd:
  do {
    puVar10 = (undefined2 *)((int)puVar10 + uVar7);
    puVar12 = (undefined2 *)((int)puVar12 + uVar7);
    puVar1 = puVar11;
    puVar11 = puVar11 + 1;
    uVar8 = *puVar1 >> 1;
    bVar13 = uVar8 == 0;
    if ((*puVar1 & 1) == 0) goto LAB_0000_2414;
    while (uVar7 = uVar8 >> 1, (uVar8 & 1) != 0) {
      while( true ) {
        puVar10 = (undefined2 *)((int)puVar10 + uVar7);
        puVar3 = puVar11 + 1;
        puVar12 = (undefined2 *)((int)puVar12 + *puVar11);
        puVar11 = puVar11 + 2;
        uVar8 = *puVar3 >> 1;
        bVar13 = uVar8 == 0;
        if ((*puVar3 & 1) != 0) break;
LAB_0000_2414:
        do {
          if (bVar13) {
            FUN_0000_2e8b();
            return;
          }
          uVar7 = (uint)((uVar8 & 1) != 0);
          for (iVar9 = (uVar8 >> 2) * 2 + (uint)((uVar8 >> 1 & 1) != 0); iVar9 != 0;
              iVar9 = iVar9 + -1) {
            puVar4 = puVar12;
            puVar12 = puVar12 + 1;
            puVar2 = puVar10;
            puVar10 = puVar10 + 1;
            *puVar4 = *puVar2;
          }
          for (; uVar7 != 0; uVar7 = uVar7 - 1) {
            puVar4 = puVar12;
            puVar12 = (undefined2 *)((int)puVar12 + 1);
            puVar2 = puVar10;
            puVar10 = (undefined2 *)((int)puVar10 + 1);
            *(undefined1 *)puVar4 = *(undefined1 *)puVar2;
          }
          puVar1 = puVar11;
          puVar11 = puVar11 + 1;
          uVar7 = *puVar1;
          uVar8 = uVar7 >> 1;
          bVar13 = uVar8 == 0;
        } while ((uVar7 & 1) == 0);
        uVar7 = uVar7 >> 2;
        if ((uVar8 & 1) == 0) goto LAB_0000_23fd;
      }
    }
  } while( true );
}


/* ===== FUN_0000_2442 @ 0000:2442 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2442(undefined2 *param_1)

{
  uint *puVar1;
  int *piVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  byte bVar5;
  uint uVar6;
  uint uVar7;
  byte bVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint *puVar12;
  int *piVar13;
  int *piVar14;
  undefined2 *puVar15;
  bool bVar16;
  
  bVar16 = false;
  DAT_1000_d57e = DAT_1000_d57e | 0x22;
  (*_DAT_1000_c0cc)();
  uVar4 = _DAT_1000_d57c;
  piVar13 = (int *)0x0;
  uVar3 = *param_1;
  puVar12 = (uint *)0x0;
  iVar11 = 0;
  iVar10 = 0;
  while( true ) {
    while( true ) {
      puVar1 = puVar12;
      puVar12 = puVar12 + (uint)bVar16 * -2 + 1;
      uVar7 = *puVar1;
      uVar6 = uVar7 >> 1;
      if ((uVar7 & 1) == 0) break;
      uVar7 = uVar7 >> 2;
      if ((uVar6 & 1) == 0) {
        if (uVar7 != 0) {
          if (uVar7 == 1) {
            iVar11 = iVar11 + 1;
            iVar10 = iVar10 + 1;
            piVar2 = piVar13;
            piVar13 = piVar13 + (uint)bVar16 * -2 + 1;
            *piVar2 = 0x4746;
          }
          else {
            iVar11 = iVar11 + uVar7;
            iVar10 = iVar10 + uVar7;
            piVar14 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
            *(undefined1 *)piVar13 = 0xbe;
            piVar13 = piVar14 + (uint)bVar16 * -2 + 1;
            *piVar14 = iVar11;
            piVar14 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
            *(undefined1 *)piVar13 = 0xbf;
            piVar13 = piVar14 + (uint)bVar16 * -2 + 1;
            *piVar14 = iVar10;
          }
        }
      }
      else {
        if (uVar7 != 0) {
          if (uVar7 == 1) {
            iVar11 = iVar11 + 1;
            piVar2 = piVar13;
            piVar13 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
            *(undefined1 *)piVar2 = 0x46;
          }
          else {
            iVar11 = iVar11 + uVar7;
            piVar14 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
            *(undefined1 *)piVar13 = 0xbe;
            piVar13 = piVar14 + (uint)bVar16 * -2 + 1;
            *piVar14 = iVar11;
          }
        }
        puVar1 = puVar12;
        puVar12 = puVar12 + (uint)bVar16 * -2 + 1;
        iVar10 = iVar10 + *puVar1;
        piVar14 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
        *(undefined1 *)piVar13 = 0xbf;
        piVar13 = piVar14 + (uint)bVar16 * -2 + 1;
        *piVar14 = iVar10;
      }
    }
    if (uVar6 == 0) break;
    if (uVar7 >> 2 != 0) {
      bVar8 = (byte)(uVar7 >> 10);
      bVar5 = (byte)(uVar7 >> 2);
      iVar9 = CONCAT11(bVar8,bVar5);
      if (bVar8 == 0) {
        iVar11 = iVar11 + iVar9 * 2;
        iVar10 = iVar10 + iVar9 * 2;
        if (bVar5 < 4) {
          for (; iVar9 != 0; iVar9 = iVar9 + -1) {
            piVar2 = piVar13;
            piVar13 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
            *(undefined1 *)piVar2 = 0xa5;
          }
        }
        else {
          piVar14 = piVar13 + (uint)bVar16 * -2 + 1;
          *piVar13 = CONCAT11(bVar5,0xb1);
          piVar13 = piVar14 + (uint)bVar16 * -2 + 1;
          *piVar14 = -0x5a0d;
        }
      }
      else {
        iVar11 = iVar11 + iVar9 * 2;
        iVar10 = iVar10 + iVar9 * 2;
        *(undefined1 *)piVar13 = 0xb9;
        puVar15 = (undefined2 *)((int)piVar13 + (uint)bVar16 * -4 + 3);
        *(int *)((int)piVar13 + 1) = iVar9;
        piVar13 = puVar15 + (uint)bVar16 * -2 + 1;
        *puVar15 = 0xa5f3;
      }
    }
    bVar16 = bVar16 != false;
    if ((uVar6 & 1) != 0) {
      iVar11 = iVar11 + 1;
      iVar10 = iVar10 + 1;
      piVar2 = piVar13;
      piVar13 = (int *)((int)piVar13 + (uint)bVar16 * -2 + 1);
      *(undefined1 *)piVar2 = 0xa4;
    }
  }
  *piVar13 = 0xcb;
  (*_DAT_1000_c0d4)();
  return;
}


/* ===== FUN_0000_24d3 @ 0000:24d3 ===== */

/* WARNING: Instruction at (ram,0x000024d4) overlaps instruction at (ram,0x000024d3)
    */
/* WARNING: Variable defined which should be unmapped: param_8 */

void __allregs
FUN_0000_24d3(undefined1 param_1,int param_2,int param_3,undefined2 *param_4,uint *param_5,
             undefined1 *param_6,uint param_7,undefined2 param_8)

{
  uint *puVar1;
  int *piVar2;
  char cVar3;
  byte bVar4;
  uint uVar5;
  uint uVar6;
  byte bVar7;
  int iVar8;
  undefined2 **ppuVar9;
  int *piVar10;
  undefined2 *puVar11;
  int *piVar12;
  undefined2 unaff_ES;
  bool bVar13;
  undefined2 *puStack_2;
  
  ppuVar9 = &puStack_2;
  cVar3 = '\x12';
  puStack_2 = param_4;
  do {
    param_4 = param_4 + -1;
    ppuVar9 = ppuVar9 + -1;
    *ppuVar9 = (undefined2 *)*param_4;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  piVar12 = (int *)(param_6 + 1);
  *param_6 = param_1;
  while( true ) {
    bVar13 = (param_7 & 0x400) != 0;
    if ((param_7 & 1) != 0) {
      param_3 = param_3 + 1;
      param_2 = param_2 + 1;
      piVar2 = piVar12;
      piVar12 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
      *(undefined1 *)piVar2 = 0xa4;
    }
    while( true ) {
      puVar1 = param_5;
      param_5 = param_5 + (uint)bVar13 * -2 + 1;
      uVar6 = *puVar1;
      uVar5 = uVar6 >> 1;
      if ((uVar6 & 1) == 0) break;
      uVar6 = uVar6 >> 2;
      if ((uVar5 & 1) == 0) {
        if (uVar6 != 0) {
          if (uVar6 == 1) {
            param_3 = param_3 + 1;
            param_2 = param_2 + 1;
            piVar2 = piVar12;
            piVar12 = piVar12 + (uint)bVar13 * -2 + 1;
            *piVar2 = 0x4746;
          }
          else {
            param_3 = param_3 + uVar6;
            param_2 = param_2 + uVar6;
            piVar10 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
            *(undefined1 *)piVar12 = 0xbe;
            piVar12 = piVar10 + (uint)bVar13 * -2 + 1;
            *piVar10 = param_3;
            piVar10 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
            *(undefined1 *)piVar12 = 0xbf;
            piVar12 = piVar10 + (uint)bVar13 * -2 + 1;
            *piVar10 = param_2;
          }
        }
      }
      else {
        if (uVar6 != 0) {
          if (uVar6 == 1) {
            param_3 = param_3 + 1;
            piVar2 = piVar12;
            piVar12 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
            *(undefined1 *)piVar2 = 0x46;
          }
          else {
            param_3 = param_3 + uVar6;
            piVar10 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
            *(undefined1 *)piVar12 = 0xbe;
            piVar12 = piVar10 + (uint)bVar13 * -2 + 1;
            *piVar10 = param_3;
          }
        }
        puVar1 = param_5;
        param_5 = param_5 + (uint)bVar13 * -2 + 1;
        param_2 = param_2 + *puVar1;
        piVar10 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
        *(undefined1 *)piVar12 = 0xbf;
        piVar12 = piVar10 + (uint)bVar13 * -2 + 1;
        *piVar10 = param_2;
      }
    }
    if (uVar5 == 0) break;
    param_7 = (uint)((param_7 & 0x4000) != 0) * 0x4000 | (uint)bVar13 * 0x400 |
              (uint)((param_7 & 0x200) != 0) * 0x200 | (uint)((param_7 & 0x100) != 0) * 0x100 |
              (uint)((param_7 & 0x10) != 0) * 0x10 | (uint)((uVar5 & 1) != 0);
    if (uVar6 >> 2 != 0) {
      bVar7 = (byte)(uVar6 >> 10);
      bVar4 = (byte)(uVar6 >> 2);
      iVar8 = CONCAT11(bVar7,bVar4);
      if (bVar7 == 0) {
        param_3 = param_3 + iVar8 * 2;
        param_2 = param_2 + iVar8 * 2;
        if (bVar4 < 4) {
          for (; iVar8 != 0; iVar8 = iVar8 + -1) {
            piVar2 = piVar12;
            piVar12 = (int *)((int)piVar12 + (uint)bVar13 * -2 + 1);
            *(undefined1 *)piVar2 = 0xa5;
          }
        }
        else {
          piVar10 = piVar12 + (uint)bVar13 * -2 + 1;
          *piVar12 = CONCAT11(bVar4,0xb1);
          piVar12 = piVar10 + (uint)bVar13 * -2 + 1;
          *piVar10 = -0x5a0d;
        }
      }
      else {
        param_3 = param_3 + iVar8 * 2;
        param_2 = param_2 + iVar8 * 2;
        *(undefined1 *)piVar12 = 0xb9;
        puVar11 = (undefined2 *)((int)piVar12 + (uint)bVar13 * -4 + 3);
        *(int *)((int)piVar12 + 1) = iVar8;
        piVar12 = puVar11 + (uint)bVar13 * -2 + 1;
        *puVar11 = 0xa5f3;
      }
    }
  }
  *piVar12 = 0xcb;
  (*(code *)*(undefined2 *)&DAT_1000_c0d4)();
  return;
}


/* ===== FUN_0000_253c @ 0000:253c ===== */

void __allregs FUN_0000_253c(undefined2 *param_1,char *param_2)

{
  char cVar1;
  undefined2 **ppuVar2;
  char in_CF;
  undefined2 uStack008a;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x1d';
  puStack_2 = param_1;
  do {
    param_1 = param_1 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_1;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_2 = *param_2 + '\x1f' + in_CF;
  uStack008a = 0x2546;
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_2547 @ 0000:2547 ===== */

void __allregs FUN_0000_2547(undefined2 param_1)

{
  undefined2 uVar1;
  
  uVar1 = FUN_0000_2c54();
  FUN_0000_23d8(uVar1,param_1);
  return;
}


/* ===== FUN_0000_2552 @ 0000:2552 ===== */

void __allregs FUN_0000_2552(undefined2 param_1,int param_2)

{
                    /* WARNING: Subroutine does not return */
  FUN_0000_2d6d(param_2 + 4);
}


/* ===== FUN_0000_256f @ 0000:256f ===== */

/* WARNING: Instruction at (ram,0x00002572) overlaps instruction at (ram,0x0000256f)
    */

void __allregs
FUN_0000_256f(uint param_1,undefined2 param_2,undefined2 *param_3,uint *param_4,undefined2 *param_5)

{
  uint *puVar1;
  undefined2 *puVar2;
  char cVar3;
  uint uVar4;
  uint uVar5;
  undefined2 **ppuVar6;
  undefined2 unaff_ES;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar6 = &puStack_2;
  cVar3 = '\f';
  puStack_2 = param_3;
  do {
    param_3 = param_3 + -1;
    ppuVar6 = ppuVar6 + -1;
    *ppuVar6 = (undefined2 *)*param_3;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  do {
    uVar4 = param_1 >> 1;
    if ((param_1 & 1) == 0) {
      if (uVar4 == 0) {
        FUN_0000_2e12();
        return;
      }
      for (uVar5 = param_1 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        puVar2 = param_5;
        param_5 = param_5 + 1;
        *puVar2 = param_2;
      }
      for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
        puVar2 = param_5;
        param_5 = (undefined2 *)((int)param_5 + 1);
        *(char *)puVar2 = (char)param_2;
      }
    }
    else if ((uVar4 & 1) == 0) {
      param_5 = (undefined2 *)((int)param_5 + (param_1 >> 2));
    }
    else {
      puVar1 = param_4;
      param_4 = param_4 + 1;
      param_5 = (undefined2 *)((int)param_5 + *puVar1);
    }
    puVar1 = param_4;
    param_4 = param_4 + 1;
    param_1 = *puVar1;
  } while( true );
}


/* ===== FUN_0000_2595 @ 0000:2595 ===== */

void __allregs FUN_0000_2595(undefined2 param_1,undefined2 *param_2)

{
  char *pcVar1;
  undefined2 uVar2;
  char *pcVar3;
  char cVar4;
  int iVar5;
  char cVar6;
  int iVar7;
  undefined1 uVar8;
  char *pcVar9;
  char *pcVar10;
  bool bVar11;
  
  uVar2 = *param_2;
  iVar7 = 0;
  iVar5 = -0x600;
  pcVar3 = (char *)0x300;
  do {
    while( true ) {
      pcVar9 = pcVar3;
      if (iVar5 == 0) {
        return;
      }
      cVar6 = *pcVar9;
      uVar8 = (undefined1)((uint)iVar7 >> 8);
      pcVar10 = pcVar9 + 1;
      cVar4 = (char)((uint)param_1 >> 8);
      if (-1 < cVar6) break;
      cVar6 = -cVar6;
      iVar7 = CONCAT11(uVar8,cVar6);
      pcVar3 = pcVar10;
      if (-1 < cVar6) {
        iVar7 = CONCAT11(uVar8,cVar6 + '\x01');
        iVar5 = iVar5 - iVar7;
        pcVar3 = pcVar9 + 2;
        if ((char)param_1 == *pcVar10) {
          pcVar9[1] = cVar4;
        }
      }
    }
    iVar7 = CONCAT11(uVar8,cVar6 + '\x01');
    iVar5 = iVar5 - iVar7;
    bVar11 = iVar5 == 0;
code_r0x000025c0:
    do {
      if (iVar7 != 0) {
        iVar7 = iVar7 + -1;
        pcVar1 = pcVar10;
        pcVar10 = pcVar10 + 1;
        bVar11 = (char)param_1 == *pcVar1;
        if (!bVar11) goto code_r0x000025c0;
      }
      pcVar3 = pcVar10;
      if (!bVar11) break;
      pcVar10[-1] = cVar4;
      bVar11 = iVar7 == 0;
    } while (!bVar11);
  } while( true );
}


/* ===== FUN_0000_25d1 @ 0000:25d1 ===== */

void __allregs FUN_0000_25d1(int param_1,int param_2,undefined2 *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  uint *puVar5;
  uint uVar6;
  
  uVar3 = param_2 + *(int *)(param_1 * 2 + 0x7c8);
  uVar6 = 0;
  puVar4 = (uint *)0x0;
  uVar1 = 0;
  do {
    uVar6 = uVar6 + uVar1;
    puVar5 = puVar4;
    while( true ) {
      while( true ) {
        puVar4 = puVar5 + 1;
        uVar1 = *puVar5;
        uVar2 = uVar1 >> 1;
        if ((uVar1 & 1) != 0) break;
        if ((uVar2 == 0) || (uVar3 < uVar6)) {
          return;
        }
        uVar6 = uVar6 + uVar2;
        puVar5 = puVar4;
        if (uVar3 < uVar6) {
          return;
        }
      }
      uVar1 = uVar1 >> 2;
      if ((uVar2 & 1) == 0) break;
      uVar6 = uVar6 + *puVar4;
      puVar5 = puVar5 + 2;
    }
  } while( true );
}


/* ===== FUN_0000_2604 @ 0000:2604 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_2604(char param_1)

{
  undefined2 uVar1;
  
  uVar1 = _DAT_1000_d586;
  LOCK();
  _DAT_1000_d586 = (int)param_1;
  UNLOCK();
  return uVar1;
}


/* ===== FUN_0000_260c @ 0000:260c ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_260c(undefined4 *param_1,int *param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  uVar1 = *(uint *)((int)*param_1 + -4) & _DAT_1000_d586;
  iVar4 = -(uVar1 >> 9);
  iVar2 = -(uVar1 >> 1 & 0x7f);
  iVar3 = iVar2 + param_2[1];
  if (-1 < iVar3) {
    _DAT_1000_d58a = iVar3;
    uVar1 = iVar2 + param_2[3];
    if (uVar1 <= *(uint *)(param_3 + 0xc)) {
      _DAT_1000_d58e = uVar1;
      iVar2 = iVar4 + *param_2;
      if (-1 < iVar2) {
        _DAT_1000_d588 = iVar2;
        uVar1 = iVar4 + param_2[2];
        if (uVar1 <= *(uint *)(param_3 + 10)) {
          _DAT_1000_d58c = uVar1;
          return;
        }
      }
    }
  }
  return;
}


/* ===== FUN_0000_2660 @ 0000:2660 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_2660(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  int iVar1;
  undefined1 in_CF;
  undefined1 in_ZF;
  
  iVar1 = FUN_0000_260c(param_1,param_3,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_26de(0xffff,param_2,param_4,iVar1 + -1);
  }
  return;
}


/* ===== FUN_0000_2678 @ 0000:2678 ===== */

void __allregs
FUN_0000_2678(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_26de(0xffff,*(undefined2 *)0x158a,param_4,*(int *)0x158c + -1);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_26a1 @ 0000:26a1 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_26a1(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_4,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_26de(0,param_3,param_5,param_2);
  }
  return;
}


/* ===== FUN_0000_26b9 @ 0000:26b9 ===== */

void __allregs
FUN_0000_26b9(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_26de(0,*(undefined2 *)0x158a,param_4,*(undefined2 *)0x1588);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_26de @ 0000:26de ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_26de(uint param_1,int param_2,byte *param_3,int param_4)

{
  byte bVar1;
  byte *pbVar2;
  byte *pbVar3;
  undefined2 uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  byte *pbVar10;
  byte *pbVar11;
  undefined2 unaff_ES;
  
  pbVar10 = (byte *)(*(int *)(param_4 * 2 + _DAT_1000_c794) + param_2);
  uVar4 = *_DAT_1000_c724;
  iVar9 = (_DAT_1000_c724[2] ^ param_1) - param_1;
  uVar7 = *(uint *)(param_3 + -4);
  uVar5 = uVar7 >> 8;
  uVar8 = uVar7 & 0xff;
  if (param_3[-2] == 2) {
    do {
      pbVar11 = pbVar10 + *param_3;
      bVar1 = param_3[1];
      for (uVar7 = (uint)(bVar1 >> 1); param_3 = param_3 + 2, uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar2 = pbVar11;
        pbVar11 = pbVar11 + 2;
        *(undefined2 *)pbVar2 = *(undefined2 *)param_3;
      }
      for (uVar7 = (uint)((bVar1 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar3 = pbVar11;
        pbVar11 = pbVar11 + 1;
        pbVar2 = param_3;
        param_3 = param_3 + 1;
        *pbVar3 = *pbVar2;
      }
      pbVar10 = pbVar10 + iVar9;
      uVar5 = uVar5 - 1;
    } while (uVar5 != 0);
    return 0;
  }
  if (((uint)param_3 & (uint)pbVar10 & 1) != 0) {
    do {
      pbVar11 = pbVar10 + 1;
      pbVar2 = param_3;
      param_3 = param_3 + 1;
      *pbVar10 = *pbVar2;
      for (uVar7 = uVar8 - 1 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar3 = pbVar11;
        pbVar11 = pbVar11 + 2;
        pbVar2 = param_3;
        param_3 = param_3 + 2;
        *(undefined2 *)pbVar3 = *(undefined2 *)pbVar2;
      }
      for (uVar7 = (uint)((uVar8 - 1 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar3 = pbVar11;
        pbVar11 = pbVar11 + 1;
        pbVar2 = param_3;
        param_3 = param_3 + 1;
        *pbVar3 = *pbVar2;
      }
      pbVar10 = pbVar11 + (iVar9 - uVar8);
      uVar5 = uVar5 - 1;
    } while (uVar5 != 0);
    return 0;
  }
  do {
    for (uVar6 = uVar8 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
      pbVar3 = pbVar10;
      pbVar10 = pbVar10 + 2;
      pbVar2 = param_3;
      param_3 = param_3 + 2;
      *(undefined2 *)pbVar3 = *(undefined2 *)pbVar2;
    }
    for (uVar6 = (uint)((uVar7 & 1) != 0); uVar6 != 0; uVar6 = uVar6 - 1) {
      pbVar3 = pbVar10;
      pbVar10 = pbVar10 + 1;
      pbVar2 = param_3;
      param_3 = param_3 + 1;
      *pbVar3 = *pbVar2;
    }
    pbVar10 = pbVar10 + (iVar9 - uVar8);
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0);
  return 0;
}


/* ===== FUN_0000_2758 @ 0000:2758 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2758(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  undefined1 in_CF;
  undefined1 in_ZF;
  
  iVar1 = FUN_0000_260c(param_1,param_2,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_27de(0xffff,_DAT_1000_d58e,param_3,iVar1 + -1);
  }
  return;
}


/* ===== FUN_0000_2770 @ 0000:2770 ===== */

void __allregs
FUN_0000_2770(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_27de(0xffff,*(undefined2 *)0x158e,param_4,*(int *)0x158c + -1);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_279d @ 0000:279d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_279d(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_3,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_27de(0,_DAT_1000_d58e,param_4,param_2);
  }
  return;
}


/* ===== FUN_0000_27b5 @ 0000:27b5 ===== */

void __allregs
FUN_0000_27b5(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_27de(0,*(undefined2 *)0x158e,param_4,*(undefined2 *)0x1588);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_27de @ 0000:27de ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs FUN_0000_27de(uint param_1,int param_2,byte *param_3,int param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  undefined2 unaff_ES;
  
  pbVar9 = (byte *)(*(int *)(param_4 * 2 + _DAT_1000_c794) + param_2 + -1);
  uVar3 = *_DAT_1000_c724;
  iVar8 = (_DAT_1000_c724[2] ^ param_1) - param_1;
  uVar6 = *(uint *)(param_3 + -4);
  uVar7 = uVar6 & 0xff;
  uVar5 = uVar7;
  if (param_3[-2] != 2) {
    do {
      do {
        pbVar1 = param_3;
        param_3 = param_3 + 1;
        *pbVar9 = *pbVar1;
        pbVar9 = pbVar9 + -1;
        uVar5 = uVar5 - 1;
      } while (uVar5 != 0);
      pbVar9 = pbVar9 + iVar8 + uVar7;
      cVar4 = (char)(uVar6 >> 8) + -1;
      uVar6 = CONCAT11(cVar4,(char)uVar6);
      uVar5 = uVar7;
    } while (cVar4 != '\0');
    return uVar6;
  }
  uVar6 = uVar6 >> 8;
  do {
    pbVar2 = param_3 + 1;
    pbVar10 = pbVar9 + -(uint)*param_3;
    param_3 = param_3 + 2;
    for (uVar5 = (uint)*pbVar2; uVar5 != 0; uVar5 = uVar5 - 1) {
      pbVar1 = param_3;
      param_3 = param_3 + 1;
      *pbVar10 = *pbVar1;
      pbVar10 = pbVar10 + -1;
    }
    pbVar9 = pbVar9 + iVar8;
    uVar6 = uVar6 - 1;
  } while (uVar6 != 0);
  return 0;
}


/* ===== FUN_0000_283a @ 0000:283a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_283a(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_2,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_286f((undefined1 *)&DAT_1000_d588,param_3);
  }
  return;
}


/* ===== FUN_0000_2852 @ 0000:2852 ===== */

void __allregs
FUN_0000_2852(undefined2 param_1,undefined2 param_2,int *param_3,undefined2 *param_4,int param_5,
             undefined2 param_6,undefined2 param_7,undefined2 param_8)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_4;
  do {
    param_4 = param_4 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_4;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_3 = *param_3 + param_5 + 1;
  FUN_0000_286f(0x1588,param_7);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_286f @ 0000:286f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_286f(int *param_1,byte *param_2)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  int iVar4;
  uint uVar5;
  byte bVar8;
  uint uVar6;
  uint uVar7;
  uint uVar9;
  byte *pbVar10;
  byte *pbVar11;
  undefined2 unaff_ES;
  
  uVar7 = *(uint *)(param_2 + -4);
  bVar8 = (byte)(uVar7 >> 8);
  uVar9 = uVar7 & 0xff;
  uVar5 = (uint)bVar8;
  pbVar10 = (byte *)((*(int *)((((uint)(*param_1 + param_1[2]) >> 1) - (uint)(bVar8 >> 1)) * 2 +
                              _DAT_1000_c794) + ((uint)(param_1[1] + param_1[3]) >> 1)) -
                    (uVar9 >> 1));
  if (param_2[-2] == 2) {
    uVar3 = *_DAT_1000_c724;
    iVar4 = _DAT_1000_c724[2];
    do {
      pbVar11 = pbVar10 + *param_2;
      bVar8 = param_2[1];
      for (uVar7 = (uint)(bVar8 >> 1); param_2 = param_2 + 2, uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar1 = pbVar11;
        pbVar11 = pbVar11 + 2;
        *(undefined2 *)pbVar1 = *(undefined2 *)param_2;
      }
      for (uVar7 = (uint)((bVar8 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
        pbVar2 = pbVar11;
        pbVar11 = pbVar11 + 1;
        pbVar1 = param_2;
        param_2 = param_2 + 1;
        *pbVar2 = *pbVar1;
      }
      pbVar10 = pbVar10 + iVar4;
      uVar5 = uVar5 - 1;
    } while (uVar5 != 0);
    return 0;
  }
  uVar3 = *_DAT_1000_c724;
  iVar4 = _DAT_1000_c724[2];
  if (((uint)param_2 & (uint)pbVar10 & 1) == 0) {
    do {
      for (uVar6 = uVar9 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
        pbVar2 = pbVar10;
        pbVar10 = pbVar10 + 2;
        pbVar1 = param_2;
        param_2 = param_2 + 2;
        *(undefined2 *)pbVar2 = *(undefined2 *)pbVar1;
      }
      for (uVar6 = (uint)((uVar7 & 1) != 0); uVar6 != 0; uVar6 = uVar6 - 1) {
        pbVar2 = pbVar10;
        pbVar10 = pbVar10 + 1;
        pbVar1 = param_2;
        param_2 = param_2 + 1;
        *pbVar2 = *pbVar1;
      }
      pbVar10 = pbVar10 + (iVar4 - uVar9);
      uVar5 = uVar5 - 1;
    } while (uVar5 != 0);
    return 0;
  }
  do {
    pbVar11 = pbVar10 + 1;
    pbVar1 = param_2;
    param_2 = param_2 + 1;
    *pbVar10 = *pbVar1;
    for (uVar7 = uVar9 - 1 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      pbVar2 = pbVar11;
      pbVar11 = pbVar11 + 2;
      pbVar1 = param_2;
      param_2 = param_2 + 2;
      *(undefined2 *)pbVar2 = *(undefined2 *)pbVar1;
    }
    for (uVar7 = (uint)((uVar9 - 1 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      pbVar2 = pbVar11;
      pbVar11 = pbVar11 + 1;
      pbVar1 = param_2;
      param_2 = param_2 + 1;
      *pbVar2 = *pbVar1;
    }
    pbVar10 = pbVar11 + (iVar4 - uVar9);
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0);
  return 0;
}


/* ===== FUN_0000_290c @ 0000:290c ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_290c(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  int iVar1;
  undefined1 in_CF;
  undefined1 in_ZF;
  
  iVar1 = FUN_0000_260c(param_1,param_3,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_298a(0xffff,param_2,param_4,iVar1 + -1);
  }
  return;
}


/* ===== FUN_0000_2924 @ 0000:2924 ===== */

void __allregs
FUN_0000_2924(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_298a(0xffff,*(undefined2 *)0x158a,param_4,*(int *)0x158c + -1);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_294d @ 0000:294d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_294d(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_4,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_298a(0,param_3,param_5,param_2);
  }
  return;
}


/* ===== FUN_0000_2965 @ 0000:2965 ===== */

void __allregs
FUN_0000_2965(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_298a(0,*(undefined2 *)0x158a,param_4,*(undefined2 *)0x1588);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_298a @ 0000:298a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_298a(uint param_1,int param_2,byte *param_3,int param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  byte *pbVar10;
  undefined2 unaff_ES;
  
  pbVar8 = (byte *)(*(int *)(param_4 * 2 + _DAT_1000_c794) + param_2);
  uVar3 = *_DAT_1000_c724;
  iVar7 = (_DAT_1000_c724[2] ^ param_1) - param_1;
  uVar4 = *(uint *)(param_3 + -4) >> 8;
  uVar6 = *(uint *)(param_3 + -4) & 0xff;
  uVar5 = uVar6;
  if (param_3[-2] != 2) {
    do {
      do {
        pbVar9 = pbVar8;
        pbVar1 = param_3;
        param_3 = param_3 + 1;
        if (*pbVar1 != 0) {
          *pbVar9 = *pbVar1;
        }
        uVar5 = uVar5 - 1;
        pbVar8 = pbVar9 + 1;
      } while (uVar5 != 0);
      uVar4 = uVar4 - 1;
      uVar5 = uVar6;
      pbVar8 = pbVar9 + (iVar7 - uVar6) + 1;
    } while (uVar4 != 0);
    return;
  }
  do {
    pbVar9 = param_3 + 1;
    pbVar10 = pbVar8 + *param_3;
    param_3 = param_3 + 2;
    for (uVar5 = (uint)*pbVar9; uVar5 != 0; uVar5 = uVar5 - 1) {
      while( true ) {
        pbVar1 = param_3;
        param_3 = param_3 + 1;
        if (*pbVar1 == 0) break;
        pbVar2 = pbVar10;
        pbVar10 = pbVar10 + 1;
        *pbVar2 = *pbVar1;
        uVar5 = uVar5 - 1;
        if (uVar5 == 0) goto LAB_0000_29eb;
      }
      pbVar10 = pbVar10 + 1;
    }
LAB_0000_29eb:
    pbVar8 = pbVar8 + iVar7;
    uVar4 = uVar4 - 1;
    if (uVar4 == 0) {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_29f4 @ 0000:29f4 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_29f4(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  undefined1 in_CF;
  undefined1 in_ZF;
  
  iVar1 = FUN_0000_260c(param_1,param_2,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_2a7a(0xffff,_DAT_1000_d58e,param_3,iVar1 + -1);
  }
  return;
}


/* ===== FUN_0000_2a0c @ 0000:2a0c ===== */

void __allregs
FUN_0000_2a0c(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_2a7a(0xffff,*(undefined2 *)0x158e,param_4,*(int *)0x158c + -1);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_2a39 @ 0000:2a39 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_2a39(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_3,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_2a7a(0,_DAT_1000_d58e,param_4,param_2);
  }
  return;
}


/* ===== FUN_0000_2a51 @ 0000:2a51 ===== */

void __allregs
FUN_0000_2a51(int *param_1,undefined2 *param_2,int param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_2;
  do {
    param_2 = param_2 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_1 = *param_1 + param_3 + 1;
  FUN_0000_2a7a(0,*(undefined2 *)0x158e,param_4,*(undefined2 *)0x1588);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_2a7a @ 0000:2a7a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2a7a(uint param_1,int param_2,byte *param_3,int param_4)

{
  byte *pbVar1;
  undefined2 uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  byte *pbVar7;
  byte *pbVar8;
  byte *pbVar9;
  undefined2 unaff_ES;
  
  pbVar7 = (byte *)(*(int *)(param_4 * 2 + _DAT_1000_c794) + param_2 + -1);
  uVar2 = *_DAT_1000_c724;
  iVar6 = (_DAT_1000_c724[2] ^ param_1) - param_1;
  uVar3 = *(uint *)(param_3 + -4) >> 8;
  uVar5 = *(uint *)(param_3 + -4) & 0xff;
  uVar4 = uVar5;
  if (param_3[-2] != 2) {
    do {
      do {
        pbVar8 = pbVar7;
        pbVar1 = param_3;
        param_3 = param_3 + 1;
        if (*pbVar1 != 0) {
          *pbVar8 = *pbVar1;
        }
        uVar4 = uVar4 - 1;
        pbVar7 = pbVar8 + -1;
      } while (uVar4 != 0);
      uVar3 = uVar3 - 1;
      uVar4 = uVar5;
      pbVar7 = pbVar8 + iVar6 + uVar5 + -1;
    } while (uVar3 != 0);
    return;
  }
  do {
    pbVar8 = param_3 + 1;
    pbVar9 = pbVar7 + -(uint)*param_3;
    param_3 = param_3 + 2;
    for (uVar4 = (uint)*pbVar8; uVar4 != 0; uVar4 = uVar4 - 1) {
      while( true ) {
        pbVar1 = param_3;
        param_3 = param_3 + 1;
        if (*pbVar1 == 0) break;
        *pbVar9 = *pbVar1;
        pbVar9 = pbVar9 + -1;
        uVar4 = uVar4 - 1;
        if (uVar4 == 0) goto LAB_0000_2ae0;
      }
      pbVar9 = pbVar9 + -1;
    }
LAB_0000_2ae0:
    pbVar7 = pbVar7 + iVar6;
    uVar3 = uVar3 - 1;
    if (uVar3 == 0) {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_2ae9 @ 0000:2ae9 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2ae9(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  undefined1 in_CF;
  undefined1 in_ZF;
  
  FUN_0000_260c(param_1,param_2,_DAT_1000_c724);
  if (!(bool)in_CF) {
    if ((bool)in_ZF) {
                    /* WARNING: Subroutine does not return */
      FUN_0000_2d6d((undefined1 *)&DAT_1000_d588);
    }
    FUN_0000_2b1e((undefined1 *)&DAT_1000_d588,param_3);
  }
  return;
}


/* ===== FUN_0000_2b01 @ 0000:2b01 ===== */

void __allregs
FUN_0000_2b01(undefined2 param_1,undefined2 param_2,int *param_3,undefined2 *param_4,int param_5,
             undefined2 param_6,undefined2 param_7,undefined2 param_8)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_4;
  do {
    param_4 = param_4 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_4;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *param_3 = *param_3 + param_5 + 1;
  FUN_0000_2b1e(0x1588,param_7);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_2b1e @ 0000:2b1e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2b1e(int *param_1,byte *param_2)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined2 uVar3;
  int iVar4;
  byte bVar8;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  byte *pbVar9;
  byte *pbVar10;
  byte *pbVar11;
  undefined2 unaff_ES;
  
  bVar8 = (byte)(*(uint *)(param_2 + -4) >> 8);
  uVar7 = *(uint *)(param_2 + -4) & 0xff;
  uVar6 = (uint)bVar8;
  pbVar9 = (byte *)((*(int *)((((uint)(*param_1 + param_1[2]) >> 1) - (uint)(bVar8 >> 1)) * 2 +
                             _DAT_1000_c794) + ((uint)(param_1[1] + param_1[3]) >> 1)) -
                   (uVar7 >> 1));
  if (param_2[-2] != 2) {
    uVar3 = *_DAT_1000_c724;
    iVar4 = _DAT_1000_c724[2];
    uVar5 = uVar7;
    do {
      do {
        pbVar10 = pbVar9;
        pbVar1 = param_2;
        param_2 = param_2 + 1;
        if (*pbVar1 != 0) {
          *pbVar10 = *pbVar1;
        }
        uVar5 = uVar5 - 1;
        pbVar9 = pbVar10 + 1;
      } while (uVar5 != 0);
      uVar6 = uVar6 - 1;
      uVar5 = uVar7;
      pbVar9 = pbVar10 + (iVar4 - uVar7) + 1;
    } while (uVar6 != 0);
    return;
  }
  uVar3 = *_DAT_1000_c724;
  iVar4 = _DAT_1000_c724[2];
  do {
    pbVar10 = param_2 + 1;
    pbVar11 = pbVar9 + *param_2;
    param_2 = param_2 + 2;
    for (uVar7 = (uint)*pbVar10; uVar7 != 0; uVar7 = uVar7 - 1) {
      while( true ) {
        pbVar1 = param_2;
        param_2 = param_2 + 1;
        if (*pbVar1 == 0) break;
        pbVar2 = pbVar11;
        pbVar11 = pbVar11 + 1;
        *pbVar2 = *pbVar1;
        uVar7 = uVar7 - 1;
        if (uVar7 == 0) goto LAB_0000_2ba1;
      }
      pbVar11 = pbVar11 + 1;
    }
LAB_0000_2ba1:
    pbVar9 = pbVar9 + iVar4;
    uVar6 = uVar6 - 1;
    if (uVar6 == 0) {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_2baa @ 0000:2baa ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2baa(undefined4 *param_1,uint param_2,int param_3,undefined2 param_4)

{
  byte bVar1;
  undefined1 uVar2;
  int iVar3;
  char cVar4;
  char cVar5;
  int iVar6;
  bool bVar7;
  
  iVar6 = (int)*param_1;
  bVar1 = *(byte *)(iVar6 + -4);
  cVar5 = (char)(param_2 >> 8);
  iVar3 = (param_2 & 0xff) * (uint)bVar1;
  do {
    uVar2 = (undefined1)iVar3;
    cVar4 = (char)((uint)iVar3 >> 8) + bVar1;
    iVar3 = CONCAT11(cVar4,uVar2);
    bVar7 = cVar5 != '\0';
    cVar5 = cVar5 + -1;
  } while (bVar7);
  if (*_DAT_1000_c724 == -0x6000) {
                    /* WARNING: Subroutine does not return */
    FUN_0000_2d6d(param_4);
  }
  FUN_0000_2bf3(param_3 + CONCAT11(cVar4 - bVar1,uVar2),param_4,iVar6);
  return;
}


/* ===== FUN_0000_2bdb @ 0000:2bdb ===== */

void __allregs
FUN_0000_2bdb(int param_1,undefined2 param_2,int param_3,undefined2 *param_4,undefined2 param_5,
             undefined2 param_6,undefined2 param_7)

{
  char cVar1;
  undefined2 **ppuVar2;
  undefined2 unaff_SS;
  undefined2 unaff_DS;
  undefined2 *puStack_2;
  
  ppuVar2 = &puStack_2;
  cVar1 = '\x05';
  puStack_2 = param_4;
  do {
    param_4 = param_4 + -1;
    ppuVar2 = ppuVar2 + -1;
    *ppuVar2 = (undefined2 *)*param_4;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  *(int *)(&stack0x0005 + param_1) = *(int *)(&stack0x0005 + param_1) + param_3;
  FUN_0000_2bf3(param_2,param_3,param_6);
  FUN_0000_2e12();
  return;
}


/* ===== FUN_0000_2bf3 @ 0000:2bf3 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2bf3(int param_1,int *param_2,int param_3)

{
  char *pcVar1;
  byte bVar2;
  undefined2 uVar3;
  int iVar4;
  char *pcVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  char *pcVar9;
  char *pcVar10;
  undefined2 unaff_ES;
  
  uVar3 = *_DAT_1000_c724;
  iVar4 = _DAT_1000_c724[2];
  iVar6 = param_2[3] - param_2[1];
  bVar2 = *(byte *)(param_3 + -4);
  pcVar9 = (char *)(param_3 + param_1);
  iVar8 = param_2[2] - *param_2;
  iVar7 = iVar6;
  pcVar5 = (char *)(*(int *)(*param_2 * 2 + _DAT_1000_c794) + param_2[1]);
  do {
    do {
      pcVar10 = pcVar5;
      pcVar1 = pcVar9;
      pcVar9 = pcVar9 + 1;
      if (*pcVar1 != '\0') {
        *pcVar10 = *pcVar1;
      }
      iVar7 = iVar7 + -1;
      pcVar5 = pcVar10 + 1;
    } while (iVar7 != 0);
    pcVar9 = pcVar9 + ((uint)bVar2 - iVar6);
    iVar8 = iVar8 + -1;
    iVar7 = iVar6;
    pcVar5 = pcVar10 + (iVar4 - iVar6) + 1;
  } while (iVar8 != 0);
  return;
}


/* ===== FUN_0000_2c54 @ 0000:2c54 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2c54(void)

{
  uint *puVar1;
  uint uVar2;
  undefined1 *puVar3;
  undefined2 uVar4;
  uint uVar5;
  uint *puVar6;
  undefined1 *puVar7;
  
  uVar4 = _DAT_1000_d56c;
  puVar6 = (uint *)0x0;
  puVar7 = (undefined1 *)0x0;
  uVar5 = (uint)(_DAT_1000_d576 * _DAT_1000_d578) >> 1;
  do {
    puVar1 = puVar6;
    puVar6 = puVar6 + 1;
    uVar2 = *puVar1;
    puVar3 = puVar7 + 1;
    *puVar7 = *(undefined1 *)(ulong)((uVar2 & 0xff) + 0x476);
    puVar7 = puVar7 + 2;
    *puVar3 = *(undefined1 *)(ulong)((uVar2 >> 8) + 0x476);
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0);
  return;
}


/* ===== FUN_0000_2c7b @ 0000:2c7b ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2c7b(void)

{
  _DAT_2000_baca = _DAT_2000_bd98;
  (*_DAT_1000_c03c)();
  return;
}


/* ===== FUN_0000_2c8b @ 0000:2c8b ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2c8b(void)

{
  (*_DAT_1000_c0ec)();
  return;
}


/* ===== FUN_0000_2c92 @ 0000:2c92 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2c92(void)

{
  undefined1 extraout_AH;
  uint uVar1;
  
  _DAT_1000_d5e6 = 0x41a;
  _DAT_1000_d5e8 = 0x41a;
  (*_DAT_1000_c012)();
  DAT_1000_d5d7 = 0;
  uVar1 = FUN_0000_2ccd(CONCAT11(extraout_AH,1));
  uVar1 = FUN_0000_2ccd(uVar1 & 0xff00);
  FUN_0000_2d1f(CONCAT11((char)(uVar1 >> 8),0xc));
  DAT_1000_d5d9 = 0xff;
  return;
}


/* ===== FUN_0000_2cc1 @ 0000:2cc1 ===== */

void __allregs FUN_0000_2cc1(void)

{
  DAT_1000_d5d9 = 0;
  return;
}


/* ===== FUN_0000_2ccd @ 0000:2ccd ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs FUN_0000_2ccd(uint param_1)

{
  byte *pbVar1;
  byte bVar2;
  uint uVar3;
  
  for (; bVar2 = (byte)param_1, 0xf < bVar2; param_1 = param_1 & 0xff00) {
  }
  uVar3 = param_1;
  if (DAT_1000_d5b4 != bVar2) {
    LOCK();
    uVar3 = CONCAT11(bVar2,DAT_1000_d5b4);
    UNLOCK();
    pbVar1 = (byte *)*(undefined2 *)((param_1 & 0xff) << 1);
    _DAT_1000_d5da = (uint)*pbVar1;
    _DAT_1000_d5dc = (uint)pbVar1[1];
    _DAT_1000_d5e0 = (uint)pbVar1[2];
    _DAT_1000_d5de = (uint)pbVar1[3];
    _DAT_1000_d5e2 = *(undefined2 *)(pbVar1 + 4);
    _DAT_1000_d5e4 = *(undefined2 *)(pbVar1 + 6);
    DAT_1000_d5b4 = bVar2;
  }
  return uVar3;
}


/* ===== FUN_0000_2d1f @ 0000:2d1f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_2d1f(undefined2 param_1)

{
  char cVar1;
  bool bVar2;
  
  cVar1 = (char)param_1;
  bVar2 = DAT_1000_d5c2 != cVar1;
  if (bVar2) {
    LOCK();
    UNLOCK();
    DAT_1000_d5b5 = 0xff;
    param_1 = _DAT_1000_d5c2;
    _DAT_1000_d5c2 = CONCAT11(cVar1,cVar1);
  }
  return param_1;
}


/* ===== FUN_0000_2d3d @ 0000:2d3d ===== */

void __allregs FUN_0000_2d3d(void)

{
  DAT_1000_d5d6 = DAT_1000_d5d6 + '\x01';
  if ((DAT_1000_d5d6 == '\0') && (DAT_1000_d5d7 == '\0')) {
    FUN_0000_0e3b();
  }
  return;
}


/* ===== FUN_0000_2d52 @ 0000:2d52 ===== */

void __allregs FUN_0000_2d52(void)

{
  bool bVar1;
  
  bVar1 = false;
  if (DAT_1000_d5d6 == '\0') {
    FUN_0000_2ed7();
    if (!bVar1) {
      FUN_0000_2fd7();
      FUN_0000_0db8();
    }
  }
  DAT_1000_d5d6 = DAT_1000_d5d6 + -1;
  return;
}


/* ===== FUN_0000_2d6d @ 0000:2d6d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2d6d(int *param_1)

{
  bool bVar1;
  
  DAT_1000_d5d6 = DAT_1000_d5d6 + '\x01';
  _DAT_1000_d5ce = (*param_1 - _DAT_1000_d5dc) + _DAT_1000_d5de;
  _DAT_1000_d5d0 = (param_1[1] - _DAT_1000_d5da) + _DAT_1000_d5e0;
  _DAT_1000_d5d2 = param_1[2] + _DAT_1000_d5de;
  _DAT_1000_d5d4 = param_1[3] + _DAT_1000_d5e0;
  bVar1 = false;
  if (DAT_1000_d5d7 == '\0') {
    FUN_0000_2ed7();
    if ((bVar1) && (DAT_1000_d5d6 == '\0')) {
      FUN_0000_0e3b();
    }
  }
  DAT_1000_d5d6 = DAT_1000_d5d6 + -1;
  return;
}


/* ===== FUN_0000_2dbd @ 0000:2dbd ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2dbd(int *param_1)

{
  bool bVar1;
  
  DAT_1000_d5d6 = DAT_1000_d5d6 + '\x01';
  _DAT_1000_d5ce = (*param_1 - _DAT_1000_d5dc) + _DAT_1000_d5de;
  _DAT_1000_d5d0 = (param_1[1] - _DAT_1000_d5da) + _DAT_1000_d5e0;
  _DAT_1000_d5d2 = *param_1 + 1 + _DAT_1000_d5de;
  _DAT_1000_d5d4 = param_1[1] + 1 + _DAT_1000_d5e0;
  bVar1 = false;
  if (DAT_1000_d5d7 == '\0') {
    FUN_0000_2ed7();
    if ((bVar1) && (DAT_1000_d5d6 == '\0')) {
      FUN_0000_0e3b();
    }
  }
  DAT_1000_d5d6 = DAT_1000_d5d6 + -1;
  return;
}


/* ===== FUN_0000_2e12 @ 0000:2e12 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2e12(void)

{
  _DAT_1000_d5ce = 0x7fff;
  _DAT_1000_d5d0 = 0x7fff;
  _DAT_1000_d5d2 = 0x7fff;
  _DAT_1000_d5d4 = 0x7fff;
  if (DAT_1000_d5d7 == '\0') {
    return;
  }
  DAT_1000_d5b5 = 0xff;
  return;
}


/* ===== FUN_0000_2e3f @ 0000:2e3f ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2e3f(int *param_1)

{
  DAT_1000_d5d6 = DAT_1000_d5d6 + '\x01';
  _DAT_1000_d5ce = (*param_1 - _DAT_1000_d5dc) + _DAT_1000_d5de;
  _DAT_1000_d5d0 = (param_1[1] - _DAT_1000_d5da) + _DAT_1000_d5e0;
  _DAT_1000_d5d2 = param_1[2] + _DAT_1000_d5de;
  _DAT_1000_d5d4 = param_1[3] + _DAT_1000_d5e0;
  if ((DAT_1000_d5d7 != '\0') && (DAT_1000_d5d6 == '\0')) {
    FUN_0000_0db8();
  }
  FUN_0000_2ed7();
  return;
}


/* ===== FUN_0000_2e8b @ 0000:2e8b ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2e8b(void)

{
  bool bVar1;
  
  _DAT_1000_d5ce = 0x7fff;
  _DAT_1000_d5d0 = 0x7fff;
  _DAT_1000_d5d2 = 0x7fff;
  _DAT_1000_d5d4 = 0x7fff;
  if (DAT_1000_d5d6 == '\0') {
    if (DAT_1000_d5d7 == '\0') {
      bVar1 = (bool)(DAT_1000_d5d8 & 1);
      DAT_1000_d5d8 = DAT_1000_d5d8 >> 1;
      if (bVar1) {
        DAT_1000_d5d6 = 0xff;
        (*_DAT_1000_d5b0)();
        return;
      }
    }
    else {
      DAT_1000_d5d7 = '\0';
      FUN_0000_0e63();
      bVar1 = (bool)(DAT_1000_d5d8 & 1);
      DAT_1000_d5d8 = DAT_1000_d5d8 >> 1;
      if (bVar1) {
        DAT_1000_d5d6 = DAT_1000_d5d6 + -1;
        (*_DAT_1000_d5b0)();
        return;
      }
    }
  }
  DAT_1000_d5d6 = DAT_1000_d5d6 + -1;
  return;
}


/* ===== FUN_0000_2ed7 @ 0000:2ed7 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2ed7(void)

{
  if ((((_DAT_1000_d5ce <= _DAT_1000_d5b8) && (_DAT_1000_d5b8 < _DAT_1000_d5d2)) &&
      (_DAT_1000_d5d0 <= _DAT_1000_d5ba)) && (_DAT_1000_d5ba < _DAT_1000_d5d4)) {
    DAT_1000_d5d7 = 0xff;
    return;
  }
  DAT_1000_d5d7 = 0;
  return;
}


/* ===== FUN_0000_2f03 @ 0000:2f03 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2f03(uint param_1,undefined2 param_2)

{
  DAT_1000_d5d9 = DAT_1000_d5d9 + '\x01';
  if (DAT_1000_d5d9 == '\0') {
    _DAT_1000_d5b6 = param_2;
    if ((param_1 & 6) != 0) {
      (*_DAT_1000_c3fe)();
    }
    if ((param_1 & 0x18) != 0) {
      (*_DAT_1000_c3fe)();
    }
    if ((param_1 & 0x60) != 0) {
      (*_DAT_1000_c3fe)();
    }
  }
  DAT_1000_d5d9 = DAT_1000_d5d9 + -1;
  return;
}


/* ===== FUN_0000_2fd7 @ 0000:2fd7 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_2fd7(void)

{
  int iVar1;
  int iVar2;
  
  _DAT_1000_d5c0 = _DAT_1000_d5dc;
  _DAT_1000_d5bc = _DAT_1000_d5da;
  _DAT_1000_d5be = 0;
  _DAT_1000_d5c4 = _DAT_1000_d5e2;
  _DAT_1000_d5c6 = _DAT_1000_d5e4;
  iVar1 = _DAT_1000_d5b8 - _DAT_1000_d5de;
  if (_DAT_1000_d5b8 < _DAT_1000_d5de) {
    iVar2 = -iVar1;
    _DAT_1000_d5c0 = _DAT_1000_d5dc + iVar1;
    _DAT_1000_d5c6 = 0;
    do {
      _DAT_1000_d5c6 = _DAT_1000_d5c6 + _DAT_1000_d5da;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    _DAT_1000_d5c4 = _DAT_1000_d5e2 + _DAT_1000_d5c6;
    _DAT_1000_d5c6 = _DAT_1000_d5e4 + _DAT_1000_d5c6;
    iVar1 = 0;
  }
  if (_DAT_1000_c730 <= (uint)(iVar1 + _DAT_1000_d5dc)) {
    _DAT_1000_d5c0 = _DAT_1000_d5c0 - ((iVar1 + _DAT_1000_d5dc) - _DAT_1000_c730);
  }
  iVar2 = _DAT_1000_d5ba - _DAT_1000_d5e0;
  if (_DAT_1000_d5ba < _DAT_1000_d5e0) {
    _DAT_1000_d5be = -iVar2;
    _DAT_1000_d5bc = _DAT_1000_d5da + iVar2;
    _DAT_1000_d5c4 = _DAT_1000_d5c4 + _DAT_1000_d5be;
    _DAT_1000_d5c6 = _DAT_1000_d5c6 + _DAT_1000_d5be;
    iVar2 = 0;
  }
  _DAT_1000_d5c8 = iVar2 + *(int *)(iVar1 * 2 + 0x7c8);
  iVar1 = (iVar2 + _DAT_1000_d5da) - _DAT_1000_c732;
  if (_DAT_1000_c732 <= (uint)(iVar2 + _DAT_1000_d5da)) {
    _DAT_1000_d5bc = _DAT_1000_d5bc - iVar1;
    _DAT_1000_d5be = iVar1;
  }
  return;
}


/* ===== FUN_0000_3068 @ 0000:3068 ===== */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_3068(undefined1 param_1,uint param_2,uint param_3,uint param_4,uint param_5,uint param_6)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  uint unaff_ES;
  
  uVar1 = *(uint *)(_DAT_2000_c1b4 + 0xc);
  if (((((uVar1 <= param_4) || (uVar1 <= param_3)) || (uVar1 <= param_6)) ||
      ((uVar1 = *(uint *)(_DAT_2000_c1b4 + 10), uVar1 <= unaff_ES || (uVar1 <= param_2)))) ||
     (uVar1 <= param_5)) {
    return;
  }
  if (((int)unaff_ES <= (int)param_2) &&
     (uVar1 = unaff_ES, uVar12 = param_2, uVar5 = param_3, uVar6 = param_4, uVar11 = param_5,
     uVar10 = param_6, (int)unaff_ES <= (int)param_5)) goto LAB_0000_30b6;
  uVar1 = param_5;
  uVar12 = unaff_ES;
  uVar5 = param_4;
  uVar6 = param_6;
  uVar11 = param_2;
  uVar10 = param_3;
  if ((int)param_5 < (int)param_2) goto LAB_0000_30b6;
  while( true ) {
    uVar5 = param_6;
    uVar1 = param_2;
    uVar12 = param_5;
    uVar6 = param_3;
    uVar11 = unaff_ES;
    uVar10 = param_4;
LAB_0000_30b6:
    uVar2 = uVar12 - uVar1;
    if (uVar1 <= uVar12 && uVar2 != 0) break;
    if (uVar6 <= uVar5) {
      return;
    }
    param_2 = uVar2 + uVar1;
    unaff_ES = uVar1;
    param_3 = uVar5;
    param_4 = uVar6;
    param_5 = uVar11;
    param_6 = uVar10;
  }
  uVar4 = uVar5 - uVar6;
  if (uVar6 <= uVar5) {
    if ((int)uVar2 <= (int)uVar4) {
      uVar5 = uVar11 - uVar1;
      if (uVar11 < uVar1 || uVar5 == 0) {
        if (uVar6 <= uVar10) {
          iVar3 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
          iVar9 = (uVar10 - uVar6) + iVar3;
          iVar13 = -(uVar4 >> 1);
          uVar1 = uVar11 - uVar12;
          if (uVar11 < uVar12) goto LAB_0000_3714;
        }
      }
      else {
        uVar12 = uVar10 - uVar6;
        if ((uVar6 <= uVar10) && ((int)uVar5 <= (int)uVar12)) {
          iVar3 = (uVar2 - uVar5 & -(uint)(uVar2 < uVar5)) + uVar5;
          iVar9 = -(uVar4 >> 1);
          iVar13 = -(uVar12 >> 1);
          do {
            while (iVar13 = iVar13 + uVar5, -1 < iVar13) {
              if ((int)(iVar9 + uVar2) < 0) {
                return;
              }
              iVar13 = iVar13 - uVar12;
              iVar9 = (iVar9 + uVar2) - uVar4;
              iVar3 = iVar3 + -1;
              if (iVar3 == 0) {
                return;
              }
            }
            iVar9 = iVar9 + uVar2;
          } while (iVar9 < 0);
          iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
          iVar13 = -(uVar4 >> 1);
          iVar7 = -(uVar12 >> 1);
          iVar3 = iVar9;
          FUN_0000_3f38(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
          uVar1 = *(uint *)((int)register0x00000010 + -0xe) -
                  *(uint *)((int)register0x00000010 + -10);
          if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe))
          {
            if (uVar1 == 0) {
              return;
            }
LAB_0000_3874:
            *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
            *(uint *)((int)register0x00000010 + -0x12) = uVar1;
            uVar12 = *(uint *)((int)register0x00000010 + -0x10) -
                     *(uint *)((int)register0x00000010 + -0xc);
            if (*(uint *)((int)register0x00000010 + -0x10) <
                *(uint *)((int)register0x00000010 + -0xc)) {
              uVar12 = -uVar12;
              *(uint *)((int)register0x00000010 + -0x14) = uVar12;
              *(uint *)((int)register0x00000010 + -0x1a) = uVar12;
              if ((int)uVar12 < (int)uVar1) {
                FUN_0000_3bd9(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
                return;
              }
              FUN_0000_3a3a(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
              return;
            }
            *(uint *)((int)register0x00000010 + -0x14) = uVar12;
            *(int *)((int)register0x00000010 + -0x1a) = uVar12 + 1;
            if ((int)uVar12 < (int)uVar1) {
              FUN_0000_3d57(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
              return;
            }
            FUN_0000_3f38(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
            return;
          }
LAB_0000_3714:
          uVar1 = -uVar1;
          *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
          *(uint *)((int)register0x00000010 + -0x16) = uVar1;
          uVar12 = *(uint *)((int)register0x00000010 + -0xc) -
                   *(uint *)((int)register0x00000010 + -0x10);
          if (*(uint *)((int)register0x00000010 + -0xc) < *(uint *)((int)register0x00000010 + -0x10)
             ) {
            uVar12 = -uVar12;
            *(uint *)((int)register0x00000010 + -0x18) = uVar12;
            *(uint *)((int)register0x00000010 + -0x1c) = uVar12;
            if ((int)uVar12 < (int)uVar1) {
              FUN_0000_3e58(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
              return;
            }
            FUN_0000_3dc7(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
            return;
          }
          *(uint *)((int)register0x00000010 + -0x18) = uVar12;
          *(int *)((int)register0x00000010 + -0x1c) = uVar12 + 1;
          if ((int)uVar12 < (int)uVar1) {
            FUN_0000_3ec8(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
            return;
          }
          FUN_0000_3f38(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
          return;
        }
      }
      return;
    }
    uVar5 = uVar11 - uVar1;
    if (uVar11 < uVar1 || uVar5 == 0) {
      if (uVar6 <= uVar10) {
        iVar3 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar9 = (uVar10 - uVar6) + iVar3;
        iVar13 = -(uVar2 >> 1);
        if (uVar11 < uVar12) {
          uVar1 = -(uVar11 - uVar12);
          goto LAB_0000_363c;
        }
      }
    }
    else {
      uVar12 = uVar10 - uVar6;
      if (uVar6 <= uVar10) {
        if ((int)uVar12 < (int)uVar5) {
          iVar3 = (uVar2 - uVar5 & -(uint)(uVar2 < uVar5)) + uVar5;
          iVar9 = -(uVar2 >> 1);
          iVar13 = -(uVar5 >> 1);
          while( true ) {
            while (iVar9 = iVar9 + uVar4, -1 < iVar9) {
              if ((int)(iVar13 + uVar12) < 0) {
                return;
              }
              iVar9 = iVar9 - uVar2;
              iVar13 = (iVar13 + uVar12) - uVar5;
              iVar3 = iVar3 + -1;
              if (iVar3 == 0) {
                return;
              }
            }
            iVar13 = iVar13 + uVar12;
            if (-1 < iVar13) break;
            iVar3 = iVar3 + -1;
            if (iVar3 == 0) {
              return;
            }
          }
          iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
          iVar13 = -(uVar2 >> 1);
          iVar7 = -(uVar5 >> 1);
          iVar3 = iVar9;
          FUN_0000_3d08(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
          uVar1 = *(uint *)((int)register0x00000010 + -0xe) -
                  *(uint *)((int)register0x00000010 + -10);
          if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe))
          {
            if (uVar1 == 0) {
              return;
            }
            goto LAB_0000_381c;
          }
          uVar1 = -uVar1;
        }
        else {
          iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
          iVar13 = -(uVar2 >> 1);
          iVar7 = -(uVar12 >> 1);
          iVar3 = iVar9;
          FUN_0000_3d57(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
          uVar1 = *(uint *)((int)register0x00000010 + -0xe) -
                  *(uint *)((int)register0x00000010 + -10);
          if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe))
          {
            if (uVar1 == 0) {
              return;
            }
            goto LAB_0000_3874;
          }
          uVar1 = -uVar1;
        }
LAB_0000_363c:
        *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
        *(uint *)((int)register0x00000010 + -0x16) = uVar1;
        uVar12 = *(uint *)((int)register0x00000010 + -0xc) -
                 *(uint *)((int)register0x00000010 + -0x10);
        if (*(uint *)((int)register0x00000010 + -0xc) < *(uint *)((int)register0x00000010 + -0x10))
        {
          uVar12 = -uVar12;
          *(uint *)((int)register0x00000010 + -0x18) = uVar12;
          *(uint *)((int)register0x00000010 + -0x1c) = uVar12;
          if ((int)uVar12 < (int)uVar1) {
            FUN_0000_3cb9(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
            return;
          }
          FUN_0000_3c49(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
          return;
        }
        *(uint *)((int)register0x00000010 + -0x18) = uVar12;
        *(int *)((int)register0x00000010 + -0x1c) = uVar12 + 1;
        if ((int)uVar12 < (int)uVar1) {
          FUN_0000_3d08(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
          return;
        }
        FUN_0000_3d57(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
        return;
      }
    }
    return;
  }
  uVar5 = -uVar4;
  if (-uVar2 == uVar4 || (int)uVar5 < (int)uVar2) {
    uVar4 = uVar11 - uVar1;
    if (uVar1 <= uVar11 && uVar4 != 0) {
      uVar12 = uVar10 - uVar6;
      if (uVar10 < uVar6) {
        if (-uVar4 != uVar12 && (int)uVar4 <= (int)-uVar12) {
          return;
        }
        iVar3 = (uVar2 - uVar4 & -(uint)(uVar2 < uVar4)) + uVar4;
        iVar9 = -(uVar2 >> 1);
        iVar13 = -(uVar4 >> 1);
        while( true ) {
          while (iVar13 = iVar13 + -uVar12, -1 < iVar13) {
            if ((int)(iVar9 + uVar5) < 0) {
              return;
            }
            iVar13 = iVar13 - uVar4;
            iVar9 = (iVar9 + uVar5) - uVar2;
            iVar3 = iVar3 + -1;
            if (iVar3 == 0) {
              return;
            }
          }
          iVar9 = iVar9 + uVar5;
          if (-1 < iVar9) break;
          iVar3 = iVar3 + -1;
          if (iVar3 == 0) {
            return;
          }
        }
        iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar13 = -(uVar2 >> 1);
        iVar7 = -(uVar4 >> 1);
        iVar3 = iVar9;
        FUN_0000_3b3b(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
        uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10)
        ;
        if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
          if (uVar1 == 0) {
            return;
          }
LAB_0000_37c4:
          *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
          *(uint *)((int)register0x00000010 + -0x12) = uVar1;
          uVar12 = *(uint *)((int)register0x00000010 + -0x10) -
                   *(uint *)((int)register0x00000010 + -0xc);
          if (*(uint *)((int)register0x00000010 + -0x10) < *(uint *)((int)register0x00000010 + -0xc)
             ) {
            uVar12 = -uVar12;
            *(uint *)((int)register0x00000010 + -0x14) = uVar12;
            *(uint *)((int)register0x00000010 + -0x1a) = uVar12;
            if ((int)uVar12 < (int)uVar1) {
              FUN_0000_3b3b(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
              return;
            }
            FUN_0000_395a(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
            return;
          }
          *(uint *)((int)register0x00000010 + -0x14) = uVar12;
          *(int *)((int)register0x00000010 + -0x1a) = uVar12 + 1;
          if ((int)uVar12 < (int)uVar1) {
            FUN_0000_3cb9(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
            return;
          }
          FUN_0000_3e58(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
          return;
        }
        uVar1 = -uVar1;
      }
      else if ((int)uVar12 < (int)uVar4) {
        iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar13 = -(uVar2 >> 1);
        iVar7 = -(uVar4 >> 1);
        iVar3 = iVar9;
        FUN_0000_3b8a(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
        uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10)
        ;
        if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
          if (uVar1 == 0) {
            return;
          }
          goto LAB_0000_381c;
        }
        uVar1 = -uVar1;
      }
      else {
        iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar13 = -(uVar2 >> 1);
        iVar7 = -(uVar12 >> 1);
        iVar3 = iVar9;
        FUN_0000_3bd9(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
        uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10)
        ;
        if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
          if (uVar1 == 0) {
            return;
          }
          goto LAB_0000_3874;
        }
        uVar1 = -uVar1;
      }
LAB_0000_3518:
      *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
      *(uint *)((int)register0x00000010 + -0x16) = uVar1;
      uVar12 = *(uint *)((int)register0x00000010 + -0xc) -
               *(uint *)((int)register0x00000010 + -0x10);
      if (*(uint *)((int)register0x00000010 + -0xc) < *(uint *)((int)register0x00000010 + -0x10)) {
        uVar12 = -uVar12;
        *(uint *)((int)register0x00000010 + -0x18) = uVar12;
        *(uint *)((int)register0x00000010 + -0x1c) = uVar12;
        if ((int)uVar12 < (int)uVar1) {
          FUN_0000_3b3b(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
          return;
        }
        FUN_0000_3acb(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
        return;
      }
      *(uint *)((int)register0x00000010 + -0x18) = uVar12;
      *(int *)((int)register0x00000010 + -0x1c) = uVar12 + 1;
      if ((int)uVar12 < (int)uVar1) {
        FUN_0000_3b8a(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
        return;
      }
      FUN_0000_3bd9(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
      return;
    }
    if (uVar6 <= uVar10) {
      iVar3 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
      iVar9 = (uVar10 - uVar6) + iVar3;
      iVar13 = -(uVar2 >> 1);
      if (uVar11 < uVar12) {
        uVar1 = -(uVar11 - uVar12);
        goto LAB_0000_3518;
      }
    }
    return;
  }
  uVar8 = uVar11 - uVar1;
  if (uVar1 <= uVar11 && uVar8 != 0) {
    uVar12 = uVar10 - uVar6;
    if (uVar10 < uVar6) {
      uVar11 = -uVar12;
      if (-uVar8 == uVar12 || (int)uVar11 < (int)uVar8) {
        iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar13 = -(uVar5 >> 1);
        iVar7 = -(uVar8 >> 1);
        iVar3 = iVar9;
        FUN_0000_395a(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
        uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10)
        ;
        if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
          if (uVar1 == 0) {
            return;
          }
          goto LAB_0000_37c4;
        }
        uVar1 = -uVar1;
      }
      else {
        iVar3 = (uVar2 - uVar8 & -(uint)(uVar2 < uVar8)) + uVar8;
        iVar9 = -(uVar5 >> 1);
        iVar13 = -(uVar11 >> 1);
        do {
          while (iVar9 = iVar9 + uVar2, -1 < iVar9) {
            if ((int)(iVar13 + uVar8) < 0) {
              return;
            }
            iVar9 = iVar9 + uVar4;
            iVar13 = iVar13 + uVar8 + uVar12;
            iVar3 = iVar3 + -1;
            if (iVar3 == 0) {
              return;
            }
          }
          iVar13 = iVar13 + uVar8;
        } while (iVar13 < 0);
        iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
        iVar13 = -(uVar5 >> 1);
        iVar7 = -(uVar11 >> 1);
        iVar3 = iVar9;
        FUN_0000_38c9(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
        uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10)
        ;
        if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
          if (uVar1 == 0) {
            return;
          }
          *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
          *(uint *)((int)register0x00000010 + -0x12) = uVar1;
          uVar12 = *(uint *)((int)register0x00000010 + -0x10) -
                   *(uint *)((int)register0x00000010 + -0xc);
          if (*(uint *)((int)register0x00000010 + -0xc) <=
              *(uint *)((int)register0x00000010 + -0x10)) {
            *(uint *)((int)register0x00000010 + -0x14) = uVar12;
            *(int *)((int)register0x00000010 + -0x1a) = uVar12 + 1;
            if ((int)uVar12 < (int)uVar1) {
              FUN_0000_3c49(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
              return;
            }
            FUN_0000_3dc7(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
            return;
          }
          uVar12 = -uVar12;
          *(uint *)((int)register0x00000010 + -0x14) = uVar12;
          *(uint *)((int)register0x00000010 + -0x1a) = uVar12;
          if ((int)uVar12 < (int)uVar1) {
            FUN_0000_3acb(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
            return;
          }
          FUN_0000_38c9(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
          return;
        }
        uVar1 = -uVar1;
      }
    }
    else if ((int)uVar12 < (int)uVar8) {
      iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
      iVar13 = -(uVar5 >> 1);
      iVar7 = -(uVar8 >> 1);
      iVar3 = iVar9;
      FUN_0000_39ca(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
      uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10);
      if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
        if (uVar1 == 0) {
          return;
        }
LAB_0000_381c:
        *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
        *(uint *)((int)register0x00000010 + -0x12) = uVar1;
        uVar12 = *(uint *)((int)register0x00000010 + -0x10) -
                 *(uint *)((int)register0x00000010 + -0xc);
        if (*(uint *)((int)register0x00000010 + -0x10) < *(uint *)((int)register0x00000010 + -0xc))
        {
          uVar12 = -uVar12;
          *(uint *)((int)register0x00000010 + -0x14) = uVar12;
          *(uint *)((int)register0x00000010 + -0x1a) = uVar12;
          if ((int)uVar12 < (int)uVar1) {
            FUN_0000_3b8a(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
            return;
          }
          FUN_0000_39ca(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
          return;
        }
        *(uint *)((int)register0x00000010 + -0x14) = uVar12;
        *(int *)((int)register0x00000010 + -0x1a) = uVar12 + 1;
        if ((int)uVar12 < (int)uVar1) {
          FUN_0000_3d08(iVar9,iVar7,register0x00000010,-(uVar1 >> 1),iVar3);
          return;
        }
        FUN_0000_3ec8(iVar9,iVar7,register0x00000010,-(uVar12 >> 1),iVar3);
        return;
      }
      uVar1 = -uVar1;
    }
    else {
      iVar9 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
      iVar13 = -(uVar5 >> 1);
      iVar7 = -(uVar12 >> 1);
      iVar3 = iVar9;
      FUN_0000_3a3a(iVar9,iVar7,&stack0x0000,iVar13,iVar9);
      uVar1 = *(uint *)((int)register0x00000010 + -0xe) - *(uint *)((int)register0x00000010 + -10);
      if (*(uint *)((int)register0x00000010 + -10) <= *(uint *)((int)register0x00000010 + -0xe)) {
        if (uVar1 == 0) {
          return;
        }
        goto LAB_0000_3874;
      }
      uVar1 = -uVar1;
    }
LAB_0000_33a8:
    *(uint *)((int)register0x00000010 + -0x1e) = uVar1;
    *(uint *)((int)register0x00000010 + -0x16) = uVar1;
    uVar12 = *(uint *)((int)register0x00000010 + -0xc) - *(uint *)((int)register0x00000010 + -0x10);
    if (*(uint *)((int)register0x00000010 + -0xc) < *(uint *)((int)register0x00000010 + -0x10)) {
      uVar12 = -uVar12;
      *(uint *)((int)register0x00000010 + -0x18) = uVar12;
      *(uint *)((int)register0x00000010 + -0x1c) = uVar12;
      if ((int)uVar12 < (int)uVar1) {
        FUN_0000_395a(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
        return;
      }
      FUN_0000_38c9(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
      return;
    }
    *(uint *)((int)register0x00000010 + -0x18) = uVar12;
    *(int *)((int)register0x00000010 + -0x1c) = uVar12 + 1;
    if ((int)uVar12 < (int)uVar1) {
      FUN_0000_39ca(iVar9,-(uVar1 >> 1),register0x00000010,iVar13,iVar3);
      return;
    }
    FUN_0000_3a3a(iVar9,-(uVar12 >> 1),register0x00000010,iVar13,iVar3);
    return;
  }
  if (uVar6 <= uVar10) {
    iVar3 = uVar6 + *(int *)(uVar1 * 2 + _DAT_2000_c224);
    iVar9 = (uVar10 - uVar6) + iVar3;
    iVar13 = -(uVar5 >> 1);
    if (uVar11 < uVar12) {
      uVar1 = -(uVar11 - uVar12);
      goto LAB_0000_33a8;
    }
  }
  return;
}


/* ===== FUN_0000_38c9 @ 0000:38c9 ===== */

void __allregs FUN_0000_38c9(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + -1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      puVar9 = (undefined2 *)((int)puVar9 + -1);
      iVar4 = iVar4 + -1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + -1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2) + -1;
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + -1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  return;
}


/* ===== FUN_0000_395a @ 0000:395a ===== */

void __allregs FUN_0000_395a(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + -1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar11 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    puVar9 = (undefined2 *)((int)puVar11 + -1);
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar9 = puVar11 + -1;
    }
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + -1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + -1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_39ca @ 0000:39ca ===== */

void __allregs FUN_0000_39ca(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + -1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar11 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    puVar9 = (undefined2 *)((int)puVar11 + -1);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar9 = puVar11;
    }
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + -1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + -1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3a3a @ 0000:3a3a ===== */

void __allregs FUN_0000_3a3a(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar9 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar9 + -1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar4 = iVar4 + -1;
      param_1 = param_1 + 1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar10 = puVar9;
    for (uVar7 = (uint)(param_1 - (int)puVar9) >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar10;
      puVar10 = puVar10 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((param_1 - (int)puVar9 & 1U) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar10;
      puVar10 = (undefined2 *)((int)puVar10 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    param_5 = (undefined2 *)((int)puVar9 + *(int *)(param_3 + -2) + -1);
    param_1 = (int)puVar10 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar9 = param_5;
    do {
      param_5 = puVar9;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar9 = (undefined2 *)((int)param_5 + -1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar6 = param_1;
    do {
      param_1 = iVar6;
      iVar6 = iVar4 + -1;
      iVar5 = iVar6;
      if (iVar6 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar5 = iVar4;
      iVar4 = iVar6;
      iVar6 = param_1 + 1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar5;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3a9f @ 0000:3a9f ===== */

void __allregs FUN_0000_3a9f(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  undefined2 *puVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  undefined2 unaff_ES;
  
  if (param_1 != 0) {
    iVar3 = param_2;
    do {
      param_2 = iVar3;
      iVar3 = param_1 + -1;
      iVar4 = iVar3;
      if (iVar3 == 0) break;
      param_3 = param_3 + *(int *)(param_4 + -0x16);
      iVar4 = param_1;
      param_1 = iVar3;
      iVar3 = param_2 + 1;
    } while (param_3 < 0);
    *(int *)(param_4 + -0x1c) = iVar4;
  }
  uVar5 = (param_2 - (int)param_5) + 1;
  uVar2 = *(undefined2 *)(param_4 + -4);
  for (uVar6 = uVar5 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
    puVar1 = param_5;
    param_5 = param_5 + 1;
    *puVar1 = uVar2;
  }
  for (uVar5 = (uint)((uVar5 & 1) != 0); uVar5 != 0; uVar5 = uVar5 - 1) {
    puVar1 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar1 = (char)uVar2;
  }
  return;
}


/* ===== FUN_0000_3acb @ 0000:3acb ===== */

void __allregs FUN_0000_3acb(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar5 = param_1;
      iVar4 = iVar4 + -1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      param_1 = iVar5 + -1;
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar7 = (iVar5 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = param_5;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + -1);
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar9 + *(int *)(param_3 + -2) + -2;
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar5 = param_1;
    do {
      param_1 = iVar5;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
      iVar5 = param_1 + -1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3b3b @ 0000:3b3b ===== */

void __allregs FUN_0000_3b3b(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  undefined2 unaff_ES;
  
  do {
    uVar4 = (param_1 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar7 = param_5;
    for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      *puVar2 = uVar3;
    }
    for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = (undefined2 *)((int)puVar7 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    puVar6 = (undefined2 *)((int)puVar7 + -1);
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + -1);
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar6 = puVar7 + -1;
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar6 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  uVar4 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3b8a @ 0000:3b8a ===== */

void __allregs FUN_0000_3b8a(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  undefined2 unaff_ES;
  
  do {
    uVar4 = (param_1 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar7 = param_5;
    for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      *puVar2 = uVar3;
    }
    for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = (undefined2 *)((int)puVar7 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + -1);
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    puVar6 = (undefined2 *)((int)puVar7 + -1);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar6 = puVar7;
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar6 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  uVar4 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3bd9 @ 0000:3bd9 ===== */

void __allregs FUN_0000_3bd9(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar4 = iVar4 + -1;
      param_1 = param_1 + 1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = param_5;
    for (uVar7 = (uint)(param_1 - (int)param_5) >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((param_1 - (int)param_5 & 1U) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + -1);
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar5 = param_1;
    do {
      param_1 = iVar5;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
      iVar5 = param_1 + 1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3c49 @ 0000:3c49 ===== */

void __allregs FUN_0000_3c49(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar5 = param_1;
      iVar4 = iVar4 + -1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      param_1 = iVar5 + -1;
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar7 = (iVar5 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = param_5;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + 1);
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar9 + *(int *)(param_3 + -2) + -2;
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar5 = param_1;
    do {
      param_1 = iVar5;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
      iVar5 = param_1 + -1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3cb9 @ 0000:3cb9 ===== */

void __allregs FUN_0000_3cb9(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  undefined2 unaff_ES;
  
  do {
    uVar4 = (param_1 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar7 = param_5;
    for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      *puVar2 = uVar3;
    }
    for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = (undefined2 *)((int)puVar7 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    puVar6 = (undefined2 *)((int)puVar7 + -1);
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + 1);
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar6 = puVar7 + -1;
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar6 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  uVar4 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3d08 @ 0000:3d08 ===== */

void __allregs FUN_0000_3d08(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  undefined2 unaff_ES;
  
  do {
    uVar4 = (param_1 - (int)param_5) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar7 = param_5;
    for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
      puVar2 = puVar7;
      puVar7 = puVar7 + 1;
      *puVar2 = uVar3;
    }
    for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
      puVar2 = puVar7;
      puVar7 = (undefined2 *)((int)puVar7 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + 1);
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    puVar6 = (undefined2 *)((int)puVar7 + -1);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar6 = puVar7;
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar6 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  uVar4 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3d57 @ 0000:3d57 ===== */

void __allregs FUN_0000_3d57(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar4 = iVar4 + -1;
      param_1 = param_1 + 1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = param_5;
    for (uVar7 = (uint)(param_1 - (int)param_5) >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((param_1 - (int)param_5 & 1U) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1a) = *(int *)(param_3 + -0x1a) + -1;
    param_4 = param_4 + *(int *)(param_3 + -0x14);
    if (-1 < param_4) {
      param_4 = param_4 - *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)param_5 + 1);
    }
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar5 = param_1;
    do {
      param_1 = iVar5;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
      iVar5 = param_1 + 1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3dc7 @ 0000:3dc7 ===== */

void __allregs FUN_0000_3dc7(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + 1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      puVar9 = (undefined2 *)((int)puVar9 + -1);
      iVar4 = iVar4 + -1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + 1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2) + -1;
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + 1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  return;
}


/* ===== FUN_0000_3e58 @ 0000:3e58 ===== */

void __allregs FUN_0000_3e58(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + 1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar11 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    puVar9 = (undefined2 *)((int)puVar11 + -1);
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar9 = puVar11 + -1;
    }
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + 1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + 1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3ec8 @ 0000:3ec8 ===== */

void __allregs FUN_0000_3ec8(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 *puVar10;
  undefined2 *puVar11;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      puVar10 = param_5;
      iVar4 = iVar4 + -1;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      param_5 = (undefined2 *)((int)puVar10 + 1);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    uVar7 = (param_1 - (int)puVar10) + 1;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar11 = puVar10;
    for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
      puVar2 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar11;
      puVar11 = (undefined2 *)((int)puVar11 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    *(int *)(param_3 + -0x1c) = *(int *)(param_3 + -0x1c) + -1;
    param_2 = param_2 + *(int *)(param_3 + -0x18);
    puVar9 = (undefined2 *)((int)puVar11 + -1);
    if (-1 < param_2) {
      param_2 = param_2 - *(int *)(param_3 + -0x16);
      puVar9 = puVar11;
    }
    param_5 = (undefined2 *)((int)puVar10 + *(int *)(param_3 + -2) + 1);
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    puVar10 = param_5;
    do {
      param_5 = puVar10;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar6 = iVar4;
      iVar4 = iVar5;
      puVar10 = (undefined2 *)((int)param_5 + 1);
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  return;
}


/* ===== FUN_0000_3f38 @ 0000:3f38 ===== */

void __allregs FUN_0000_3f38(int param_1,int param_2,int param_3,int param_4,undefined2 *param_5)

{
  int *piVar1;
  undefined2 *puVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined2 *puVar9;
  undefined2 unaff_ES;
  
  do {
    iVar4 = *(int *)(param_3 + -0x1c);
    do {
      iVar4 = iVar4 + -1;
      param_1 = param_1 + 1;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
    } while (param_2 < 0);
    param_2 = param_2 - *(int *)(param_3 + -0x18);
    *(int *)(param_3 + -0x1c) = iVar4;
    uVar3 = *(undefined2 *)(param_3 + -4);
    puVar9 = param_5;
    for (uVar7 = (uint)(param_1 - (int)param_5) >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = puVar9 + 1;
      *puVar2 = uVar3;
    }
    for (uVar7 = (uint)((param_1 - (int)param_5 & 1U) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
      puVar2 = puVar9;
      puVar9 = (undefined2 *)((int)puVar9 + 1);
      *(char *)puVar2 = (char)uVar3;
    }
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      iVar4 = iVar4 + -1;
      param_5 = (undefined2 *)((int)param_5 + 1);
      param_4 = param_4 + *(int *)(param_3 + -0x12);
    } while (param_4 < 0);
    param_4 = param_4 - *(int *)(param_3 + -0x14);
    *(int *)(param_3 + -0x1a) = iVar4;
    param_5 = (undefined2 *)((int)param_5 + *(int *)(param_3 + -2));
    param_1 = (int)puVar9 + *(int *)(param_3 + -2);
    piVar1 = (int *)(param_3 + -0x1e);
    *piVar1 = *piVar1 + -1;
  } while (*piVar1 != 0);
  if (*(int *)(param_3 + -0x1c) != 0) {
    iVar4 = *(int *)(param_3 + -0x1c);
    iVar5 = param_1;
    do {
      param_1 = iVar5;
      iVar5 = iVar4 + -1;
      iVar6 = iVar5;
      if (iVar5 == 0) break;
      param_2 = param_2 + *(int *)(param_3 + -0x16);
      iVar6 = iVar4;
      iVar4 = iVar5;
      iVar5 = param_1 + 1;
    } while (param_2 < 0);
    *(int *)(param_3 + -0x1c) = iVar6;
  }
  uVar7 = (param_1 - (int)param_5) + 1;
  uVar3 = *(undefined2 *)(param_3 + -4);
  for (uVar8 = uVar7 >> 1; uVar8 != 0; uVar8 = uVar8 - 1) {
    puVar2 = param_5;
    param_5 = param_5 + 1;
    *puVar2 = uVar3;
  }
  for (uVar7 = (uint)((uVar7 & 1) != 0); uVar7 != 0; uVar7 = uVar7 - 1) {
    puVar2 = param_5;
    param_5 = (undefined2 *)((int)param_5 + 1);
    *(char *)puVar2 = (char)uVar3;
  }
  if (*(int *)(param_3 + -0x1a) != 0) {
    iVar4 = *(int *)(param_3 + -0x1a);
    do {
      iVar6 = iVar4 + -1;
      iVar5 = iVar6;
      if (iVar6 == 0) break;
      param_4 = param_4 + *(int *)(param_3 + -0x12);
      iVar5 = iVar4;
      iVar4 = iVar6;
    } while (param_4 < 0);
    *(int *)(param_3 + -0x1a) = iVar5;
  }
  return;
}


/* ===== FUN_0000_3fca @ 0000:3fca ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __allregs FUN_0000_3fca(undefined2 param_1,undefined2 *param_2,int *param_3)

{
  int iVar1;
  undefined2 uVar2;
  
  uVar2 = _DAT_2000_c19a;
  *(undefined2 *)&DAT_1000_c578 = param_1;
  *(undefined2 *)&DAT_1000_c57a = param_3;
  *(int *)&DAT_1000_c57c = param_3[1] - *param_3;
  *(undefined2 *)&DAT_1000_c57e = *param_2;
  *(undefined2 *)&DAT_1000_c580 = param_2[1];
  iVar1 = param_2[2];
  *(int *)&DAT_1000_c582 = iVar1;
  *(int *)&DAT_1000_c586 = iVar1 - *(int *)&DAT_1000_c57e;
  iVar1 = param_2[3];
  *(int *)&DAT_1000_c584 = iVar1;
  *(int *)&DAT_1000_c588 = iVar1 - *(int *)&DAT_1000_c580;
  return 0x1c00;
}


/* ===== FUN_0000_400b @ 0000:400b ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_400b(undefined2 param_1,undefined2 param_2)

{
  undefined2 uVar1;
  
  uVar1 = _DAT_2000_c19a;
  *(undefined2 *)&DAT_1000_c58a = param_1;
  *(undefined2 *)&DAT_1000_c58c = param_2;
  return;
}


/* ===== FUN_0000_401a @ 0000:401a ===== */

void __allregs FUN_0000_401a(void)

{
  return;
}


/* ===== FUN_0000_4030 @ 0000:4030 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs FUN_0000_4030(uint param_1)

{
  undefined2 uVar1;
  
  uVar1 = _DAT_2000_c19a;
  if (param_1 < 0xc) {
    if (param_1 != *(uint *)0x5bd) {
      *(uint *)0x5bd = param_1;
      *(undefined2 *)0x58e = 0;
    }
    return param_1;
  }
  return 10;
}


/* ===== FUN_0000_405e @ 0000:405e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_405e(int param_1)

{
  char *pcVar1;
  uint *puVar2;
  uint uVar3;
  undefined2 *puVar4;
  undefined2 uVar5;
  byte bVar6;
  undefined1 uVar7;
  int iVar8;
  uint uVar9;
  uint *puVar10;
  undefined2 *puVar11;
  
  uVar5 = _DAT_2000_c19a;
  LOCK();
  iVar8 = *(int *)0x58e;
  *(int *)0x58e = param_1;
  UNLOCK();
  if (iVar8 != param_1) {
    uVar7 = (undefined1)((uint)(param_1 + 0xff) >> 8);
    puVar11 = (undefined2 *)0x1448;
    for (iVar8 = 0x80; iVar8 != 0; iVar8 = iVar8 + -1) {
      puVar4 = puVar11;
      puVar11 = puVar11 + 1;
      *puVar4 = CONCAT11(uVar7,uVar7);
    }
    bVar6 = -*(char *)0x58e;
    if (bVar6 != 0) {
      puVar10 = (uint *)*(undefined2 *)(*(int *)0x5bd + 0x5bf);
      uVar9 = (uint)(bVar6 >> 1);
      if ((bVar6 & 1) != 0) {
        puVar2 = puVar10;
        puVar10 = (uint *)((int)puVar10 + 1);
        pcVar1 = (char *)((byte)*puVar2 + 0x1448);
        *pcVar1 = *pcVar1 + -1;
        if (uVar9 == 0) {
          return;
        }
      }
      do {
        puVar2 = puVar10;
        puVar10 = puVar10 + 1;
        uVar3 = *puVar2;
        pcVar1 = (char *)((uVar3 & 0xff) + 0x1448);
        *pcVar1 = *pcVar1 + -1;
        pcVar1 = (char *)((uVar3 >> 8) + 0x1448);
        *pcVar1 = *pcVar1 + -1;
        uVar9 = uVar9 - 1;
      } while (uVar9 != 0);
    }
  }
  return;
}


/* ===== FUN_0000_40b6 @ 0000:40b6 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_40b6(undefined2 param_1,int *param_2)

{
  undefined2 uVar1;
  int *piVar2;
  
  uVar1 = _DAT_2000_c19a;
  if (*param_2 != 0) {
    *(undefined2 *)0x576 = param_2;
    *(int *)&DAT_1000_c58a = param_2[1];
    *(int *)&DAT_1000_c58c = param_2[2];
    FUN_0000_4030((uint)*(byte *)(param_2 + 5));
    FUN_0000_405e(param_2[3]);
    piVar2 = (int *)*(undefined2 *)*(undefined2 *)0x576;
    do {
      FUN_0000_410e(CONCAT11((char)piVar2[1],(char)((uint)piVar2[1] >> 8)),
                    CONCAT11((char)piVar2[2],(char)((uint)piVar2[2] >> 8)),(char)piVar2[3],0);
      piVar2 = (int *)*piVar2;
    } while (piVar2 != (int *)0x0);
  }
  return;
}


/* ===== FUN_0000_410a @ 0000:410a ===== */

void __allregs
FUN_0000_410a(undefined2 param_1,undefined2 param_2,undefined1 param_3,undefined2 param_4)

{
  FUN_0000_410e(param_1,param_2,param_3,param_4);
  return;
}


/* ===== FUN_0000_410e @ 0000:410e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000_410e(undefined2 param_1,undefined2 param_2,undefined1 param_3,undefined2 param_4)

{
  undefined2 uVar1;
  undefined2 unaff_ES;
  
  uVar1 = _DAT_2000_c19a;
  *(undefined2 *)&DAT_1000_c590 = param_4;
  *(undefined2 *)0x592 = unaff_ES;
  FUN_0000_4149(param_1,param_2,param_3);
  if ((*(byte *)&DAT_1000_c5a4 & 1) != 0) {
    FUN_0000_453d((int)*(undefined4 *)&DAT_1000_c590);
    return;
  }
  if (0x100 < *(uint *)0x58e) {
    FUN_0000_43e4((int)*(undefined4 *)&DAT_1000_c590);
    return;
  }
  FUN_0000_42b0((int)*(undefined4 *)&DAT_1000_c590);
  return;
}


/* ===== FUN_0000_4149 @ 0000:4149 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_4149(undefined2 param_1,uint param_2,undefined1 param_3)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  byte *pbVar8;
  byte bVar9;
  
  bVar9 = 0;
  DAT_1000_c5a4 = param_3;
  iVar2 = (int)(char)param_1;
  _DAT_1000_c596 = iVar2;
  iVar3 = (int)(char)((uint)param_1 >> 8);
  _DAT_1000_c594 = iVar3;
  uVar4 = param_2 >> 8;
  _DAT_1000_c598 = uVar4;
  uVar5 = param_2 & 0xff;
  _DAT_1000_c59a = uVar5;
  uVar7 = 0;
  uVar6 = 0;
  pbVar8 = (byte *)0x14c8;
  if (iVar3 != 0) {
    if (iVar3 < 0) {
      iVar3 = -iVar3;
      pbVar8 = (byte *)0x14c7;
      uVar6 = 0xffff;
      bVar9 = 1;
    }
    do {
      pbVar1 = pbVar8;
      pbVar8 = pbVar8 + (uint)bVar9 * -2 + 1;
      uVar7 = uVar7 + *pbVar1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  bVar9 = 0;
  pbVar8 = pbVar8 + -uVar6;
  _DAT_1000_c59c = ((uVar7 ^ uVar6) - uVar6) + _DAT_1000_c58a;
  _DAT_1000_c5a0 = _DAT_1000_c59c;
  do {
    pbVar1 = pbVar8;
    pbVar8 = pbVar8 + 1;
    _DAT_1000_c5a0 = _DAT_1000_c5a0 + *pbVar1;
    uVar4 = uVar4 - 1;
  } while (uVar4 != 0);
  uVar6 = 0;
  uVar4 = 0;
  pbVar8 = (byte *)0x14c8;
  if (iVar2 != 0) {
    if (iVar2 < 0) {
      iVar2 = -iVar2;
      pbVar8 = (byte *)0x14c7;
      uVar4 = 0xffff;
      bVar9 = 1;
    }
    do {
      pbVar1 = pbVar8;
      pbVar8 = pbVar8 + (uint)bVar9 * -2 + 1;
      uVar6 = uVar6 + *pbVar1;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  pbVar8 = pbVar8 + -uVar4;
  _DAT_1000_c59e = ((uVar6 ^ uVar4) - uVar4) + _DAT_1000_c58c;
  _DAT_1000_c5a2 = _DAT_1000_c59e;
  do {
    pbVar1 = pbVar8;
    pbVar8 = pbVar8 + 1;
    _DAT_1000_c5a2 = _DAT_1000_c5a2 + (uint)*pbVar1;
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0);
  iVar2 = _DAT_1000_c59e;
  if (_DAT_1000_c588 <= (uint)(_DAT_1000_c584 - _DAT_1000_c59e)) {
    iVar2 = _DAT_1000_c580;
  }
  iVar3 = _DAT_1000_c5a2;
  if (_DAT_1000_c588 <= (uint)(_DAT_1000_c5a2 - _DAT_1000_c580)) {
    iVar3 = _DAT_1000_c584;
  }
  _DAT_1000_c5a6 = _DAT_1000_c59c;
  if (_DAT_1000_c586 <= _DAT_1000_c582 - _DAT_1000_c59c) {
    _DAT_1000_c5a6 = _DAT_1000_c57e;
  }
  _DAT_1000_c5a8 = _DAT_1000_c5a0;
  if (_DAT_1000_c586 <= _DAT_1000_c5a0 - _DAT_1000_c57e) {
    _DAT_1000_c5a8 = _DAT_1000_c582;
  }
  uVar4 = _DAT_1000_c5a2 - _DAT_1000_c59e;
  _DAT_1000_c5ac = iVar2 - _DAT_1000_c59e;
  if (uVar4 <= (uint)(iVar2 - _DAT_1000_c59e)) {
    _DAT_1000_c5ac = uVar4;
  }
  uVar4 = uVar4 - _DAT_1000_c5ac;
  _DAT_1000_c5b0 = _DAT_1000_c5a2 - iVar3;
  if (uVar4 <= (uint)(_DAT_1000_c5a2 - iVar3)) {
    _DAT_1000_c5b0 = uVar4;
  }
  _DAT_1000_c5ae = uVar4 - _DAT_1000_c5b0;
  if (uVar4 - _DAT_1000_c5b0 != 0) {
    uVar4 = _DAT_1000_c5a0 - _DAT_1000_c59c;
    _DAT_1000_c5a6 = _DAT_1000_c5a6 - _DAT_1000_c59c;
    if (uVar4 <= _DAT_1000_c5a6) {
      _DAT_1000_c5a6 = uVar4;
    }
    uVar4 = uVar4 - _DAT_1000_c5a6;
    _DAT_1000_c5aa = _DAT_1000_c5a0 - _DAT_1000_c5a8;
    if (uVar4 <= _DAT_1000_c5a0 - _DAT_1000_c5a8) {
      _DAT_1000_c5aa = uVar4;
    }
    _DAT_1000_c5a8 = uVar4 - _DAT_1000_c5aa;
  }
  return;
}


/* ===== FUN_0000_42b0 @ 0000:42b0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_42b0(undefined1 *param_1)

{
  byte *pbVar1;
  int *piVar2;
  byte bVar3;
  char *pcVar4;
  char *pcVar5;
  byte bVar6;
  byte bVar7;
  uint uVar8;
  uint uVar9;
  char cVar10;
  char cVar11;
  char cVar12;
  byte *pbVar13;
  byte *pbVar14;
  int iVar15;
  char *pcVar16;
  char *pcVar17;
  undefined2 unaff_ES;
  undefined2 uVar18;
  bool bVar19;
  
  uVar18 = 0x1c00;
  if ((_DAT_1000_c5ae == 0) || (_DAT_1000_c5a8 == 0)) {
    return;
  }
  _DAT_1000_c5b2 = _DAT_1000_c59e + _DAT_1000_c5ac;
  _DAT_1000_cc1a = 0;
  _DAT_1000_c5ba = (byte *)(_DAT_1000_c594 + 0x14c8);
  uVar9 = _DAT_1000_c5a6;
  while( true ) {
    pbVar1 = _DAT_1000_c5ba;
    _DAT_1000_c5ba = _DAT_1000_c5ba + 1;
    bVar19 = uVar9 < *pbVar1;
    uVar9 = uVar9 - *pbVar1;
    if (bVar19) break;
    param_1 = param_1 + (byte)param_1[1] + 2;
  }
  _DAT_1000_c5b4 = *(undefined2 *)((_DAT_1000_c59c + _DAT_1000_c5a6) * 2 + _DAT_1000_c57a);
  _DAT_1000_c5b8 = _DAT_1000_c5a8;
  _DAT_1000_c590 = param_1;
  do {
    *(undefined1 *)&DAT_1000_cc1a = *param_1;
    pcVar16 = param_1 + 2;
    *(undefined1 *)&DAT_1000_cc1b = param_1[1];
    pbVar13 = (byte *)(*(int *)&DAT_1000_c596 + 0x14c8);
    uVar9 = *(uint *)&DAT_1000_c5ac;
    pbVar14 = pbVar13;
    do {
      bVar6 = *pbVar14;
      uVar8 = (uint)bVar6;
      pbVar14 = pbVar14 + 1;
      bVar19 = uVar8 <= uVar9;
      uVar9 = uVar9 - uVar8;
    } while (bVar19);
    bVar7 = ~((char)pbVar13 - (char)pbVar14);
    pbVar1 = (byte *)&DAT_1000_cc1a;
    bVar3 = *pbVar1;
    *pbVar1 = *pbVar1 - bVar7;
    if (bVar3 < bVar7) {
      LOCK();
      bVar3 = *(byte *)&DAT_1000_cc1a;
      *(byte *)&DAT_1000_cc1a = 0;
      UNLOCK();
      pcVar16 = pcVar16 + -(int)(char)bVar3;
      pbVar1 = (byte *)&DAT_1000_cc1b;
      bVar7 = *pbVar1;
      *pbVar1 = *pbVar1 + bVar3;
      if (CARRY1(bVar7,bVar3)) goto LAB_0000_436a;
    }
    else {
LAB_0000_436a:
      cVar10 = (char)*(undefined2 *)&DAT_1000_cc1a;
      cVar11 = (char)((uint)*(undefined2 *)&DAT_1000_cc1a >> 8);
      pcVar17 = (char *)(*(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c5b2);
      iVar15 = *(int *)&DAT_1000_c5ae;
      uVar18 = *(undefined2 *)&DAT_1000_c578;
      cVar12 = cVar11 + '\x01';
      if (cVar10 == '\0') {
        if (cVar11 == '\0') goto LAB_0000_43c9;
        pcVar4 = pcVar16;
        pcVar16 = pcVar16 + 1;
        cVar10 = '\x01';
        cVar12 = cVar11;
        if (*pcVar4 != '\0') {
          uVar8 = (uint)(byte)(bVar6 - 1);
          pcVar5 = pcVar17;
          pcVar17 = pcVar17 + 1;
          *pcVar5 = *pcVar4;
        }
      }
      pcVar17 = pcVar17 + uVar8;
      while (cVar10 = cVar10 + -1, cVar10 != '\0') {
        while( true ) {
          pbVar1 = pbVar14;
          pbVar14 = pbVar14 + 1;
          if (*pbVar1 == 0) break;
          iVar15 = iVar15 + -1;
          if (iVar15 == 0) goto LAB_0000_43c9;
          pcVar17 = pcVar17 + 1;
          cVar10 = cVar10 + -1;
          if (cVar10 == '\0') goto LAB_0000_43ae;
        }
      }
LAB_0000_43ae:
      uVar9 = 0;
LAB_0000_43b0:
      pcVar17 = pcVar17 + uVar9;
      cVar12 = cVar12 + -1;
      if (cVar12 != '\0') {
        while( true ) {
          pcVar4 = pcVar16;
          pcVar16 = pcVar16 + 1;
          uVar9 = (uint)*pbVar14;
          pbVar14 = pbVar14 + 1;
          if (uVar9 == 0) break;
          iVar15 = iVar15 + -1;
          if (iVar15 == 0) goto LAB_0000_43c9;
          if (*pcVar4 == '\0') break;
          pcVar5 = pcVar17;
          pcVar17 = pcVar17 + 1;
          *pcVar5 = *pcVar4;
          cVar12 = cVar12 + -1;
          if (cVar12 == '\0') goto LAB_0000_43c9;
        }
        goto LAB_0000_43b0;
      }
    }
LAB_0000_43c9:
    uVar18 = _DAT_2000_c19a;
    unaff_ES = *(undefined2 *)0x592;
    *(int *)&DAT_1000_c5b4 = *(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c57c;
    piVar2 = (int *)&DAT_1000_c5b8;
    *piVar2 = *piVar2 + -1;
    if (*piVar2 == 0) {
      return;
    }
    pcVar16 = (char *)*(undefined2 *)&DAT_1000_c5ba;
    param_1 = (undefined1 *)*(undefined2 *)&DAT_1000_c590;
    do {
      param_1 = param_1 + (byte)param_1[1] + 2;
      pcVar4 = pcVar16;
      pcVar16 = pcVar16 + 1;
    } while (*pcVar4 == '\0');
    *(undefined2 *)&DAT_1000_c5ba = pcVar16;
    *(undefined2 *)&DAT_1000_c590 = param_1;
  } while( true );
}


/* ===== FUN_0000_43e4 @ 0000:43e4 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_43e4(int param_1)

{
  byte *pbVar1;
  char *pcVar2;
  int *piVar3;
  byte bVar4;
  char cVar5;
  undefined2 uVar6;
  byte bVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  char cVar12;
  char cVar13;
  char cVar14;
  char *pcVar15;
  byte *pbVar16;
  byte *pbVar17;
  undefined1 *puVar18;
  char *pcVar19;
  undefined2 unaff_ES;
  undefined2 uVar20;
  bool bVar21;
  
  uVar20 = 0x1c00;
  if ((_DAT_1000_c5ae == 0) || (_DAT_1000_c5a8 == 0)) {
    return;
  }
  _DAT_1000_c5b2 = _DAT_1000_c59e + _DAT_1000_c5ac;
  _DAT_1000_cc1a = 0;
  _DAT_1000_c5ba = (byte *)(_DAT_1000_c594 + 0x14c8);
  uVar8 = _DAT_1000_c5a6;
  while( true ) {
    pbVar1 = _DAT_1000_c5ba;
    _DAT_1000_c5ba = _DAT_1000_c5ba + 1;
    bVar21 = uVar8 < *pbVar1;
    uVar8 = uVar8 - *pbVar1;
    if (bVar21) break;
    param_1 = param_1 + 2 + (uint)*(byte *)(param_1 + 1);
  }
  DAT_1000_c5bc = -(char)uVar8;
  _DAT_1000_c5b4 = *(undefined2 *)((_DAT_1000_c59c + _DAT_1000_c5a6) * 2 + _DAT_1000_c57a);
  _DAT_1000_c5b8 = _DAT_1000_c5a8;
  _DAT_1000_c590 = param_1;
  do {
    puVar18 = (undefined1 *)*(undefined2 *)&DAT_1000_c590;
    pcVar2 = (char *)&DAT_1000_c5bc;
    cVar12 = *pcVar2;
    *pcVar2 = *pcVar2 + -1;
    if (cVar12 == '\0') {
      pcVar15 = (char *)*(undefined2 *)&DAT_1000_c5ba;
      do {
        puVar18 = puVar18 + (byte)puVar18[1] + 2;
        pcVar2 = pcVar15;
        pcVar15 = pcVar15 + 1;
      } while (*pcVar2 == '\0');
      *(char *)&DAT_1000_c5bc = *pcVar2 + -1;
      *(undefined2 *)&DAT_1000_c5ba = pcVar15;
      *(undefined2 *)&DAT_1000_c590 = puVar18;
    }
    *(undefined1 *)&DAT_1000_cc1a = *puVar18;
    pcVar15 = puVar18 + 2;
    *(undefined1 *)&DAT_1000_cc1b = puVar18[1];
    pbVar16 = (byte *)(*(int *)&DAT_1000_c596 + 0x14c8);
    uVar8 = *(uint *)&DAT_1000_c5ac;
    pbVar17 = pbVar16;
    do {
      pbVar1 = pbVar17;
      pbVar17 = pbVar17 + 1;
      bVar21 = *pbVar1 <= uVar8;
      uVar8 = uVar8 - *pbVar1;
    } while (bVar21);
    bVar7 = ~((char)pbVar16 - (char)pbVar17);
    pbVar1 = (byte *)&DAT_1000_cc1a;
    bVar4 = *pbVar1;
    *pbVar1 = *pbVar1 - bVar7;
    if (bVar4 < bVar7) {
      LOCK();
      bVar4 = *(byte *)&DAT_1000_cc1a;
      *(byte *)&DAT_1000_cc1a = 0;
      UNLOCK();
      pcVar15 = pcVar15 + -(int)(char)bVar4;
      pbVar1 = (byte *)&DAT_1000_cc1b;
      bVar7 = *pbVar1;
      *pbVar1 = *pbVar1 + bVar4;
      if (CARRY1(bVar7,bVar4)) goto LAB_0000_44ae;
    }
    else {
LAB_0000_44ae:
      uVar9 = -uVar8;
      cVar12 = (char)*(undefined2 *)&DAT_1000_cc1a;
      cVar13 = (char)((uint)*(undefined2 *)&DAT_1000_cc1a >> 8);
      pcVar19 = (char *)(*(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c5b2);
      uVar6 = *(undefined2 *)&DAT_1000_c578;
      uVar8 = *(uint *)&DAT_1000_c5ae + uVar8;
      if (*(uint *)&DAT_1000_c5ae < uVar9) {
        uVar9 = uVar9 + uVar8;
        uVar8 = 0;
      }
      uVar10 = uVar9;
      cVar14 = cVar13 + '\x01';
      if (cVar12 == '\0') {
        if (cVar13 == '\0') goto LAB_0000_4522;
        pcVar2 = pcVar15;
        pcVar15 = pcVar15 + 1;
        cVar5 = *pcVar2;
        cVar12 = '\x01';
        cVar14 = cVar13;
        if (cVar5 != '\0') {
          for (; uVar10 = 0, uVar9 != 0; uVar9 = uVar9 - 1) {
            pcVar2 = pcVar19;
            pcVar19 = pcVar19 + 1;
            *pcVar2 = cVar5;
          }
        }
      }
      pcVar19 = pcVar19 + uVar10;
      uVar9 = 0;
      do {
        pcVar19 = pcVar19 + uVar9;
        cVar12 = cVar12 + -1;
        if (cVar12 == '\0') goto LAB_0000_4500;
        uVar9 = (uint)*pbVar17;
        pbVar17 = pbVar17 + 1;
        bVar21 = uVar9 <= uVar8;
        uVar8 = uVar8 - uVar9;
      } while (bVar21);
      iVar11 = uVar9 + uVar8;
      uVar8 = 0;
      pcVar19 = pcVar19 + iVar11;
LAB_0000_4500:
      uVar9 = 0;
      while( true ) {
        pcVar19 = pcVar19 + uVar9;
        cVar14 = cVar14 + -1;
        if (cVar14 == '\0') break;
        uVar9 = (uint)*pbVar17;
        pbVar17 = pbVar17 + 1;
        bVar21 = uVar8 < uVar9;
        uVar8 = uVar8 - uVar9;
        if (bVar21) {
          iVar11 = uVar9 + uVar8;
          cVar12 = *pcVar15;
          if (cVar12 != '\0') {
            for (; iVar11 != 0; iVar11 = iVar11 + -1) {
              pcVar2 = pcVar19;
              pcVar19 = pcVar19 + 1;
              *pcVar2 = cVar12;
            }
          }
          break;
        }
        pcVar2 = pcVar15;
        pcVar15 = pcVar15 + 1;
        cVar12 = *pcVar2;
        if (cVar12 != '\0') {
          for (; uVar9 != 0; uVar9 = uVar9 - 1) {
            pcVar2 = pcVar19;
            pcVar19 = pcVar19 + 1;
            *pcVar2 = cVar12;
          }
          uVar9 = 0;
        }
      }
    }
LAB_0000_4522:
    uVar20 = _DAT_2000_c19a;
    unaff_ES = *(undefined2 *)0x592;
    *(int *)&DAT_1000_c5b4 = *(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c57c;
    piVar3 = (int *)&DAT_1000_c5b8;
    *piVar3 = *piVar3 + -1;
    if (*piVar3 == 0) {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_453d @ 0000:453d ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000_453d(int param_1)

{
  byte *pbVar1;
  char *pcVar2;
  int *piVar3;
  byte bVar4;
  char cVar5;
  undefined2 uVar6;
  byte bVar7;
  uint uVar8;
  uint uVar9;
  char cVar10;
  char cVar11;
  char cVar12;
  char *pcVar13;
  byte *pbVar14;
  byte *pbVar15;
  undefined1 *puVar16;
  char *pcVar17;
  undefined2 unaff_ES;
  undefined2 uVar18;
  bool bVar19;
  
  uVar18 = 0x1c00;
  if ((_DAT_1000_c5ae == 0) || (_DAT_1000_c5a8 == 0)) {
    return;
  }
  _DAT_1000_c5b2 = (_DAT_1000_c5a2 + -1) - _DAT_1000_c5b0;
  _DAT_1000_cc1a = 0;
  _DAT_1000_c5ba = (byte *)(_DAT_1000_c594 + 0x14c8);
  uVar8 = _DAT_1000_c5a6;
  while( true ) {
    pbVar1 = _DAT_1000_c5ba;
    _DAT_1000_c5ba = _DAT_1000_c5ba + 1;
    bVar19 = uVar8 < *pbVar1;
    uVar8 = uVar8 - *pbVar1;
    if (bVar19) break;
    param_1 = param_1 + 2 + (uint)*(byte *)(param_1 + 1);
  }
  DAT_1000_c5bc = -(char)uVar8;
  _DAT_1000_c5b4 = *(undefined2 *)((_DAT_1000_c59c + _DAT_1000_c5a6) * 2 + _DAT_1000_c57a);
  _DAT_1000_c5b8 = _DAT_1000_c5a8;
  _DAT_1000_c590 = param_1;
  do {
    puVar16 = (undefined1 *)*(undefined2 *)&DAT_1000_c590;
    pcVar2 = (char *)&DAT_1000_c5bc;
    cVar10 = *pcVar2;
    *pcVar2 = *pcVar2 + -1;
    if (cVar10 == '\0') {
      pcVar13 = (char *)*(undefined2 *)&DAT_1000_c5ba;
      do {
        puVar16 = puVar16 + (byte)puVar16[1] + 2;
        pcVar2 = pcVar13;
        pcVar13 = pcVar13 + 1;
      } while (*pcVar2 == '\0');
      *(char *)&DAT_1000_c5bc = *pcVar2 + -1;
      *(undefined2 *)&DAT_1000_c5ba = pcVar13;
      *(undefined2 *)&DAT_1000_c590 = puVar16;
    }
    *(undefined1 *)&DAT_1000_cc1a = *puVar16;
    pcVar13 = puVar16 + 2;
    *(undefined1 *)&DAT_1000_cc1b = puVar16[1];
    pbVar14 = (byte *)(*(int *)&DAT_1000_c59a + 0x14c7 + *(int *)&DAT_1000_c596);
    uVar8 = *(uint *)&DAT_1000_c5b0;
    pbVar15 = pbVar14;
    do {
      pbVar1 = pbVar15;
      pbVar15 = pbVar15 + -1;
      bVar19 = *pbVar1 <= uVar8;
      uVar8 = uVar8 - *pbVar1;
    } while (bVar19);
    bVar7 = ((char)pbVar14 - (char)pbVar15) - 1;
    pbVar1 = (byte *)&DAT_1000_cc1a;
    bVar4 = *pbVar1;
    *pbVar1 = *pbVar1 - bVar7;
    if (bVar4 < bVar7) {
      LOCK();
      bVar4 = *(byte *)&DAT_1000_cc1a;
      *(byte *)&DAT_1000_cc1a = 0;
      UNLOCK();
      pcVar13 = pcVar13 + -(int)(char)bVar4;
      pbVar1 = (byte *)&DAT_1000_cc1b;
      bVar7 = *pbVar1;
      *pbVar1 = *pbVar1 + bVar4;
      if (CARRY1(bVar7,bVar4)) goto LAB_0000_4609;
    }
    else {
LAB_0000_4609:
      uVar9 = -uVar8;
      cVar10 = (char)*(undefined2 *)&DAT_1000_cc1a;
      cVar11 = (char)((uint)*(undefined2 *)&DAT_1000_cc1a >> 8);
      pcVar17 = (char *)(*(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c5b2);
      uVar6 = *(undefined2 *)&DAT_1000_c578;
      uVar8 = *(uint *)&DAT_1000_c5ae + uVar8;
      if (*(uint *)&DAT_1000_c5ae < uVar9) {
        uVar9 = uVar9 + uVar8;
        uVar8 = 0;
      }
      cVar12 = cVar11 + '\x01';
      if (cVar10 == '\0') {
        if (cVar11 == '\0') goto LAB_0000_467b;
        pcVar2 = pcVar13;
        pcVar13 = pcVar13 + 1;
        cVar5 = *pcVar2;
        cVar10 = '\x01';
        cVar12 = cVar11;
        if (cVar5 != '\0') {
          for (; uVar9 != 0; uVar9 = uVar9 - 1) {
            pcVar2 = pcVar17;
            pcVar17 = pcVar17 + -1;
            *pcVar2 = cVar5;
          }
          uVar9 = 0;
        }
      }
      pcVar17 = pcVar17 + -uVar9;
      do {
        cVar10 = cVar10 + -1;
        if (cVar10 == '\0') break;
        uVar9 = CONCAT11((char)(uVar9 >> 8),*pbVar15);
        pbVar15 = pbVar15 + -1;
        bVar19 = uVar8 < uVar9;
        uVar8 = uVar8 - uVar9;
        if (bVar19) {
          uVar9 = uVar9 + uVar8;
          uVar8 = 0;
        }
        pcVar17 = pcVar17 + -uVar9;
      } while (uVar8 != 0);
      do {
        cVar12 = cVar12 + -1;
        if (cVar12 == '\0') break;
        uVar9 = CONCAT11((char)(uVar9 >> 8),*pbVar15);
        pbVar15 = pbVar15 + -1;
        bVar19 = uVar8 < uVar9;
        uVar8 = uVar8 - uVar9;
        if (bVar19) {
          uVar9 = uVar9 + uVar8;
          uVar8 = 0;
        }
        pcVar2 = pcVar13;
        pcVar13 = pcVar13 + 1;
        cVar10 = *pcVar2;
        if (cVar10 != '\0') {
          for (; uVar9 != 0; uVar9 = uVar9 - 1) {
            pcVar2 = pcVar17;
            pcVar17 = pcVar17 + -1;
            *pcVar2 = cVar10;
          }
          uVar9 = 0;
        }
        pcVar17 = pcVar17 + -uVar9;
      } while (uVar8 != 0);
    }
LAB_0000_467b:
    uVar18 = _DAT_2000_c19a;
    unaff_ES = *(undefined2 *)0x592;
    *(int *)&DAT_1000_c5b4 = *(int *)&DAT_1000_c5b4 + *(int *)&DAT_1000_c57c;
    piVar3 = (int *)&DAT_1000_c5b8;
    *piVar3 = *piVar3 + -1;
    if (*piVar3 == 0) {
      return;
    }
  } while( true );
}


/* ===== FUN_0000_4a69 @ 0000:4a69 ===== */

/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_4a69(undefined2 param_1,int param_2,int param_3,int param_4,int param_5,uint *param_6,
             int param_7)

{
  byte *pbVar1;
  char *pcVar2;
  undefined1 *puVar3;
  uint *puVar4;
  undefined1 *puVar5;
  char cVar6;
  byte bVar7;
  char cVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  byte bVar14;
  byte bVar15;
  char cVar17;
  byte bVar18;
  byte bVar19;
  int iVar16;
  byte bVar20;
  byte bVar21;
  int iVar22;
  int iVar23;
  byte bVar25;
  uint *puVar24;
  undefined1 *puVar26;
  uint uVar27;
  uint uVar28;
  uint *puVar29;
  undefined2 unaff_ES;
  bool bVar30;
  bool bVar31;
  bool bVar32;
  byte in_AF;
  undefined1 auStack_5 [5];
  
  iVar22 = param_4 + -1;
  *(byte *)(iVar22 + (int)param_6) = *(byte *)(iVar22 + (int)param_6) & (byte)((uint)param_1 >> 8);
  *(byte *)((int)param_6 + 0x41) = *(byte *)((int)param_6 + 0x41) + (char)param_3;
  bVar7 = (byte)((uint)iVar22 >> 8);
  pcVar2 = (char *)(param_5 + param_7 + 0x49);
  *pcVar2 = *pcVar2 + (char)param_3;
  *(byte *)(iVar22 + (int)param_6) = *(byte *)(iVar22 + (int)param_6) & bVar7;
  *(byte *)(iVar22 + (int)param_6) = *(byte *)(iVar22 + (int)param_6) & bVar7;
  puVar3 = (undefined1 *)(iVar22 + param_7 + 0x52);
  *puVar3 = *puVar3;
  bVar7 = (byte)((uint)&stack0x0000 >> 8);
  *(byte *)(iVar22 + (int)param_6) = *(byte *)(iVar22 + (int)param_6) & bVar7;
  uVar28 = param_5 - 2;
  pbVar1 = (byte *)((int)param_6 + param_5 + 0x3f);
  *pbVar1 = *pbVar1;
  iVar23 = param_4 + -1;
  *(char *)(iVar23 + param_7) = *(char *)(iVar23 + param_7) + bVar7;
  *(uint *)(iVar23 + param_7) = *(uint *)(iVar23 + param_7) & (uint)&stack0x0000;
  *(uint *)(iVar23 + param_7) = *(uint *)(iVar23 + param_7) & (uint)&stack0x0000;
  *(uint *)(iVar23 + param_7) = *(uint *)(iVar23 + param_7) & (uint)&stack0x0000;
  puVar4 = (uint *)((int)param_6 + param_4 + -3);
  *puVar4 = *puVar4 & (uint)&stack0x0000;
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  bVar21 = (byte)((uint)(param_3 + 1) >> 8);
  *pbVar1 = *pbVar1 ^ bVar21;
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  bVar25 = (byte)((uint)iVar23 >> 8);
  *pbVar1 = *pbVar1 ^ bVar25;
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  *pbVar1 = *pbVar1;
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  *pbVar1 = *pbVar1;
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  *pbVar1 = *pbVar1;
  cVar6 = -*(byte *)((int)param_6 + param_4 + -3) - *(byte *)((int)param_6 + param_4 + -3);
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  cVar17 = (char)((uint)(param_2 + -1) >> 8);
  *pbVar1 = *pbVar1 + cVar17;
  cVar8 = (char)(param_2 + -1);
  bVar18 = cVar17 - *(byte *)((int)param_6 + param_5 + -4);
  pbVar1 = (byte *)((int)param_6 + param_4 + -3);
  *pbVar1 = *pbVar1 + cVar6;
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  bVar19 = bVar18 - *pbVar1;
  uVar9 = CONCAT11(bVar7,cVar6 - *(byte *)((int)param_6 + param_4 + -3)) + 0x2a00 +
          (uint)(bVar18 < *pbVar1);
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  uVar12 = (uint)(bVar19 < *pbVar1);
  uVar13 = uVar9 + 0x1515;
  uVar10 = uVar13 + uVar12;
  uVar12 = (uint)(0xeaea < uVar9 || CARRY2(uVar13,uVar12));
  uVar13 = uVar10 + 0x3f15;
  uVar9 = uVar13 + uVar12;
  uVar12 = (uint)(0xc0ea < uVar10 || CARRY2(uVar13,uVar12));
  uVar13 = uVar9 + 0x153f;
  iVar22 = uVar13 + uVar12 + 0x3f3f + (uint)(0xeac0 < uVar9 || CARRY2(uVar13,uVar12));
  bVar7 = (byte)iVar22;
  bVar15 = 9 < (bVar7 & 0xf) | in_AF;
  uVar13 = CONCAT11((char)((uint)iVar22 >> 8) - bVar15,bVar7 + bVar15 * -6) & 0xff0f;
  uVar12 = uVar13 + 0x3f15;
  iVar22 = uVar12 + bVar15 + 0x3f3f + (uint)(0xc0ea < uVar13 || CARRY2(uVar12,(uint)bVar15));
  bVar7 = (byte)iVar22;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  iVar22 = (CONCAT11((char)((uint)iVar22 >> 8) - bVar15,bVar7 + bVar15 * -6) & 0xff0f) + 0x3f3f +
           (uint)bVar15;
  bVar7 = (byte)iVar22;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  iVar16 = CONCAT11(bVar19 - *pbVar1,cVar8) + 1;
  *(byte *)((int)param_6 + 0x49) = *(byte *)((int)param_6 + 0x49) + cVar8;
  puVar29 = (uint *)(param_7 + 2);
  iVar22 = (CONCAT11((char)((uint)iVar22 >> 8) - bVar15,bVar7 + bVar15 * -6) & 0xff0f) - 1;
  bVar7 = (byte)((uint)iVar22 >> 8);
  *(byte *)(iVar23 + (int)param_6) = *(byte *)(iVar23 + (int)param_6) & bVar7;
  bVar14 = (byte)(param_3 + 1);
  *(char *)(param_5 + 0x45) = *(char *)(param_5 + 0x45) + bVar14;
  bVar21 = bVar21 ^ (byte)*param_6;
  *(byte *)(iVar23 + (int)param_6) = *(byte *)(iVar23 + (int)param_6) ^ bVar7;
  cVar8 = (char)((uint)iVar16 >> 8);
  cVar6 = (char)iVar22;
  *(byte *)(iVar23 + (int)param_6) = *(byte *)(iVar23 + (int)param_6) + cVar6;
  *(byte *)(iVar23 + (int)puVar29) = *(byte *)(iVar23 + (int)puVar29) & bVar7;
  uVar12 = CONCAT11(bVar7 & *(byte *)(uVar28 + (int)puVar29),cVar6) & 0xff25;
  bVar7 = (byte)uVar12;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  bVar7 = bVar7 + bVar15 * '\x06' + (0x99 < bVar7) * '`';
  *(char *)(iVar23 + (int)puVar29) = *(char *)(iVar23 + (int)puVar29) - cVar8;
  bVar20 = cVar8 - *(char *)(uVar28 + (int)puVar29);
  bVar18 = bVar7 - 0x2d;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  bVar30 = 0x99 < bVar18 || bVar7 < 0x2d;
  bVar19 = bVar18 + bVar15 * -6 + bVar30 * -0x60;
  pbVar1 = (byte *)(iVar23 + (int)puVar29);
  bVar18 = *pbVar1;
  bVar7 = *pbVar1;
  *pbVar1 = bVar7 + bVar14 + bVar30;
  bVar30 = CARRY1(bVar14,*(byte *)(uVar28 + (int)puVar29)) ||
           CARRY1(bVar14 + *(byte *)(uVar28 + (int)puVar29),
                  CARRY1(bVar18,bVar14) || CARRY1(bVar7 + bVar14,bVar30));
  bVar7 = bVar19 + 0x15;
  bVar31 = 0xea < bVar19 || CARRY1(bVar7,bVar30);
  pbVar1 = (byte *)(iVar23 + (int)puVar29);
  bVar19 = (byte)iVar23;
  bVar32 = *pbVar1 < bVar19 || (byte)(*pbVar1 - bVar19) < bVar31;
  *pbVar1 = (*pbVar1 - bVar19) - bVar31;
  bVar18 = bVar19 - *(byte *)(uVar28 + (int)puVar29);
  puVar24 = (uint *)CONCAT11(bVar25,bVar18 - bVar32);
  bVar7 = (bVar7 + bVar30 + -0x1d) - (bVar19 < *(byte *)(uVar28 + (int)puVar29) || bVar18 < bVar32);
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  uVar12 = CONCAT11((char)(uVar12 >> 8),bVar7 + bVar15 * -6 + (0x99 < bVar7) * -0x60) & 0xff25;
  bVar7 = (byte)uVar12;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  bVar19 = (byte)(uVar12 >> 8);
  bVar7 = bVar7 + bVar15 * '\x06' + (0x99 < bVar7) * '`';
  *(char *)(uVar28 + (int)puVar29) = *(char *)(uVar28 + (int)puVar29) - bVar20;
  bVar18 = bVar7 - 0x2d;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  bVar30 = 0x99 < bVar18 || bVar7 < 0x2d;
  bVar7 = bVar18 + bVar15 * -6 + bVar30 * -0x60;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  bVar7 = bVar7 + bVar15 * -6 + (0x99 < bVar7 || bVar30) * -0x60;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)auStack_5;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) & bVar19;
  *(uint *)(uVar28 + (int)param_6) = *(uint *)(uVar28 + (int)param_6) & (uint)auStack_5;
  *(uint *)(uVar28 + (int)puVar29) = *(uint *)(uVar28 + (int)puVar29) & uVar28;
  uVar12 = *param_6;
  uVar13 = *param_6;
  uVar9 = *(uint *)(uVar28 + (int)puVar29);
  uVar11 = CONCAT11(bVar19,bVar7 + bVar15 * -6 + (0x99 < bVar7) * -0x60) & 0x2425;
  bVar7 = (byte)uVar11;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  bVar18 = bVar7 + bVar15 * '\x06' + (0x99 < bVar7) * '`';
  pbVar1 = (byte *)((int)puVar24 + (int)param_6);
  bVar7 = *pbVar1;
  *pbVar1 = *pbVar1 - bVar20;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  bVar7 = bVar18 + bVar15 * '\x06' + (0x99 < bVar18 || bVar7 < bVar20) * '`';
  uVar10 = *puVar24;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) - bVar20;
  uVar28 = (((uVar28 - *param_6) - *(int *)(byte *)((uVar28 - *param_6) + (int)param_6)) - *puVar29)
           - *puVar24;
  puVar26 = (undefined1 *)((uint)auStack_5 & uVar12 & uVar13 & uVar9 & *puVar24);
  puVar4 = puVar29;
  puVar5 = (undefined1 *)*puVar4;
  *puVar4 = *puVar4 - (int)puVar26;
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  uVar12 = CONCAT11((char)(uVar11 >> 8) - (byte)uVar10,
                    bVar7 + bVar15 * '\x06' + (0x99 < bVar7 || puVar5 < puVar26) * '`') & 0x202a;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & (uint)puVar26;
  *(uint *)(uVar28 + (int)param_6) = *(uint *)(uVar28 + (int)param_6) & (uint)puVar26;
  uVar12 = CONCAT11((byte)(uVar12 >> 8) & *(byte *)((int)puVar24 + (int)puVar29),(char)uVar12) &
           0xff23;
  bVar18 = (byte)uVar12;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  cVar6 = (char)(uVar12 >> 8);
  bVar7 = bVar18 + bVar15 * '\x06' + (0x99 < bVar18) * '`';
  bVar15 = 9 < (bVar7 & 0xf) | bVar15;
  *(byte *)puVar24 = (byte)*puVar24 - cVar6;
  *(byte *)puVar24 = (byte)*puVar24 - cVar6;
  uVar13 = CONCAT11(cVar6,bVar7 + bVar15 * '\x06' + (0x99 < bVar7 || 0x99 < bVar18) * '`') & 0x2225;
  bVar7 = *(byte *)((int)puVar24 + (int)param_6);
  cVar6 = *(char *)(uVar28 + (int)puVar29);
  bVar18 = (char)uVar13 - 0x2a;
  uVar12 = *puVar29;
  *puVar29 = *puVar29 - uVar28;
  iVar23 = uVar28 - *param_6;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  bVar30 = 0x99 < bVar18 || uVar28 < *param_6;
  bVar18 = bVar18 + bVar15 * -6 + bVar30 * -0x60;
  bVar15 = 9 < (bVar18 & 0xf) | bVar15;
  iVar22 = CONCAT11((char)(uVar13 >> 8),bVar18 + bVar15 * -6 + (0x99 < bVar18 || bVar30) * -0x60) +
           -0x2b2b;
  bVar18 = *(byte *)((int)puVar24 + (int)puVar29);
  *(int *)(byte *)((int)puVar24 + (int)param_6) =
       *(int *)(byte *)((int)puVar24 + (int)param_6) - iVar23;
  puVar4 = puVar24;
  uVar13 = *puVar4;
  bVar14 = (byte)((uint)iVar22 >> 8);
  *(byte *)puVar4 = (byte)*puVar4 - bVar14;
  bVar19 = (byte)iVar22;
  bVar15 = 9 < (bVar19 & 0xf) | bVar15;
  bVar30 = 0x99 < bVar19 || (byte)uVar13 < bVar14;
  bVar19 = bVar19 + bVar15 * '\x06' + bVar30 * '`';
  uVar13 = CONCAT11(bVar14,bVar19 + (9 < (bVar19 & 0xf) | bVar15) * '\x06' +
                           (0x99 < bVar19 || bVar30) * '`') & 0x2520;
  uVar27 = (int)puVar26 - uVar12 & *param_6 & *(uint *)(iVar23 + (int)puVar29);
  cVar8 = (char)uVar13;
  bVar15 = (byte)(uVar13 >> 8) & *(byte *)(iVar23 + (int)param_6) & *(byte *)(iVar23 + (int)param_6)
  ;
  *(uint *)((int)puVar24 + (int)param_6) = *(uint *)((int)puVar24 + (int)param_6) & uVar27;
  *(byte *)((int)puVar24 + (int)puVar29) = *(byte *)((int)puVar24 + (int)puVar29) & bVar15;
  *(uint *)((int)puVar24 + (int)puVar29) = *(uint *)((int)puVar24 + (int)puVar29) & uVar27;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(byte *)((int)puVar24 + (int)param_6) = *(byte *)((int)puVar24 + (int)param_6) + cVar8;
  *(int *)(iVar23 + (int)puVar29) =
       (*(int *)(iVar23 + (int)puVar29) - CONCAT11(((bVar20 - bVar7) - cVar6) - bVar18,(char)iVar16)
       ) - (uint)CARRY1(bVar15,bVar21);
  uVar12 = CONCAT11(bVar15 + bVar21,cVar8) | 10;
  out(6,uVar12);
  *(int *)(byte *)((int)puVar24 + (int)param_6) = *(int *)(byte *)((int)puVar24 + (int)param_6) + -1
  ;
  out(7,uVar12);
  *(int *)(byte *)((int)puVar24 + (int)puVar29) = *(int *)(byte *)((int)puVar24 + (int)puVar29) + -1
  ;
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


/* ===== FUN_0000_4a83 @ 0000:4a83 ===== */

/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_4a83(int param_1,int param_2,int param_3,int param_4,int param_5,uint *param_6,int param_7)

{
  byte *pbVar1;
  char *pcVar2;
  uint *puVar3;
  undefined1 *puVar4;
  char cVar5;
  byte bVar6;
  char cVar12;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  byte bVar13;
  byte bVar14;
  char cVar15;
  char cVar17;
  byte bVar18;
  byte bVar19;
  int iVar16;
  byte bVar20;
  byte bVar21;
  uint *puVar22;
  undefined1 *puVar23;
  uint uVar24;
  uint uVar25;
  int iVar26;
  int iVar27;
  uint *puVar28;
  undefined2 unaff_ES;
  bool bVar29;
  bool bVar30;
  bool bVar31;
  byte in_AF;
  undefined1 auStack_5 [3];
  byte local_2;
  byte bStack_1;
  
  local_2 = (byte)param_4;
  bStack_1 = (byte)((uint)param_4 >> 8);
  iVar27 = param_7 + -1;
  bVar18 = (byte)((uint)(param_1 + -1) >> 8);
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) & bVar18;
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) & bVar18;
  pcVar2 = (char *)(param_4 + iVar27 + 0x52);
  *pcVar2 = *pcVar2 + local_2;
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) & bStack_1;
  uVar25 = param_5 - 2;
  pbVar1 = (byte *)((int)param_6 + param_5 + 0x3f);
  *pbVar1 = *pbVar1;
  cVar12 = (char)((uint)&stack0x0000 >> 8);
  *(char *)(param_4 + iVar27) = *(char *)(param_4 + iVar27) + cVar12;
  *(uint *)(param_4 + iVar27) = *(uint *)(param_4 + iVar27) & (uint)&stack0x0000;
  *(uint *)(param_4 + iVar27) = *(uint *)(param_4 + iVar27) & (uint)&stack0x0000;
  *(uint *)(param_4 + iVar27) = *(uint *)(param_4 + iVar27) & (uint)&stack0x0000;
  puVar3 = (uint *)((int)param_6 + param_4 + -2);
  *puVar3 = *puVar3 & (uint)&stack0x0000;
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  bVar21 = (byte)((uint)(param_3 + 1) >> 8);
  *pbVar1 = *pbVar1 ^ bVar21;
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  *pbVar1 = *pbVar1 ^ bStack_1;
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  *pbVar1 = *pbVar1;
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  *pbVar1 = *pbVar1;
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  *pbVar1 = *pbVar1;
  cVar5 = -*(byte *)((int)param_6 + param_4 + -2) - *(byte *)((int)param_6 + param_4 + -2);
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  cVar17 = (char)((uint)(param_2 + -1) >> 8);
  *pbVar1 = *pbVar1 + cVar17;
  cVar15 = (char)(param_2 + -1);
  bVar18 = cVar17 - *(byte *)((int)param_6 + param_5 + -4);
  pbVar1 = (byte *)((int)param_6 + param_4 + -2);
  *pbVar1 = *pbVar1 + cVar5;
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  bVar19 = bVar18 - *pbVar1;
  uVar7 = CONCAT11(cVar12,cVar5 - *(byte *)((int)param_6 + param_4 + -2)) + 0x2a00 +
          (uint)(bVar18 < *pbVar1);
  pbVar1 = (byte *)((int)param_6 + param_5 + -4);
  uVar10 = (uint)(bVar19 < *pbVar1);
  uVar11 = uVar7 + 0x1515;
  uVar8 = uVar11 + uVar10;
  uVar10 = (uint)(0xeaea < uVar7 || CARRY2(uVar11,uVar10));
  uVar11 = uVar8 + 0x3f15;
  uVar7 = uVar11 + uVar10;
  uVar10 = (uint)(0xc0ea < uVar8 || CARRY2(uVar11,uVar10));
  uVar11 = uVar7 + 0x153f;
  iVar27 = uVar11 + uVar10 + 0x3f3f + (uint)(0xeac0 < uVar7 || CARRY2(uVar11,uVar10));
  bVar18 = (byte)iVar27;
  bVar14 = 9 < (bVar18 & 0xf) | in_AF;
  uVar11 = CONCAT11((char)((uint)iVar27 >> 8) - bVar14,bVar18 + bVar14 * -6) & 0xff0f;
  uVar10 = uVar11 + 0x3f15;
  iVar27 = uVar10 + bVar14 + 0x3f3f + (uint)(0xc0ea < uVar11 || CARRY2(uVar10,(uint)bVar14));
  bVar18 = (byte)iVar27;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  iVar27 = (CONCAT11((char)((uint)iVar27 >> 8) - bVar14,bVar18 + bVar14 * -6) & 0xff0f) + 0x3f3f +
           (uint)bVar14;
  bVar18 = (byte)iVar27;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  iVar16 = CONCAT11(bVar19 - *pbVar1,cVar15) + 1;
  *(byte *)((int)param_6 + 0x49) = *(byte *)((int)param_6 + 0x49) + cVar15;
  puVar28 = (uint *)(param_7 + 1);
  iVar27 = (CONCAT11((char)((uint)iVar27 >> 8) - bVar14,bVar18 + bVar14 * -6) & 0xff0f) - 1;
  bVar18 = (byte)((uint)iVar27 >> 8);
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) & bVar18;
  bVar13 = (byte)(param_3 + 1);
  *(char *)(param_5 + 0x45) = *(char *)(param_5 + 0x45) + bVar13;
  bVar21 = bVar21 ^ (byte)*param_6;
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) ^ bVar18;
  cVar12 = (char)((uint)iVar16 >> 8);
  cVar5 = (char)iVar27;
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) + cVar5;
  *(byte *)(param_4 + (int)puVar28) = *(byte *)(param_4 + (int)puVar28) & bVar18;
  uVar10 = CONCAT11(bVar18 & *(byte *)(uVar25 + (int)puVar28),cVar5) & 0xff25;
  bVar18 = (byte)uVar10;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  bVar18 = bVar18 + bVar14 * '\x06' + (0x99 < bVar18) * '`';
  *(char *)(param_4 + (int)puVar28) = *(char *)(param_4 + (int)puVar28) - cVar12;
  bVar20 = cVar12 - *(char *)(uVar25 + (int)puVar28);
  bVar19 = bVar18 - 0x2d;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  bVar29 = 0x99 < bVar19 || bVar18 < 0x2d;
  bVar6 = bVar19 + bVar14 * -6 + bVar29 * -0x60;
  pbVar1 = (byte *)(param_4 + (int)puVar28);
  bVar19 = *pbVar1;
  bVar18 = *pbVar1;
  *pbVar1 = bVar18 + bVar13 + bVar29;
  bVar29 = CARRY1(bVar13,*(byte *)(uVar25 + (int)puVar28)) ||
           CARRY1(bVar13 + *(byte *)(uVar25 + (int)puVar28),
                  CARRY1(bVar19,bVar13) || CARRY1(bVar18 + bVar13,bVar29));
  bVar18 = bVar6 + 0x15;
  bVar30 = 0xea < bVar6 || CARRY1(bVar18,bVar29);
  pbVar1 = (byte *)(param_4 + (int)puVar28);
  bVar31 = *pbVar1 < local_2 || (byte)(*pbVar1 - local_2) < bVar30;
  *pbVar1 = (*pbVar1 - local_2) - bVar30;
  bVar19 = local_2 - *(byte *)(uVar25 + (int)puVar28);
  puVar22 = (uint *)CONCAT11(bStack_1,bVar19 - bVar31);
  bVar18 = (bVar18 + bVar29 + -0x1d) -
           (local_2 < *(byte *)(uVar25 + (int)puVar28) || bVar19 < bVar31);
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  uVar10 = CONCAT11((char)(uVar10 >> 8),bVar18 + bVar14 * -6 + (0x99 < bVar18) * -0x60) & 0xff25;
  bVar18 = (byte)uVar10;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  bVar6 = (byte)(uVar10 >> 8);
  bVar18 = bVar18 + bVar14 * '\x06' + (0x99 < bVar18) * '`';
  *(char *)(uVar25 + (int)puVar28) = *(char *)(uVar25 + (int)puVar28) - bVar20;
  bVar19 = bVar18 - 0x2d;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  bVar29 = 0x99 < bVar19 || bVar18 < 0x2d;
  bVar18 = bVar19 + bVar14 * -6 + bVar29 * -0x60;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  bVar18 = bVar18 + bVar14 * -6 + (0x99 < bVar18 || bVar29) * -0x60;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)auStack_5;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) & bVar6;
  *(uint *)(uVar25 + (int)param_6) = *(uint *)(uVar25 + (int)param_6) & (uint)auStack_5;
  *(uint *)(uVar25 + (int)puVar28) = *(uint *)(uVar25 + (int)puVar28) & uVar25;
  uVar10 = *param_6;
  uVar11 = *param_6;
  uVar7 = *(uint *)(uVar25 + (int)puVar28);
  uVar9 = CONCAT11(bVar6,bVar18 + bVar14 * -6 + (0x99 < bVar18) * -0x60) & 0x2425;
  bVar18 = (byte)uVar9;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  bVar19 = bVar18 + bVar14 * '\x06' + (0x99 < bVar18) * '`';
  pbVar1 = (byte *)((int)puVar22 + (int)param_6);
  bVar18 = *pbVar1;
  *pbVar1 = *pbVar1 - bVar20;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  bVar18 = bVar19 + bVar14 * '\x06' + (0x99 < bVar19 || bVar18 < bVar20) * '`';
  uVar8 = *puVar22;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) - bVar20;
  uVar25 = (((uVar25 - *param_6) - *(int *)(byte *)((uVar25 - *param_6) + (int)param_6)) - *puVar28)
           - *puVar22;
  puVar23 = (undefined1 *)((uint)auStack_5 & uVar10 & uVar11 & uVar7 & *puVar22);
  puVar3 = puVar28;
  puVar4 = (undefined1 *)*puVar3;
  *puVar3 = *puVar3 - (int)puVar23;
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  uVar10 = CONCAT11((char)(uVar9 >> 8) - (byte)uVar8,
                    bVar18 + bVar14 * '\x06' + (0x99 < bVar18 || puVar4 < puVar23) * '`') & 0x202a;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & (uint)puVar23;
  *(uint *)(uVar25 + (int)param_6) = *(uint *)(uVar25 + (int)param_6) & (uint)puVar23;
  uVar10 = CONCAT11((byte)(uVar10 >> 8) & *(byte *)((int)puVar22 + (int)puVar28),(char)uVar10) &
           0xff23;
  bVar19 = (byte)uVar10;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  cVar5 = (char)(uVar10 >> 8);
  bVar18 = bVar19 + bVar14 * '\x06' + (0x99 < bVar19) * '`';
  bVar14 = 9 < (bVar18 & 0xf) | bVar14;
  *(byte *)puVar22 = (byte)*puVar22 - cVar5;
  *(byte *)puVar22 = (byte)*puVar22 - cVar5;
  uVar11 = CONCAT11(cVar5,bVar18 + bVar14 * '\x06' + (0x99 < bVar18 || 0x99 < bVar19) * '`') &
           0x2225;
  bVar18 = *(byte *)((int)puVar22 + (int)param_6);
  cVar5 = *(char *)(uVar25 + (int)puVar28);
  bVar19 = (char)uVar11 - 0x2a;
  uVar10 = *puVar28;
  *puVar28 = *puVar28 - uVar25;
  iVar26 = uVar25 - *param_6;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  bVar29 = 0x99 < bVar19 || uVar25 < *param_6;
  bVar19 = bVar19 + bVar14 * -6 + bVar29 * -0x60;
  bVar14 = 9 < (bVar19 & 0xf) | bVar14;
  iVar27 = CONCAT11((char)(uVar11 >> 8),bVar19 + bVar14 * -6 + (0x99 < bVar19 || bVar29) * -0x60) +
           -0x2b2b;
  bVar19 = *(byte *)((int)puVar22 + (int)puVar28);
  *(int *)(byte *)((int)puVar22 + (int)param_6) =
       *(int *)(byte *)((int)puVar22 + (int)param_6) - iVar26;
  puVar3 = puVar22;
  uVar11 = *puVar3;
  bVar13 = (byte)((uint)iVar27 >> 8);
  *(byte *)puVar3 = (byte)*puVar3 - bVar13;
  bVar6 = (byte)iVar27;
  bVar14 = 9 < (bVar6 & 0xf) | bVar14;
  bVar29 = 0x99 < bVar6 || (byte)uVar11 < bVar13;
  bVar6 = bVar6 + bVar14 * '\x06' + bVar29 * '`';
  uVar11 = CONCAT11(bVar13,bVar6 + (9 < (bVar6 & 0xf) | bVar14) * '\x06' +
                           (0x99 < bVar6 || bVar29) * '`') & 0x2520;
  uVar24 = (int)puVar23 - uVar10 & *param_6 & *(uint *)(iVar26 + (int)puVar28);
  cVar12 = (char)uVar11;
  bVar14 = (byte)(uVar11 >> 8) & *(byte *)(iVar26 + (int)param_6) & *(byte *)(iVar26 + (int)param_6)
  ;
  *(uint *)((int)puVar22 + (int)param_6) = *(uint *)((int)puVar22 + (int)param_6) & uVar24;
  *(byte *)((int)puVar22 + (int)puVar28) = *(byte *)((int)puVar22 + (int)puVar28) & bVar14;
  *(uint *)((int)puVar22 + (int)puVar28) = *(uint *)((int)puVar22 + (int)puVar28) & uVar24;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(byte *)((int)puVar22 + (int)param_6) = *(byte *)((int)puVar22 + (int)param_6) + cVar12;
  *(int *)(iVar26 + (int)puVar28) =
       (*(int *)(iVar26 + (int)puVar28) -
       CONCAT11(((bVar20 - bVar18) - cVar5) - bVar19,(char)iVar16)) - (uint)CARRY1(bVar14,bVar21);
  uVar10 = CONCAT11(bVar14 + bVar21,cVar12) | 10;
  out(6,uVar10);
  *(int *)(byte *)((int)puVar22 + (int)param_6) = *(int *)(byte *)((int)puVar22 + (int)param_6) + -1
  ;
  out(7,uVar10);
  *(int *)(byte *)((int)puVar22 + (int)puVar28) = *(int *)(byte *)((int)puVar22 + (int)puVar28) + -1
  ;
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


/* ===== FUN_0000_4aaa @ 0000:4aaa ===== */

/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_4aaa(uint param_1,undefined2 param_2,undefined2 param_3,int param_4,uint param_5,
             int param_6,int param_7)

{
  byte *pbVar1;
  uint *puVar2;
  undefined1 *puVar3;
  char cVar4;
  byte bVar5;
  char cVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  byte bVar13;
  byte bVar14;
  byte bVar16;
  byte bVar17;
  int iVar15;
  byte bVar18;
  byte bVar19;
  byte bVar21;
  uint *puVar20;
  undefined1 *puVar22;
  uint uVar23;
  uint uVar24;
  int iVar25;
  uint *puVar26;
  uint *puVar27;
  undefined2 unaff_ES;
  bool bVar28;
  bool bVar29;
  bool bVar30;
  byte in_AF;
  undefined1 auStack_5 [5];
  
  *(uint *)(param_4 + param_7) = *(uint *)(param_4 + param_7) & (uint)&stack0x0000;
  *(uint *)(param_4 + param_7) = *(uint *)(param_4 + param_7) & (uint)&stack0x0000;
  *(uint *)(param_4 + param_7) = *(uint *)(param_4 + param_7) & (uint)&stack0x0000;
  *(uint *)(param_4 + param_7) = *(uint *)(param_4 + param_7) & (uint)&stack0x0000;
  *(uint *)(param_4 + param_7) = *(uint *)(param_4 + param_7) & (uint)&stack0x0000;
  *(uint *)(param_4 + param_6) = *(uint *)(param_4 + param_6) & param_1;
  bVar19 = (byte)((uint)param_3 >> 8);
  *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) ^ bVar19;
  bVar21 = (byte)((uint)param_4 >> 8);
  *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) ^ bVar21;
  cVar4 = (char)param_1;
  *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + cVar4;
  *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + cVar4;
  *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + cVar4;
  cVar4 = (cVar4 - *(char *)(param_4 + param_6)) - *(char *)(param_4 + param_6);
  cVar6 = (char)((uint)param_2 >> 8);
  *(char *)(param_5 + param_6) = *(char *)(param_5 + param_6) + cVar6;
  bVar16 = cVar6 - *(char *)(param_5 + param_6);
  *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + cVar4;
  bVar17 = bVar16 - *(byte *)(param_5 + param_6);
  uVar7 = CONCAT11((char)(param_1 >> 8),cVar4 - *(char *)(param_4 + param_6)) + 0x2a00 +
          (uint)(bVar16 < *(byte *)(param_5 + param_6));
  uVar11 = (uint)(bVar17 < *(byte *)(param_5 + param_6));
  uVar12 = uVar7 + 0x1515;
  uVar8 = uVar12 + uVar11;
  uVar11 = (uint)(0xeaea < uVar7 || CARRY2(uVar12,uVar11));
  uVar12 = uVar8 + 0x3f15;
  uVar7 = uVar12 + uVar11;
  uVar11 = (uint)(0xc0ea < uVar8 || CARRY2(uVar12,uVar11));
  uVar12 = uVar7 + 0x153f;
  iVar9 = uVar12 + uVar11 + 0x3f3f + (uint)(0xeac0 < uVar7 || CARRY2(uVar12,uVar11));
  bVar16 = (byte)iVar9;
  bVar14 = 9 < (bVar16 & 0xf) | in_AF;
  uVar12 = CONCAT11((char)((uint)iVar9 >> 8) - bVar14,bVar16 + bVar14 * -6) & 0xff0f;
  uVar11 = uVar12 + 0x3f15;
  iVar9 = uVar11 + bVar14 + 0x3f3f + (uint)(0xc0ea < uVar12 || CARRY2(uVar11,(uint)bVar14));
  bVar16 = (byte)iVar9;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  iVar9 = (CONCAT11((char)((uint)iVar9 >> 8) - bVar14,bVar16 + bVar14 * -6) & 0xff0f) + 0x3f3f +
          (uint)bVar14;
  bVar16 = (byte)iVar9;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  iVar15 = CONCAT11(bVar17 - *(byte *)(param_5 + param_6),(char)param_2) + 1;
  puVar26 = (uint *)(param_6 + 2);
  *(char *)(param_6 + 0x4b) = *(char *)(param_6 + 0x4b) + (char)param_2;
  puVar27 = (uint *)(param_7 + 2);
  iVar9 = (CONCAT11((char)((uint)iVar9 >> 8) - bVar14,bVar16 + bVar14 * -6) & 0xff0f) - 1;
  bVar16 = (byte)((uint)iVar9 >> 8);
  *(byte *)(param_4 + (int)puVar26) = *(byte *)(param_4 + (int)puVar26) & bVar16;
  bVar13 = (byte)param_3;
  *(char *)(param_5 + 0x47) = *(char *)(param_5 + 0x47) + bVar13;
  bVar19 = bVar19 ^ (byte)*puVar26;
  *(byte *)(param_4 + (int)puVar26) = *(byte *)(param_4 + (int)puVar26) ^ bVar16;
  cVar6 = (char)((uint)iVar15 >> 8);
  cVar4 = (char)iVar9;
  *(byte *)(param_4 + (int)puVar26) = *(byte *)(param_4 + (int)puVar26) + cVar4;
  *(byte *)(param_4 + (int)puVar27) = *(byte *)(param_4 + (int)puVar27) & bVar16;
  uVar11 = CONCAT11(bVar16 & *(byte *)(param_5 + (int)puVar27),cVar4) & 0xff25;
  bVar16 = (byte)uVar11;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  bVar16 = bVar16 + bVar14 * '\x06' + (0x99 < bVar16) * '`';
  *(char *)(param_4 + (int)puVar27) = *(char *)(param_4 + (int)puVar27) - cVar6;
  bVar18 = cVar6 - *(char *)(param_5 + (int)puVar27);
  bVar17 = bVar16 - 0x2d;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  bVar28 = 0x99 < bVar17 || bVar16 < 0x2d;
  bVar5 = bVar17 + bVar14 * -6 + bVar28 * -0x60;
  pbVar1 = (byte *)(param_4 + (int)puVar27);
  bVar17 = *pbVar1;
  bVar16 = *pbVar1;
  *pbVar1 = bVar16 + bVar13 + bVar28;
  bVar28 = CARRY1(bVar13,*(byte *)(param_5 + (int)puVar27)) ||
           CARRY1(bVar13 + *(byte *)(param_5 + (int)puVar27),
                  CARRY1(bVar17,bVar13) || CARRY1(bVar16 + bVar13,bVar28));
  bVar16 = bVar5 + 0x15;
  bVar29 = 0xea < bVar5 || CARRY1(bVar16,bVar28);
  pbVar1 = (byte *)(param_4 + (int)puVar27);
  bVar5 = (byte)param_4;
  bVar30 = *pbVar1 < bVar5 || (byte)(*pbVar1 - bVar5) < bVar29;
  *pbVar1 = (*pbVar1 - bVar5) - bVar29;
  bVar17 = bVar5 - *(byte *)(param_5 + (int)puVar27);
  puVar20 = (uint *)CONCAT11(bVar21,bVar17 - bVar30);
  bVar16 = (bVar16 + bVar28 + -0x1d) -
           (bVar5 < *(byte *)(param_5 + (int)puVar27) || bVar17 < bVar30);
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  uVar11 = CONCAT11((char)(uVar11 >> 8),bVar16 + bVar14 * -6 + (0x99 < bVar16) * -0x60) & 0xff25;
  bVar16 = (byte)uVar11;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  bVar5 = (byte)(uVar11 >> 8);
  bVar16 = bVar16 + bVar14 * '\x06' + (0x99 < bVar16) * '`';
  *(char *)(param_5 + (int)puVar27) = *(char *)(param_5 + (int)puVar27) - bVar18;
  bVar17 = bVar16 - 0x2d;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  bVar28 = 0x99 < bVar17 || bVar16 < 0x2d;
  bVar16 = bVar17 + bVar14 * -6 + bVar28 * -0x60;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  bVar16 = bVar16 + bVar14 * -6 + (0x99 < bVar16 || bVar28) * -0x60;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)auStack_5;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) & bVar5;
  *(uint *)(param_5 + (int)puVar26) = *(uint *)(param_5 + (int)puVar26) & (uint)auStack_5;
  *(uint *)(param_5 + (int)puVar27) = *(uint *)(param_5 + (int)puVar27) & param_5;
  uVar11 = *puVar26;
  uVar12 = *puVar26;
  uVar7 = *(uint *)(param_5 + (int)puVar27);
  uVar10 = CONCAT11(bVar5,bVar16 + bVar14 * -6 + (0x99 < bVar16) * -0x60) & 0x2425;
  bVar16 = (byte)uVar10;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  bVar17 = bVar16 + bVar14 * '\x06' + (0x99 < bVar16) * '`';
  pbVar1 = (byte *)((int)puVar20 + (int)puVar26);
  bVar16 = *pbVar1;
  *pbVar1 = *pbVar1 - bVar18;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  bVar16 = bVar17 + bVar14 * '\x06' + (0x99 < bVar17 || bVar16 < bVar18) * '`';
  uVar8 = *puVar20;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) - bVar18;
  uVar24 = (((param_5 - *puVar26) - *(int *)(byte *)((param_5 - *puVar26) + (int)puVar26)) -
           *puVar27) - *puVar20;
  puVar22 = (undefined1 *)((uint)auStack_5 & uVar11 & uVar12 & uVar7 & *puVar20);
  puVar2 = puVar27;
  puVar3 = (undefined1 *)*puVar2;
  *puVar2 = *puVar2 - (int)puVar22;
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  uVar11 = CONCAT11((char)(uVar10 >> 8) - (byte)uVar8,
                    bVar16 + bVar14 * '\x06' + (0x99 < bVar16 || puVar3 < puVar22) * '`') & 0x202a;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & (uint)puVar22;
  *(uint *)(uVar24 + (int)puVar26) = *(uint *)(uVar24 + (int)puVar26) & (uint)puVar22;
  uVar11 = CONCAT11((byte)(uVar11 >> 8) & *(byte *)((int)puVar20 + (int)puVar27),(char)uVar11) &
           0xff23;
  bVar17 = (byte)uVar11;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  cVar4 = (char)(uVar11 >> 8);
  bVar16 = bVar17 + bVar14 * '\x06' + (0x99 < bVar17) * '`';
  bVar14 = 9 < (bVar16 & 0xf) | bVar14;
  *(byte *)puVar20 = (byte)*puVar20 - cVar4;
  *(byte *)puVar20 = (byte)*puVar20 - cVar4;
  uVar12 = CONCAT11(cVar4,bVar16 + bVar14 * '\x06' + (0x99 < bVar16 || 0x99 < bVar17) * '`') &
           0x2225;
  bVar16 = *(byte *)((int)puVar20 + (int)puVar26);
  cVar4 = *(char *)(uVar24 + (int)puVar27);
  bVar17 = (char)uVar12 - 0x2a;
  uVar11 = *puVar27;
  *puVar27 = *puVar27 - uVar24;
  iVar25 = uVar24 - *puVar26;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  bVar28 = 0x99 < bVar17 || uVar24 < *puVar26;
  bVar17 = bVar17 + bVar14 * -6 + bVar28 * -0x60;
  bVar14 = 9 < (bVar17 & 0xf) | bVar14;
  iVar9 = CONCAT11((char)(uVar12 >> 8),bVar17 + bVar14 * -6 + (0x99 < bVar17 || bVar28) * -0x60) +
          -0x2b2b;
  bVar17 = *(byte *)((int)puVar20 + (int)puVar27);
  *(int *)(byte *)((int)puVar20 + (int)puVar26) =
       *(int *)(byte *)((int)puVar20 + (int)puVar26) - iVar25;
  puVar2 = puVar20;
  uVar12 = *puVar2;
  bVar13 = (byte)((uint)iVar9 >> 8);
  *(byte *)puVar2 = (byte)*puVar2 - bVar13;
  bVar5 = (byte)iVar9;
  bVar14 = 9 < (bVar5 & 0xf) | bVar14;
  bVar28 = 0x99 < bVar5 || (byte)uVar12 < bVar13;
  bVar5 = bVar5 + bVar14 * '\x06' + bVar28 * '`';
  uVar12 = CONCAT11(bVar13,bVar5 + (9 < (bVar5 & 0xf) | bVar14) * '\x06' +
                           (0x99 < bVar5 || bVar28) * '`') & 0x2520;
  uVar23 = (int)puVar22 - uVar11 & *puVar26 & *(uint *)(iVar25 + (int)puVar27);
  cVar6 = (char)uVar12;
  bVar14 = (byte)(uVar12 >> 8) & *(byte *)(iVar25 + (int)puVar26) & *(byte *)(iVar25 + (int)puVar26)
  ;
  *(uint *)((int)puVar20 + (int)puVar26) = *(uint *)((int)puVar20 + (int)puVar26) & uVar23;
  *(byte *)((int)puVar20 + (int)puVar27) = *(byte *)((int)puVar20 + (int)puVar27) & bVar14;
  *(uint *)((int)puVar20 + (int)puVar27) = *(uint *)((int)puVar20 + (int)puVar27) & uVar23;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(byte *)((int)puVar20 + (int)puVar26) = *(byte *)((int)puVar20 + (int)puVar26) + cVar6;
  *(int *)(iVar25 + (int)puVar27) =
       (*(int *)(iVar25 + (int)puVar27) -
       CONCAT11(((bVar18 - bVar16) - cVar4) - bVar17,(char)iVar15)) - (uint)CARRY1(bVar14,bVar19);
  uVar11 = CONCAT11(bVar14 + bVar19,cVar6) | 10;
  out(6,uVar11);
  *(int *)(byte *)((int)puVar20 + (int)puVar26) = *(int *)(byte *)((int)puVar20 + (int)puVar26) + -1
  ;
  out(7,uVar11);
  *(int *)(byte *)((int)puVar20 + (int)puVar27) = *(int *)(byte *)((int)puVar20 + (int)puVar27) + -1
  ;
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


/* ===== FUN_0000_4b08 @ 0000:4b08 ===== */

/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_4b08(undefined2 param_1,int param_2,undefined2 param_3,int param_4,uint param_5,
             uint *param_6,int param_7)

{
  byte *pbVar1;
  uint *puVar2;
  undefined1 *puVar3;
  uint uVar4;
  uint uVar5;
  byte bVar6;
  byte bVar7;
  byte bVar8;
  char cVar9;
  uint uVar10;
  uint uVar11;
  char cVar14;
  uint uVar12;
  int iVar13;
  byte bVar15;
  byte bVar16;
  byte bVar17;
  byte bVar18;
  uint *puVar19;
  undefined1 *puVar20;
  uint uVar21;
  uint uVar22;
  int iVar23;
  uint *puVar24;
  undefined2 unaff_ES;
  bool bVar25;
  bool bVar26;
  bool bVar27;
  byte in_AF;
  undefined1 uStack_1;
  
  puVar24 = (uint *)(param_7 + 1);
  bVar15 = (byte)param_3;
  bVar18 = (byte)((uint)param_3 >> 8) ^ (byte)*param_6;
  bVar6 = (byte)((uint)param_1 >> 8);
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) ^ bVar6;
  cVar14 = (char)((uint)(param_2 + 1) >> 8);
  *(byte *)(param_4 + (int)param_6) = *(byte *)(param_4 + (int)param_6) + (char)param_1;
  *(byte *)(param_4 + (int)puVar24) = *(byte *)(param_4 + (int)puVar24) & bVar6;
  uVar11 = CONCAT11(bVar6 & *(byte *)(param_5 + (int)puVar24),(char)param_1) & 0xff25;
  bVar6 = (byte)uVar11;
  bVar16 = 9 < (bVar6 & 0xf) | in_AF;
  bVar6 = bVar6 + bVar16 * '\x06' + (0x99 < bVar6) * '`';
  *(char *)(param_4 + (int)puVar24) = *(char *)(param_4 + (int)puVar24) - cVar14;
  bVar17 = cVar14 - *(char *)(param_5 + (int)puVar24);
  bVar7 = bVar6 - 0x2d;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  bVar25 = 0x99 < bVar7 || bVar6 < 0x2d;
  bVar8 = bVar7 + bVar16 * -6 + bVar25 * -0x60;
  pbVar1 = (byte *)(param_4 + (int)puVar24);
  bVar7 = *pbVar1;
  bVar6 = *pbVar1;
  *pbVar1 = bVar6 + bVar15 + bVar25;
  bVar25 = CARRY1(bVar15,*(byte *)(param_5 + (int)puVar24)) ||
           CARRY1(bVar15 + *(byte *)(param_5 + (int)puVar24),
                  CARRY1(bVar7,bVar15) || CARRY1(bVar6 + bVar15,bVar25));
  bVar6 = bVar8 + 0x15;
  bVar26 = 0xea < bVar8 || CARRY1(bVar6,bVar25);
  pbVar1 = (byte *)(param_4 + (int)puVar24);
  bVar8 = (byte)param_4;
  bVar27 = *pbVar1 < bVar8 || (byte)(*pbVar1 - bVar8) < bVar26;
  *pbVar1 = (*pbVar1 - bVar8) - bVar26;
  bVar7 = bVar8 - *(byte *)(param_5 + (int)puVar24);
  puVar19 = (uint *)CONCAT11((char)((uint)param_4 >> 8),bVar7 - bVar27);
  bVar6 = (bVar6 + bVar25 + -0x1d) - (bVar8 < *(byte *)(param_5 + (int)puVar24) || bVar7 < bVar27);
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  uVar11 = CONCAT11((char)(uVar11 >> 8),bVar6 + bVar16 * -6 + (0x99 < bVar6) * -0x60) & 0xff25;
  bVar6 = (byte)uVar11;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  bVar8 = (byte)(uVar11 >> 8);
  bVar6 = bVar6 + bVar16 * '\x06' + (0x99 < bVar6) * '`';
  *(char *)(param_5 + (int)puVar24) = *(char *)(param_5 + (int)puVar24) - bVar17;
  bVar7 = bVar6 - 0x2d;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  bVar25 = 0x99 < bVar7 || bVar6 < 0x2d;
  bVar6 = bVar7 + bVar16 * -6 + bVar25 * -0x60;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  bVar6 = bVar6 + bVar16 * -6 + (0x99 < bVar6 || bVar25) * -0x60;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)&uStack_1;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) & bVar8;
  *(uint *)(param_5 + (int)param_6) = *(uint *)(param_5 + (int)param_6) & (uint)&uStack_1;
  *(uint *)(param_5 + (int)puVar24) = *(uint *)(param_5 + (int)puVar24) & param_5;
  uVar11 = *param_6;
  uVar12 = *param_6;
  uVar4 = *(uint *)(param_5 + (int)puVar24);
  uVar10 = CONCAT11(bVar8,bVar6 + bVar16 * -6 + (0x99 < bVar6) * -0x60) & 0x2425;
  bVar6 = (byte)uVar10;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  bVar7 = bVar6 + bVar16 * '\x06' + (0x99 < bVar6) * '`';
  pbVar1 = (byte *)((int)puVar19 + (int)param_6);
  bVar6 = *pbVar1;
  *pbVar1 = *pbVar1 - bVar17;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  bVar6 = bVar7 + bVar16 * '\x06' + (0x99 < bVar7 || bVar6 < bVar17) * '`';
  uVar5 = *puVar19;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) - bVar17;
  uVar22 = (((param_5 - *param_6) - *(int *)(byte *)((param_5 - *param_6) + (int)param_6)) -
           *puVar24) - *puVar19;
  puVar20 = (undefined1 *)((uint)&uStack_1 & uVar11 & uVar12 & uVar4 & *puVar19);
  puVar2 = puVar24;
  puVar3 = (undefined1 *)*puVar2;
  *puVar2 = *puVar2 - (int)puVar20;
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  uVar11 = CONCAT11((char)(uVar10 >> 8) - (byte)uVar5,
                    bVar6 + bVar16 * '\x06' + (0x99 < bVar6 || puVar3 < puVar20) * '`') & 0x202a;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & (uint)puVar20;
  *(uint *)(uVar22 + (int)param_6) = *(uint *)(uVar22 + (int)param_6) & (uint)puVar20;
  uVar11 = CONCAT11((byte)(uVar11 >> 8) & *(byte *)((int)puVar19 + (int)puVar24),(char)uVar11) &
           0xff23;
  bVar7 = (byte)uVar11;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  cVar14 = (char)(uVar11 >> 8);
  bVar6 = bVar7 + bVar16 * '\x06' + (0x99 < bVar7) * '`';
  bVar16 = 9 < (bVar6 & 0xf) | bVar16;
  *(byte *)puVar19 = (byte)*puVar19 - cVar14;
  *(byte *)puVar19 = (byte)*puVar19 - cVar14;
  uVar12 = CONCAT11(cVar14,bVar6 + bVar16 * '\x06' + (0x99 < bVar6 || 0x99 < bVar7) * '`') & 0x2225;
  bVar6 = *(byte *)((int)puVar19 + (int)param_6);
  cVar14 = *(char *)(uVar22 + (int)puVar24);
  bVar7 = (char)uVar12 - 0x2a;
  uVar11 = *puVar24;
  *puVar24 = *puVar24 - uVar22;
  iVar23 = uVar22 - *param_6;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  bVar25 = 0x99 < bVar7 || uVar22 < *param_6;
  bVar7 = bVar7 + bVar16 * -6 + bVar25 * -0x60;
  bVar16 = 9 < (bVar7 & 0xf) | bVar16;
  iVar13 = CONCAT11((char)(uVar12 >> 8),bVar7 + bVar16 * -6 + (0x99 < bVar7 || bVar25) * -0x60) +
           -0x2b2b;
  bVar7 = *(byte *)((int)puVar19 + (int)puVar24);
  *(int *)(byte *)((int)puVar19 + (int)param_6) =
       *(int *)(byte *)((int)puVar19 + (int)param_6) - iVar23;
  puVar2 = puVar19;
  uVar12 = *puVar2;
  bVar15 = (byte)((uint)iVar13 >> 8);
  *(byte *)puVar2 = (byte)*puVar2 - bVar15;
  bVar8 = (byte)iVar13;
  bVar16 = 9 < (bVar8 & 0xf) | bVar16;
  bVar25 = 0x99 < bVar8 || (byte)uVar12 < bVar15;
  bVar8 = bVar8 + bVar16 * '\x06' + bVar25 * '`';
  uVar12 = CONCAT11(bVar15,bVar8 + (9 < (bVar8 & 0xf) | bVar16) * '\x06' +
                           (0x99 < bVar8 || bVar25) * '`') & 0x2520;
  uVar21 = (int)puVar20 - uVar11 & *param_6 & *(uint *)(iVar23 + (int)puVar24);
  cVar9 = (char)uVar12;
  bVar16 = (byte)(uVar12 >> 8) & *(byte *)(iVar23 + (int)param_6) & *(byte *)(iVar23 + (int)param_6)
  ;
  *(uint *)((int)puVar19 + (int)param_6) = *(uint *)((int)puVar19 + (int)param_6) & uVar21;
  *(byte *)((int)puVar19 + (int)puVar24) = *(byte *)((int)puVar19 + (int)puVar24) & bVar16;
  *(uint *)((int)puVar19 + (int)puVar24) = *(uint *)((int)puVar19 + (int)puVar24) & uVar21;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(byte *)((int)puVar19 + (int)param_6) = *(byte *)((int)puVar19 + (int)param_6) + cVar9;
  *(int *)(iVar23 + (int)puVar24) =
       (*(int *)(iVar23 + (int)puVar24) -
       CONCAT11(((bVar17 - bVar6) - cVar14) - bVar7,(char)(param_2 + 1))) -
       (uint)CARRY1(bVar16,bVar18);
  uVar11 = CONCAT11(bVar16 + bVar18,cVar9) | 10;
  out(6,uVar11);
  *(int *)(byte *)((int)puVar19 + (int)param_6) = *(int *)(byte *)((int)puVar19 + (int)param_6) + -1
  ;
  out(7,uVar11);
  *(int *)(byte *)((int)puVar19 + (int)puVar24) = *(int *)(byte *)((int)puVar19 + (int)puVar24) + -1
  ;
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


/* ===== FUN_0000_4e71 @ 0000:4e71 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs
FUN_0000_4e71(undefined2 param_1,char param_2,int param_3,undefined2 *param_4,int param_5)

{
  char *pcVar1;
  byte *pbVar2;
  char cVar3;
  undefined2 *puVar4;
  char in_CF;
  undefined2 uStack_2;
  
  puVar4 = &uStack_2;
  cVar3 = '\x0f';
  do {
    param_4 = param_4 + -1;
    puVar4 = puVar4 + -1;
    *puVar4 = *param_4;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  pcVar1 = (char *)(param_3 + param_5 + 0x60d0);
  *pcVar1 = *pcVar1 + param_2 + in_CF;
  pbVar2 = (byte *)(param_3 + param_5 + 0x40);
  *pbVar2 = *pbVar2 & (byte)((uint)param_1 >> 8);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000_4f80 @ 0000:4f80 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000_4f80(undefined2 *param_1)

{
  char cVar1;
  undefined2 *puVar2;
  undefined2 uStack_2;
  
  puVar2 = &uStack_2;
  cVar1 = '\a';
  do {
    param_1 = param_1 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *param_1;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000_506d @ 0000:506d ===== */

/* WARNING: Control flow encountered bad instruction data */

undefined2 __allregs
FUN_0000_506d(byte param_1,uint param_2,undefined2 param_3,int param_4,int param_5,byte *param_6,
             uint *param_7)

{
  byte *pbVar1;
  byte bVar2;
  bool bVar3;
  undefined1 uVar4;
  byte bVar5;
  byte bVar6;
  int iVar7;
  char cVar9;
  undefined2 uVar8;
  undefined2 uVar10;
  undefined2 unaff_ES;
  byte in_AF;
  longdouble in_ST1;
  
  uVar10 = CONCAT11((byte)((uint)param_3 >> 8) ^ (byte)param_2,(char)param_3);
  pbVar1 = param_6;
  bVar2 = *pbVar1;
  *pbVar1 = *pbVar1 >> 1;
  pbVar1 = param_6 + param_5;
  bVar6 = (byte)param_2 & 0x1f;
  bVar5 = *pbVar1;
  *pbVar1 = *pbVar1 >> bVar6;
  bVar3 = (param_2 & 0x1f) != 0;
  if (param_2 == 1) {
    uVar4 = in(0x18);
    out(0x16,uVar4);
    FUN_0000_3a9f(CONCAT11(param_1 / 0x28,uVar4),uVar10,param_4,param_5,param_7);
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  param_6[0x4f] = param_6[0x4f] + 1;
  iVar7 = CONCAT11(param_1 / 0x28,-(!bVar3 & bVar2 & 1 | (bVar3 && (bVar5 >> bVar6 - 1 & 1) != 0)))
          + -0x4f44;
  bVar5 = (byte)iVar7;
  *(byte *)(param_5 + 0x49) = *(byte *)(param_5 + 0x49) & bVar5;
  *(char *)(param_4 + -0x41) = *(char *)(param_4 + -0x41) + (char)((uint)param_4 >> 8);
  bVar2 = 9 < (bVar5 & 0xf) | in_AF;
  bVar5 = bVar5 + bVar2 * -6;
  bVar6 = bVar5 & 0xf;
  cVar9 = (char)((uint)iVar7 >> 8) - bVar2;
  *(int *)(param_4 + 0x6f1f) = (int)ROUND(in_ST1);
  uVar8 = CONCAT11(cVar9,bVar6 + (9 < bVar6 | bVar2) * -6 +
                         (0x99 < bVar6 || (CONCAT11(cVar9,bVar5) & 0xff0f) < *param_7) * -0x60);
  out(uVar10,uVar8);
  return uVar8;
}


/* ===== FUN_0000_50ac @ 0000:50ac ===== */

/* WARNING: Control flow encountered bad instruction data */

undefined2 __allregs
FUN_0000_50ac(int param_1,uint param_2,undefined2 param_3,int param_4,int param_5,byte *param_6,
             undefined2 param_7,uint *param_8)

{
  byte *pbVar1;
  bool bVar2;
  byte bVar3;
  byte bVar4;
  undefined1 uVar5;
  byte bVar6;
  byte bVar8;
  char cVar9;
  undefined2 uVar7;
  undefined2 unaff_ES;
  byte in_AF;
  bool in_ZF;
  char in_SF;
  char in_OF;
  longdouble in_ST0;
  longdouble in_ST1;
  
  if (!in_ZF && in_OF == in_SF) {
    param_3 = CONCAT11((byte)((uint)param_3 >> 8) ^ (byte)param_2,(char)param_3);
    pbVar1 = param_6;
    bVar3 = *pbVar1;
    *pbVar1 = *pbVar1 >> 1;
    pbVar1 = param_6 + param_5;
    bVar6 = (byte)param_2 & 0x1f;
    bVar4 = *pbVar1;
    *pbVar1 = *pbVar1 >> bVar6;
    bVar2 = (param_2 & 0x1f) != 0;
    bVar8 = (byte)param_1 / 0x28;
    if (param_2 == 1) {
      uVar5 = in(0x18);
      out(0x16,uVar5);
      FUN_0000_3a9f(CONCAT11(bVar8,uVar5),param_3,param_4,param_5,param_7);
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    param_6[0x4f] = param_6[0x4f] + 1;
    param_1 = CONCAT11(bVar8,-(!bVar2 & bVar3 & 1 | (bVar2 && (bVar4 >> bVar6 - 1 & 1) != 0))) +
              -0x4f44;
    *(byte *)(param_5 + 0x49) = *(byte *)(param_5 + 0x49) & (byte)param_1;
    param_4._1_1_ = (char)((uint)param_4 >> 8);
    *(char *)(param_4 + -0x41) = *(char *)(param_4 + -0x41) + param_4._1_1_;
    in_ST0 = in_ST1;
  }
  bVar3 = 9 < ((byte)param_1 & 0xf) | in_AF;
  bVar4 = (byte)param_1 + bVar3 * -6;
  bVar6 = bVar4 & 0xf;
  cVar9 = (char)((uint)param_1 >> 8) - bVar3;
  *(int *)(param_4 + 0x6f1f) = (int)ROUND(in_ST0);
  uVar7 = CONCAT11(cVar9,bVar6 + (9 < bVar6 | bVar3) * -6 +
                         (0x99 < bVar6 || (CONCAT11(cVar9,bVar4) & 0xff0f) < *param_8) * -0x60);
  out(param_3,uVar7);
  return uVar7;
}


/* ===== FUN_0000_515a @ 0000:515a ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x00005138) */
/* WARNING: Stack frame is not setup normally: Input value of stackpointer is not used */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs
FUN_0000_515a(uint param_1,int param_2,uint param_3,int param_4,int *param_5,int param_6,
             uint *param_7)

{
  uint *puVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  int *piVar4;
  int iVar5;
  undefined2 unaff_ES;
  bool in_CF;
  longdouble in_ST0;
  longdouble in_ST1;
  
  if (!in_CF) {
    return CONCAT11((char)(param_3 >> 8),(byte)param_3 ^ *(byte *)((int)param_5 + param_6 + -0x2e));
  }
  *(float *)((int)param_7 + param_4 + 0x6919) = (float)in_ST0;
  iVar5 = *param_5;
  *(int *)(param_4 + (int)param_7) = param_2 + -1;
  if (SBORROW2(param_2,1)) {
    puVar1 = (uint *)((int)param_7 + param_4 + -0x2f);
    *puVar1 = *puVar1 ^ param_3;
    uRam000f0000 = _DAT_2000_a121;
    iRam000f000a = param_5[1];
    iVar5 = param_5[3];
    iRam000f0002 = param_5[5];
    uRam000f0006 = param_5[6];
    piVar4 = param_5 + 9;
    iRam000f0004 = param_5[7] + 1;
    puVar1 = (uint *)(iRam000f0002 + iRam000f000a + -0xff);
    *puVar1 = *puVar1 << 0xe | *puVar1 >> 2;
    iRam000f000a = iRam000f000a + 1;
    out(uRam000f0006,(char)uRam000f0000);
    uRam000f0014 = 4;
    uRam000f0008 = 0x5e3d;
    uRam000f000e = 0x1c00;
    iRam000f000c = iVar5;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar3 = uRam000f0008;
    uVar2 = _DAT_2000_7676;
    *(undefined2 *)((int)piVar4 + -2) = uRam000f0008;
    *(undefined2 *)((int)piVar4 + -4) = 0x2ba9;
    out(0x46,(char)uVar3 * *(char *)(iVar5 + -0x5a));
    *(undefined1 *)(iVar5 + 0x7a06) = 0xba;
    *(undefined2 *)((int)piVar4 + -6) = uVar2;
    *(undefined2 *)((int)piVar4 + -8) = 0x5155;
    func_0x000314ba();
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  *(long *)((int)param_7 + iVar5 + 0x6b1b) = (long)ROUND(in_ST1);
  *param_7 = param_1;
  return param_1;
}


/* ===== FUN_0000_5198 @ 0000:5198 ===== */

undefined2 __allregs
FUN_0000_5198(undefined2 param_1,undefined2 param_2,int param_3,undefined2 *param_4,
             undefined2 *param_5,undefined1 *param_6,undefined1 param_7)

{
  byte *pbVar1;
  byte bVar2;
  char cVar3;
  byte bVar4;
  byte bVar5;
  undefined2 uVar6;
  byte bVar7;
  undefined2 *puVar8;
  undefined2 unaff_ES;
  byte in_AF;
  bool in_OF;
  int iStack_8ce;
  undefined2 uStack_2;
  
  puVar8 = &uStack_2;
  cVar3 = '\x17';
  do {
    param_4 = param_4 + -1;
    puVar8 = puVar8 + -1;
    *puVar8 = *param_4;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  if (!in_OF) {
    pbVar1 = (byte *)((int)param_5 + param_3 + -0x30);
    *pbVar1 = *pbVar1 ^ (byte)param_2;
    bVar4 = DAT_2000_a020 + 1;
    pbVar1 = (byte *)((int)param_5 + param_3 + 0x5400);
    *pbVar1 = *pbVar1;
    bVar2 = 9 < (bVar4 & 0xf) | in_AF;
    bVar5 = bVar4 + bVar2 * '\x06' + (0x99 < bVar4 || (*pbVar1 & 1) != 0) * '`';
    _param_7 = CONCAT21(param_3,param_7);
    param_6[param_3 + 0x4c] = param_6[param_3 + 0x4c] & bVar5;
    pbVar1 = (byte *)(param_3 + 0x3f);
    bVar4 = *pbVar1;
    bVar7 = (byte)((uint)param_3 >> 8);
    *pbVar1 = *pbVar1 + bVar7;
    out(*param_5,param_2);
    uVar6 = CONCAT11(((char)*pbVar1 < '\0') << 7 | (*pbVar1 == 0) << 6 | bVar2 << 4 |
                     ((POPCOUNT(*pbVar1) & 1U) == 0) << 2 | 2 | CARRY1(bVar4,bVar7),
                     bVar5 + (9 < (bVar5 & 0xf) | bVar2) * -6 +
                     (0x99 < bVar5 || CARRY1(bVar4,bVar7)) * -0x60);
    out(param_2,uVar6);
    return uVar6;
  }
  *param_6 = (char)param_1;
  return param_1;
}


/* ===== FUN_0000_51b9 @ 0000:51b9 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x00005204) */
/* WARNING: Removing unreachable block (ram,0x00005245) */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Removing unreachable block (ram,0x0000521a) */

undefined2 __allregs
FUN_0000_51b9(int param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 *param_5,undefined4 *param_6,undefined2 param_7)

{
  undefined4 *puVar1;
  byte bVar2;
  undefined1 uVar3;
  undefined2 uVar4;
  undefined2 unaff_ES;
  byte in_CF;
  char in_PF;
  byte in_AF;
  bool in_ZF;
  char in_SF;
  char in_OF;
  
  if (!in_ZF && in_OF == in_SF) {
    iRam000f0000 = (param_1 - (uint)in_CF) + -0x5b4a;
    uRam000f0014 = 0xd;
    uRam000f000c = 0xdd5c;
    uRam000f000e = 0x1c00;
    uRam000f0002 = param_4;
    uRam000f0004 = param_2;
    uRam000f0006 = param_3;
    puRam000f0008 = param_5;
    puRam000f000a = param_6;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    puVar1 = puRam000f000a;
    uVar3 = (undefined1)*puRam000f000a;
    out(0x46,uVar3);
    *(undefined1 *)0x7a86 = 0x3a;
    *(undefined1 *)puVar1 = uVar3;
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  bVar2 = (byte)param_1;
  out(*param_5,param_3);
  uVar4 = CONCAT11(in_SF << 7 | in_ZF << 6 | in_AF << 4 | in_PF << 2 | 2 | in_CF,
                   bVar2 + (9 < (bVar2 & 0xf) | in_AF) * -6 + (0x99 < bVar2 | in_CF) * -0x60);
  out(param_3,uVar4);
  return uVar4;
}


/* ===== FUN_0000_534b @ 0000:534b ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000_534b(byte *param_1)

{
  byte *pbVar1;
  
  pbVar1 = param_1;
  *pbVar1 = *pbVar1 << 1 | (char)*pbVar1 < '\0';
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000_5427 @ 0000:5427 ===== */

void __allregs FUN_0000_5427(void)

{
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


/* ===== FUN_0000_60f8 @ 0000:60f8 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x00006178) overlaps instruction at (ram,0x00006176)
    */

void __allregs
FUN_0000_60f8(byte param_1,int param_2,int param_3,int param_4,undefined2 param_5,int param_6,
             int param_7,char param_8,char param_9,char param_10)

{
  byte *pbVar1;
  char cVar2;
  char cVar3;
  undefined1 uVar4;
  char cVar5;
  byte bVar8;
  int iVar6;
  undefined1 *puVar7;
  char cVar10;
  char cVar13;
  char *pcVar12;
  int iVar14;
  int *piVar16;
  undefined2 uStack_2;
  char cVar9;
  int iVar11;
  char cVar15;
  
  uStack_2 = param_5;
  bVar8 = param_1 / 1;
  iVar6 = (uint)bVar8 << 8;
  if (param_2 == 1 || bVar8 == 0) {
    out(1,0);
    uVar4 = in(param_3);
    iVar6 = CONCAT11(bVar8,uVar4);
  }
  iVar6 = iVar6 + param_7;
  piVar16 = (int *)(param_6 + param_7);
  *piVar16 = *piVar16 + iVar6;
  cVar3 = DAT_2000_2004;
  cVar13 = (char)((uint)(param_3 + param_6) >> 8);
  cVar10 = (char)(param_3 + param_6) + *(char *)(param_4 + (int)piVar16) + DAT_1000_dc02;
  iVar11 = CONCAT11(cVar13,cVar10);
  bVar8 = (char)((uint)iVar6 >> 8) + *(char *)((int)&uStack_2 + (int)piVar16);
  cVar15 = (char)((uint)param_4 >> 8) + *(char *)((int)&uStack_2 + (int)piVar16);
  iVar14 = CONCAT11(cVar15,(char)param_4);
  cVar5 = (char)iVar6 + *(char *)((int)piVar16 + iVar14 + 2);
  cVar2 = *(char *)((int)piVar16 + iVar14 + 3);
  pbVar1 = (byte *)(iVar11 + 2);
  cVar9 = bVar8 + *pbVar1;
  puVar7 = (undefined1 *)CONCAT11(cVar9,cVar5);
  if (((SCARRY1(bVar8,*pbVar1)) || (!CARRY1(bVar8,*pbVar1) && cVar9 != '\0')) &&
     (SCARRY1(bVar8,*pbVar1) != cVar9 < '\0')) {
    *(char *)((int)&uStack_2 + iVar11) = cVar5;
    puVar7 = &stack0x31f9;
  }
  *(char *)((int)&uStack_2 + iVar11) = *(char *)((int)&uStack_2 + iVar11) + -0x78;
  cVar9 = (char)param_4 + cVar2 + (&stack0xa000)[iVar11];
  cVar15 = cVar15 + param_9;
  pcVar12 = (char *)CONCAT11(cVar13 + (&stack0xb800)[iVar11],
                             cVar10 + *(char *)(CONCAT11(cVar15,cVar9) + iVar11) + DAT_1000_dc02);
  bVar8 = (char)((uint)puVar7 >> 8) + param_8 + *(char *)((int)&uStack_2 + iVar11);
  cVar15 = cVar15 + *(char *)((int)&uStack_2 + iVar11);
  iVar6 = CONCAT11(cVar15,cVar9);
  cVar2 = *(char *)(iVar6 + iVar11 + 3);
  pbVar1 = (byte *)(pcVar12 + 2);
  cVar5 = bVar8 + *pbVar1;
  if (((SCARRY1(bVar8,*pbVar1)) || (!CARRY1(bVar8,*pbVar1) && cVar5 != '\0')) &&
     (SCARRY1(bVar8,*pbVar1) != cVar5 < '\0')) {
    *(char *)((int)&uStack_2 + (int)pcVar12) = (char)puVar7 + *(char *)(iVar6 + iVar11 + 2);
    cVar5 = (char)((uint)&stack0x31f7 >> 8);
  }
  *(char *)((int)&uStack_2 + (int)pcVar12) = *(char *)((int)&uStack_2 + (int)pcVar12) + -0x78;
  pcVar12[CONCAT11(cVar15,cVar9 + cVar2 + (&stack0xa000)[(int)pcVar12] + cVar5 + param_10)] =
       pcVar12[CONCAT11(cVar15,cVar9 + cVar2 + (&stack0xa000)[(int)pcVar12] + cVar5 + param_10)] +
       DAT_2000_2004;
  *pcVar12 = *pcVar12 + cVar3;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000_65c6 @ 0000:65c6 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x000065c7) overlaps instruction at (ram,0x000065c6)
    */
/* WARNING: Removing unreachable block (ram,0x0000649b) */
/* WARNING: Removing unreachable block (ram,0x000064b0) */
/* WARNING: Removing unreachable block (ram,0x000064d0) */
/* WARNING: Removing unreachable block (ram,0x000064f7) */
/* WARNING: Removing unreachable block (ram,0x00006501) */
/* WARNING: Removing unreachable block (ram,0x00006503) */

void __allregs
FUN_0000_65c6(undefined2 param_1,byte param_2,undefined2 param_3,int param_4,undefined2 *param_5,
             int param_6)

{
  byte *pbVar1;
  char *pcVar2;
  char cVar3;
  byte bVar4;
  int iVar5;
  uint uVar6;
  byte bVar7;
  byte bVar8;
  undefined2 *puVar9;
  bool bVar10;
  bool in_OF;
  undefined2 uStack_2;
  
  puVar9 = &uStack_2;
  cVar3 = '\a';
  do {
    param_5 = param_5 + -1;
    puVar9 = puVar9 + -1;
    *puVar9 = *param_5;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  if (in_OF) {
    *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + (char)((uint)param_1 >> 8);
    pbVar1 = (byte *)(param_4 + param_6 + -0x80);
    *pbVar1 = *pbVar1 & (byte)param_1;
    bVar4 = (byte)param_1 + 1;
    DAT_2000_3000 = DAT_2000_3000 & bVar4;
    bVar8 = (byte)((uint)param_3 >> 8);
    *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + bVar8;
    bVar7 = (byte)param_3;
    *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) & bVar7;
    *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) | bVar7;
    DAT_1000_d070 = DAT_1000_d070 & bVar4;
    *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) ^ bVar4;
    DAT_2000_48f8 = DAT_2000_48f8 & bVar4;
    bVar10 = (char)DAT_2000_48f8 < '\0';
    *(byte *)(param_4 + param_6 + -0xffa) = param_2;
    if (bVar10) {
      *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + bVar8;
      *(char *)(param_4 + param_6) = *(char *)(param_4 + param_6) + bVar8;
      iVar5 = CONCAT11(0xf8,bVar8) + 1;
      *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) | bVar7;
      pbVar1 = (byte *)(param_4 + param_6 + -0x80);
      bVar4 = (byte)iVar5;
      *pbVar1 = *pbVar1 & bVar4;
      *(byte *)(param_4 + param_6 + 0x670) = param_2;
      pbVar1 = (byte *)(param_4 + param_6 + 0x20);
      *pbVar1 = *pbVar1 & (byte)((uint)iVar5 >> 8);
      pbVar1 = (byte *)(param_4 + param_6 + 6);
      *pbVar1 = *pbVar1 & bVar8;
      LOCK();
      *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) | bVar8;
      UNLOCK();
      LOCK();
      *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) | bVar8;
      UNLOCK();
      *(byte *)(param_4 + param_6 + 0x8f8) = param_2;
      DAT_2000_40f8 = DAT_2000_40f8 | bVar4 + 1 | bVar8;
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    pcVar2 = (char *)(param_4 + param_6 + 0x678);
    *pcVar2 = *pcVar2 + -0x10;
    *(byte *)(param_4 + param_6 + -0xf78) = param_2;
    uVar6 = CONCAT11(0xf8,bVar8) & 0xff80;
    DAT_2000_4070 = DAT_2000_4070 + -0x68;
    *(undefined1 *)(param_4 + param_6 + 6) = (char)((uint)param_4 >> 8);
    ((undefined1 *)&DAT_2000_48f8)[param_4 + param_6] = param_2;
    DAT_2000_4080 = (undefined1)uVar6;
    DAT_1000_e070 = DAT_2000_4080;
    *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) & (byte)(uVar6 >> 8);
    *(byte *)(param_4 + param_6) = *(byte *)(param_4 + param_6) | param_2;
    *(byte *)(param_4 + param_6 + 0x670) = param_2;
    *(byte *)(param_4 + param_6 + -0x6f20) = bVar7;
    pcVar2 = (char *)(param_4 + param_6 + 0x6f8);
    *pcVar2 = *pcVar2 + -0x78;
    bRam00024870 = param_2;
  }
  *(byte *)(param_4 + param_6 + 0x670) = param_2;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000_66dc @ 0000:66dc ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x0000677b) overlaps instruction at (ram,0x0000677a)
    */
/* WARNING: Unable to track spacebase fully for stack */

void __allregs
FUN_0000_66dc(undefined2 param_1,uint param_2,uint param_3,byte *param_4,undefined2 *param_5,
             uint *param_6,uint *param_7)

{
  uint *puVar1;
  int *piVar2;
  char cVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  undefined1 uVar7;
  uint uVar8;
  uint *puVar9;
  undefined2 **ppuVar10;
  undefined1 *puVar11;
  uint uVar12;
  uint uVar13;
  undefined2 *puVar14;
  undefined2 unaff_CS;
  undefined2 *puStack_2;
  
  ppuVar10 = &puStack_2;
  puStack_2 = param_5;
  cVar3 = '\t';
  do {
    param_5 = param_5 + -1;
    ppuVar10 = ppuVar10 + -1;
    *ppuVar10 = (undefined2 *)*param_5;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  puVar9 = (uint *)((uint)param_4 | *param_6);
  uVar13 = (uint)param_6 | *(uint *)((int)puVar9 + (int)param_7) | *puVar9;
  uVar4 = CONCAT11((char)((uint)param_1 >> 8),(byte)param_1 | *param_4) |
          *(uint *)(&stack0x0009 + uVar13);
  puVar11 = (undefined1 *)((uint)&stack0x22de | *(uint *)((int)puVar9 + 0xb));
  uVar5 = uVar4;
  if (puVar11 == (undefined1 *)0x0) {
    *(undefined2 *)(char *)((int)&puStack_2 + (int)param_7) = unaff_CS;
    puVar11 = *(undefined1 **)(&stack0xb209 + uVar13);
    uVar5 = param_3 | uVar4;
    param_3 = uVar4;
  }
  uVar4 = *(uint *)((int)&puStack_2 + (uVar13 | param_3));
  *param_7 = *param_7 ^ param_2;
  *(uint *)(((uint)puVar11 | param_3) - 2) = param_3;
  uVar4 = uVar5 | uVar4 | 0xdff;
  puVar1 = param_7;
  iVar6 = (param_2 + 1 & 3) - (*puVar1 & 3);
  *puVar1 = *puVar1 + (uint)(0 < iVar6) * iVar6;
  uVar8 = *param_7 * 0xb92;
  DAT_2000_720b = (undefined1)uVar4;
  uVar12 = ((uint)puVar11 | param_3) - 2 | param_3;
  puVar14 = (undefined2 *)(uVar13 | param_3 | param_3);
  uVar5 = *(uint *)((int)&puStack_2 + (int)puVar14);
  *param_7 = *param_7 ^ uVar8;
  piVar2 = puVar14 + 1;
  out(*puVar14,param_3);
  uVar5 = uVar4 | param_3 | uVar5 | 0xdff;
  if ((POPCOUNT(uVar5 & 0xff) & 1U) != 0) {
    iVar6 = uVar5 + *(int *)((int)puVar9 + (int)param_7);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)param_7)) +
            *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = cVar3 + '\x03' + *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
                           *(char *)((int)&puStack_2 + (int)piVar2));
    *piVar2 = *piVar2 + iVar6;
    *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3);
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + '\x03' + *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
                           *(char *)((int)&puStack_2 + (int)piVar2));
    *piVar2 = *piVar2 + iVar6;
    iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) +
            *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                     (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) + '\x03' +
                     *(char *)((int)&puStack_2 + (int)piVar2)) +
            *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)piVar2 = (char)*piVar2 + cVar3;
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
    ;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)puVar9 + (int)piVar2);
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                     cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
                     *(char *)((int)&puStack_2 + (int)piVar2)) +
            *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2) +
            *(int *)((int)&puStack_2 + (int)piVar2);
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2)
                    ) + *(int *)((int)puVar9 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) =
         *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + '\x03';
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) =
         *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
    *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7);
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(int *)((int)puVar9 + (int)piVar2) =
         *(int *)((int)puVar9 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(int *)((int)&puStack_2 + (int)param_7) =
         *(int *)((int)&puStack_2 + (int)param_7) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    iVar6 = CONCAT11(uVar7,cVar3);
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(int *)((int)puVar9 + (int)piVar2) =
         *(int *)((int)puVar9 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(int *)((int)puVar9 + (int)piVar2) =
         *(int *)((int)puVar9 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(int *)((int)&puStack_2 + (int)param_7) =
         *(int *)((int)&puStack_2 + (int)param_7) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7) + '\x03' +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)piVar2 = (char)*piVar2 + cVar3;
    *(int *)((int)&puStack_2 + (int)param_7) =
         *(int *)((int)&puStack_2 + (int)param_7) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)puVar9 + (int)piVar2)) +
            *(int *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) + '\x03' +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)&puStack_2 + (int)piVar2);
    *(int *)((int)&puStack_2 + (int)piVar2) =
         *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3) + *piVar2;
    *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
    uVar7 = (undefined1)((uint)iVar6 >> 8);
    cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
            *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(int *)((int)&puStack_2 + (int)param_7) =
         *(int *)((int)&puStack_2 + (int)param_7) + CONCAT11(uVar7,cVar3);
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)puVar9 + (int)param_7)) +
            *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
    *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
    cVar3 = (char)iVar6 + '\x03';
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
    cVar3 = (char)iVar6;
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
    *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
    iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                     cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                     *(char *)((int)&puStack_2 + (int)piVar2));
    *param_7 = *param_7 + iVar6;
    uVar5 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  }
  *param_7 = *param_7 | 0xd87;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + uVar5;
  cVar3 = (char)uVar5 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)(uVar5 >> 8),cVar3 + '\x01') + *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *piVar2 +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + '\x01') +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  *piVar2 = *piVar2 + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + '\x03' +
                   *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)&puStack_2 + (int)piVar2)) + *piVar2 +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
          *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + '\x01' + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                   *(char *)((int)&puStack_2 + (int)piVar2));
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
          *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2) +
          *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                   *(char *)((int)&puStack_2 + (int)piVar2));
  *piVar2 = *piVar2 + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)piVar2) +
                   *(char *)((int)puVar9 + (int)param_7)) + *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2) +
                   *(char *)((int)puVar9 + (int)param_7)) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) +
          *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                         *(char *)((int)&puStack_2 + (int)piVar2) +
                         *(char *)((int)puVar9 + (int)piVar2));
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)puVar9 + (int)param_7)) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  *piVar2 = *piVar2 + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + '\x01') +
          *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                   *(char *)((int)&puStack_2 + (int)piVar2)) + *piVar2 +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2)
  ;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + '\x01' + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
          *(char *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)param_7) =
       *(int *)((int)&puStack_2 + (int)param_7) + CONCAT11(uVar7,cVar3);
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)piVar2 = (char)*piVar2 + cVar3;
  iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)puVar9 + (int)param_7)) + 3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)puVar9 + (int)param_7) +
          *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)param_7)) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + 0x103;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) +
          *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)param_7) +
                   *(char *)((int)puVar9 + (int)param_7)) + *(int *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *param_7 + *(int *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)param_7) +
          *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)param_7 = (char)*param_7 + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *piVar2 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *piVar2;
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + 0x200;
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)param_7 = (char)*param_7 + cVar3;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  *param_7 = *param_7 + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *piVar2;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)puVar9 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)puVar9 + (int)param_7)) +
          *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + 2;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2) +
                   *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)&puStack_2 + (int)param_7)
                   + *(char *)((int)puVar9 + (int)piVar2) +
                   *(char *)((int)&puStack_2 + (int)param_7)) +
          *(int *)((int)&puStack_2 + (int)param_7);
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)&puStack_2 + (int)param_7) + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)&puStack_2 + (int)param_7) +
          *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11(uVar7,cVar3) + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *param_7;
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)piVar2 = (char)*piVar2 + cVar3;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)&puStack_2 + (int)param_7);
  iVar6 = CONCAT11(uVar7,cVar3);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *piVar2 = *piVar2 + iVar6;
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)&puStack_2 + (int)param_7)) +
          *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)param_7)) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2)
  ;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + '\x02') +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7)
  ;
  *piVar2 = *piVar2 + iVar6;
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + '\x03') +
          *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *piVar2 = *piVar2 + iVar6;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)param_7)) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + '\x03' + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *piVar2;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + '\x01') +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(int *)((int)puVar9 + (int)param_7) = *(int *)((int)puVar9 + (int)param_7) + iVar6;
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)&puStack_2 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  uVar7 = (undefined1)((uint)iVar6 >> 8);
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  cVar3 = cVar3 + *(char *)((int)puVar9 + (int)piVar2) + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)puVar9 + (int)piVar2);
  *(char *)piVar2 = (char)*piVar2 + cVar3;
  *(int *)((int)&puStack_2 + (int)piVar2) =
       *(int *)((int)&puStack_2 + (int)piVar2) + CONCAT11(uVar7,cVar3);
  iVar6 = CONCAT11(uVar7,cVar3 + *(char *)((int)puVar9 + (int)param_7)) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2) +
                   *(char *)((int)&puStack_2 + (int)param_7)) + *(int *)((int)puVar9 + (int)param_7)
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)puVar9 + (int)param_7)) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7)
          + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)puVar9 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3 + *(char *)((int)puVar9 + (int)piVar2)) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7) +
                   *(char *)((int)puVar9 + (int)param_7)) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),cVar3) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + 3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *piVar2 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)param_7))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7);
  cVar3 = (char)iVar6 + *(char *)((int)puVar9 + (int)piVar2) +
          *(char *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   cVar3 + *(char *)((int)puVar9 + (int)piVar2) +
                   *(char *)((int)puVar9 + (int)param_7) + *(char *)((int)&puStack_2 + (int)param_7)
                  ) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2);
  *(char *)((int)puVar9 + (int)param_7) = *(char *)((int)puVar9 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)param_7);
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2) + 3;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7);
  cVar3 = (char)iVar6;
  *(char *)((int)puVar9 + (int)piVar2) = *(char *)((int)puVar9 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)piVar2);
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(char *)((int)&puStack_2 + (int)piVar2) = *(char *)((int)&puStack_2 + (int)piVar2) + (char)iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)&puStack_2 + (int)param_7);
  *(char *)((int)&puStack_2 + (int)param_7) =
       *(char *)((int)&puStack_2 + (int)param_7) + (char)iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)puVar9 + (int)piVar2);
  cVar3 = (char)iVar6;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  *(char *)((int)&puStack_2 + (int)param_7) = *(char *)((int)&puStack_2 + (int)param_7) + cVar3;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)puVar9 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  *(int *)((int)&puStack_2 + (int)piVar2) = *(int *)((int)&puStack_2 + (int)piVar2) + iVar6;
  *(int *)((int)puVar9 + (int)piVar2) = *(int *)((int)puVar9 + (int)piVar2) + iVar6;
  iVar6 = iVar6 + *(int *)((int)&puStack_2 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2)
          + *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  *(int *)((int)&puStack_2 + (int)param_7) = *(int *)((int)&puStack_2 + (int)param_7) + iVar6;
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),
                   (char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2) +
                   *(char *)((int)&puStack_2 + (int)param_7) +
                   *(char *)((int)&puStack_2 + (int)piVar2)) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)piVar2) +
          *(int *)((int)&puStack_2 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2) +
          *(int *)((int)puVar9 + (int)piVar2) + *(int *)((int)puVar9 + (int)param_7) +
          *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  iVar6 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)param_7) + *(int *)((int)&puStack_2 + (int)piVar2);
  uVar5 = CONCAT11((char)((uint)iVar6 >> 8),(char)iVar6 + *(char *)((int)&puStack_2 + (int)piVar2))
          + *(int *)((int)puVar9 + (int)piVar2);
  *(char *)param_7 = (char)*param_7 + (char)uVar5;
  *(uint *)((int)puVar9 + (int)piVar2) = *(uint *)((int)puVar9 + (int)piVar2) | uVar5;
  *(char *)(puVar9 + 7) = (char)puVar9[7] + (char)param_3;
  *(undefined2 *)(uVar12 - 2) = param_7;
  *(undefined2 *)(uVar12 - 4) = unaff_CS;
  *(undefined2 *)(uVar12 - 6) = param_7;
  *(undefined2 *)(uVar12 - 8) = unaff_CS;
  *(undefined2 *)(uVar12 - 10) = param_7;
  *(undefined2 *)(uVar12 - 0xc) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xe) = param_7;
  *(undefined2 *)(uVar12 - 0x10) = unaff_CS;
  *(undefined2 *)(uVar12 - 0x12) = param_7;
  *(undefined2 *)(uVar12 - 0x14) = unaff_CS;
  *(undefined2 *)(uVar12 - 0x16) = param_7;
  *(undefined2 *)(uVar12 - 0x18) = unaff_CS;
  *(undefined2 *)(uVar12 - 0x1a) = param_7;
  *(undefined2 *)(uVar12 - 0x1c) = unaff_CS;
  *(uint *)(uVar12 - 0x1e) = uVar5;
  *(uint *)(uVar12 - 0x20) = uVar8;
  *(uint *)(uVar12 - 0x22) = param_3;
  *(undefined2 *)(uVar12 - 0x24) = puVar9;
  *(uint *)(uVar12 - 0x26) = uVar12 - 0x1c;
  *(undefined2 ***)(uVar12 - 0x28) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x2a) = piVar2;
  *(undefined2 *)(uVar12 - 0x2c) = param_7;
  *(undefined2 *)(uVar12 - 0x2e) = unaff_CS;
  *(uint *)(uVar12 - 0x30) = uVar5;
  *(uint *)(uVar12 - 0x32) = uVar8;
  *(uint *)(uVar12 - 0x34) = param_3;
  *(undefined2 *)(uVar12 - 0x36) = puVar9;
  *(uint *)(uVar12 - 0x38) = uVar12 - 0x2e;
  *(undefined2 ***)(uVar12 - 0x3a) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x3c) = piVar2;
  *(undefined2 *)(uVar12 - 0x3e) = param_7;
  *(undefined2 *)(uVar12 - 0x40) = unaff_CS;
  *(uint *)(uVar12 - 0x42) = uVar5;
  *(uint *)(uVar12 - 0x44) = uVar8;
  *(uint *)(uVar12 - 0x46) = param_3;
  *(undefined2 *)(uVar12 - 0x48) = puVar9;
  *(uint *)(uVar12 - 0x4a) = uVar12 - 0x40;
  *(undefined2 ***)(uVar12 - 0x4c) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x4e) = piVar2;
  *(undefined2 *)(uVar12 - 0x50) = param_7;
  *(undefined2 *)(uVar12 - 0x52) = unaff_CS;
  *(uint *)(uVar12 - 0x54) = uVar5;
  *(uint *)(uVar12 - 0x56) = uVar8;
  *(uint *)(uVar12 - 0x58) = param_3;
  *(undefined2 *)(uVar12 - 0x5a) = puVar9;
  *(uint *)(uVar12 - 0x5c) = uVar12 - 0x52;
  *(undefined2 ***)(uVar12 - 0x5e) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x60) = piVar2;
  *(undefined2 *)(uVar12 - 0x62) = param_7;
  *(undefined2 *)(uVar12 - 100) = unaff_CS;
  *(uint *)(uVar12 - 0x66) = uVar5;
  *(uint *)(uVar12 - 0x68) = uVar8;
  *(uint *)(uVar12 - 0x6a) = param_3;
  *(undefined2 *)(uVar12 - 0x6c) = puVar9;
  *(uint *)(uVar12 - 0x6e) = uVar12 - 100;
  *(undefined2 ***)(uVar12 - 0x70) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x72) = piVar2;
  *(undefined2 *)(uVar12 - 0x74) = param_7;
  *(undefined2 *)(uVar12 - 0x76) = unaff_CS;
  *(uint *)(uVar12 - 0x78) = uVar5;
  *(uint *)(uVar12 - 0x7a) = uVar8;
  *(uint *)(uVar12 - 0x7c) = param_3;
  *(undefined2 *)(uVar12 - 0x7e) = puVar9;
  *(uint *)(uVar12 - 0x80) = uVar12 - 0x76;
  *(undefined2 ***)(uVar12 - 0x82) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x84) = piVar2;
  *(undefined2 *)(uVar12 - 0x86) = param_7;
  *(undefined2 *)(uVar12 - 0x88) = unaff_CS;
  *(uint *)(uVar12 - 0x8a) = uVar5;
  *(uint *)(uVar12 - 0x8c) = uVar8;
  *(uint *)(uVar12 - 0x8e) = param_3;
  *(undefined2 *)(uVar12 - 0x90) = puVar9;
  *(uint *)(uVar12 - 0x92) = uVar12 - 0x88;
  *(undefined2 ***)(uVar12 - 0x94) = &puStack_2;
  *(undefined2 *)(uVar12 - 0x96) = piVar2;
  *(undefined2 *)(uVar12 - 0x98) = param_7;
  *(undefined2 *)(uVar12 - 0x9a) = unaff_CS;
  *(undefined2 *)(uVar12 - 0x9c) = param_7;
  *(undefined2 *)(uVar12 - 0x9e) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xa0) = param_7;
  *(undefined2 *)(uVar12 - 0xa2) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xa4) = param_7;
  *(undefined2 *)(uVar12 - 0xa6) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xa8) = param_7;
  *(undefined2 *)(uVar12 - 0xaa) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xac) = param_7;
  *(undefined2 *)(uVar12 - 0xae) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xb0) = param_7;
  *(undefined2 *)(uVar12 - 0xb2) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xb4) = param_7;
  *(undefined2 *)(uVar12 - 0xb6) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xb8) = param_7;
  *(undefined2 *)(uVar12 - 0xba) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xbc) = param_7;
  *(undefined2 *)(uVar12 - 0xbe) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xc0) = param_7;
  *(undefined2 *)(uVar12 - 0xc2) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xc4) = param_7;
  *(undefined2 *)(uVar12 - 0xc6) = unaff_CS;
  *(undefined2 *)(uVar12 - 200) = param_7;
  *(undefined2 *)(uVar12 - 0xca) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xcc) = param_7;
  *(undefined2 *)(uVar12 - 0xce) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xd0) = param_7;
  *(undefined2 *)(uVar12 - 0xd2) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xd4) = param_7;
  *(undefined2 *)(uVar12 - 0xd6) = unaff_CS;
  *(undefined2 *)(uVar12 - 0xd8) = param_7;
  *(undefined2 *)(uVar12 - 0xda) = unaff_CS;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

