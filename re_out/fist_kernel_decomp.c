
/* ===== FUN_00000944 @ 00000944 ===== */

/* WARNING: Removing unreachable block (ram,0x00000950) */
/* WARNING: Removing unreachable block (ram,0x0000095e) */

undefined2 __allregs
FUN_00000944(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5)

{
  undefined2 unaff_ES;
  undefined2 uVar1;
  
  uVar1 = 0x1c00;
  uRam000f0004._1_1_ = (undefined1)((ushort)param_1 >> 8);
  uRam000f0004._0_1_ = 0x24;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2505;
  uRam000f0002 = (undefined2)DAT_0000093c;
  uRam000f000e = 0x1c00;
  uRam000f0006 = param_2;
  uRam000f0008 = param_4;
  uRam000f000a = param_5;
  uRam000f000c = param_3;
  (*(code *)0xf0100)();
  uRam000f0004 = CONCAT11(uRam000f0004._1_1_,0x23);
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2505;
  uRam000f0002 = (undefined2)DAT_00000940;
  uRam000f000c = param_3;
  uRam000f000e = uVar1;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return uRam000f0000;
}


/* ===== FUN_00000961 @ 00000961 ===== */

/* WARNING: Removing unreachable block (ram,0x00000990) */
/* WARNING: Removing unreachable block (ram,0x00000975) */
/* WARNING: Removing unreachable block (ram,0x00000967) */
/* WARNING: Removing unreachable block (ram,0x000009a5) */

undefined2 __allregs
FUN_00000961(undefined2 param_1,undefined2 param_2,undefined4 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 uVar1;
  undefined2 unaff_ES;
  undefined2 uVar2;
  
  uVar2 = 0x1c00;
  uRam000f0004._1_1_ = (undefined1)((ushort)param_1 >> 8);
  uRam000f0004._0_1_ = 0x24;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2503;
  uRam000f0002 = (undefined2)param_3;
  uRam000f000e = 0x1c00;
  uRam000f0006 = param_2;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  (*(code *)0xf0100)();
  uVar1 = (undefined2)((uint)param_3 >> 0x10);
  DAT_0000093c = CONCAT22(uVar1,uRam000f0002);
  uRam000f0004._0_1_ = 0x23;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2503;
  uRam000f000c = param_4;
  uRam000f000e = uVar2;
  (*(code *)0xf0100)();
  DAT_00000940 = CONCAT22(uVar1,uRam000f0002);
  uRam000f0004._0_1_ = 0x23;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2505;
  uRam000f0002 = 6;
  uRam000f000c = param_4;
  uRam000f000e = uVar2;
  (*(code *)0xf0100)();
  uRam000f0004 = CONCAT11(uRam000f0004._1_1_,0x24);
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x2505;
  uRam000f0002 = 3;
  uRam000f000c = param_4;
  uRam000f000e = uVar2;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return uRam000f0000;
}


/* ===== FUN_00000ac4 @ 00000ac4 ===== */

/* WARNING: Removing unreachable block (ram,0x00000b10) */
/* WARNING: Removing unreachable block (ram,0x00000b1b) */

void __allregs
FUN_00000ac4(uint param_1,byte param_2,undefined2 param_3,undefined4 param_4,undefined2 param_5,
            uint *param_6,undefined2 param_7,undefined2 param_8,undefined2 param_9)

{
  uint uVar1;
  byte *pbVar2;
  undefined1 uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  uint *puVar8;
  undefined2 unaff_ES;
  
  pbVar2 = (byte *)((param_1 & 0xffffff24) * 2);
  *pbVar2 = *pbVar2 & param_2;
  *(char *)(param_6 + 0x2ad) = (char)param_6[0x2ad] + (char)((uint)param_4 >> 8);
  uVar5 = (uint)((int)&LAB_0000bf03 - (int)param_6) >> 3;
  uVar4 = 0;
  do {
    uVar4 = ((uVar4 ^ *param_6) >> 1 | (uint)(((uVar4 ^ *param_6) & 1) != 0) << 0x1f) ^ param_6[1];
    uVar7 = uVar4 >> 1;
    uVar4 = uVar7 | (uint)((uVar4 & 1) != 0) << 0x1f;
    param_6 = param_6 + 2;
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0);
  if (uVar4 != DAT_00000aa8) {
    uVar3 = (undefined1)uVar7;
    uRam000f0002 = (undefined2)DAT_00000aa8;
    uRam000f0008 = SUB42(param_6,0);
    uRam000f000a = param_7;
    uRam000f000c = param_5;
    uRam000f0010 = unaff_ES;
    if (DAT_00000aa8 != 0x1234dead) {
      uRam000f0000 = CONCAT11(9,uVar3);
      uRam000f0014 = 0x21;
      uRam000f0004 = 0;
      uRam000f0006 = 0xc00;
      uRam000f000e = 0x1c00;
      (*(code *)0xf0100)();
      FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,param_5,uRam000f0008,uRam000f000a);
      return;
    }
    uRam000f0000 = CONCAT11(9,uVar3);
    uRam000f0014 = 0x21;
    uRam000f0004 = 0;
    uRam000f0006 = 0xbf2;
    uRam000f000e = 0x1c00;
    (*(code *)0xf0100)();
    app_entry(uRam000f0004,uRam000f0006,uRam000f0002,param_5,uRam000f0008,uRam000f000a,param_9);
    return;
  }
  iVar6 = 4;
  puVar8 = &DAT_00000ab4;
  uVar4 = 0x9324abe1;
  uVar5 = 0x439a9efa;
  do {
    *puVar8 = *puVar8 ^ 0x9324abe1;
    puVar8[1] = puVar8[1] ^ 0x439a9efa;
    puVar8 = puVar8 + 2;
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  puVar8 = (uint *)(s_EXTENDER__Memory_Error__EXTENDER_00000b91 + 0x8f);
  uVar7 = 0x165c;
  do {
    *puVar8 = *puVar8 ^ uVar4;
    puVar8[1] = puVar8[1] ^ uVar5;
    uVar1 = uVar5 >> 1;
    uVar5 = uVar1 | (uint)((uVar5 & 1) != 0) << 0x1f;
    uVar4 = (uVar4 >> 1 | (uint)((uVar4 & 1) != 0) << 0x1f) ^ uVar7;
    puVar8 = puVar8 + 2;
    uVar7 = uVar7 - 1;
  } while (uVar7 != 0);
  app_entry(0,param_3,(short)uVar1,param_5,(short)puVar8,param_7,param_8);
  return;
}


/* ===== FUN_00000aec @ 00000aec ===== */

/* WARNING: Instruction at (ram,0x00000aed) overlaps instruction at (ram,0x00000aec)
    */
/* WARNING: Removing unreachable block (ram,0x00000b10) */
/* WARNING: Removing unreachable block (ram,0x00000b1b) */
/* WARNING: Type propagation algorithm not settling */

void __allregs
FUN_00000aec(uint param_1,int param_2,undefined2 param_3,undefined4 *param_4,uint *param_5,
            undefined2 param_6,undefined2 param_7,undefined2 param_8)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  char cVar3;
  uint uVar4;
  undefined1 uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  BADSPACEBASE *in_ESP;
  undefined2 uVar10;
  uint *puVar11;
  undefined2 unaff_ES;
  undefined4 *apuStack_8 [2];
  
  puVar1 = apuStack_8 + 1;
  cVar3 = '\x03';
  puVar2 = param_4;
  do {
    puVar2 = puVar2 + -1;
    puVar1 = puVar1 + -1;
    *puVar1 = *puVar2;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  while( true ) {
    uVar6 = param_1 >> 1 | (uint)((param_1 & 1) != 0) << 0x1f;
    puVar11 = param_5 + 2;
    param_2 = param_2 + -1;
    if (param_2 == 0) break;
    uVar6 = uVar6 ^ *puVar11;
    param_1 = uVar6 >> 1 | (uint)((uVar6 & 1) != 0) << 0x1f;
    param_1 = param_1 ^ param_5[3];
    param_5 = puVar11;
  }
  uVar10 = SUB42(apuStack_8 + 1,0);
  apuStack_8[1] = param_4;
  if (uVar6 != DAT_00000aa8) {
    uVar5 = (undefined1)(param_1 >> 1);
    uRam000f0002 = (undefined2)DAT_00000aa8;
    uRam000f0008 = SUB42(puVar11,0);
    uRam000f000a = param_6;
    uRam000f000c = uVar10;
    uRam000f0010 = unaff_ES;
    if (DAT_00000aa8 != 0x1234dead) {
      uRam000f0000 = CONCAT11(9,uVar5);
      uRam000f0014 = 0x21;
      uRam000f0004 = 0;
      uRam000f0006 = 0xc00;
      uRam000f000e = 0x1c00;
      (*(code *)0xf0100)();
      FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,uVar10,uRam000f0008,uRam000f000a);
      return;
    }
    uRam000f0000 = CONCAT11(9,uVar5);
    uRam000f0014 = 0x21;
    uRam000f0004 = 0;
    uRam000f0006 = 0xbf2;
    uRam000f000e = 0x1c00;
    (*(code *)0xf0100)();
    app_entry(uRam000f0004,uRam000f0006,uRam000f0002,uVar10,uRam000f0008,uRam000f000a,param_8);
    return;
  }
  iVar7 = 4;
  puVar11 = &DAT_00000ab4;
  uVar6 = 0x9324abe1;
  uVar9 = 0x439a9efa;
  do {
    *puVar11 = *puVar11 ^ 0x9324abe1;
    puVar11[1] = puVar11[1] ^ 0x439a9efa;
    puVar11 = puVar11 + 2;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  puVar11 = (uint *)(s_EXTENDER__Memory_Error__EXTENDER_00000b91 + 0x8f);
  uVar8 = 0x165c;
  do {
    *puVar11 = *puVar11 ^ uVar6;
    puVar11[1] = puVar11[1] ^ uVar9;
    uVar4 = uVar9 >> 1;
    uVar9 = uVar4 | (uint)((uVar9 & 1) != 0) << 0x1f;
    uVar6 = (uVar6 >> 1 | (uint)((uVar6 & 1) != 0) << 0x1f) ^ uVar8;
    puVar11 = puVar11 + 2;
    uVar8 = uVar8 - 1;
  } while (uVar8 != 0);
  app_entry(0,param_3,(short)uVar4,uVar10,(short)puVar11,param_6,param_7);
  return;
}


/* ===== FUN_00000b64 @ 00000b64 ===== */

/* WARNING: Removing unreachable block (ram,0x00000b6e) */
/* WARNING: Removing unreachable block (ram,0x00000b77) */

void __allregs
FUN_00000b64(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  undefined2 uVar1;
  
  uVar1 = 0x1c00;
  do {
    FUN_00000944(param_1,param_2,param_4,param_5,param_6);
    uRam000f0014 = 0x21;
    uRam000f0000 = 0x4c00;
    uRam000f0002 = param_3;
    uRam000f0004 = param_1;
    uRam000f0006 = param_2;
    uRam000f0008 = param_5;
    uRam000f000a = param_6;
    uRam000f000c = param_4;
    uRam000f000e = uVar1;
    (*(code *)0xf0100)();
    uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
    uRam000f0014 = 0x21;
    uRam000f0006 = 0xb91;
    uRam000f000c = param_4;
    uRam000f000e = uVar1;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    param_1 = uRam000f0004;
    param_3 = uRam000f0002;
    param_5 = uRam000f0008;
    param_6 = uRam000f000a;
    param_2 = uRam000f0006;
  } while( true );
}


/* ===== FUN_00000b7b @ 00000b7b ===== */

/* WARNING: Removing unreachable block (ram,0x00000b82) */

void __allregs
FUN_00000b7b(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  
  uRam000f0000 = CONCAT11(9,param_1);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xbac;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_2;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,param_4,uRam000f0008,uRam000f000a);
  return;
}


/* ===== FUN_00000b86 @ 00000b86 ===== */

/* WARNING: Removing unreachable block (ram,0x00000b8d) */

void __allregs
FUN_00000b86(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  
  uRam000f0000 = CONCAT11(9,param_1);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xbd0;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_2;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,param_4,uRam000f0008,uRam000f000a);
  return;
}


/* ===== FUN_00000d86 @ 00000d86 ===== */

/* WARNING: Removing unreachable block (ram,0x00000e84) */
/* WARNING: Removing unreachable block (ram,0x00000dbb) */
/* WARNING: Removing unreachable block (ram,0x00000db2) */
/* WARNING: Removing unreachable block (ram,0x00000da9) */
/* WARNING: Removing unreachable block (ram,0x00000e9f) */
/* WARNING: Removing unreachable block (ram,0x00000dec) */
/* WARNING: Removing unreachable block (ram,0x00000e3c) */
/* WARNING: Removing unreachable block (ram,0x00000e3e) */
/* WARNING: Removing unreachable block (ram,0x00000e9d) */
/* WARNING: Removing unreachable block (ram,0x00000ea2) */

undefined4 __allregs
FUN_00000d86(uint param_1,undefined2 param_2,undefined2 param_3,int param_4,int param_5,
            undefined1 *param_6,undefined4 *param_7)

{
  ushort *puVar1;
  byte *pbVar2;
  short sVar3;
  uint uVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  ushort uVar7;
  undefined2 unaff_ES;
  undefined2 uVar8;
  
  uVar8 = 0x1c00;
  uRam000f0002 = (undefined2)param_4;
  puVar1 = (ushort *)(param_4 + 0x69);
  uVar7 = (ushort)param_5;
  sVar3 = (uVar7 & 3) - (*puVar1 & 3);
  *puVar1 = *puVar1 + (ushort)(0 < sVar3) * sVar3;
  sRam000f0008 = (short)param_6 + 1;
  out(*param_6,param_3);
  pbVar2 = (byte *)segment(0x2ba9,uVar7 + 0x75);
  *pbVar2 = *pbVar2 & (byte)param_2;
  sRam000f000a = (short)param_7 + 4;
  uVar6 = in(param_3);
  *param_7 = uVar6;
  uVar4 = param_1 & 0xffffff00;
  puVar5 = (undefined1 *)
           ((uint)CONCAT21((short)(uVar4 >> 0x10),
                           (byte)(uVar4 >> 8) | *(byte *)(uVar4 + param_5 * 8)) << 8);
  *puVar5 = 0;
  *puVar5 = *puVar5;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x900;
  uRam000f0006 = 0xd88;
  uRam000f000e = 0x1c00;
  uRam000f0004 = param_2;
  uRam000f000c = uVar7;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xab4;
  uRam000f000c = uVar7;
  uRam000f000e = uVar8;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xd9a;
  uRam000f000c = uVar7;
  uRam000f000e = uVar8;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar6 = FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,uVar7,sRam000f0008,sRam000f000a);
  return uVar6;
}


/* ===== app_entry @ 00000d9d ===== */

/* WARNING: Removing unreachable block (ram,0x00000db2) */
/* WARNING: Removing unreachable block (ram,0x00000da9) */
/* WARNING: Removing unreachable block (ram,0x00000dbb) */

void __allregs
app_entry(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
         undefined2 param_5,undefined2 param_6,undefined2 param_7)

{
  undefined4 uVar1;
  undefined2 uVar2;
  undefined2 in_register_00000016;
  undefined4 uVar3;
  undefined2 unaff_ES;
  undefined2 uVar4;
  
  uVar3 = CONCAT22(in_register_00000016,param_4);
  uVar4 = 0x1c00;
  uVar1 = FUN_00000dc2(param_1,param_2,param_3,uVar3,param_5,param_6,0xda2,param_7);
  uRam000f0000 = CONCAT11(9,(char)uVar1);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xd88;
  uVar2 = (undefined2)uVar3;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_1;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = uVar2;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xab4;
  uRam000f000c = uVar2;
  uRam000f000e = uVar4;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0xd9a;
  uRam000f000c = uVar2;
  uRam000f000e = uVar4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  FUN_00000b64(uRam000f0004,uRam000f0006,uRam000f0002,uVar2,uRam000f0008,uRam000f000a);
  return;
}


/* ===== FUN_00000dc2 @ 00000dc2 ===== */

/* WARNING: Removing unreachable block (ram,0x00000e84) */
/* WARNING: Removing unreachable block (ram,0x00000de0) */
/* WARNING: Removing unreachable block (ram,0x00000dd1) */
/* WARNING: Removing unreachable block (ram,0x00000e9f) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs
FUN_00000dc2(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined4 param_4,
            undefined2 param_5,undefined2 param_6,undefined2 param_7,undefined2 param_8)

{
  int *piVar1;
  ushort uVar2;
  undefined1 uVar3;
  undefined2 uVar4;
  undefined2 extraout_var;
  undefined4 uVar5;
  undefined2 extraout_var_00;
  undefined2 extraout_var_01;
  int iVar6;
  undefined2 *puVar7;
  uint uVar8;
  BADSPACEBASE *in_ESP;
  undefined2 uVar9;
  undefined2 uVar10;
  byte *pbVar11;
  short sVar12;
  byte *pbVar13;
  undefined2 uVar14;
  undefined2 uVar15;
  undefined2 uStack00000002;
  undefined2 uStack00000004;
  
  uVar15 = 0x1c00;
  uVar14 = 0x2ba9;
  DAT_00000d82 = 0xd86;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x250d;
  uVar9 = (undefined2)param_4;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_1;
  uRam000f0006 = param_2;
  uRam000f0008 = param_5;
  sRam000f000a = param_6;
  uRam000f000c = uVar9;
  (*(code *)0xf0100)();
  _DAT_00000c83 = CONCAT22(extraout_var,uRam000f0000);
  uRam000f0000 = CONCAT11(0x48,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0002 = 1;
  uRam000f0010 = param_7;
  uRam000f000c = uVar9;
  uRam000f000e = uVar15;
  (*(code *)0xf0100)();
  uVar4 = uRam000f0006;
  uVar2 = uRam000f0002;
  puVar7 = (undefined2 *)(uint)uRam000f0002;
  if ((bool)uRam000f0012) {
    uVar5 = FUN_00000f64(0xdee);
    return uVar5;
  }
  DAT_00000c91 = uRam000f0000;
  DAT_00000c8f = 0;
  DAT_00000c93 = puVar7;
  _DAT_00000c97 = uVar15;
  *puVar7 = 0;
  piVar1 = (int *)segment(uVar15,0x91f);
  pbVar11 = (byte *)(*piVar1 + 0x80);
  pbVar13 = (byte *)(puVar7 + 0x1fb);
  for (iVar6 = *(byte *)(*piVar1 + 0x80) + 1; iVar6 != 0; iVar6 = iVar6 + -1) {
    *pbVar13 = *pbVar11;
    pbVar11 = pbVar11 + 1;
    pbVar13 = pbVar13 + 1;
  }
  sVar12 = (short)pbVar13 + 1;
  *pbVar13 = 0;
  uStack00000004 = 0xe46;
  param_8 = 0;
  FUN_00000ea8(0,0,uVar4,uVar2,uVar9,(short)pbVar11,sVar12);
  uVar9 = SUB42(pbVar11,0);
  uVar4 = (undefined2)iVar6;
  DAT_00000c24 = &param_8;
  uStack00000002 = 0xe5e;
  uStack00000004 = 0;
  s_EXTENDER__Memory_Error__EXTENDER_00000b91._143_4_ = param_4;
  DAT_00000c28._0_2_ = uVar14;
  FUN_0000108a();
  uVar10 = (undefined2)param_4;
  uStack00000002 = 0;
  uVar5 = FUN_000011f0();
  FUN_000084c0(uVar5);
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x4b00;
  uRam000f0002 = 0xc35;
  uRam000f0006 = 0xc2c;
  _DAT_00000c39 = uVar15;
  _DAT_00000c3f = uVar15;
  uRam000f0004 = uVar4;
  uRam000f0008 = uVar9;
  sRam000f000a = sVar12;
  uRam000f000c = uVar10;
  uRam000f000e = uVar15;
  uRam000f0010 = param_7;
  (*(code *)0xf0100)();
  uVar8 = (uint)uRam000f0002;
  uStack00000002 = 0;
  uVar4 = uRam000f0004;
  uVar9 = uRam000f0008;
  sVar12 = sRam000f000a;
  FUN_000078da(uRam000f0000,uVar8);
  uVar14 = (undefined2)uVar8;
  uVar3 = FUN_000011f1();
  if (DAT_00000d82 != 1) {
    uRam000f0000 = CONCAT11(9,uVar3);
    uRam000f0014 = 0x21;
    uRam000f0006 = (undefined2)DAT_00000d82;
    uRam000f0002 = uVar14;
    uRam000f0004 = uVar4;
    uRam000f0008 = uVar9;
    sRam000f000a = sVar12;
    uRam000f000c = uVar10;
    uRam000f000e = uVar15;
    uRam000f0010 = param_7;
    (*(code *)0xf0100)();
    return CONCAT22(extraout_var_00,uRam000f0000);
  }
  uVar4 = FUN_00005e5d(uVar3,uVar4,uVar14,uVar10,uVar9,sVar12);
  return CONCAT22(extraout_var_01,uVar4);
}


/* ===== FUN_00000dee @ 00000dee ===== */

/* WARNING: Removing unreachable block (ram,0x00000e84) */
/* WARNING: Removing unreachable block (ram,0x00000e9f) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs
FUN_00000dee(undefined4 param_1,int param_2,undefined4 param_3,undefined2 *param_4,int param_5,
            int param_6)

{
  int *piVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  char *pcVar4;
  undefined4 uVar5;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  undefined2 uVar6;
  int iVar7;
  undefined2 uVar8;
  uint uVar9;
  BADSPACEBASE *in_ESP;
  undefined2 uVar10;
  int iVar11;
  undefined2 uVar12;
  byte *pbVar13;
  short sVar14;
  byte *pbVar15;
  undefined2 unaff_ES;
  undefined2 uStack_1a;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined1 *local_10;
  undefined1 *local_c;
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  uVar3 = 0x1c00;
  local_c = (undefined1 *)&uStack_8;
  local_10 = (undefined1 *)&local_c;
  iVar11 = param_5 + -1;
  *(byte *)(param_5 + 0x51) = *(byte *)(param_5 + 0x51) & (byte)param_1;
  *(char *)(param_6 + -0x3b) = *(char *)(param_6 + -0x3b) + (char)((uint)(param_2 + -2) >> 8);
  pcVar4 = (char *)(param_2 + 0xc8d);
  *pcVar4 = *pcVar4 + (char)pcVar4;
  _DAT_00000c97 = 0x1c00;
  DAT_00000c93 = param_4;
  *param_4 = 0;
  piVar1 = (int *)segment(0x1c00,0x91f);
  pbVar13 = (byte *)(*piVar1 + 0x80);
  pbVar15 = (byte *)(param_4 + 0x1fb);
  for (iVar7 = *(byte *)(*piVar1 + 0x80) + 1; iVar7 != 0; iVar7 = iVar7 + -1) {
    *pbVar15 = *pbVar13;
    pbVar13 = pbVar13 + 1;
    pbVar15 = pbVar15 + 1;
  }
  sVar14 = (short)pbVar15 + 1;
  *pbVar15 = 0;
  uStack_1a = 0;
  uStack_18 = param_3;
  uStack_14 = param_3;
  uStack_8 = param_3;
  uStack_4 = param_1;
  FUN_00000ea8(0,0,(short)param_3,(short)param_4,(short)iVar11,(short)pbVar13,sVar14);
  uVar12 = SUB42(pbVar13,0);
  uVar6 = (undefined2)iVar7;
  DAT_00000c24 = &uStack_1a;
  DAT_00000c28._0_2_ = 0x2ba9;
  s_EXTENDER__Memory_Error__EXTENDER_00000b91._143_4_ = iVar11;
  FUN_0000108a();
  uVar10 = (undefined2)iVar11;
  uVar5 = FUN_000011f0();
  FUN_000084c0(uVar5);
  _DAT_00000c39 = 0x1c00;
  _DAT_00000c3f = 0x1c00;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x4b00;
  uRam000f0002 = 0xc35;
  uRam000f0006 = 0xc2c;
  uRam000f000e = 0x1c00;
  uRam000f0004 = uVar6;
  uRam000f0008 = uVar12;
  sRam000f000a = sVar14;
  uRam000f000c = uVar10;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar9 = (uint)uRam000f0002;
  uVar6 = uRam000f0004;
  uVar12 = uRam000f0008;
  sVar14 = sRam000f000a;
  FUN_000078da(uRam000f0000,uVar9);
  uVar8 = (undefined2)uVar9;
  uVar2 = FUN_000011f1();
  if (DAT_00000d82 != 1) {
    uRam000f0000 = CONCAT11(9,uVar2);
    uRam000f0014 = 0x21;
    uRam000f0006 = (undefined2)DAT_00000d82;
    uRam000f0002 = uVar8;
    uRam000f0004 = uVar6;
    uRam000f0008 = uVar12;
    sRam000f000a = sVar14;
    uRam000f000c = uVar10;
    uRam000f000e = uVar3;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    return CONCAT22(extraout_var,uRam000f0000);
  }
  uVar3 = FUN_00005e5d(uVar2,uVar6,uVar8,uVar10,uVar12,sVar14);
  return CONCAT22(extraout_var_00,uVar3);
}


/* ===== FUN_00000ea8 @ 00000ea8 ===== */

/* WARNING: Removing unreachable block (ram,0x00000eaa) */

void __allregs
FUN_00000ea8(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6,undefined2 param_7)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined2 unaff_ES;
  undefined2 extraout_var;
  
  uRam000f0000 = CONCAT11(0x62,param_1);
  uRam000f0014 = 0x21;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_4;
  uRam000f0004 = param_2;
  uRam000f0006 = param_3;
  uRam000f0008 = param_6;
  uRam000f000a = param_7;
  uRam000f000c = param_5;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar1 = CONCAT22(extraout_var,uRam000f0004);
  DAT_00000c8d = uRam000f0002;
  puVar2 = &DAT_00000c43;
  FUN_00000f0e(uVar1,uRam000f0002,0xc43);
  FUN_00000f0e(uVar1,DAT_00000c85,puVar2);
  FUN_00000f0e(uVar1,DAT_00000c83,puVar2);
  FUN_00000f0e(uVar1,DAT_00000c89,puVar2);
  FUN_00000f0e(uVar1,DAT_00000c87,puVar2);
  FUN_00000f0e(uVar1,DAT_00000c8b,puVar2);
  *puVar2 = 0;
  puVar2[1] = 0;
  puVar2[2] = 0xd;
  puVar2[3] = 10;
  puVar2[4] = 0x24;
  return;
}


/* ===== FUN_00000f0e @ 00000f0e ===== */

void __allregs FUN_00000f0e(undefined4 param_1,short param_2,char *param_3)

{
  int iVar1;
  
  iVar1 = CONCAT22((short)((uint)param_1 >> 0x10),4);
  do {
    *param_3 = (byte)((ushort)param_2 >> 0xc) + 0x30;
    param_2 = param_2 << 4;
    iVar1 = iVar1 + -1;
    param_3 = param_3 + 1;
  } while (iVar1 != 0);
  return;
}


/* ===== FUN_00000f23 @ 00000f23 ===== */

void __allregs FUN_00000f23(uint param_1)

{
  BADSPACEBASE *in_ESP;
  
  DAT_00000caf = *(code **)((param_1 & 0xffff) + 0xcb3);
  DAT_00000f60 = (undefined1 *)in_ESP;
  (*DAT_00000caf)();
  return;
}


/* ===== FUN_00000f64 @ 00000f64 ===== */

void __allregs FUN_00000f64(undefined4 param_1)

{
  DAT_00000d82 = param_1;
  *DAT_00000c93 = 0xffff;
  return;
}


/* ===== FUN_00000f77 @ 00000f77 ===== */

undefined2 __allregs FUN_00000f77(void)

{
  return DAT_00000c91;
}


/* ===== FUN_0000108a @ 0000108a ===== */

void __allregs FUN_0000108a(void)

{
  return;
}


/* ===== FUN_000010d4 @ 000010d4 ===== */

void __allregs FUN_000010d4(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_000089b0(param_1,param_2,param_3);
  return;
}


/* ===== FUN_000011a6 @ 000011a6 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000011a6(undefined2 param_1,undefined2 param_2,ushort param_3)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)segment(0x1c00,0x85bc);
  FUN_00008480(((int *)((uint)param_3 + _DAT_00000ca1))[1] * -0x2000,
               *(int *)((uint)param_3 + _DAT_00000ca1) << 0xd,*puVar1);
  return;
}


/* ===== FUN_000011cb @ 000011cb ===== */

void __allregs
FUN_000011cb(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5)

{
  FUN_00006e95(param_1,param_2,param_3,param_4,DAT_00000c93 + 0xba,param_5);
  return;
}


/* ===== FUN_000011dd @ 000011dd ===== */

void __allregs FUN_000011dd(undefined2 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00006f3e(param_1,param_2,param_3);
  return;
}


/* ===== FUN_000011f0 @ 000011f0 ===== */

void __allregs FUN_000011f0(void)

{
  return;
}


/* ===== FUN_000011f1 @ 000011f1 ===== */

void __allregs FUN_000011f1(void)

{
  return;
}


/* ===== FUN_000011f4 @ 000011f4 ===== */

undefined4 __allregs FUN_000011f4(uint param_1,uint param_2,uint param_3)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  uVar3 = (param_1 & 0xffff) + 0xf & 0xfffffff0;
  puVar2 = (undefined4 *)FUN_00003439(2,uVar3,&LAB_0000123b,0x10,param_3,0x1214);
  if (puVar2 != (undefined4 *)0x0) {
    piVar1 = (int *)segment(0x1c00,0x807);
    puVar4 = (undefined4 *)((param_3 & 0xffff) + (param_2 & 0xffff) * 0x10 + *piVar1);
    for (uVar3 = uVar3 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
      *puVar2 = *puVar4;
      puVar4 = puVar4 + 1;
      puVar2 = puVar2 + 1;
    }
    return 0xffffffff;
  }
  return 0;
}


/* ===== FUN_000012a7 @ 000012a7 ===== */

void __allregs FUN_000012a7(void)

{
  return;
}


/* ===== FUN_000012ac @ 000012ac ===== */

void __allregs FUN_000012ac(ushort param_1,undefined4 param_2)

{
  FUN_000023ec(param_1 & 0xff00,param_2);
  return;
}


/* ===== FUN_000012b4 @ 000012b4 ===== */

undefined4 __allregs FUN_000012b4(char *param_1,char *param_2,char *param_3,char *param_4)

{
  int *piVar1;
  char cVar2;
  char cVar3;
  int *piVar4;
  int iVar5;
  char cVar6;
  char cVar7;
  undefined2 uVar8;
  char *pcVar9;
  char cVar10;
  
  cVar3 = (char)param_1;
  *param_1 = *param_1 + cVar3;
  *param_1 = *param_1 + cVar3;
  *param_1 = *param_1 + cVar3;
  *param_2 = *param_2 + cVar3;
  *param_1 = *param_1 + cVar3;
  *param_1 = *param_1 + (char)((uint)param_1 >> 8);
  piVar4 = (int *)CONCAT31((int3)((uint)param_1 >> 8),cVar3 + *param_1);
  LOCK();
  piVar1 = (int *)*piVar4;
  *piVar4 = (int)piVar4;
  UNLOCK();
  *piVar1 = *piVar1 + -0x7f;
  cVar3 = (char)piVar1;
  param_3[-0x74ff7100] = param_3[-0x74ff7100] + cVar3;
  cVar6 = (char)param_2;
  param_2[0x8a00] = param_2[0x8a00] + cVar6;
  *param_3 = *param_3 + cVar3;
  *(char *)((int)piVar1 * 2) = *(char *)((int)piVar1 * 2) + cVar3;
  cVar2 = cVar3 * '\x02' + cVar6;
  *param_2 = *param_2 + cVar2;
  *param_4 = *param_4 + cVar2;
  cRamc2000700 = cRamc2000700 + cVar2;
  cVar7 = (char)param_3 + cVar6;
  cVar10 = (char)((uint)param_3 >> 8) + cVar2 + cVar6;
  pcVar9 = (char *)CONCAT22((short)((uint)param_3 >> 0x10),CONCAT11(cVar10,cVar7));
  *pcVar9 = *pcVar9 + cVar6;
  *pcVar9 = *pcVar9 + cVar6;
  *pcVar9 = *pcVar9 + cVar6;
  *pcVar9 = *pcVar9 + cVar6;
  iVar5 = CONCAT22((short)((uint)piVar1 >> 0x10),
                   CONCAT11((char)((uint)piVar1 >> 8) + cVar3 * '\x02' + cVar6 + cVar7 * '\x04',
                            cVar2));
  *param_4 = *param_4 + cVar6;
  *param_4 = *param_4 + cVar6;
  *param_4 = *param_4 + cVar6;
  *param_4 = *param_4 + cVar6;
  uVar8 = CONCAT11(cVar10 + cVar7 * '\x04',cVar7);
  pcVar9 = (char *)(iVar5 * 2);
  *pcVar9 = *pcVar9 + cVar6;
  do {
    cVar3 = in(uVar8);
  } while (cVar3 < '\0');
  out(uVar8,cVar2);
  return CONCAT31((int3)((uint)iVar5 >> 8),cVar2);
}


/* ===== FUN_000012eb @ 000012eb ===== */

undefined4 __allregs FUN_000012eb(int *param_1,char param_2,undefined4 param_3,char *param_4)

{
  char cVar1;
  int iVar2;
  char cVar3;
  undefined2 uVar4;
  char *pcVar5;
  char cVar6;
  
  *param_1 = *param_1 + (int)param_1;
  iVar2 = (int)param_1 + *param_1 + -0x3dfff900;
  cVar1 = (char)iVar2;
  cVar3 = (char)param_3 + param_2;
  cVar6 = (char)((uint)param_3 >> 8) + cVar1 + param_2;
  pcVar5 = (char *)CONCAT22((short)((uint)param_3 >> 0x10),CONCAT11(cVar6,cVar3));
  *pcVar5 = *pcVar5 + param_2;
  *pcVar5 = *pcVar5 + param_2;
  *pcVar5 = *pcVar5 + param_2;
  *pcVar5 = *pcVar5 + param_2;
  iVar2 = CONCAT22((short)((uint)iVar2 >> 0x10),
                   CONCAT11((char)((uint)iVar2 >> 8) + cVar3 * '\x04',cVar1));
  *param_4 = *param_4 + param_2;
  *param_4 = *param_4 + param_2;
  *param_4 = *param_4 + param_2;
  *param_4 = *param_4 + param_2;
  uVar4 = CONCAT11(cVar6 + cVar3 * '\x04',cVar3);
  pcVar5 = (char *)(iVar2 * 2);
  *pcVar5 = *pcVar5 + param_2;
  do {
    cVar3 = in(uVar4);
  } while (cVar3 < '\0');
  out(uVar4,cVar1);
  return CONCAT31((int3)((uint)iVar2 >> 8),cVar1);
}


/* ===== FUN_0000132f @ 0000132f ===== */

undefined4 __allregs FUN_0000132f(undefined4 param_1,undefined2 param_2)

{
  char cVar1;
  
  do {
    cVar1 = in(param_2);
  } while (cVar1 < '\0');
  out(param_2,(char)param_1);
  return param_1;
}


/* ===== FUN_0000161d @ 0000161d ===== */

undefined1 __allregs FUN_0000161d(int *param_1,int *param_2,int *param_3,char *param_4,int *param_5)

{
  char cVar1;
  char cVar2;
  
  *param_1 = (int)(*param_1 + (int)param_1);
  cVar1 = (char)param_1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + -0x1fdfdfe0) = (char)param_1[-0x1fdfdfe0] + -0x80;
  *(char *)(param_1 + 0x2020) = (char)param_1[0x2020];
  *(char *)param_5 = (char)*param_5 + (char)param_2;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_5 = (char)*param_5 + (char)param_3;
  *(char *)param_1 = (char)*param_1 + cVar1;
  cVar2 = (char)((uint)param_3 >> 8);
  *(char *)param_5 = (char)*param_5 + cVar2;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_5 = (char)*param_5 + cVar2;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = *param_1 + 1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_2 = *param_2 + 1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_3 = *param_3 + 1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_5 = *param_5 + 1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_4 = *param_4 + (char)param_3;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_4 = *param_4 + cVar2;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_4 = *param_4 + cVar2;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + '\x01';
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_2 = (char)*param_2 + '\x01';
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_3 = (char)*param_3 + '\x01';
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_5 = (char)*param_5 + '\x01';
  *(char *)param_1 = (char)*param_1 + cVar1;
  return uRama0b3d3cd;
}


/* ===== FUN_00001e75 @ 00001e75 ===== */

/* WARNING: Instruction at (ram,0x0000200f) overlaps instruction at (ram,0x0000200d)
    */
/* WARNING: Removing unreachable block (ram,0x00001e86) */
/* WARNING: Removing unreachable block (ram,0x00001e88) */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Removing unreachable block (ram,0x00001f8f) */
/* WARNING: Removing unreachable block (ram,0x00001f91) */
/* WARNING: Removing unreachable block (ram,0x00001f93) */
/* WARNING: Removing unreachable block (ram,0x00001f83) */
/* WARNING: Removing unreachable block (ram,0x00001f85) */
/* WARNING: Removing unreachable block (ram,0x00001f77) */
/* WARNING: Removing unreachable block (ram,0x00001f79) */
/* WARNING: Removing unreachable block (ram,0x00001f7b) */
/* WARNING: Removing unreachable block (ram,0x00001f7d) */
/* WARNING: Removing unreachable block (ram,0x00001ff4) */
/* WARNING: Removing unreachable block (ram,0x00001f7f) */
/* WARNING: Removing unreachable block (ram,0x00001f81) */
/* WARNING: Removing unreachable block (ram,0x00001ffa) */
/* WARNING: Removing unreachable block (ram,0x00002000) */
/* WARNING: Removing unreachable block (ram,0x00002009) */
/* WARNING: Removing unreachable block (ram,0x00001f87) */
/* WARNING: Removing unreachable block (ram,0x00001f89) */
/* WARNING: Removing unreachable block (ram,0x00002006) */
/* WARNING: Removing unreachable block (ram,0x00001f8b) */
/* WARNING: Removing unreachable block (ram,0x00001f8d) */
/* WARNING: Removing unreachable block (ram,0x0000200c) */
/* WARNING: Removing unreachable block (ram,0x00002016) */
/* WARNING: Removing unreachable block (ram,0x0000200f) */

undefined4 __allregs
FUN_00001e75(int param_1,undefined2 param_2,undefined4 param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6)

{
  ushort *puVar1;
  undefined4 *puVar2;
  short sVar3;
  longlong lVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int *piVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined2 uVar10;
  undefined2 uVar11;
  byte bVar12;
  char cVar13;
  undefined2 extraout_var;
  undefined3 uVar24;
  byte *pbVar15;
  byte *pbVar17;
  uint uVar18;
  int iVar20;
  uint uVar21;
  undefined4 uVar22;
  undefined4 uVar23;
  char cVar26;
  undefined2 extraout_var_00;
  uint *puVar25;
  undefined2 uVar27;
  undefined2 extraout_var_01;
  byte *pbVar28;
  byte bVar29;
  uint *puVar30;
  BADSPACEBASE *in_ESP;
  undefined4 **ppuVar31;
  undefined1 *puVar32;
  undefined4 **ppuVar33;
  uint uVar34;
  undefined1 *puVar35;
  undefined4 *puVar36;
  undefined2 unaff_ES;
  bool bVar37;
  byte in_AF;
  char in_SF;
  undefined1 in_OF;
  undefined4 uStack_8;
  undefined4 *puStack_4;
  byte bVar14;
  uint uVar16;
  char *pcVar19;
  
  ppuVar31 = &puStack_4;
  ppuVar33 = &puStack_4;
  cVar13 = '\x10';
  puVar36 = param_4;
  do {
    puVar36 = puVar36 + -1;
    ppuVar31 = ppuVar31 + -1;
    *ppuVar31 = (undefined4 *)*puVar36;
    cVar13 = cVar13 + -1;
  } while ('\0' < cVar13);
  uRam000f000e = (undefined2)((uint6)*(undefined6 *)(param_1 + -0x3a3a2d3a) >> 0x20);
  puVar32 = (undefined1 *)*(undefined6 *)(param_1 + -0x3a3a2d3a);
  uRam000f0014 = 0xc1;
  uRam000f0000 = (undefined2)param_1;
  uRam000f0002 = (undefined2)param_3;
  uRam000f0004 = 0xb0b9;
  uRam000f0008 = (undefined2)param_5;
  uRam000f000a = (undefined2)param_6;
  uRam000f0006 = param_2;
  uRam000f000c = (short)&puStack_4;
  puStack_4 = param_4;
  (*(code *)0xf0100)();
  uRam000f0014 = 4;
  uRam000f000c = (short)&puStack_4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  bVar14 = DAT_604000a0;
  uVar11 = uRam000f0006;
  uVar10 = uRam000f0004;
  uVar27 = uRam000f0002;
  puVar30 = (uint *)CONCAT22((short)((uint)param_3 >> 0x10),uRam000f0002);
  puVar25 = (uint *)CONCAT22(extraout_var_00,uRam000f0004);
  pbVar17 = (byte *)CONCAT22(extraout_var_01,uRam000f0006);
  uVar34 = CONCAT22((short)((uint)param_5 >> 0x10),uRam000f0008);
  uVar23 = CONCAT22((short)((uint)param_6 >> 0x10),uRam000f000a);
  uVar24 = (undefined3)(CONCAT22(extraout_var,uRam000f0000) >> 8);
  iVar20 = CONCAT31(uVar24,DAT_604000a0);
  if (!(bool)in_OF) {
    if (in_SF == '\0') {
      *(byte *)puVar25 = (char)*puVar25 + DAT_604000a0;
      *(int *)pbVar17 = *(int *)pbVar17 + iVar20;
      iVar20 = CONCAT31(uVar24,bVar14 + (char)*puVar30);
      pbVar15 = (byte *)(iVar20 + *(int *)(puVar32 + iVar20) + 0x7060605);
      *pbVar15 = *pbVar15 | (byte)uVar10;
      *puVar25 = *puVar25 | (uint)puVar25;
      puVar25 = (uint *)(CONCAT31((int3)((uint)puVar25 >> 8),(byte)uVar10 | *pbVar17) | *puVar30);
      uVar18 = *puVar25;
      *puVar25 = (uint)(pbVar17 + *puVar25);
      bVar14 = (byte)uVar11 + *pbVar17;
      uVar16 = CONCAT31((int3)((uint)pbVar17 >> 8),bVar14 + CARRY4(uVar18,(uint)pbVar17));
      uVar18 = (uint)(CARRY1((byte)uVar11,*pbVar17) || CARRY1(bVar14,CARRY4(uVar18,(uint)pbVar17)));
      uVar21 = uVar16 + *puVar30;
      bVar37 = CARRY4(uVar16,*puVar30) || CARRY4(uVar21,uVar18);
      pbVar28 = (byte *)(uVar21 + uVar18);
      bVar12 = (byte)((uint)pbVar15 | 0xf0e0e0d);
      bVar14 = bVar12 + 0x14;
      uVar16 = CONCAT31((int3)(((uint)pbVar15 | 0xf0e0e0d) >> 8),bVar14 + bVar37);
      uVar18 = (uint)(0xeb < bVar12 || CARRY1(bVar14,bVar37));
      uVar21 = uVar16 + 0x17161615;
      bVar37 = 0xe8e9e9ea < uVar16 || CARRY4(uVar21,uVar18);
      pbVar17 = (byte *)(uVar21 + uVar18);
      bVar14 = *pbVar17;
      bVar29 = (byte)uVar27;
      bVar12 = *pbVar17;
      *pbVar17 = (bVar12 - bVar29) - bVar37;
      uVar18 = (uint)(bVar14 < bVar29 || (byte)(bVar12 - bVar29) < bVar37);
      bVar37 = (uint *)*puVar25 < puVar30 || *puVar25 - (int)puVar30 < uVar18;
      *puVar25 = (*puVar25 - (int)puVar30) - uVar18;
      puVar30 = (uint *)CONCAT31((int3)((uint)puVar30 >> 8),(bVar29 - *pbVar28) - bVar37);
      uVar18 = (uint)(bVar29 < *pbVar28 || (byte)(bVar29 - *pbVar28) < bVar37);
      bVar37 = puVar30 < (uint *)*puVar30 || (int)puVar30 - *puVar30 < uVar18;
      puVar30 = (uint *)(((int)puVar30 - *puVar30) - uVar18);
      bVar14 = (byte)pbVar17 - 0x1c;
      pbVar17 = (byte *)((CONCAT31((int3)((uint)pbVar17 >> 8),bVar14 - bVar37) + -0x1f1e1e1d) -
                        (uint)((byte)pbVar17 < 0x1c || bVar14 < bVar37));
      bVar14 = (byte)((uint)pbVar17 >> 8);
      *pbVar17 = *pbVar17 & bVar14;
      *puVar25 = *puVar25 & (uint)(puVar32 + 0xc);
      puVar32 = (undefined1 *)((uint)(puVar32 + 0xc) & *puVar30);
      uVar18 = CONCAT31((int3)(CONCAT22((short)((uint)pbVar17 >> 0x10),
                                        CONCAT11(bVar14 & *pbVar28,(char)pbVar17)) >> 8),
                        (char)pbVar17) & 0x27262624;
      bVar14 = (byte)uVar18;
      in_AF = 9 < (bVar14 & 0xf) | in_AF;
      uVar24 = (undefined3)(uVar18 >> 8);
      cVar13 = bVar14 + in_AF * '\x06' + (0x99 < bVar14) * '`';
      pcVar19 = (char *)CONCAT31(uVar24,cVar13);
      cVar26 = (char)((uint)puVar25 >> 8);
      *pcVar19 = *pcVar19 - cVar26;
      *puVar25 = *puVar25 - (int)&puStack_4;
      puVar25 = (uint *)CONCAT22((short)((uint)puVar25 >> 0x10),
                                 CONCAT11(cVar26 - *pbVar28,(char)puVar25));
      uVar18 = *puVar30;
      uVar21 = CONCAT31(uVar24,cVar13 + -0x2c);
      iVar20 = uVar21 + 0xd0d1d1d3;
      bVar14 = (byte)iVar20;
      in_AF = 9 < (bVar14 & 0xf) | in_AF;
      uVar24 = (undefined3)((uint)iVar20 >> 8);
      cVar13 = bVar14 + in_AF * -6 + (0x99 < bVar14 || uVar21 < 0x2f2e2e2d) * -0x60;
      pbVar17 = (byte *)CONCAT31(uVar24,cVar13);
      bVar14 = (byte)((uint)pbVar28 >> 8);
      *pbVar17 = *pbVar17 ^ bVar14;
      *puVar25 = *puVar25 ^ uVar34;
      pbVar17 = (byte *)CONCAT22((short)((uint)pbVar28 >> 0x10),
                                 CONCAT11(bVar14 ^ *pbVar28,(char)pbVar28));
      uVar34 = uVar34 ^ *puVar30;
      uVar21 = CONCAT31(uVar24,cVar13) ^ 0x37363601;
      in_AF = 9 < ((byte)uVar21 & 0xf) | in_AF;
      uVar16 = CONCAT31((int3)(uVar21 >> 8),(byte)uVar21 + in_AF * '\x06') & 0xffffff0f;
      iVar20 = CONCAT22((short)(uVar16 >> 0x10),CONCAT11((char)(uVar21 >> 8) + in_AF,(char)uVar16));
      ppuVar33 = (undefined4 **)((int)&puStack_4 - uVar18);
    }
    else {
      in_AF = 9 < (DAT_604000a0 & 0xf) | in_AF;
      uVar18 = CONCAT31(uVar24,DAT_604000a0 + in_AF * -6) & 0xffffff0f;
      iVar20 = CONCAT22((short)(uVar18 >> 0x10),
                        CONCAT11((char)((ushort)uRam000f0000 >> 8) - in_AF,(char)uVar18));
      ppuVar33 = &puStack_4;
    }
  }
  in_AF = 9 < ((byte)iVar20 & 0xf) | in_AF;
  uVar18 = CONCAT31((int3)((uint)iVar20 >> 8),(byte)iVar20 + in_AF * -6) & 0xffffff0f;
  uVar22 = CONCAT22((short)(uVar18 >> 0x10),CONCAT11((char)((uint)iVar20 >> 8) - in_AF,(char)uVar18)
                   );
  *(undefined4 *)(puVar32 + -4) = uVar22;
  *(undefined4 *)(puVar32 + -8) = uVar22;
  *(uint **)(puVar32 + -0xc) = puVar25;
  *(uint **)(puVar32 + -0x10) = puVar25;
  *(byte **)(puVar32 + -0x14) = pbVar17;
  *(byte **)(puVar32 + -0x18) = pbVar17;
  *(uint **)(puVar32 + -0x1c) = puVar30;
  *(uint **)(puVar32 + -0x20) = puVar30;
  *(undefined1 **)(puVar32 + -0x24) = puVar32 + -0x20;
  *(undefined1 **)(puVar32 + -0x28) = puVar32 + -0x24;
  *(undefined4 ***)(puVar32 + -0x2c) = ppuVar33;
  *(undefined4 ***)(puVar32 + -0x30) = ppuVar33;
  *(uint *)(puVar32 + -0x34) = uVar34;
  *(uint *)(puVar32 + -0x38) = uVar34;
  *(undefined4 *)(puVar32 + -0x3c) = uVar23;
  *(undefined4 *)(puVar32 + -0x40) = uVar23;
  uVar23 = *(undefined4 *)(puVar32 + -0x3c);
  uVar22 = *(undefined4 *)(puVar32 + -0x34);
  uVar5 = *(undefined4 *)(puVar32 + -0x2c);
  uVar6 = *(undefined4 *)(puVar32 + -0x24);
  piVar7 = (int *)**(int **)(puVar32 + -0x20);
  iVar20 = piVar7[1];
  uVar8 = piVar7[3];
  uVar9 = piVar7[5];
  piVar7[5] = uVar23;
  piVar7[4] = uVar22;
  piVar7[3] = uVar5;
  piVar7[2] = uVar6;
  piVar7[1] = (int)(piVar7 + 6);
  *piVar7 = iVar20;
  piVar7[-1] = uVar8;
  piVar7[-2] = uVar9;
  piVar7[-3] = uVar23;
  piVar7[-4] = uVar22;
  piVar7[-5] = uVar5;
  piVar7[-6] = uVar6;
  piVar7[-7] = (int)(piVar7 + -2);
  piVar7[-8] = iVar20;
  piVar7[-9] = uVar8;
  piVar7[-10] = uVar9;
  puVar36 = (undefined4 *)piVar7[-2];
  puVar35 = (undefined1 *)piVar7[-1];
  uVar23 = piVar7[5];
  puVar1 = (ushort *)((int)piVar7 + 0x7d);
  sVar3 = ((short)piVar7 + 0x18U & 3) - (*puVar1 & 3);
  *puVar1 = *puVar1 + (ushort)(0 < sVar3) * sVar3;
  piVar7[5] = 0x6a696968;
  piVar7[4] = 0x6b;
  lVar4 = (longlong)*(int *)((int)piVar7 + *piVar7 * 2 + 0x7d) * 0x6d;
  uVar27 = (undefined2)piVar7[3];
  out(*puVar35,uVar27);
  out(puVar35[1],uVar27);
  out(*(undefined4 *)(puVar35 + 2),uVar27);
  out(*(undefined4 *)(puVar35 + 6),uVar27);
  if ((int)lVar4 == lVar4) {
    puVar2 = (undefined4 *)((int)puVar36 + 1);
    *(char *)puVar36 = (char)uVar23;
    puVar36 = (undefined4 *)((int)puVar36 + 5);
    *puVar2 = uVar23;
  }
  *puVar36 = uVar23;
  return 0xbab9b9b8;
}


/* ===== FUN_0000201a @ 0000201a ===== */

void __allregs FUN_0000201a(void)

{
  return;
}


/* ===== FUN_00002023 @ 00002023 ===== */

void __allregs FUN_00002023(undefined4 *param_1)

{
  char cVar1;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar2;
  undefined4 auStack_8 [2];
  
  puVar2 = auStack_8 + 1;
  cVar1 = '\b';
  do {
    param_1 = param_1 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *param_1;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  return;
}


/* ===== FUN_0000202a @ 0000202a ===== */

void __allregs FUN_0000202a(void)

{
  return;
}


/* ===== FUN_000021e0 @ 000021e0 ===== */

/* WARNING: Removing unreachable block (ram,0x000021e0) */

undefined2 __allregs
FUN_000021e0(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6,undefined2 param_7)

{
  undefined2 unaff_ES;
  
  uRam000f0014 = 0xce;
  uRam000f000e = 0x1c00;
  uRam000f0000 = param_1;
  uRam000f0002 = param_4;
  uRam000f0004 = param_2;
  uRam000f0006 = param_3;
  uRam000f0008 = param_6;
  uRam000f000a = param_7;
  uRam000f000c = param_5;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return uRam000f0000;
}


/* ===== FUN_00002294 @ 00002294 ===== */

void __allregs FUN_00002294(void)

{
  return;
}


/* ===== FUN_00002295 @ 00002295 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs FUN_00002295(uint param_1)

{
  if ((byte)param_1 < 0x12) {
    _FUN_0000161d = *(undefined4 *)(&DAT_00001e21 + (param_1 & 0xff) * 4);
  }
  return param_1;
}


/* ===== FUN_000022ab @ 000022ab ===== */

ushort * __allregs FUN_000022ab(ushort *param_1,uint param_2,byte param_3,byte param_4)

{
  uint uVar1;
  
  if ((((char)param_4 < '\0') &&
      ((param_4 == 0xfe ||
       (((param_4 = 0, (undefined2 *)PTR_DAT_000015d7 != &DAT_0000156b &&
         (param_4 = 1, (undefined2 *)PTR_DAT_000015db != &DAT_0000156b)) &&
        (param_4 = 2, (undefined2 *)PTR_DAT_000015df != &DAT_0000156b)))))) &&
     (((param_4 = 0, (ushort *)PTR_DAT_000015d7 != param_1 &&
       (param_4 = 1, (ushort *)PTR_DAT_000015db != param_1)) &&
      (param_4 = 2, (ushort *)PTR_DAT_000015df != param_1)))) {
    if (param_3 != 0) {
      return param_1;
    }
    param_4 = 0;
  }
  (&PTR_DAT_000015d7)[param_4] = (undefined *)param_1;
  (&DAT_000015e3)[param_4] = (uint)*param_1;
  if (param_2 == 0) {
    param_2 = (uint)param_1[1];
  }
  (&DAT_000015cb)[param_4] = param_2;
  (&DAT_000015ef)[param_4] = 0x40000;
  uVar1 = (uint)param_3;
  if (param_4 == 0) {
    uRam0000268b = *(undefined1 *)(uVar1 + 0x1e8b);
    bRam00002689 = param_3;
  }
  else if (param_4 == 1) {
    uRam000026b6 = *(undefined1 *)(uVar1 + 0x1e8b);
    bRam000026b3 = param_3;
  }
  else {
    uRam000026e2 = *(undefined1 *)(uVar1 + 0x1e8b);
    bRam000026df = param_3;
  }
  return param_1;
}


/* ===== FUN_00002377 @ 00002377 ===== */

ushort * __allregs FUN_00002377(ushort *param_1,uint param_2,byte param_3,byte param_4)

{
  ushort *puVar1;
  
  if (param_2 == 0) {
    param_2 = (uint)param_1[1];
  }
  (&PTR_DAT_000015fb)[param_4] = (undefined *)param_1;
  (&DAT_00001607)[param_4] = param_2;
  (&DAT_00001616)[param_4] = param_3;
  (&DAT_00001613)[param_4] = *(undefined1 *)(param_3 + 0x1e8b);
  if ((undefined2 *)(&PTR_DAT_000015d7)[param_4] == &DAT_0000156b) {
    puVar1 = FUN_000022ab(param_1,param_2,param_3,param_4);
    return puVar1;
  }
  return param_1;
}


/* ===== FUN_000023b4 @ 000023b4 ===== */

ushort * __allregs
FUN_000023b4(undefined4 param_1,undefined1 param_2,undefined1 param_3,ushort *param_4)

{
  ushort *puVar1;
  bool in_ZF;
  
  if (in_ZF) {
    puVar1 = FUN_000022ab(param_4,param_1,param_2,param_3);
    return puVar1;
  }
  return param_4;
}


/* ===== FUN_000023ec @ 000023ec ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs FUN_000023ec(ushort param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  if (DAT_00002293 != '\0') {
    (*(code *)PTR_FUN_000015b3)(param_1,param_2);
  }
  DAT_000023e8 = DAT_0000092b * 0x10 + 0x10;
  _DAT_000023dc = _DAT_0000092f + 0x10;
  DAT_000023e0 = _DAT_0000092f + 0x410;
  if ((DAT_000023e8 & 0xffff0000) != (DAT_0000092b * 0x10 + 0x810U & 0xffff0000)) {
    DAT_000023e8 = DAT_0000092b * 0x10 + 0x810;
    _DAT_000023dc = _DAT_0000092f + 0x810;
    DAT_000023e0 = _DAT_0000092f + 0xc10;
  }
  puRam00002716 = &DAT_00002093;
  DAT_000023e4 = 0;
  func_0x00002630();
  if (4 < (byte)param_1) {
    return 0;
  }
  PTR_FUN_00001587 = (&PTR_FUN_0000158b)[param_1 & 0xff];
  PTR_FUN_000015b3 = (&PTR_FUN_000015b7)[param_1 & 0xff];
  DAT_00002293 = (byte)param_1;
  FUN_000024c8(PTR_FUN_000015b3);
  uVar1 = (*(code *)PTR_FUN_00001587)();
  return uVar1;
}


/* ===== FUN_000024c8 @ 000024c8 ===== */

undefined4 __allregs FUN_000024c8(undefined4 param_1)

{
  PTR_DAT_000015fb = (undefined *)&DAT_0000156b;
  PTR_DAT_000015ff = (undefined *)&DAT_0000156b;
  PTR_DAT_00001603 = (undefined *)&DAT_0000156b;
  DAT_00001607 = 0;
  DAT_0000160b = 0;
  DAT_0000160f = 0;
  DAT_00001616 = 0;
  DAT_00001617 = 0;
  DAT_00001618 = 0;
  DAT_00001613 = 0;
  DAT_00001614 = 0;
  DAT_00001615 = 0;
  if ((ushort *)PTR_DAT_000015d7 != &DAT_0000156b) {
    PTR_DAT_000015d7 = (undefined *)&DAT_0000156b;
    DAT_000015e3 = (uint)DAT_0000156b;
    DAT_000015cb = 0;
    DAT_000015ef = 0x40000;
  }
  if ((ushort *)PTR_DAT_000015db != &DAT_0000156b) {
    PTR_DAT_000015db = (undefined *)&DAT_0000156b;
    DAT_000015e7 = (uint)DAT_0000156b;
    DAT_000015cf = 0;
    DAT_000015f3 = 0x40000;
  }
  if ((ushort *)PTR_DAT_000015df != &DAT_0000156b) {
    PTR_DAT_000015df = (undefined *)&DAT_0000156b;
    DAT_000015eb = (uint)DAT_0000156b;
    DAT_000015d3 = 0;
    DAT_000015f7 = 0x40000;
  }
  return param_1;
}


/* ===== FUN_00002625 @ 00002625 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00002625(void)

{
  byte *pbVar1;
  byte bVar2;
  ushort uVar3;
  short sVar4;
  undefined4 uVar5;
  undefined2 uVar7;
  int iVar6;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  int iVar8;
  undefined4 extraout_EDX;
  undefined4 uVar9;
  undefined4 extraout_EDX_00;
  undefined4 extraout_EDX_01;
  uint uVar10;
  uint uVar11;
  undefined *puVar12;
  int iVar13;
  int iVar14;
  undefined1 *puVar15;
  
  DAT_000023e4 = DAT_000023e4 ^ 0x400;
  puVar15 = (undefined1 *)(DAT_000023e4 + _DAT_000023dc);
  DAT_000023e0 = puVar15 + 0x400;
  uVar5 = 0;
  uVar9 = 0;
  iVar8 = DAT_000015cf;
  iVar13 = DAT_000015d3;
  iVar14 = DAT_000015cb;
  do {
    iVar6 = DAT_00001607;
    puVar12 = PTR_DAT_000015fb;
    uVar10 = DAT_000015ef >> 0x10;
    if (DAT_000015e3 <= uVar10) {
      DAT_000015e3 = (uint)*(ushort *)PTR_DAT_000015fb;
      DAT_000015cb = DAT_00001607;
      uRam00002689 = DAT_00001616;
      uRam0000268b = DAT_00001613;
      DAT_000015ef = 0x40000;
      if (PTR_DAT_000015fb != PTR_DAT_000015d7) {
        (*(code *)PTR_FUN_0000157b)(uVar5);
        iVar8 = extraout_ECX;
        uVar9 = extraout_EDX;
      }
      PTR_DAT_000015d7 = puVar12;
      uVar10 = 4;
      iVar14 = iVar6;
    }
    uVar7 = (undefined2)((uint)uVar5 >> 0x10);
    uVar3 = (ushort)(byte)PTR_DAT_000015d7[uVar10];
    DAT_000015ef = DAT_000015ef + iVar14;
    uVar10 = DAT_000015f3 >> 0x10;
    if (DAT_000015e7 <= uVar10) {
      puVar12 = PTR_DAT_000015ff;
      DAT_000015e7 = (uint)*(ushort *)PTR_DAT_000015ff;
      DAT_000015cf = DAT_0000160b;
      uRam000026b3 = DAT_00001617;
      uRam000026b6 = DAT_00001614;
      DAT_000015f3 = 0x40000;
      iVar8 = DAT_0000160b;
      if (PTR_DAT_000015ff != PTR_DAT_000015db) {
        (*(code *)PTR_FUN_0000157f)();
        iVar8 = extraout_ECX_00;
        uVar9 = extraout_EDX_00;
      }
      uVar10 = 4;
      PTR_DAT_000015db = puVar12;
    }
    iVar6 = DAT_0000160f;
    uVar9 = CONCAT31((int3)((uint)uVar9 >> 8),PTR_DAT_000015db[uVar10]);
    sVar4 = uVar3 + (short)uVar9;
    DAT_000015f3 = DAT_000015f3 + iVar8;
    uVar10 = DAT_000015f7 >> 0x10;
    if (DAT_000015eb <= uVar10) {
      puVar12 = PTR_DAT_00001603;
      DAT_000015eb = (uint)*(ushort *)PTR_DAT_00001603;
      DAT_000015d3 = DAT_0000160f;
      uRam000026df = DAT_00001618;
      uRam000026e2 = DAT_00001615;
      DAT_000015f7 = 0x40000;
      if (PTR_DAT_00001603 != PTR_DAT_000015df) {
        (*(code *)PTR_FUN_00001583)();
        iVar8 = extraout_ECX_01;
        uVar9 = extraout_EDX_01;
      }
      uVar10 = 4;
      iVar13 = iVar6;
      PTR_DAT_000015df = puVar12;
    }
    uVar9 = CONCAT31((int3)((uint)uVar9 >> 8),PTR_DAT_000015df[uVar10]);
    sVar4 = sVar4 + (short)uVar9;
    DAT_000015f7 = DAT_000015f7 + iVar13;
    uVar11 = DAT_00001619 & _FUN_0000161d;
    DAT_00001619 = DAT_00001619 + 1;
    pbVar1 = (byte *)(uVar11 + 0x1621);
    bVar2 = (byte)sVar4;
    uVar10 = CONCAT22(uVar7,CONCAT11((char)((ushort)sVar4 >> 8) + CARRY1(bVar2,*pbVar1),
                                     bVar2 + *pbVar1)) + 1;
    *(char *)(uVar11 + 0x1621) = (char)(uVar10 >> 2);
    iVar6 = CONCAT22((ushort)(uVar10 >> 0x12),
                     (short)(uVar10 >> 2) << 1 | (ushort)((uVar10 >> 1 & 1) != 0));
    uVar5 = CONCAT31((int3)((uint)iVar6 >> 8),(&DAT_00002093)[iVar6]);
    *puVar15 = (&DAT_00002093)[iVar6];
    puVar15 = puVar15 + 1;
  } while (puVar15 != DAT_000023e0);
  return;
}


/* ===== FUN_00002630 @ 00002630 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00002630(void)

{
  byte *pbVar1;
  byte bVar2;
  ushort uVar3;
  short sVar4;
  undefined4 uVar5;
  undefined2 uVar7;
  int iVar6;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  int iVar8;
  undefined4 extraout_EDX;
  undefined4 uVar9;
  undefined4 extraout_EDX_00;
  undefined4 extraout_EDX_01;
  uint uVar10;
  uint uVar11;
  undefined *puVar12;
  int iVar13;
  int iVar14;
  undefined1 *puVar15;
  
  DAT_000023e4 = DAT_000023e4 ^ 0x400;
  puVar15 = (undefined1 *)(DAT_000023e4 + _DAT_000023dc);
  DAT_000023e0 = puVar15 + 0x400;
  uVar5 = 0;
  uVar9 = 0;
  iVar8 = DAT_000015cf;
  iVar13 = DAT_000015d3;
  iVar14 = DAT_000015cb;
  do {
    iVar6 = DAT_00001607;
    puVar12 = PTR_DAT_000015fb;
    uVar10 = DAT_000015ef >> 0x10;
    if (DAT_000015e3 <= uVar10) {
      DAT_000015e3 = (uint)*(ushort *)PTR_DAT_000015fb;
      DAT_000015cb = DAT_00001607;
      uRam00002689 = DAT_00001616;
      uRam0000268b = DAT_00001613;
      DAT_000015ef = 0x40000;
      if (PTR_DAT_000015fb != PTR_DAT_000015d7) {
        (*(code *)PTR_FUN_0000157b)(uVar5);
        iVar8 = extraout_ECX;
        uVar9 = extraout_EDX;
      }
      PTR_DAT_000015d7 = puVar12;
      uVar10 = 4;
      iVar14 = iVar6;
    }
    uVar7 = (undefined2)((uint)uVar5 >> 0x10);
    uVar3 = (ushort)(byte)PTR_DAT_000015d7[uVar10];
    DAT_000015ef = DAT_000015ef + iVar14;
    uVar10 = DAT_000015f3 >> 0x10;
    if (DAT_000015e7 <= uVar10) {
      puVar12 = PTR_DAT_000015ff;
      DAT_000015e7 = (uint)*(ushort *)PTR_DAT_000015ff;
      DAT_000015cf = DAT_0000160b;
      uRam000026b3 = DAT_00001617;
      uRam000026b6 = DAT_00001614;
      DAT_000015f3 = 0x40000;
      iVar8 = DAT_0000160b;
      if (PTR_DAT_000015ff != PTR_DAT_000015db) {
        (*(code *)PTR_FUN_0000157f)();
        iVar8 = extraout_ECX_00;
        uVar9 = extraout_EDX_00;
      }
      uVar10 = 4;
      PTR_DAT_000015db = puVar12;
    }
    iVar6 = DAT_0000160f;
    uVar9 = CONCAT31((int3)((uint)uVar9 >> 8),PTR_DAT_000015db[uVar10]);
    sVar4 = uVar3 + (short)uVar9;
    DAT_000015f3 = DAT_000015f3 + iVar8;
    uVar10 = DAT_000015f7 >> 0x10;
    if (DAT_000015eb <= uVar10) {
      puVar12 = PTR_DAT_00001603;
      DAT_000015eb = (uint)*(ushort *)PTR_DAT_00001603;
      DAT_000015d3 = DAT_0000160f;
      uRam000026df = DAT_00001618;
      uRam000026e2 = DAT_00001615;
      DAT_000015f7 = 0x40000;
      if (PTR_DAT_00001603 != PTR_DAT_000015df) {
        (*(code *)PTR_FUN_00001583)();
        iVar8 = extraout_ECX_01;
        uVar9 = extraout_EDX_01;
      }
      uVar10 = 4;
      iVar13 = iVar6;
      PTR_DAT_000015df = puVar12;
    }
    uVar9 = CONCAT31((int3)((uint)uVar9 >> 8),PTR_DAT_000015df[uVar10]);
    sVar4 = sVar4 + (short)uVar9;
    DAT_000015f7 = DAT_000015f7 + iVar13;
    uVar11 = DAT_00001619 & _FUN_0000161d;
    DAT_00001619 = DAT_00001619 + 1;
    pbVar1 = (byte *)(uVar11 + 0x1621);
    bVar2 = (byte)sVar4;
    uVar10 = CONCAT22(uVar7,CONCAT11((char)((ushort)sVar4 >> 8) + CARRY1(bVar2,*pbVar1),
                                     bVar2 + *pbVar1)) + 1;
    *(char *)(uVar11 + 0x1621) = (char)(uVar10 >> 2);
    iVar6 = CONCAT22((ushort)(uVar10 >> 0x12),
                     (short)(uVar10 >> 2) << 1 | (ushort)((uVar10 >> 1 & 1) != 0));
    uVar5 = CONCAT31((int3)((uint)iVar6 >> 8),(&DAT_00002093)[iVar6]);
    *puVar15 = (&DAT_00002093)[iVar6];
    puVar15 = puVar15 + 1;
  } while (puVar15 != DAT_000023e0);
  return;
}


/* ===== FUN_0000272a @ 0000272a ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000272a(undefined4 param_1,short param_2,int param_3,int param_4,undefined1 *param_5)

{
  byte *pbVar1;
  byte bVar2;
  short sVar3;
  undefined2 uVar6;
  int iVar4;
  undefined4 uVar5;
  int extraout_ECX;
  int iVar7;
  int extraout_ECX_00;
  int extraout_ECX_01;
  short extraout_DX;
  short sVar8;
  short extraout_DX_00;
  short extraout_DX_01;
  uint uVar9;
  undefined *puVar10;
  uint uVar11;
  
  do {
    puVar10 = PTR_DAT_000015ff;
    DAT_000015e7 = (uint)*(ushort *)PTR_DAT_000015ff;
    DAT_000015cf = DAT_0000160b;
    uRam000026b3 = DAT_00001617;
    uRam000026b6 = DAT_00001614;
    DAT_000015f3 = 0x40000;
    iVar7 = DAT_0000160b;
    if (PTR_DAT_000015ff != PTR_DAT_000015db) {
      (*(code *)PTR_FUN_0000157f)();
      iVar7 = extraout_ECX_00;
      param_2 = extraout_DX_00;
    }
    uVar11 = 4;
    PTR_DAT_000015db = puVar10;
    do {
      iVar4 = DAT_0000160f;
      sVar8 = CONCAT11((char)((ushort)param_2 >> 8),PTR_DAT_000015db[uVar11]);
      uVar6 = (undefined2)((uint)param_1 >> 0x10);
      sVar3 = (short)param_1 + sVar8;
      DAT_000015f3 = DAT_000015f3 + iVar7;
      uVar11 = DAT_000015f7 >> 0x10;
      if (DAT_000015eb <= uVar11) {
        puVar10 = PTR_DAT_00001603;
        DAT_000015eb = (uint)*(ushort *)PTR_DAT_00001603;
        DAT_000015d3 = DAT_0000160f;
        uRam000026df = DAT_00001618;
        uRam000026e2 = DAT_00001615;
        DAT_000015f7 = 0x40000;
        if (PTR_DAT_00001603 != PTR_DAT_000015df) {
          (*(code *)PTR_FUN_00001583)();
          iVar7 = extraout_ECX_01;
          sVar8 = extraout_DX_01;
        }
        uVar11 = 4;
        param_3 = iVar4;
        PTR_DAT_000015df = puVar10;
      }
      param_2 = CONCAT11((char)((ushort)sVar8 >> 8),PTR_DAT_000015df[uVar11]);
      DAT_000015f7 = DAT_000015f7 + param_3;
      uVar9 = DAT_00001619 & _FUN_0000161d;
      DAT_00001619 = DAT_00001619 + 1;
      pbVar1 = (byte *)(uVar9 + 0x1621);
      bVar2 = (byte)(sVar3 + param_2);
      uVar11 = CONCAT22(uVar6,CONCAT11((char)((ushort)(sVar3 + param_2) >> 8) +
                                       CARRY1(bVar2,*pbVar1),bVar2 + *pbVar1)) + 1;
      *(char *)(uVar9 + 0x1621) = (char)(uVar11 >> 2);
      iVar4 = CONCAT22((ushort)(uVar11 >> 0x12),
                       (short)(uVar11 >> 2) << 1 | (ushort)((uVar11 >> 1 & 1) != 0));
      uVar5 = CONCAT31((int3)((uint)iVar4 >> 8),(&DAT_00002093)[iVar4]);
      *param_5 = (&DAT_00002093)[iVar4];
      iVar4 = DAT_00001607;
      puVar10 = PTR_DAT_000015fb;
      param_5 = param_5 + 1;
      if (param_5 == DAT_000023e0) {
        return;
      }
      uVar11 = DAT_000015ef >> 0x10;
      if (DAT_000015e3 <= uVar11) {
        DAT_000015e3 = (uint)*(ushort *)PTR_DAT_000015fb;
        DAT_000015cb = DAT_00001607;
        uRam00002689 = DAT_00001616;
        uRam0000268b = DAT_00001613;
        DAT_000015ef = 0x40000;
        if (PTR_DAT_000015fb != PTR_DAT_000015d7) {
          (*(code *)PTR_FUN_0000157b)(uVar5);
          iVar7 = extraout_ECX;
          param_2 = extraout_DX;
        }
        PTR_DAT_000015d7 = puVar10;
        uVar11 = 4;
        param_4 = iVar4;
      }
      param_1 = CONCAT22((short)((uint)uVar5 >> 0x10),(ushort)(byte)PTR_DAT_000015d7[uVar11]);
      DAT_000015ef = DAT_000015ef + param_4;
      uVar11 = DAT_000015f3 >> 0x10;
    } while (uVar11 < DAT_000015e7);
  } while( true );
}


/* ===== FUN_00002788 @ 00002788 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00002788(undefined4 param_1,int param_2,short param_3,int param_4,undefined1 *param_5)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;
  ushort uVar4;
  short sVar5;
  undefined2 uVar8;
  int iVar6;
  undefined4 uVar7;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  short extraout_DX;
  undefined1 uVar10;
  short sVar9;
  undefined1 extraout_DH;
  short extraout_DX_00;
  uint uVar11;
  undefined *puVar12;
  uint uVar13;
  
  do {
    iVar2 = DAT_0000160f;
    puVar12 = PTR_DAT_00001603;
    DAT_000015eb = (uint)*(ushort *)PTR_DAT_00001603;
    DAT_000015d3 = DAT_0000160f;
    uRam000026df = DAT_00001618;
    uRam000026e2 = DAT_00001615;
    DAT_000015f7 = 0x40000;
    if (PTR_DAT_00001603 != PTR_DAT_000015df) {
      (*(code *)PTR_FUN_00001583)();
      param_2 = extraout_ECX_01;
      param_3 = extraout_DX_00;
    }
    uVar13 = 4;
    PTR_DAT_000015df = puVar12;
    do {
      sVar9 = CONCAT11((char)((ushort)param_3 >> 8),PTR_DAT_000015df[uVar13]);
      sVar5 = (short)param_1 + sVar9;
      DAT_000015f7 = DAT_000015f7 + iVar2;
      uVar11 = DAT_00001619 & _FUN_0000161d;
      DAT_00001619 = DAT_00001619 + 1;
      pbVar1 = (byte *)(uVar11 + 0x1621);
      bVar3 = (byte)sVar5;
      uVar13 = CONCAT22((short)((uint)param_1 >> 0x10),
                        CONCAT11((char)((ushort)sVar5 >> 8) + CARRY1(bVar3,*pbVar1),bVar3 + *pbVar1)
                       ) + 1;
      *(char *)(uVar11 + 0x1621) = (char)(uVar13 >> 2);
      iVar6 = CONCAT22((ushort)(uVar13 >> 0x12),
                       (short)(uVar13 >> 2) << 1 | (ushort)((uVar13 >> 1 & 1) != 0));
      uVar7 = CONCAT31((int3)((uint)iVar6 >> 8),(&DAT_00002093)[iVar6]);
      *param_5 = (&DAT_00002093)[iVar6];
      iVar6 = DAT_00001607;
      puVar12 = PTR_DAT_000015fb;
      param_5 = param_5 + 1;
      if (param_5 == DAT_000023e0) {
        return;
      }
      uVar13 = DAT_000015ef >> 0x10;
      if (DAT_000015e3 <= uVar13) {
        DAT_000015e3 = (uint)*(ushort *)PTR_DAT_000015fb;
        DAT_000015cb = DAT_00001607;
        uRam00002689 = DAT_00001616;
        uRam0000268b = DAT_00001613;
        DAT_000015ef = 0x40000;
        if (PTR_DAT_000015fb != PTR_DAT_000015d7) {
          (*(code *)PTR_FUN_0000157b)(uVar7);
          param_2 = extraout_ECX;
          sVar9 = extraout_DX;
        }
        PTR_DAT_000015d7 = puVar12;
        uVar13 = 4;
        param_4 = iVar6;
      }
      uVar10 = (undefined1)((ushort)sVar9 >> 8);
      uVar8 = (undefined2)((uint)uVar7 >> 0x10);
      uVar4 = (ushort)(byte)PTR_DAT_000015d7[uVar13];
      DAT_000015ef = DAT_000015ef + param_4;
      uVar13 = DAT_000015f3 >> 0x10;
      if (DAT_000015e7 <= uVar13) {
        puVar12 = PTR_DAT_000015ff;
        DAT_000015e7 = (uint)*(ushort *)PTR_DAT_000015ff;
        DAT_000015cf = DAT_0000160b;
        uRam000026b3 = DAT_00001617;
        uRam000026b6 = DAT_00001614;
        DAT_000015f3 = 0x40000;
        param_2 = DAT_0000160b;
        if (PTR_DAT_000015ff != PTR_DAT_000015db) {
          (*(code *)PTR_FUN_0000157f)();
          param_2 = extraout_ECX_00;
          uVar10 = extraout_DH;
        }
        uVar13 = 4;
        PTR_DAT_000015db = puVar12;
      }
      param_3 = CONCAT11(uVar10,PTR_DAT_000015db[uVar13]);
      param_1 = CONCAT22(uVar8,uVar4 + param_3);
      DAT_000015f3 = DAT_000015f3 + param_2;
      uVar13 = DAT_000015f7 >> 0x10;
    } while (uVar13 < DAT_000015eb);
  } while( true );
}


/* ===== FUN_000027e8 @ 000027e8 ===== */

code __allregs FUN_000027e8(int param_1,uint param_2)

{
  undefined2 uVar1;
  code cVar2;
  uint uVar3;
  short sVar4;
  uint uVar5;
  char in_ZF;
  
  uVar3 = param_1 - 1;
  if (uVar3 == 0 || in_ZF != '\0') {
    sVar4 = (short)DAT_000012cc;
    out(sVar4 + 4,10);
    out(sVar4 + 5,0);
    out(sVar4 + 4,4);
    out(sVar4 + 5,0xff);
    out(sVar4 + 4,0x22);
    out(sVar4 + 5,FUN_000027e8);
    return FUN_000027e8;
  }
  sVar4 = (short)(param_2 >> 1) + -1;
  out(*(undefined2 *)(FUN_000012eb + uVar3 * 2 + 4),(char)sVar4);
  out(*(undefined2 *)(FUN_000012eb + uVar3 * 2 + 4),(char)((ushort)sVar4 >> 8));
  uVar1 = *(undefined2 *)(uVar3 * 2 + 0x12df);
  uVar5 = DAT_000023e8;
  if (3 < uVar3) {
    uVar5 = DAT_000023e8 >> 1;
  }
  out(uVar1,(char)uVar5);
  out(uVar1,(char)(uVar5 >> 8));
  out(*(undefined2 *)((int)&DAT_000012cc + uVar3 * 2 + 3),(char)(DAT_000023e8 >> 0x10));
  cVar2 = (code)((byte)uVar3 & 3);
  out(*(undefined2 *)(uVar3 * 2 + 0x12ff),cVar2);
  return cVar2;
}


/* ===== FUN_00002810 @ 00002810 ===== */

ushort __allregs FUN_00002810(void)

{
  undefined2 uVar1;
  ushort uVar2;
  byte bVar3;
  short sVar4;
  uint uVar5;
  
  bVar3 = (byte)DAT_000012c8 & 3;
  out(*(undefined2 *)(DAT_000012c8 * 2 + 0x12ff),bVar3 | 4);
  out(*(undefined2 *)(DAT_000012c8 * 2 + 0x131f),0);
  out(*(undefined2 *)(DAT_000012c8 * 2 + 0x130f),bVar3 | 0x58);
  sVar4 = 0x800;
  if (3 < DAT_000012c8) {
    sVar4 = 0x400;
  }
  out(*(undefined2 *)(FUN_000012eb + DAT_000012c8 * 2 + 4),(char)(sVar4 + -1));
  out(*(undefined2 *)(FUN_000012eb + DAT_000012c8 * 2 + 4),(char)((ushort)(sVar4 + -1) >> 8));
  uVar1 = *(undefined2 *)(DAT_000012c8 * 2 + 0x12df);
  uVar5 = DAT_000023e8;
  if (3 < DAT_000012c8) {
    uVar5 = DAT_000023e8 >> 1;
  }
  out(uVar1,(char)uVar5);
  out(uVar1,(char)(uVar5 >> 8));
  out(*(undefined2 *)((int)&DAT_000012cc + DAT_000012c8 * 2 + 3),(char)(DAT_000023e8 >> 0x10));
  uVar2 = CONCAT11((char)(DAT_000023e8 >> 0x18),(byte)DAT_000012c8) & 0xff03;
  out(*(undefined2 *)(DAT_000012c8 * 2 + 0x12ff),(char)uVar2);
  return uVar2;
}


/* ===== FUN_00002f7c @ 00002f7c ===== */

undefined4 __allregs FUN_00002f7c(undefined4 param_1)

{
  DAT_00002f50 = DAT_0000090b;
  DAT_00002f54 = 0;
  DAT_00002f70 = 0;
  return param_1;
}


/* ===== FUN_00002f95 @ 00002f95 ===== */

void __allregs FUN_00002f95(undefined4 param_1,undefined4 param_2)

{
  DAT_00002f5c = param_2;
  FUN_00003661(param_1);
  return;
}


/* ===== FUN_00002faa @ 00002faa ===== */

/* WARNING: Removing unreachable block (ram,0x00002fb1) */

void __allregs
FUN_00002faa(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  char *pcVar4;
  char *pcVar5;
  undefined2 unaff_ES;
  
  uRam000f0000 = CONCAT11(9,param_1);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0x30ed;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_2;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  iVar3 = 0x7ff;
  pcVar4 = s_KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK_00000005;
  do {
    pcVar5 = pcVar4;
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 == 'K');
  FUN_00003289(0x805 - (int)(pcVar5 + -1),&DAT_00003102);
  FUN_00003289(DAT_00002f68,&DAT_00003126);
  uVar2 = DAT_00002f6c - DAT_0000090b;
  if (DAT_00002f6c < DAT_0000090b) {
    uVar2 = 0;
  }
  FUN_00003289(uVar2 >> 10,&DAT_00003146);
  FUN_00003289(DAT_0000090f - DAT_0000090b >> 10,&DAT_0000316a);
  uVar2 = DAT_00002f50 - DAT_0000090b;
  if (DAT_00002f50 < DAT_0000090b) {
    uVar2 = 0;
  }
  FUN_00003289(uVar2 >> 10,&DAT_00003191);
  FUN_00003289(DAT_00002f54,&DAT_000031af);
  FUN_00003289(DAT_00002f70,&DAT_000031ca);
  FUN_00003289(DAT_00002f74,&DAT_000031e3);
  FUN_00003289(DAT_00002f78,&DAT_000031e3);
  FUN_0000327d(DAT_00002f58,&DAT_00003099);
  FUN_0000327d(DAT_00002f5c,&DAT_000030c3);
  return;
}


/* ===== FUN_0000327d @ 0000327d ===== */

/* WARNING: Removing unreachable block (ram,0x0000331b) */

undefined4 __allregs FUN_0000327d(uint param_1,undefined4 *param_2)

{
  uint uVar1;
  undefined2 extraout_var;
  int iVar2;
  char cVar3;
  undefined4 *puVar4;
  char *pcVar5;
  char *pcVar6;
  char *pcVar7;
  undefined2 unaff_ES;
  
  DAT_0000320c = 0x10;
  puVar4 = (undefined4 *)&DAT_00003210;
  for (iVar2 = 0x14; uVar1 = DAT_0000320c, iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *param_2;
    param_2 = param_2 + 1;
    puVar4 = puVar4 + 1;
  }
  iVar2 = 0x50;
  pcVar6 = &DAT_00003210;
  do {
    pcVar5 = pcVar6;
    if (iVar2 == 0) break;
    iVar2 = iVar2 + -1;
    pcVar5 = pcVar6 + 1;
    cVar3 = *pcVar6;
    pcVar6 = pcVar5;
  } while (cVar3 != '#');
  pcVar6 = pcVar5;
  do {
    do {
      pcVar7 = pcVar6;
      pcVar6 = pcVar7 + 1;
    } while (*pcVar6 == ',');
  } while (*pcVar6 == '#');
  pcVar6 = pcVar6 + (1 - (int)pcVar5);
  DAT_00003260 = '\0';
  if ((int)param_1 < 0) {
    param_1 = -param_1;
    DAT_00003260 = '\x01';
  }
  do {
    if (*pcVar7 != ',') {
      *pcVar7 = (&DAT_00003261)[param_1 % uVar1];
      param_1 = param_1 / uVar1;
    }
    pcVar7 = pcVar7 + -1;
    if (param_1 == 0) goto joined_r0x000032fc;
    pcVar6 = pcVar6 + -1;
  } while (pcVar6 != (char *)0x0);
LAB_00003317:
  uRam000f0000 = CONCAT11(9,(char)param_1);
  uRam000f0014 = 0x21;
  uRam000f0002 = 0x232c;
  uRam000f0004 = 0;
  uRam000f0006 = 0x3210;
  uRam000f0008 = (undefined2)uVar1;
  uRam000f000a = SUB42(pcVar7,0);
  uRam000f000c = 0x3210;
  uRam000f000e = 0x1c00;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return CONCAT22(extraout_var,uRam000f0000);
joined_r0x000032fc:
  while (pcVar6 = pcVar6 + -1, pcVar6 != (char *)0x0) {
    cVar3 = ' ';
    if (DAT_00003260 != '\0') {
      DAT_00003260 = '\0';
      cVar3 = '-';
    }
    *pcVar7 = cVar3;
    pcVar7 = pcVar7 + -1;
  }
  goto LAB_00003317;
}


/* ===== FUN_00003289 @ 00003289 ===== */

/* WARNING: Removing unreachable block (ram,0x0000331b) */

undefined4 __allregs FUN_00003289(uint param_1,undefined4 *param_2)

{
  uint uVar1;
  undefined2 extraout_var;
  int iVar2;
  char cVar3;
  undefined4 *puVar4;
  char *pcVar5;
  char *pcVar6;
  char *pcVar7;
  undefined2 unaff_ES;
  
  DAT_0000320c = 10;
  puVar4 = (undefined4 *)&DAT_00003210;
  for (iVar2 = 0x14; uVar1 = DAT_0000320c, iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *param_2;
    param_2 = param_2 + 1;
    puVar4 = puVar4 + 1;
  }
  iVar2 = 0x50;
  pcVar6 = &DAT_00003210;
  do {
    pcVar5 = pcVar6;
    if (iVar2 == 0) break;
    iVar2 = iVar2 + -1;
    pcVar5 = pcVar6 + 1;
    cVar3 = *pcVar6;
    pcVar6 = pcVar5;
  } while (cVar3 != '#');
  pcVar6 = pcVar5;
  do {
    do {
      pcVar7 = pcVar6;
      pcVar6 = pcVar7 + 1;
    } while (*pcVar6 == ',');
  } while (*pcVar6 == '#');
  pcVar6 = pcVar6 + (1 - (int)pcVar5);
  DAT_00003260 = '\0';
  if ((int)param_1 < 0) {
    param_1 = -param_1;
    DAT_00003260 = '\x01';
  }
  do {
    if (*pcVar7 != ',') {
      *pcVar7 = (&DAT_00003261)[param_1 % uVar1];
      param_1 = param_1 / uVar1;
    }
    pcVar7 = pcVar7 + -1;
    if (param_1 == 0) goto joined_r0x000032fc;
    pcVar6 = pcVar6 + -1;
  } while (pcVar6 != (char *)0x0);
LAB_00003317:
  uRam000f0000 = CONCAT11(9,(char)param_1);
  uRam000f0014 = 0x21;
  uRam000f0002 = 0x232c;
  uRam000f0004 = 0;
  uRam000f0006 = 0x3210;
  uRam000f0008 = (undefined2)uVar1;
  uRam000f000a = SUB42(pcVar7,0);
  uRam000f000c = 0x3210;
  uRam000f000e = 0x1c00;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return CONCAT22(extraout_var,uRam000f0000);
joined_r0x000032fc:
  while (pcVar6 = pcVar6 + -1, pcVar6 != (char *)0x0) {
    cVar3 = ' ';
    if (DAT_00003260 != '\0') {
      DAT_00003260 = '\0';
      cVar3 = '-';
    }
    *pcVar7 = cVar3;
    pcVar7 = pcVar7 + -1;
  }
  goto LAB_00003317;
}


/* ===== FUN_00003322 @ 00003322 ===== */

void __allregs FUN_00003322(undefined4 param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  
  DAT_00002f5c = param_3;
  FUN_00003661(param_1);
  if (DAT_00002f54 != 0) {
    iVar1 = DAT_00002f54 - param_2;
    iVar2 = param_2 + iVar1;
    do {
      iVar2 = iVar2 + -1;
      FUN_00003376((&DAT_000028ac)[iVar2],iVar2,0x334c);
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
  }
  return;
}


/* ===== FUN_00003352 @ 00003352 ===== */

int __allregs FUN_00003352(void)

{
  return DAT_0000090f - DAT_00002f50;
}


/* ===== FUN_00003376 @ 00003376 ===== */

void __allregs FUN_00003376(int *param_1,uint param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  DAT_00002f5c = param_3;
  if (*param_1 != 0) {
    DAT_00002f50 = (undefined4 *)FUN_00003661(param_1);
    *param_1 = 0;
    DAT_00002f54 = DAT_00002f54 - 1;
    iVar3 = DAT_00002f54 - param_2;
    if (param_2 <= DAT_00002f54 && iVar3 != 0) {
      do {
        puVar1 = (undefined4 *)(&DAT_000028b0)[param_2];
        (&DAT_000028ac)[param_2] = puVar1;
        uVar4 = *(uint *)(&DAT_00002bd0 + param_2 * 4);
        (&DAT_00002bcc)[param_2] = uVar4;
        (&DAT_00002eec)[param_2] = (&DAT_00002eed)[param_2];
        *(int *)(&DAT_00002d5c + param_2 * 4) = *(int *)(&DAT_00002d60 + param_2 * 4);
        puVar5 = (undefined4 *)*puVar1;
        uVar2 = *(int *)(&DAT_00002d60 + param_2 * 4) - 1;
        puVar6 = (undefined4 *)((uint)((int)DAT_00002f50 + uVar2) & ~uVar2);
        if (((undefined4 *)*puVar1 != puVar6) && (((&DAT_00002eec)[param_2] & 2) != 0)) {
          if ((undefined4 *)*puVar1 < puVar6) {
            thunk_FUN_00000f64(0x389a);
            return;
          }
          puVar6 = (undefined4 *)*puVar1;
          DAT_00002f70 = DAT_00002f70 + 1;
        }
        *puVar1 = puVar6;
        *(undefined4 **)(&DAT_00002a3c + param_2 * 4) = puVar6;
        for (uVar2 = uVar4 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
          *puVar6 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar6 = puVar6 + 1;
        }
        DAT_00002f50 = puVar6;
        for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined1 *)DAT_00002f50 = *(undefined1 *)puVar5;
          puVar5 = (undefined4 *)((int)puVar5 + 1);
          DAT_00002f50 = (undefined4 *)((int)DAT_00002f50 + 1);
        }
        param_2 = param_2 + 1;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
    }
  }
  return;
}


/* ===== FUN_00003439 @ 00003439 ===== */

int __allregs
FUN_00003439(undefined4 param_1,int param_2,int *param_3,int param_4,int param_5,undefined4 param_6)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  
  if (*param_3 != 0) {
    DAT_00002f5c = param_6;
    FUN_00003661(param_3);
    uVar2 = param_2 + (&DAT_00002bcc)[param_5];
    DAT_00002f60 = param_5;
    DAT_00002f64 = (&DAT_00002bcc)[param_5] + *param_3;
    uVar1 = *param_3 + uVar2;
    if (DAT_0000090f <= uVar1) {
      if (((&DAT_00002eec)[param_5] & 1) != 0) {
        iVar6 = thunk_FUN_00000f64(FUN_000037aa);
        return iVar6;
      }
      return 0;
    }
    DAT_00002f50 = uVar1;
    if ((&DAT_00002bcc)[param_5] != uVar2) {
      if (uVar2 <= (uint)(&DAT_00002bcc)[param_5]) {
        FUN_00003584(uVar2,param_5);
        iVar6 = DAT_00002f60 + 1;
        iVar5 = DAT_00002f54 - iVar6;
        do {
          uVar1 = (&DAT_00002bcc)[iVar6];
          puVar8 = *(undefined4 **)(&DAT_000028ac)[iVar6];
          puVar7 = *(undefined4 **)(&DAT_00002a3c + iVar6 * 4);
          *(undefined4 **)(&DAT_00002a3c + iVar6 * 4) = puVar8;
          if (puVar7 != puVar8) {
            for (uVar2 = uVar1 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
              *puVar8 = *puVar7;
              puVar7 = puVar7 + 1;
              puVar8 = puVar8 + 1;
            }
            for (uVar1 = uVar1 & 3; uVar1 != 0; uVar1 = uVar1 - 1) {
              *(undefined1 *)puVar8 = *(undefined1 *)puVar7;
              puVar7 = (undefined4 *)((int)puVar7 + 1);
              puVar8 = (undefined4 *)((int)puVar8 + 1);
            }
          }
          iVar6 = iVar6 + 1;
          iVar5 = iVar5 + -1;
        } while (iVar5 != 0);
        return DAT_00002f64;
      }
      FUN_00003584(uVar2,param_5);
      iVar6 = DAT_00002f54 - (DAT_00002f60 + 1);
      uVar1 = DAT_00002f54;
      do {
        uVar1 = uVar1 - 1;
        uVar2 = (&DAT_00002bcc)[uVar1];
        puVar8 = *(undefined4 **)(&DAT_000028ac)[uVar1];
        puVar7 = *(undefined4 **)(&DAT_00002a3c + uVar1 * 4);
        *(undefined4 **)(&DAT_00002a3c + uVar1 * 4) = puVar8;
        if (puVar7 != puVar8) {
          uVar3 = uVar2 & 0xfffffffc;
          uVar4 = uVar3 - 4;
          if (3 < uVar3) {
            puVar7 = (undefined4 *)((int)puVar7 + uVar4);
            puVar8 = (undefined4 *)((int)puVar8 + uVar4);
            for (iVar5 = (uVar4 >> 2) + 1; iVar5 != 0; iVar5 = iVar5 + -1) {
              *puVar8 = *puVar7;
              puVar7 = puVar7 + -1;
              puVar8 = puVar8 + -1;
            }
            puVar7 = puVar7 + 1;
            puVar8 = puVar8 + 1;
          }
          uVar2 = uVar2 & 3;
          while( true ) {
            puVar8 = (undefined4 *)((int)puVar8 + -1);
            puVar7 = (undefined4 *)((int)puVar7 + -1);
            if (uVar2 == 0) break;
            uVar2 = uVar2 - 1;
            *(undefined1 *)puVar8 = *(undefined1 *)puVar7;
          }
        }
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    if (DAT_00002f6c < DAT_00002f50) {
      DAT_00002f6c = DAT_00002f50;
    }
    return DAT_00002f64;
  }
  DAT_00002f5c = param_6;
  if (DAT_00002f50 == 0) {
    param_1 = FUN_00002f7c(param_1);
  }
  uVar1 = DAT_00002f54;
  if (param_4 == 0) {
    iVar6 = thunk_FUN_00000f64(0x3847);
    return iVar6;
  }
  uVar2 = 0;
  if (DAT_00002f54 != 0) {
    do {
      if ((int *)(&DAT_000028ac)[uVar2] == param_3) {
        iVar6 = thunk_FUN_00000f64(0x3870);
        return iVar6;
      }
      uVar2 = uVar2 + 1;
    } while (uVar2 < DAT_00002f54);
  }
  if (99 < DAT_00002f54) {
    iVar6 = thunk_FUN_00000f64(0x37d1);
    return iVar6;
  }
  (&DAT_000028ac)[DAT_00002f54] = param_3;
  (&DAT_00002bcc)[uVar1] = param_2;
  *(int *)(&DAT_00002d5c + uVar1 * 4) = param_4;
  (&DAT_00002eec)[uVar1] = (char)param_1;
  uVar2 = FUN_00003772(param_1,param_2,param_4);
  uVar1 = DAT_00002f54;
  *(uint *)(&DAT_000028ac)[DAT_00002f54] = uVar2;
  *(uint *)(&DAT_00002a3c + uVar1 * 4) = uVar2;
  if (uVar2 != 0) {
    DAT_00002f54 = DAT_00002f54 + 1;
    if (DAT_00002f68 < DAT_00002f54) {
      DAT_00002f68 = DAT_00002f54;
    }
    if (DAT_00002f6c < DAT_00002f50) {
      DAT_00002f6c = DAT_00002f50;
    }
  }
  return uVar2;
}


/* ===== FUN_0000345c @ 0000345c ===== */

int __allregs FUN_0000345c(uint param_1,int *param_2,int param_3,undefined4 param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  
  DAT_00002f5c = param_4;
  FUN_00003661(param_2);
  DAT_00002f60 = param_3;
  DAT_00002f64 = (&DAT_00002bcc)[param_3] + *param_2;
  uVar1 = *param_2 + param_1;
  if (DAT_0000090f <= uVar1) {
    if (((&DAT_00002eec)[param_3] & 1) != 0) {
      iVar6 = thunk_FUN_00000f64(FUN_000037aa);
      return iVar6;
    }
    return 0;
  }
  DAT_00002f50 = uVar1;
  if ((&DAT_00002bcc)[param_3] != param_1) {
    if (param_1 <= (uint)(&DAT_00002bcc)[param_3]) {
      FUN_00003584(param_1,param_3);
      iVar6 = DAT_00002f60 + 1;
      iVar5 = DAT_00002f54 - iVar6;
      do {
        uVar1 = (&DAT_00002bcc)[iVar6];
        puVar8 = *(undefined4 **)(&DAT_000028ac)[iVar6];
        puVar7 = *(undefined4 **)(&DAT_00002a3c + iVar6 * 4);
        *(undefined4 **)(&DAT_00002a3c + iVar6 * 4) = puVar8;
        if (puVar7 != puVar8) {
          for (uVar2 = uVar1 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
            *puVar8 = *puVar7;
            puVar7 = puVar7 + 1;
            puVar8 = puVar8 + 1;
          }
          for (uVar1 = uVar1 & 3; uVar1 != 0; uVar1 = uVar1 - 1) {
            *(undefined1 *)puVar8 = *(undefined1 *)puVar7;
            puVar7 = (undefined4 *)((int)puVar7 + 1);
            puVar8 = (undefined4 *)((int)puVar8 + 1);
          }
        }
        iVar6 = iVar6 + 1;
        iVar5 = iVar5 + -1;
      } while (iVar5 != 0);
      return DAT_00002f64;
    }
    FUN_00003584(param_1,param_3);
    iVar5 = DAT_00002f54 - (DAT_00002f60 + 1);
    iVar6 = DAT_00002f54;
    do {
      iVar6 = iVar6 + -1;
      uVar1 = (&DAT_00002bcc)[iVar6];
      puVar8 = *(undefined4 **)(&DAT_000028ac)[iVar6];
      puVar7 = *(undefined4 **)(&DAT_00002a3c + iVar6 * 4);
      *(undefined4 **)(&DAT_00002a3c + iVar6 * 4) = puVar8;
      if (puVar7 != puVar8) {
        uVar2 = uVar1 & 0xfffffffc;
        uVar3 = uVar2 - 4;
        if (3 < uVar2) {
          puVar7 = (undefined4 *)((int)puVar7 + uVar3);
          puVar8 = (undefined4 *)((int)puVar8 + uVar3);
          for (iVar4 = (uVar3 >> 2) + 1; iVar4 != 0; iVar4 = iVar4 + -1) {
            *puVar8 = *puVar7;
            puVar7 = puVar7 + -1;
            puVar8 = puVar8 + -1;
          }
          puVar7 = puVar7 + 1;
          puVar8 = puVar8 + 1;
        }
        uVar1 = uVar1 & 3;
        while( true ) {
          puVar8 = (undefined4 *)((int)puVar8 + -1);
          puVar7 = (undefined4 *)((int)puVar7 + -1);
          if (uVar1 == 0) break;
          uVar1 = uVar1 - 1;
          *(undefined1 *)puVar8 = *(undefined1 *)puVar7;
        }
      }
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
  }
  if (DAT_00002f6c < DAT_00002f50) {
    DAT_00002f6c = DAT_00002f50;
  }
  return DAT_00002f64;
}


/* ===== FUN_00003584 @ 00003584 ===== */

uint __allregs FUN_00003584(undefined4 param_1,int param_2)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  
  (&DAT_00002bcc)[param_2] = param_1;
  uVar5 = param_2 + 1;
  iVar4 = DAT_00002f54 - uVar5;
  if (DAT_00002f54 < uVar5 || iVar4 == 0) {
    if (DAT_00002f6c < DAT_00002f50) {
      DAT_00002f6c = DAT_00002f50;
    }
    return DAT_00002f64;
  }
  while( true ) {
    puVar1 = (uint *)(&DAT_000028ac)[uVar5];
    iVar2 = (&DAT_00002bcc)[uVar5];
    uVar3 = ~(*(int *)(&DAT_00002d5c + uVar5 * 4) - 1U);
    uVar6 = DAT_00002f50 + (*(int *)(&DAT_00002d5c + uVar5 * 4) - 1U) & uVar3;
    if ((*puVar1 != uVar6) && (((&DAT_00002eec)[uVar5] & 2) != 0)) {
      if (*puVar1 < uVar6) {
        uVar5 = thunk_FUN_00000f64(0x389a);
        return uVar5;
      }
      uVar6 = *puVar1;
      DAT_00002f70 = DAT_00002f70 + 1;
    }
    *puVar1 = uVar6;
    uVar6 = uVar6 + iVar2;
    if (DAT_0000090f <= uVar6) break;
    DAT_00002f50 = uVar6;
    uVar5 = uVar5 + 1;
    iVar4 = iVar4 + -1;
    if (iVar4 == 0) {
      return uVar3;
    }
  }
  uVar5 = thunk_FUN_00000f64(FUN_000037aa);
  return uVar5;
}


/* ===== FUN_000035fb @ 000035fb ===== */

undefined4 __allregs FUN_000035fb(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  if (((&DAT_00002eec)[DAT_00002f60] & 1) == 0) {
    iVar3 = DAT_00002f60 + 1;
    iVar2 = DAT_00002f54 - iVar3;
    do {
      *(undefined4 *)(&DAT_000028ac)[iVar3] = *(undefined4 *)(&DAT_00002a3c + iVar3 * 4);
      iVar3 = iVar3 + 1;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    DAT_00002f50 = *(int *)(&DAT_00002a3c + (DAT_00002f54 + -1) * 4) +
                   (&DAT_00002bcc)[DAT_00002f54 + -1];
    return 0;
  }
  uVar1 = thunk_FUN_00000f64(FUN_000037aa);
  return uVar1;
}


/* ===== FUN_00003661 @ 00003661 ===== */

int __allregs FUN_00003661(int *param_1)

{
  int iVar1;
  int iVar2;
  
  DAT_00002f58 = param_1;
  iVar2 = 0;
  iVar1 = DAT_00002f54;
  while( true ) {
    if (iVar1 == 0) {
      iVar2 = thunk_FUN_00000f64(0x37fd);
      return iVar2;
    }
    if ((int *)(&DAT_000028ac)[iVar2] == param_1) break;
    iVar2 = iVar2 + 1;
    iVar1 = iVar1 + -1;
  }
  if (*(int *)(&DAT_00002a3c + iVar2 * 4) != *param_1) {
    iVar2 = thunk_FUN_00000f64(0x3822);
    return iVar2;
  }
  iVar1 = DAT_0000090b;
  if (iVar2 != 0) {
    iVar1 = **(int **)((int)&DAT_000028a5 + iVar2 * 4 + 3) + *(int *)(iVar2 * 4 + 0x2bc8);
  }
  return iVar1;
}


/* ===== FUN_000036bf @ 000036bf ===== */

int __allregs
FUN_000036bf(undefined4 param_1,undefined4 param_2,int param_3,int param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  
  DAT_00002f5c = param_5;
  if (DAT_00002f50 == 0) {
    param_1 = FUN_00002f7c(param_1);
  }
  uVar1 = DAT_00002f54;
  if (param_4 == 0) {
    iVar2 = thunk_FUN_00000f64(0x3847);
    return iVar2;
  }
  uVar3 = 0;
  if (DAT_00002f54 != 0) {
    do {
      if ((&DAT_000028ac)[uVar3] == param_3) {
        iVar2 = thunk_FUN_00000f64(0x3870);
        return iVar2;
      }
      uVar3 = uVar3 + 1;
    } while (uVar3 < DAT_00002f54);
  }
  if (99 < DAT_00002f54) {
    iVar2 = thunk_FUN_00000f64(0x37d1);
    return iVar2;
  }
  (&DAT_000028ac)[DAT_00002f54] = param_3;
  (&DAT_00002bcc)[uVar1] = param_2;
  *(int *)(&DAT_00002d5c + uVar1 * 4) = param_4;
  (&DAT_00002eec)[uVar1] = (char)param_1;
  uVar3 = FUN_00003772(param_1,param_2,param_4);
  uVar1 = DAT_00002f54;
  *(uint *)(&DAT_000028ac)[DAT_00002f54] = uVar3;
  *(uint *)(&DAT_00002a3c + uVar1 * 4) = uVar3;
  if (uVar3 != 0) {
    DAT_00002f54 = DAT_00002f54 + 1;
    if (DAT_00002f68 < DAT_00002f54) {
      DAT_00002f68 = DAT_00002f54;
    }
    if (DAT_00002f6c < DAT_00002f50) {
      DAT_00002f6c = DAT_00002f50;
    }
  }
  return uVar3;
}


/* ===== FUN_00003772 @ 00003772 ===== */

uint __allregs FUN_00003772(uint param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = DAT_00002f50 + (param_3 - 1U) & ~(param_3 - 1U);
  uVar2 = uVar1 + param_2;
  if (uVar2 < DAT_0000090f) {
    DAT_00002f50 = uVar2;
    return uVar1;
  }
  if ((param_1 & 1) == 0) {
    return 0;
  }
  uVar1 = thunk_FUN_00000f64(FUN_000037aa);
  return uVar1;
}


/* ===== FUN_000037aa @ 000037aa ===== */

/* WARNING: Instruction at (ram,0x0000392a) overlaps instruction at (ram,0x00003929)
    */
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x00003882) */
/* WARNING: Removing unreachable block (ram,0x000038ea) */
/* WARNING: Removing unreachable block (ram,0x00003885) */
/* WARNING: Removing unreachable block (ram,0x000038a7) */
/* WARNING: Removing unreachable block (ram,0x00003887) */
/* WARNING: Removing unreachable block (ram,0x00003845) */
/* WARNING: Removing unreachable block (ram,0x000038c5) */
/* WARNING: Removing unreachable block (ram,0x00003937) */
/* WARNING: Removing unreachable block (ram,0x000038d2) */
/* WARNING: Removing unreachable block (ram,0x000037bb) */

byte * __allregs
FUN_000037aa(byte *param_1,int param_2,char *param_3,int param_4,int param_5,char *param_6,
            undefined4 *param_7,int param_8,int param_9,char *param_10,int param_11,byte *param_12,
            undefined4 *param_13,undefined4 *param_14,undefined4 param_15,undefined2 param_16,
            byte *param_17,char *param_18,undefined4 *param_19,int param_20,char *param_21,
            char *param_22,byte *param_23,undefined4 param_24,undefined4 param_25,char *param_26,
            undefined4 param_27,byte *param_28)

{
  undefined1 uVar1;
  char cVar2;
  undefined4 uVar3;
  int iVar4;
  char cVar5;
  byte bVar7;
  byte *pbVar6;
  undefined2 uVar8;
  undefined2 uVar9;
  byte bVar10;
  BADSPACEBASE *in_ESP;
  short sVar11;
  int iVar12;
  undefined4 *puVar13;
  char *pcVar14;
  char *pcVar15;
  int in_FS_OFFSET;
  int in_GS_OFFSET;
  bool bVar16;
  bool bVar17;
  
  pcVar15 = param_6 + 1;
  puVar13 = (undefined4 *)(param_5 + 3);
  uVar8 = SUB42(param_3,0);
  out(*(undefined4 *)(param_5 + -1),uVar8);
  uVar9 = SUB42(param_10,0);
  bVar7 = (byte)param_11;
  if ((undefined4 *)(param_5 + -1) != (undefined4 *)0x0) {
    out(*(undefined1 *)puVar13,uVar8);
    param_19 = (undefined4 *)(param_5 + 8);
    out(*(undefined4 *)(param_5 + 4),uVar8);
    param_21 = param_3;
    param_20 = param_4;
code_r0x00003815:
    cVar5 = (char)((uint)param_2 >> 8);
    param_1[0x6f] = param_1[0x6f] & (byte)((uint)param_21 >> 8);
    param_23 = (byte *)((uint)param_1 | 0x454d240a);
    bVar7 = *param_23;
    param_18 = pcVar15 + 5;
    uVar8 = SUB42(param_21,0);
    uVar3 = in(uVar8);
    *(undefined4 *)(pcVar15 + 1) = uVar3;
    param_22 = (char *)(param_19 + 1);
    out(*param_19,uVar8);
    param_17 = param_23;
    if ((byte)((uint)param_23 >> 8) < bVar7) {
      pcVar15 = pcVar15 + 6;
      cVar2 = in(uVar8);
      *param_18 = cVar2;
      pbVar6 = (byte *)(in_GS_OFFSET + param_20 + 0x68);
      *pbVar6 = *pbVar6 & (byte)param_23;
      bVar16 = true;
      if (*pbVar6 != 0) goto code_r0x0000391e;
    }
    else {
      pbVar6 = param_23 + 0x6f;
      *pbVar6 = *pbVar6 & (byte)param_21;
      bVar16 = *pbVar6 == 0;
      bVar17 = (longlong)(int)((longlong)(int)param_19[0x1e] * 0x63207265) !=
               (longlong)(int)param_19[0x1e] * 0x63207265;
      param_19 = param_19 + 2;
      out(*(undefined4 *)param_22,uVar8);
      if ((!bVar17) && (bVar16)) {
        bVar16 = true;
        param_22 = param_21;
        goto LAB_000038a6;
      }
LAB_000038ad_2:
      param_22 = (char *)((int)param_19 + 1);
      out(*(undefined1 *)param_19,(short)param_21);
      pcVar15 = param_18;
    }
    param_26._0_2_ = SUB42(param_21,0);
    bVar17 = (longlong)(int)&stack0x0000006c != (longlong)*(int *)param_23 * 0x61;
    param_28 = param_23;
code_r0x000038b3:
    pcVar14 = param_22;
    if (!bVar16) {
      uVar3 = in(param_26._0_2_);
      *(undefined4 *)pcVar15 = uVar3;
      if (bVar17) {
        return param_28;
      }
      iVar4 = *(int *)(param_22 + 0x67);
      iVar12 = (int)((longlong)iVar4 * 0x206f7420);
      uVar3 = in(param_26._0_2_);
      *(undefined4 *)(pcVar15 + 4) = uVar3;
      pcVar14 = param_22 + 4;
      out(*(undefined4 *)param_22,param_26._0_2_);
      if ((longlong)iVar12 == (longlong)iVar4 * 0x206f7420) {
        pbVar6 = (byte *)(iVar12 + 0x70);
        *pbVar6 = *pbVar6 & (byte)((ushort)param_26._0_2_ >> 8);
LAB_000038c7:
        pbVar6 = (byte *)thunk_FUN_00000f64(0x37d1);
        return pbVar6;
      }
    }
    *param_28 = *param_28 + (char)param_28;
    *pcVar14 = *pcVar14 + cVar5;
    return param_28;
  }
  bVar16 = (byte)((uint)param_1 >> 8) < *param_1;
  bVar17 = param_6 == (char *)0x0;
  pcVar15 = param_3;
  if (bVar16 || bVar17) {
code_r0x0000384c:
    pbVar6 = (byte *)(in_FS_OFFSET + param_8 + 0x65);
    *pbVar6 = *pbVar6 & bVar7;
    uVar3 = in(uVar9);
    *(undefined4 *)pcVar15 = uVar3;
    out(*param_7,uVar9);
    pbVar6 = (byte *)(param_8 + 0x61);
    *pbVar6 = *pbVar6 & bVar7;
    bVar7 = *pbVar6;
    out(*(undefined1 *)(param_7 + 1),uVar9);
    uVar3 = in(param_16);
    *param_13 = uVar3;
    cVar5 = (char)param_17;
    if (bVar7 == 0) {
      *param_17 = *param_17 + cVar5;
      goto LAB_000038c7;
    }
    bVar7 = *param_17;
    uVar3 = in(param_16);
    *(undefined4 *)((int)param_13 + 5) = uVar3;
    out(*param_14,param_16);
    if ((byte)((uint)param_17 >> 8) < bVar7) {
      *param_17 = *param_17 + cVar5;
      *param_17 = *param_17;
    }
    else {
      pbVar6 = param_17 + 0x6f;
      *pbVar6 = *pbVar6 & (byte)param_16;
      bVar7 = *pbVar6;
      bVar16 = *pbVar6 == 0;
      iVar4 = param_14[0x1e];
      uVar1 = in(param_16);
      *(undefined1 *)((int)param_13 + 9) = uVar1;
      if ((longlong)(int)((longlong)iVar4 * 0x61207265) == (longlong)iVar4 * 0x61207265) {
        cVar5 = param_22._1_1_;
        if (-1 < (char)bVar7) goto LAB_000038ad_2;
        goto code_r0x0000388f;
      }
      *param_17 = *param_17 + cVar5;
      *(byte **)param_17 = param_17 + *(int *)param_17;
      *(char *)(param_14 + -0x1fffffff) =
           *(char *)(param_14 + -0x1fffffff) + (char)((uint)param_15 >> 8);
      *param_17 = *param_17 + cVar5;
    }
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *(byte **)param_17 = param_17 + *(int *)param_17;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + (byte)param_16;
  }
  else {
    pcVar15 = param_3;
    iVar4 = param_4;
    if (!bVar16) {
      uVar3 = in(uVar8);
      *(undefined4 *)param_6 = uVar3;
      pcVar15 = param_10;
      if (-1 < (int)param_6) {
        param_6 = param_3 + 4;
        uVar3 = in(uVar9);
        *(undefined4 *)param_3 = uVar3;
        param_1 = param_12;
        param_2 = param_11;
        iVar4 = param_9;
        puVar13 = param_7;
        if (bVar17) goto code_r0x000037e5;
        *(byte *)(param_8 + 0x65) = *(byte *)(param_8 + 0x65) & bVar7;
      }
      goto code_r0x0000384c;
    }
code_r0x000037e5:
    param_20 = iVar4;
    param_21 = pcVar15;
    bVar7 = (byte)((uint)param_2 >> 8);
    *(byte *)((int)puVar13 + 0x75) = *(byte *)((int)puVar13 + 0x75) & bVar7;
    uVar9 = SUB42(param_21,0);
    uVar3 = in(uVar9);
    *(undefined4 *)param_6 = uVar3;
    param_6[0x6a] = param_6[0x6a] & bVar7;
    bVar10 = (byte)((uint)param_21 >> 8);
    param_1[0x6f] = param_1[0x6f] & bVar10;
    iVar4 = puVar13[0x1d];
    param_2 = CONCAT31((int3)((uint)param_2 >> 8),(byte)param_2 | DAT_454d240a);
    out(*(undefined1 *)puVar13,uVar9);
    iVar12 = *(short *)((int)puVar13 + in_GS_OFFSET + 0x66) * 0x2064;
    sVar11 = (short)iVar12;
    param_18 = param_6 + 9;
    uVar3 = in(uVar9);
    *(undefined4 *)(param_6 + 5) = uVar3;
    param_19 = (undefined4 *)((int)puVar13 + 5);
    out(*(undefined4 *)((int)puVar13 + 1),uVar9);
    pcVar15 = param_18;
    param_26 = param_3;
    if (sVar11 == iVar12) goto code_r0x00003815;
    *(byte *)(param_2 + 0x6e) = *(byte *)(param_2 + 0x6e) & bVar7;
    pbVar6 = (byte *)(CONCAT22((short)((uint)(iVar4 * 0xd737265 + -2) >> 0x10),sVar11) + 0x72);
    *pbVar6 = *pbVar6 & bVar10;
    param_23 = (byte *)((uint)param_1 | 0x240a0d0a);
code_r0x0000388f:
    param_22 = (char *)(param_19[8] * 0xd657375 + -2);
    bVar7 = (byte)((uint)param_23 >> 8);
    bVar17 = bVar7 < *param_23;
    bVar16 = bVar7 == *param_23;
    uVar9 = SUB42(param_21,0);
    out(*(undefined1 *)param_19,uVar9);
    uVar3 = in(uVar9);
    *(undefined4 *)(param_18 + 1) = uVar3;
    out(*(undefined4 *)((int)param_19 + 1),uVar9);
    param_17 = param_23;
    param_26._0_2_ = uVar8;
LAB_000038a6:
    if (!bVar17 && !bVar16) {
      bVar17 = SCARRY4(param_20,1);
      bVar16 = param_20 == -1;
      pcVar15 = &DAT_206b6e75;
      cVar5 = (char)((uint)param_27 >> 8);
      goto code_r0x000038b3;
    }
    cVar5 = (char)param_17;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
    *param_17 = *param_17 + cVar5;
  }
  *param_17 = *param_17 + (char)param_17;
code_r0x0000391e:
  cVar5 = (char)param_17;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  *param_17 = *param_17 + cVar5;
  return param_17;
}


/* ===== thunk_FUN_00000f64 @ 000038e8 ===== */

void __allregs thunk_FUN_00000f64(undefined4 param_1)

{
  FUN_00000f64(param_1);
  return;
}


/* ===== FUN_00003930 @ 00003930 ===== */

void __allregs FUN_00003930(void)

{
  return;
}


/* ===== FUN_00003931 @ 00003931 ===== */

void __allregs FUN_00003931(void)

{
  FUN_000085d0();
  if (DAT_0000395d == '\0') {
    (*(code *)PTR_FUN_00003958)();
    FUN_00006980();
    return;
  }
  FUN_0000686f();
  FUN_00006c00();
  return;
}


/* ===== FUN_0000395e @ 0000395e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_0000395e(void)

{
  longlong lVar1;
  undefined1 uVar4;
  uint uVar2;
  int iVar3;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined1 *puVar10;
  undefined1 *puVar11;
  
  DAT_00003924 = (DAT_000038f1 - 1U) * 0x100;
  _DAT_00003928 = (DAT_000038f1 - 1U & 0xffffff) * 0x80 + 0x100;
  iVar9 = 0;
  iVar6 = _FUN_00003a20;
  puVar10 = LAB_00003909;
  do {
    *(undefined4 *)(iVar9 * 4 + 0x4624) = *(undefined4 *)(iVar9 * 4 + 0x3e24);
    lVar1 = (ulonglong)*(uint *)(iVar9 * 4 + 0x3a24) * (ulonglong)DAT_000090c0;
    uVar2 = (uint)lVar1 >> 0x18 | (int)((ulonglong)lVar1 >> 0x20) << 8;
    *(uint *)(iVar9 * 4 + 0x4224) = uVar2;
    uVar2 = (int)(0xffffffff / (longlong)(int)uVar2) << 5;
    iVar5 = 0x80;
    uVar8 = 0;
    do {
      iVar3 = _DAT_00003928 + (int)((ulonglong)uVar8 * (ulonglong)uVar2 >> 0x20);
      if (DAT_00003924 <= iVar3) {
        iVar3 = DAT_00003924;
      }
      puVar11 = puVar10 + 1;
      *puVar10 = (char)((uint)iVar3 >> 8);
      uVar8 = uVar8 + 0x1000000;
      iVar5 = iVar5 + -1;
      puVar10 = puVar11;
    } while (iVar5 != 0);
    iVar5 = 0x80;
    uVar8 = 0x80000000;
    do {
      uVar7 = (uint)((ulonglong)uVar8 * (ulonglong)uVar2 >> 0x20);
      uVar4 = (undefined1)(_DAT_00003928 - uVar7 >> 8);
      if (_DAT_00003928 < uVar7) {
        uVar4 = 0;
      }
      puVar10 = puVar11 + 1;
      *puVar11 = uVar4;
      uVar8 = uVar8 - 0x1000000;
      iVar5 = iVar5 + -1;
      puVar11 = puVar10;
    } while (iVar5 != 0);
    iVar9 = iVar9 + 1;
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  return;
}


/* ===== FUN_00003a18 @ 00003a18 ===== */

void __allregs FUN_00003a18(int *param_1)

{
  char cVar1;
  int iVar2;
  uint *puVar3;
  uint *puVar4;
  
  *param_1 = (int)(*param_1 + (int)param_1);
  cVar1 = (char)param_1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  iVar2 = 0xc0;
  puVar3 = &DAT_00005598;
  puVar4 = &DAT_00005598;
  do {
    *puVar4 = *puVar3 >> 2 & 0x3f3f3f3f;
    iVar2 = iVar2 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (iVar2 != 0);
  return;
}


/* ===== FUN_00003a20 @ 00003a20 ===== */

void __allregs FUN_00003a20(int *param_1)

{
  char cVar1;
  int iVar2;
  uint *puVar3;
  uint *puVar4;
  
  *param_1 = (int)(*param_1 + (int)param_1);
  cVar1 = (char)param_1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *param_1 = (int)(*param_1 + (int)param_1);
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  *(char *)param_1 = (char)*param_1 + cVar1;
  iVar2 = 0xc0;
  puVar3 = &DAT_00005598;
  puVar4 = &DAT_00005598;
  do {
    *puVar4 = *puVar3 >> 2 & 0x3f3f3f3f;
    iVar2 = iVar2 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (iVar2 != 0);
  return;
}


/* ===== FUN_00004a3c @ 00004a3c ===== */

void __allregs FUN_00004a3c(void)

{
  int iVar1;
  uint *puVar2;
  uint *puVar3;
  
  iVar1 = 0xc0;
  puVar2 = &DAT_00005598;
  puVar3 = &DAT_00005598;
  do {
    *puVar3 = *puVar2 >> 2 & 0x3f3f3f3f;
    iVar1 = iVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (iVar1 != 0);
  return;
}


/* ===== FUN_00005c98 @ 00005c98 ===== */

undefined4 __allregs
FUN_00005c98(undefined2 param_1,undefined2 param_2,undefined4 param_3,undefined2 param_4)

{
  undefined1 in_CF;
  
  FUN_00005cc2(0x4e00,0x33,param_1,param_2,param_3,param_4);
  if (!(bool)in_CF) {
    return 0xffffffff;
  }
  return 0;
}


/* ===== FUN_00005cc2 @ 00005cc2 ===== */

/* WARNING: Removing unreachable block (ram,0x00005cdd) */

void __allregs
FUN_00005cc2(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined4 param_5,undefined2 param_6)

{
  undefined4 uVar1;
  undefined2 uVar2;
  undefined2 unaff_ES;
  bool bVar3;
  undefined2 extraout_var;
  
  uRam000f0000 = CONCAT11(0x1a,(char)param_1);
  uRam000f0014 = 0x21;
  uRam000f0006 = (undefined2)DAT_00000927;
  uRam000f0008 = (undefined2)param_5;
  uRam000f000e = 0x1c00;
  DAT_00005cb8 = param_5;
  DAT_00005cbc = param_1;
  DAT_00005cbe = param_2;
  uRam000f0002 = param_3;
  uRam000f0004 = param_2;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar1 = CONCAT22(extraout_var,uRam000f0006);
  if (*(char *)(DAT_00000c93 + 0x496) != '\0') {
    DAT_00006238 = &DAT_00006228;
    DAT_00006228 = *(char *)(DAT_00000c93 + 0x496) + '@';
  }
  bVar3 = false;
  uVar2 = uRam000f0002;
  if (PTR_DAT_00006234 != (undefined *)0x0) {
    PTR_DAT_00006230 = PTR_DAT_00006234;
    FUN_00005d50(DAT_00005cbc,DAT_00005cbe,uVar1,uRam000f0002,param_4,DAT_00005cb8,0x5d24);
    if (!bVar3) {
      return;
    }
  }
  if (DAT_00006238 != (undefined1 *)0x0) {
    PTR_DAT_00006230 = DAT_00006238;
    FUN_00005d50(DAT_00005cbc,DAT_00005cbe,uVar1,uVar2,param_4,DAT_00005cb8,0x5d4c);
  }
  return;
}


/* ===== FUN_00005d50 @ 00005d50 ===== */

/* WARNING: Removing unreachable block (ram,0x00005df4) */

void __allregs
FUN_00005d50(ushort param_1,undefined2 param_2,undefined4 param_3,undefined2 param_4,
            undefined2 param_5,byte *param_6,undefined4 param_7)

{
  byte bVar1;
  ushort uVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte *pbVar5;
  undefined2 unaff_ES;
  bool bVar6;
  
  DAT_00005cb4 = s_FILEMGR__Unable_to_find_or_open___00005e7a + 0x3d;
  if ((char)(param_1 >> 8) != '<') {
    DAT_00005cb4 = s_FILEMGR__Unable_to_find_or_open___00005e7a;
  }
  pbVar3 = &DAT_00005898;
  DAT_00005cbc = param_1;
  DAT_00005cbe = param_2;
  for (pbVar4 = PTR_DAT_00006230; bVar1 = *pbVar4, bVar1 != 0; pbVar4 = pbVar4 + 1) {
    *pbVar3 = bVar1;
    pbVar3[0x100] = bVar1;
    pbVar3 = pbVar3 + 1;
  }
  pbVar4 = pbVar3;
  pbVar5 = PTR_s_RESOURCE_RES_0000622c;
  if (PTR_s_RESOURCE_RES_0000622c != (undefined *)0x0) {
    do {
      bVar1 = *pbVar5;
      pbVar4[0x100] = bVar1;
      pbVar4 = pbVar4 + 1;
      pbVar5 = pbVar5 + 1;
    } while (bVar1 != 0);
  }
  pbVar4 = (byte *)(s_FILEMGR__Unable_to_find_or_open___00005e7a + 0x57);
  s_FILEMGR__Unable_to_find_or_open___00005e7a[0x57] = '$';
  if (0x20 < *param_6) {
    while( true ) {
      bVar1 = *param_6 + (0x5f < *param_6) * -0x20;
      if (bVar1 < 0x21) break;
      *pbVar4 = bVar1;
      pbVar4 = pbVar4 + 1;
      *pbVar3 = bVar1;
      pbVar3 = pbVar3 + 1;
      param_6 = param_6 + 1;
    }
    *pbVar3 = 0;
    *pbVar4 = 0x24;
    DAT_00005cc0 = 0;
    uRam000f0014 = 0x21;
    uRam000f0000 = DAT_00005cbc;
    uRam000f0004 = DAT_00005cbe;
    uRam000f0006 = 0x5898;
    uRam000f0008 = SUB42(param_6 + 1,0);
    uRam000f000a = SUB42(pbVar4,0);
    uRam000f000e = 0x1c00;
    uRam000f0002 = param_4;
    uRam000f000c = param_5;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar2 = uRam000f0000;
    if ((!(bool)uRam000f0012) ||
       ((((char)(DAT_00005cbc >> 8) != '<' &&
         (bVar6 = false, PTR_s_RESOURCE_RES_0000622c != (undefined *)0x0)) &&
        (uVar2 = FUN_00006250(param_5,param_3), !bVar6)))) {
      DAT_00005cc0 = uVar2;
      if ((char)(DAT_00005cbc >> 8) != '<') {
        DAT_00005cb4 = s_FILEMGR__Unable_to_find_or_open___00005e7a + 0x22;
      }
      return;
    }
  }
  return;
}


/* ===== FUN_00005d78 @ 00005d78 ===== */

/* WARNING: Removing unreachable block (ram,0x00005df4) */

void __allregs
FUN_00005d78(undefined2 param_1,undefined2 param_2,byte *param_3,undefined4 param_4,
            undefined4 param_5)

{
  byte bVar1;
  ushort uVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte *pbVar5;
  undefined2 unaff_ES;
  bool bVar6;
  
  pbVar3 = &DAT_00005898;
  for (pbVar4 = PTR_DAT_00006230; bVar1 = *pbVar4, bVar1 != 0; pbVar4 = pbVar4 + 1) {
    *pbVar3 = bVar1;
    pbVar3[0x100] = bVar1;
    pbVar3 = pbVar3 + 1;
  }
  pbVar4 = pbVar3;
  pbVar5 = PTR_s_RESOURCE_RES_0000622c;
  if (PTR_s_RESOURCE_RES_0000622c != (undefined *)0x0) {
    do {
      bVar1 = *pbVar5;
      pbVar4[0x100] = bVar1;
      pbVar4 = pbVar4 + 1;
      pbVar5 = pbVar5 + 1;
    } while (bVar1 != 0);
  }
  pbVar4 = (byte *)(s_FILEMGR__Unable_to_find_or_open___00005e7a + 0x57);
  s_FILEMGR__Unable_to_find_or_open___00005e7a[0x57] = '$';
  if (0x20 < *param_3) {
    while( true ) {
      bVar1 = *param_3 + (0x5f < *param_3) * -0x20;
      if (bVar1 < 0x21) break;
      *pbVar4 = bVar1;
      pbVar4 = pbVar4 + 1;
      *pbVar3 = bVar1;
      pbVar3 = pbVar3 + 1;
      param_3 = param_3 + 1;
    }
    *pbVar3 = 0;
    *pbVar4 = 0x24;
    DAT_00005cc0 = 0;
    uRam000f0014 = 0x21;
    uRam000f0000 = DAT_00005cbc;
    uRam000f0004 = DAT_00005cbe;
    uRam000f0006 = 0x5898;
    uRam000f0008 = SUB42(param_3 + 1,0);
    uRam000f000a = SUB42(pbVar4,0);
    uRam000f000e = 0x1c00;
    uRam000f0002 = param_1;
    uRam000f000c = param_2;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar2 = uRam000f0000;
    if ((!(bool)uRam000f0012) ||
       ((((char)(DAT_00005cbc >> 8) != '<' &&
         (bVar6 = false, PTR_s_RESOURCE_RES_0000622c != (undefined *)0x0)) &&
        (uVar2 = FUN_00006250(param_2,param_4), !bVar6)))) {
      DAT_00005cc0 = uVar2;
      if ((char)(DAT_00005cbc >> 8) != '<') {
        DAT_00005cb4 = s_FILEMGR__Unable_to_find_or_open___00005e7a + 0x22;
      }
      return;
    }
  }
  return;
}


/* ===== FUN_00005e3a @ 00005e3a ===== */

/* WARNING: Removing unreachable block (ram,0x00005e48) */

void __allregs
FUN_00005e3a(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f000e = 0x1c00;
    uRam000f0004 = param_2;
    uRam000f0006 = param_3;
    uRam000f0008 = param_5;
    uRam000f000a = param_6;
    uRam000f000c = param_4;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  thunk_FUN_00000f64(1);
  return;
}


/* ===== FUN_00005e5d @ 00005e5d ===== */

/* WARNING: Removing unreachable block (ram,0x00005e6e) */
/* WARNING: Removing unreachable block (ram,0x00005e65) */
/* WARNING: Removing unreachable block (ram,0x00005e77) */

undefined2 __allregs
FUN_00005e5d(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  undefined2 uVar1;
  
  uVar1 = 0x1c00;
  uRam000f0000 = CONCAT11(9,param_1);
  uRam000f0014 = 0x21;
  uRam000f0006 = (undefined2)DAT_00005cb4;
  uRam000f000e = 0x1c00;
  uRam000f0002 = param_3;
  uRam000f0004 = param_2;
  uRam000f0008 = param_5;
  uRam000f000a = param_6;
  uRam000f000c = param_4;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0x5ed1;
  uRam000f000c = param_4;
  uRam000f000e = uVar1;
  (*(code *)0xf0100)();
  uRam000f0000 = CONCAT11(9,(char)uRam000f0000);
  uRam000f0014 = 0x21;
  uRam000f0006 = 0x5f11;
  uRam000f000c = param_4;
  uRam000f000e = uVar1;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  return uRam000f0000;
}


/* ===== FUN_00005f17 @ 00005f17 ===== */

void __allregs FUN_00005f17(uint param_1)

{
  int iVar1;
  
  PTR_LAB_00005560 = s_KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK_00000005 + 0x3fb;
  do {
    PTR_LAB_00005560 = (undefined *)((uint)PTR_LAB_00005560 >> 1);
    if (PTR_LAB_00005560 < s_KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK_00000005 + 0x3fc) {
      DAT_00005564 = &DAT_00005898;
      break;
    }
    iVar1 = FUN_000036bf(param_1 & 0xffffff00,PTR_LAB_00005560,&DAT_00005564,4,0x5f47);
    param_1 = 0;
  } while (iVar1 == 0);
  DAT_00005568 = PTR_LAB_00005560 + (int)DAT_00005564;
  return;
}


/* ===== FUN_00005f6c @ 00005f6c ===== */

void __allregs FUN_00005f6c(undefined4 param_1)

{
  if (DAT_00005564 != &DAT_00005898) {
    FUN_00003376(&DAT_00005564,param_1,0x5f7f);
  }
  return;
}


/* ===== FUN_00005f80 @ 00005f80 ===== */

/* WARNING: Removing unreachable block (ram,0x00005fa9) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

undefined8 * __allregs
FUN_00005f80(undefined1 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined2 param_5,undefined2 param_6,uint param_7)

{
  undefined8 *puVar1;
  undefined3 in_register_00000001;
  undefined2 unaff_ES;
  undefined2 uVar2;
  
  uVar2 = 0x1c00;
  if (DAT_00005564 < param_7) {
    sRam000f0006 = (short)DAT_00005564;
    uRam000f000a = (ushort)param_7;
    sRam000f0004 = uRam000f000a - sRam000f0006;
    uRam000f0000 = CONCAT11(0x40,param_1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f000e = 0x1c00;
    uRam000f0008 = param_6;
    uRam000f000c = param_5;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)(param_3,param_2,param_4);
    param_7 = (uint)uRam000f000a;
    if (!(bool)uRam000f0012) {
      return (undefined8 *)CONCAT31(in_register_00000001,param_1);
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    sRam000f0004 = (short)param_2;
    sRam000f0006 = (short)param_3;
    uRam000f000a = (ushort)param_7;
    uRam000f0008 = param_6;
    uRam000f000c = param_5;
    uRam000f000e = uVar2;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  puVar1 = (undefined8 *)thunk_FUN_00000f64(1);
  return puVar1;
}


/* ===== FUN_00005fbe @ 00005fbe ===== */

/* WARNING: Removing unreachable block (ram,0x00005e48) */
/* WARNING: Removing unreachable block (ram,0x00006004) */
/* WARNING: Removing unreachable block (ram,0x0000601d) */

undefined8 * __allregs
FUN_00005fbe(undefined4 param_1,short param_2,undefined1 param_3,short param_4,undefined4 *param_5,
            short param_6,short param_7)

{
  undefined8 *puVar1;
  undefined *puVar2;
  uint uVar3;
  uint uVar4;
  undefined3 in_register_00000009;
  undefined4 *puVar5;
  undefined2 unaff_ES;
  undefined2 uVar6;
  
  uVar6 = 0x1c00;
  puVar2 = (undefined *)((int)param_5 - (int)DAT_00005564);
  uVar3 = (int)PTR_LAB_00005560 - (int)puVar2;
  sRam000f000c = param_4;
  uRam000f0010 = unaff_ES;
  if (PTR_LAB_00005560 < puVar2 || uVar3 == 0) {
    uRam000f0000 = CONCAT11(0x3f,(char)puVar2);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    sRam000f0004 = (short)PTR_LAB_00005560;
    sRam000f0006 = (short)DAT_00005564;
    uRam000f0008 = SUB42(param_5,0);
    uRam000f000e = 0x1c00;
    sRam000f000a = param_6;
    (*(code *)0xf0100)(param_1);
  }
  else {
    puVar5 = DAT_00005564;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar5 = *param_5;
      param_5 = param_5 + 1;
      puVar5 = puVar5 + 1;
    }
    for (uVar4 = uVar3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined1 *)puVar5 = *(undefined1 *)param_5;
      param_5 = (undefined4 *)((int)param_5 + 1);
      puVar5 = (undefined4 *)((int)puVar5 + 1);
    }
    sRam000f0006 = (short)puVar5;
    sRam000f0004 = ((short)PTR_LAB_00005560 + (short)DAT_00005564) - sRam000f0006;
    uRam000f0000 = CONCAT11(0x3f,(char)uVar3);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = SUB42(param_5,0);
    uRam000f000e = 0x1c00;
    sRam000f000a = sRam000f0006;
    (*(code *)0xf0100)();
  }
  if (!(bool)uRam000f0012) {
    return (undefined8 *)CONCAT31(in_register_00000009,param_3);
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_3);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    sRam000f000c = param_7;
    sRam000f0004 = param_6;
    sRam000f0006 = param_2;
    sRam000f000a = param_4;
    uRam000f000e = uVar6;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  puVar1 = (undefined8 *)thunk_FUN_00000f64(1);
  return puVar1;
}


/* ===== FUN_00005fd3 @ 00005fd3 ===== */

/* WARNING: Removing unreachable block (ram,0x00006004) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */
/* WARNING: Type propagation algorithm not settling */

undefined8 * __allregs
FUN_00005fd3(uint param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
            undefined1 param_5,undefined3 param_6,short param_7,short param_8,short param_9,
            undefined2 param_10)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  char cVar3;
  undefined8 *puVar4;
  uint uVar5;
  BADSPACEBASE *in_ESP;
  undefined2 unaff_ES;
  undefined2 uVar6;
  undefined4 *apuStack_8 [2];
  
  uVar6 = 0x1c00;
  puVar1 = apuStack_8 + 1;
  cVar3 = '\n';
  puVar2 = param_2;
  do {
    puVar2 = puVar2 + -1;
    puVar1 = puVar1 + -1;
    *puVar1 = *puVar2;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  for (uVar5 = param_1 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *param_4 = *param_3;
    param_3 = param_3 + 1;
    param_4 = param_4 + 1;
  }
  for (uVar5 = param_1 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)param_4 = *(undefined1 *)param_3;
    param_3 = (undefined4 *)((int)param_3 + 1);
    param_4 = (undefined4 *)((int)param_4 + 1);
  }
  sRam000f0006 = (short)param_4;
  sRam000f0004 = ((short)PTR_LAB_00005560 + (short)DAT_00005564) - sRam000f0006;
  uRam000f0000 = CONCAT11(0x3f,(char)param_1);
  uRam000f0014 = 0x21;
  sRam000f0002 = DAT_00005cc0;
  uRam000f0008 = SUB42(param_3,0);
  uRam000f000c = SUB42(apuStack_8 + 1,0);
  uRam000f000e = 0x1c00;
  sRam000f000a = sRam000f0006;
  uRam000f0010 = unaff_ES;
  apuStack_8[1] = param_2;
  (*(code *)0xf0100)();
  if (!(bool)uRam000f0012) {
    return _param_5;
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_5);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    sRam000f0004 = param_8;
    sRam000f0006 = param_7;
    sRam000f000a = param_9;
    uRam000f000c = param_10;
    uRam000f000e = uVar6;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  puVar4 = (undefined8 *)thunk_FUN_00000f64(1);
  return puVar4;
}


/* ===== FUN_00006008 @ 00006008 ===== */

/* WARNING: Removing unreachable block (ram,0x0000601d) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

undefined8 __allregs
FUN_00006008(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined1 param_5,undefined3 param_6,undefined2 param_7,undefined2 param_8,
            undefined2 param_9,undefined2 param_10,undefined2 param_11)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined2 unaff_ES;
  undefined2 uVar3;
  
  uVar3 = 0x1c00;
  uRam000f0000 = CONCAT11(0x3f,param_1);
  uRam000f0014 = 0x21;
  sRam000f0002 = DAT_00005cc0;
  uRam000f0004 = SUB42(PTR_LAB_00005560,0);
  uRam000f0006 = (undefined2)DAT_00005564;
  uRam000f000e = 0x1c00;
  uRam000f0008 = param_3;
  uRam000f000a = param_4;
  uRam000f000c = param_2;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  if (!(bool)uRam000f0012) {
    return CONCAT26(param_8,CONCAT24(param_7,_param_5));
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_5);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0004 = param_9;
    uRam000f0006 = param_7;
    uRam000f000a = param_10;
    uRam000f000c = param_11;
    uRam000f000e = uVar3;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  uVar2 = 1;
  uVar1 = thunk_FUN_00000f64(1);
  return CONCAT44(uVar2,uVar1);
}


/* ===== FUN_00006032 @ 00006032 ===== */

/* WARNING: Removing unreachable block (ram,0x0000609b) */
/* WARNING: Removing unreachable block (ram,0x00006085) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

uint __allregs
FUN_00006032(int param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,uint param_5,
            undefined2 param_6)

{
  uint uVar1;
  undefined2 extraout_var;
  undefined2 uVar2;
  undefined2 unaff_ES;
  undefined2 uVar3;
  undefined1 in_CF;
  bool bVar4;
  
  uVar3 = 0x1c00;
  uVar2 = 0x33;
  DAT_0000556c = param_1;
  uVar1 = FUN_00005cc2(0x4e00,0x33,param_3,param_4,param_5,param_6);
  if (!(bool)in_CF) {
    DAT_00000937 = *(uint *)(DAT_00000927 + 0x1a);
    bVar4 = false;
    if (DAT_0000556c == 0) {
      return DAT_00000937;
    }
    uVar1 = FUN_00005cc2(0x3d00,uVar2,(short)DAT_00000927,param_4,param_5,param_6);
    if (!bVar4) {
      uRam000f0000 = CONCAT11(0x3f,(char)uVar1);
      uRam000f0014 = 0x21;
      sRam000f0002 = DAT_00005cc0;
      uRam000f0004 = (undefined2)DAT_00000937;
      uRam000f0006 = (undefined2)DAT_0000556c;
      uRam000f0008 = (ushort)param_5;
      uRam000f000e = 0x1c00;
      uRam000f000a = param_6;
      uRam000f000c = param_4;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      uVar1 = CONCAT22(extraout_var,uRam000f0000);
      param_5 = (uint)uRam000f0008;
      param_6 = uRam000f000a;
      uVar2 = uRam000f0004;
      param_2 = uRam000f0006;
      if (!(bool)uRam000f0012) {
        uRam000f0000 = CONCAT11(0x3e,(char)uRam000f0000);
        uRam000f0014 = 0x21;
        sRam000f0002 = DAT_00005cc0;
        DAT_00000937 = uVar1;
        uRam000f000c = param_4;
        uRam000f000e = uVar3;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        uVar1 = (uint)uRam000f0000;
        param_5 = (uint)uRam000f0008;
        param_6 = uRam000f000a;
        uVar2 = uRam000f0004;
        param_2 = uRam000f0006;
        if (!(bool)uRam000f0012) {
          return DAT_00000937;
        }
      }
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)uVar1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (ushort)param_5;
    uRam000f0004 = uVar2;
    uRam000f0006 = param_2;
    uRam000f000a = param_6;
    uRam000f000c = param_4;
    uRam000f000e = uVar3;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  uVar1 = thunk_FUN_00000f64(1);
  return uVar1;
}


/* ===== FUN_000060a9 @ 000060a9 ===== */

/* WARNING: Removing unreachable block (ram,0x000060e9) */
/* WARNING: Removing unreachable block (ram,0x000060d8) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

undefined4 __allregs
FUN_000060a9(undefined4 param_1,undefined4 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined4 param_6,undefined2 param_7)

{
  undefined1 uVar1;
  undefined4 uVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 unaff_ES;
  undefined2 uVar5;
  bool bVar6;
  
  uVar5 = 0x1c00;
  bVar6 = false;
  uVar3 = 0;
  DAT_00000937 = param_2;
  DAT_0000556c = param_1;
  uVar1 = FUN_00005cc2(CONCAT11(0x3c,(char)param_1),0,param_4,param_5,param_6,param_7);
  uVar4 = (short)param_6;
  if (!bVar6) {
    uRam000f0000 = CONCAT11(0x40,uVar1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0004 = (undefined2)DAT_00000937;
    uRam000f0006 = (undefined2)DAT_0000556c;
    uRam000f000e = 0x1c00;
    uRam000f0008 = (short)param_6;
    uRam000f000a = param_7;
    uRam000f000c = param_5;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar1 = (undefined1)uRam000f0000;
    param_7 = uRam000f000a;
    uVar3 = uRam000f0004;
    param_3 = uRam000f0006;
    uVar4 = uRam000f0008;
    if (!(bool)uRam000f0012) {
      uRam000f0000 = CONCAT11(0x3e,uVar1);
      uRam000f0014 = 0x21;
      sRam000f0002 = DAT_00005cc0;
      uRam000f000c = param_5;
      uRam000f000e = uVar5;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      uVar1 = (undefined1)uRam000f0000;
      param_7 = uRam000f000a;
      uVar3 = uRam000f0004;
      param_3 = uRam000f0006;
      uVar4 = uRam000f0008;
      if (!(bool)uRam000f0012) {
        return DAT_00000937;
      }
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,uVar1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0004 = uVar3;
    uRam000f0006 = param_3;
    uRam000f0008 = uVar4;
    uRam000f000a = param_7;
    uRam000f000c = param_5;
    uRam000f000e = uVar5;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  uVar2 = thunk_FUN_00000f64(1);
  return uVar2;
}


/* ===== FUN_00006250 @ 00006250 ===== */

/* WARNING: Removing unreachable block (ram,0x000063b4) */
/* WARNING: Removing unreachable block (ram,0x00006406) */
/* WARNING: Removing unreachable block (ram,0x00006380) */
/* WARNING: Removing unreachable block (ram,0x00006323) */
/* WARNING: Removing unreachable block (ram,0x000062d4) */
/* WARNING: Removing unreachable block (ram,0x0000629b) */
/* WARNING: Removing unreachable block (ram,0x000063da) */

ushort __allregs FUN_00006250(undefined2 param_1,byte *param_2)

{
  byte *pbVar1;
  uint uVar2;
  undefined2 uVar3;
  byte bVar4;
  ushort uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint *puVar9;
  undefined2 uVar11;
  uint *puVar10;
  byte *pbVar12;
  byte *pbVar13;
  undefined2 unaff_ES;
  undefined2 uVar14;
  undefined2 extraout_var;
  
  uVar14 = 0x1c00;
  iVar7 = 0xc;
  pbVar12 = (byte *)&DAT_00006244;
  do {
    pbVar1 = param_2 + 1;
    bVar4 = *param_2 + (0x5f < *param_2) * -0x20;
    pbVar13 = pbVar12;
    if (bVar4 < 0x21) break;
    pbVar13 = pbVar12 + 1;
    *pbVar12 = bVar4;
    iVar7 = iVar7 + -1;
    param_2 = pbVar1;
    pbVar12 = pbVar13;
  } while (iVar7 != 0);
  for (; iVar7 != 0; iVar7 = iVar7 + -1) {
    *pbVar13 = 0;
    pbVar13 = pbVar13 + 1;
  }
  DAT_00006244 = DAT_00006244 ^ 0xaceddead;
  DAT_00006248 = DAT_00006248 ^ 0xaceddead;
  DAT_0000624c = DAT_0000624c ^ 0xaceddead;
  DAT_00005cc0 = 0;
  uRam000f0014 = 0x21;
  uRam000f0000 = 0x3d00;
  uRam000f0002 = 0x6244;
  uRam000f0004 = 0;
  uRam000f0006 = 0x5998;
  uRam000f0008 = (ushort)pbVar1;
  uRam000f000a = 0x6250;
  uRam000f000e = 0x1c00;
  uRam000f000c = param_1;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar6 = CONCAT22(extraout_var,uRam000f0000);
  uVar8 = (uint)uRam000f0004;
  puVar9 = (uint *)CONCAT22((short)((uint)pbVar1 >> 0x10),uRam000f0008);
  uVar11 = uRam000f000a;
  if ((bool)uRam000f0012) {
LAB_000063cc:
    if (DAT_00005cc0 != 0) {
      uRam000f0000 = CONCAT11(0x3e,(char)uVar6);
      uRam000f0014 = 0x21;
      uRam000f0002 = DAT_00005cc0;
      uRam000f0004 = (ushort)uVar8;
      uRam000f0008 = (ushort)puVar9;
      uRam000f000a = uVar11;
      uRam000f000c = param_1;
      uRam000f000e = uVar14;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      puVar9 = (uint *)CONCAT22((short)((uint)puVar9 >> 0x10),uRam000f0008);
      DAT_00005cc0 = 0;
    }
    uVar5 = 0;
    if (DAT_0000623c != (uint *)0x0) {
      uVar5 = FUN_00003376(&DAT_0000623c,puVar9,0x63f6);
    }
    return uVar5;
  }
  DAT_00005cc0 = uRam000f0000;
  iVar7 = FUN_000036bf(CONCAT31((int3)(uVar6 >> 8),1),0x10,&DAT_0000623c,4,0x62c0);
  uRam000f0000 = CONCAT11(0x3f,(char)iVar7);
  uRam000f0014 = 0x21;
  uRam000f0002 = DAT_00005cc0;
  uRam000f0004 = 0x10;
  uRam000f0006 = SUB42(DAT_0000623c,0);
  uRam000f0008 = (ushort)puVar9;
  uRam000f000c = param_1;
  uRam000f000e = uVar14;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar6 = (uint)uRam000f0000;
  puVar9 = (uint *)(uint)uRam000f0008;
  if (!(bool)uRam000f0012) {
    uVar6 = *DAT_0000623c;
    puVar9 = DAT_0000623c + 1;
    if (uVar6 == 0x4f534552) {
      puVar9 = DAT_0000623c + 2;
      uVar6 = DAT_0000623c[1];
      if (uVar6 == 0x45435255) {
        puVar9 = DAT_0000623c + 4;
        DAT_00006240 = DAT_0000623c[3];
        uVar5 = 0;
        iVar7 = FUN_0000345c((DAT_00006240 + 1) * 0x10,&DAT_0000623c,puVar9,0x6313);
        uRam000f0004 = uVar5;
        uRam000f0000 = CONCAT11(0x3f,(char)iVar7);
        uRam000f0014 = 0x21;
        uRam000f0002 = DAT_00005cc0;
        uRam000f0006 = SUB42(DAT_0000623c,0);
        uRam000f0008 = (ushort)puVar9;
        uRam000f000c = param_1;
        uRam000f000e = uVar14;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        uVar3 = uRam000f0006;
        uVar6 = (uint)uRam000f0000;
        puVar9 = (uint *)(uint)uRam000f0008;
        if (!(bool)uRam000f0012) {
          uVar11 = 0x6244;
          uVar8 = DAT_00006240;
          puVar10 = DAT_0000623c + -4;
          do {
            puVar9 = puVar10;
            uVar8 = uVar8 - 1;
            if ((int)uVar8 < 0) goto LAB_000063cc;
            puVar10 = puVar9 + 4;
            uVar6 = *puVar10;
          } while (((DAT_00006244 != uVar6) || (uVar6 = puVar9[5], DAT_00006248 != uVar6)) ||
                  (uVar6 = puVar9[6], DAT_0000624c != uVar6));
          uVar11 = (undefined2)((uint)puVar10 >> 0x10);
          if ((char)((ushort)DAT_00005cbc >> 8) == 'N') {
            uVar6 = puVar9[0xb];
            uVar2 = puVar9[7];
            *(uint *)(DAT_00000927 + 0x1a) = uVar6 - uVar2;
            if (DAT_00005cc0 != 0) {
              uRam000f0000 = CONCAT11(0x3e,(char)(uVar6 - uVar2));
              uRam000f0014 = 0x21;
              uRam000f0002 = DAT_00005cc0;
              uRam000f0004 = (ushort)uVar8;
              uRam000f000a = 0x6244;
              uRam000f0006 = uVar3;
              uRam000f0008 = (ushort)puVar10;
              uRam000f000c = param_1;
              uRam000f000e = uVar14;
              uRam000f0010 = unaff_ES;
              (*(code *)0xf0100)();
              puVar10 = (uint *)CONCAT22(uVar11,uRam000f0008);
              DAT_00005cc0 = 0;
            }
            uVar5 = FUN_00003376(&DAT_0000623c,puVar10,0x63ca);
            return uVar5;
          }
          uRam000f0006 = (undefined2)puVar9[7];
          uRam000f0014 = 0x21;
          uRam000f0000 = 0x4200;
          uRam000f0002 = DAT_00005cc0;
          uRam000f0004 = (ushort)(puVar9[7] >> 0x10);
          uRam000f000a = 0x6244;
          uRam000f0008 = (ushort)puVar10;
          uRam000f000c = param_1;
          uRam000f000e = uVar14;
          uRam000f0010 = unaff_ES;
          (*(code *)0xf0100)();
          uVar6 = (uint)uRam000f0000;
          puVar9 = (uint *)CONCAT22(uVar11,uRam000f0008);
          if (!(bool)uRam000f0012) {
            FUN_00003376(&DAT_0000623c,puVar9,0x638f);
            return DAT_00005cc0;
          }
        }
      }
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)uVar6);
    uRam000f0014 = 0x21;
    uRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (ushort)puVar9;
    uRam000f000c = param_1;
    uRam000f000e = uVar14;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  uVar5 = thunk_FUN_00000f64(FUN_0000641b);
  return uVar5;
}


/* ===== FUN_000062fd @ 000062fd ===== */

/* WARNING: Removing unreachable block (ram,0x000063b4) */
/* WARNING: Removing unreachable block (ram,0x00006406) */
/* WARNING: Removing unreachable block (ram,0x00006380) */
/* WARNING: Removing unreachable block (ram,0x00006323) */
/* WARNING: Removing unreachable block (ram,0x000063da) */

short __allregs
FUN_000062fd(char *param_1,int param_2,undefined4 *param_3,undefined4 param_4,undefined2 param_5)

{
  undefined4 **ppuVar1;
  undefined4 *puVar2;
  char cVar3;
  undefined2 uVar4;
  undefined1 uVar5;
  short sVar6;
  int iVar7;
  BADSPACEBASE *in_ESP;
  uint uVar9;
  undefined2 uVar12;
  uint *puVar10;
  uint *puVar11;
  undefined2 unaff_ES;
  undefined2 uVar13;
  undefined4 *apuStack_8 [2];
  uint uVar8;
  
  uVar13 = 0x1c00;
  ppuVar1 = apuStack_8 + 1;
  sVar6 = (short)apuStack_8 + 4;
  cVar3 = '\x01';
  puVar2 = param_3;
  do {
    puVar2 = puVar2 + -1;
    ppuVar1 = ppuVar1 + -1;
    *ppuVar1 = (undefined4 *)*puVar2;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  *param_1 = *param_1 + (char)param_1;
  uVar4 = 0;
  apuStack_8[1] = param_3;
  iVar7 = FUN_0000345c((param_2 + 1) * 0x10,&DAT_0000623c,param_4,0x6313);
  uRam000f0004 = uVar4;
  uRam000f0000 = CONCAT11(0x3f,(char)iVar7);
  uRam000f0014 = 0x21;
  sRam000f0002 = DAT_00005cc0;
  uRam000f0006 = (undefined2)DAT_0000623c;
  uRam000f0008 = (ushort)param_4;
  uRam000f000e = 0x1c00;
  uRam000f000a = param_5;
  sRam000f000c = sVar6;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar4 = uRam000f0006;
  uVar8 = (uint)uRam000f0000;
  uVar5 = (undefined1)uRam000f0000;
  uVar9 = (uint)uRam000f0008;
  if (!(bool)uRam000f0012) {
    iVar7 = DAT_00006240;
    puVar10 = (uint *)(DAT_0000623c + -0x10);
    do {
      puVar11 = puVar10;
      iVar7 = iVar7 + -1;
      if (iVar7 < 0) {
        if (DAT_00005cc0 != 0) {
          uRam000f0000 = CONCAT11(0x3e,(char)uVar8);
          uRam000f0014 = 0x21;
          sRam000f0002 = DAT_00005cc0;
          uRam000f0008 = (ushort)puVar11;
          uRam000f000a = 0x6244;
          uRam000f0004 = (short)iVar7;
          sRam000f000c = sVar6;
          uRam000f000e = uVar13;
          uRam000f0010 = unaff_ES;
          (*(code *)0xf0100)();
          puVar11 = (uint *)CONCAT22((short)((uint)puVar11 >> 0x10),uRam000f0008);
          DAT_00005cc0 = 0;
        }
        sVar6 = 0;
        if (DAT_0000623c != 0) {
          sVar6 = FUN_00003376(&DAT_0000623c,puVar11,0x63f6);
        }
        return sVar6;
      }
      puVar10 = puVar11 + 4;
      uVar8 = *puVar10;
    } while (((DAT_00006244 != uVar8) || (uVar8 = puVar11[5], DAT_00006248 != uVar8)) ||
            (uVar8 = puVar11[6], DAT_0000624c != uVar8));
    uVar12 = (undefined2)((uint)puVar10 >> 0x10);
    if ((char)((ushort)DAT_00005cbc >> 8) == 'N') {
      uVar8 = puVar11[0xb];
      uVar9 = puVar11[7];
      *(uint *)(DAT_00000927 + 0x1a) = uVar8 - uVar9;
      if (DAT_00005cc0 != 0) {
        uRam000f0000 = CONCAT11(0x3e,(char)(uVar8 - uVar9));
        uRam000f0014 = 0x21;
        sRam000f0002 = DAT_00005cc0;
        uRam000f000a = 0x6244;
        uRam000f0004 = (short)iVar7;
        uRam000f0006 = uVar4;
        uRam000f0008 = (ushort)puVar10;
        sRam000f000c = sVar6;
        uRam000f000e = uVar13;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        puVar10 = (uint *)CONCAT22(uVar12,uRam000f0008);
        DAT_00005cc0 = 0;
      }
      sVar6 = FUN_00003376(&DAT_0000623c,puVar10,0x63ca);
      return sVar6;
    }
    uRam000f0006 = (undefined2)puVar11[7];
    uRam000f0014 = 0x21;
    uRam000f0000 = 0x4200;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0004 = (undefined2)(puVar11[7] >> 0x10);
    uRam000f000a = 0x6244;
    uRam000f0008 = (ushort)puVar10;
    sRam000f000c = sVar6;
    uRam000f000e = uVar13;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar5 = (undefined1)uRam000f0000;
    uVar9 = CONCAT22(uVar12,uRam000f0008);
    if (!(bool)uRam000f0012) {
      FUN_00003376(&DAT_0000623c,uVar9,0x638f);
      return DAT_00005cc0;
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,uVar5);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (ushort)uVar9;
    sRam000f000c = sVar6;
    uRam000f000e = uVar13;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  sVar6 = thunk_FUN_00000f64(FUN_0000641b);
  return sVar6;
}


/* ===== FUN_0000641b @ 0000641b ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x00005e48) */
/* WARNING: Removing unreachable block (ram,0x0000685c) */
/* WARNING: Removing unreachable block (ram,0x00006587) */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * __allregs
FUN_0000641b(byte *param_1,int param_2,undefined2 param_3,int param_4,uint *param_5,int param_6,
            int param_7)

{
  byte bVar1;
  undefined1 uVar2;
  longlong lVar3;
  ushort uVar4;
  byte bVar10;
  byte *pbVar5;
  uint uVar6;
  undefined4 uVar7;
  undefined8 *puVar8;
  undefined3 uVar11;
  uint uVar9;
  uint uVar12;
  int iVar13;
  uint uVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  int *piVar17;
  uint *puVar18;
  uint *puVar19;
  uint *puVar20;
  uint *puVar21;
  undefined2 unaff_ES;
  undefined2 uVar22;
  int in_GS_OFFSET;
  bool bVar23;
  
  uVar22 = 0x1c00;
  bVar10 = (byte)((uint)param_1 >> 8);
  uVar12 = param_2 - 2;
  puVar18 = (uint *)(param_6 + 2);
  out(*(undefined1 *)(param_6 + 1),param_3);
  if (bVar10 < *param_1 || uVar12 == 0) {
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  puVar19 = (uint *)(param_7 + 2);
  uVar2 = in(param_3);
  *(undefined1 *)(param_7 + 1) = uVar2;
  puVar15 = (undefined1 *)((longlong)*(int *)(param_1 + 0x72) * 0x756f7365);
  if ((longlong)(int)puVar15 == (longlong)*(int *)(param_1 + 0x72) * 0x756f7365) {
    pbVar5 = (byte *)(in_GS_OFFSET + param_6 + 0x6b);
    *pbVar5 = *pbVar5 & bVar10;
    puVar21 = (uint *)(param_7 + 3);
    bVar10 = in(param_3);
    *(byte *)puVar19 = bVar10;
    pbVar5 = (byte *)((uint)param_1 | 0x6ca3240a);
    *(uint **)(puVar15 + -4) = param_5;
    bVar23 = CARRY1(*pbVar5,(byte)pbVar5);
    *pbVar5 = *pbVar5 + (byte)pbVar5;
    *(undefined4 *)(puVar15 + -8) = 0x644a;
    uVar6 = FUN_00005cc2(0x3d00,(short)uVar12,(short)param_4,(short)param_5,puVar18,(short)puVar21);
    uVar4 = (ushort)uVar12;
    puVar19 = puVar21;
    if (!bVar23) {
      *(undefined4 *)(puVar15 + -10) = 0x6455;
      uVar7 = FUN_00005f17(uVar6);
      puVar16 = puVar15 + -8;
      DAT_00005568 = (uint *)((int)DAT_00005568 + -0x12);
      piVar17 = (int *)(PTR_LAB_00005560 + DAT_00005564);
      *(undefined4 *)(puVar15 + -0xc) = 0x646d;
      FUN_00005fbe(uVar7,uVar4,(char)param_3,(short)param_5,piVar17,(short)puVar21,
                   *(undefined2 *)(puVar15 + -0xc));
      if (*piVar17 != 0x31434c4b) {
        puVar8 = (undefined8 *)
                 FUN_00005e3a((char)*piVar17,uVar4,param_3,(short)param_5,(short)(piVar17 + 1),
                              (short)puVar21);
        return puVar8;
      }
      DAT_00005578 = piVar17[1];
      puVar18 = (uint *)(piVar17 + 3);
      DAT_0000557c = piVar17[2];
      lVar3 = (ulonglong)DAT_0000557c * (ulonglong)DAT_00005578;
      DAT_00000937 = (undefined8 *)lVar3;
      if (DAT_0000556c != (uint *)0x0) {
        iVar13 = 0xc0;
        puVar19 = puVar18;
        puVar21 = &DAT_00005598;
        do {
          uVar7 = (undefined4)((ulonglong)lVar3 >> 0x20);
          if (DAT_00005568 <= puVar19) {
            *(undefined4 *)(puVar15 + -0xc) = 0x65d5;
            puVar8 = FUN_00005fbe((int)lVar3,(short)iVar13,(char)((ulonglong)lVar3 >> 0x20),
                                  (short)param_5,puVar19,(short)puVar21,
                                  *(undefined2 *)(puVar15 + -0xc));
            lVar3 = CONCAT44(uVar7,puVar8);
          }
          uVar12 = (uint)lVar3;
          puVar18 = puVar19 + 1;
          *puVar21 = *puVar19;
          iVar13 = iVar13 + -1;
          puVar19 = puVar18;
          puVar21 = puVar21 + 1;
        } while (iVar13 != 0);
        param_5 = (uint *)(DAT_00005578 * 3);
        uVar14 = DAT_0000557c >> 2;
        uVar9 = CONCAT22((short)((uint)piVar17 >> 0x10),(ushort)(byte)piVar17);
        uVar6 = DAT_00005578;
        puVar21 = DAT_0000556c;
        do {
          *(uint *)(puVar15 + -0xc) = uVar14;
          uVar14 = uVar6 >> 2;
          do {
            if (DAT_00005568 <= puVar18) {
              *(undefined4 *)(puVar15 + -0x10) = 0x6606;
              FUN_00005fbe(uVar12,(short)uVar14,(char)uVar6,(short)param_5,puVar18,(short)puVar21,
                           *(undefined2 *)(puVar15 + -0x10));
            }
            uVar12 = uVar9 & 0xffff03ff;
            if ((uVar9 & 0x300) == 0) {
              uVar12 = CONCAT31((int3)(uVar12 >> 8),(byte)*puVar18);
              puVar18 = (uint *)((int)puVar18 + 1);
            }
            *(uint *)(puVar15 + -0x10) =
                 CONCAT22((short)(uVar12 >> 0x10),
                          CONCAT11((char)(uVar12 >> 8) + '\x01',(char)uVar12));
            if ((uVar12 & 2) == 0) {
              if ((uVar12 & 1) == 0) {
                uVar12 = *(uint *)((int)puVar18 + 3);
                uVar9 = uVar12 & 3;
                if ((uVar12 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)puVar21 = (byte)uVar9;
                uVar9 = uVar12 >> 2 & 3;
                if ((uVar12 >> 2 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + 1) = (byte)uVar9;
                uVar9 = uVar12 >> 4 & 3;
                if ((uVar12 >> 4 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + 2) = (byte)uVar9;
                uVar9 = uVar12 >> 6 & 3;
                if ((uVar12 >> 6 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + 3) = (byte)uVar9;
                uVar9 = uVar12 >> 8 & 3;
                if ((uVar12 & 0x300) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)(uVar6 + (int)puVar21) = (byte)uVar9;
                uVar9 = uVar12 >> 10 & 3;
                if ((uVar12 >> 10 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)(uVar6 + 1 + (int)puVar21) = (byte)uVar9;
                uVar9 = uVar12 >> 0xc & 3;
                if ((uVar12 >> 0xc & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)(uVar6 + 2 + (int)puVar21) = (byte)uVar9;
                uVar9 = uVar12 >> 0xe & 3;
                if ((uVar12 >> 0xe & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)(uVar6 + 3 + (int)puVar21) = (byte)uVar9;
                uVar9 = uVar12 >> 0x10 & 3;
                if ((uVar12 & 0x30000) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + uVar6 * 2) = (byte)uVar9;
                uVar9 = uVar12 >> 0x12 & 3;
                if ((uVar12 >> 0x12 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + uVar6 * 2 + 1) = (byte)uVar9;
                uVar9 = uVar12 >> 0x14 & 3;
                if ((uVar12 >> 0x14 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + uVar6 * 2 + 2) = (byte)uVar9;
                uVar9 = uVar12 >> 0x16 & 3;
                if ((uVar12 >> 0x16 & 3) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)puVar21 + uVar6 * 2 + 3) = (byte)uVar9;
                bVar10 = (byte)(uVar12 >> 0x18);
                uVar9 = bVar10 & 0xffffff03;
                if ((uVar12 & 0x3000000) != 0) {
                  uVar9 = (uint)*(byte *)((int)puVar18 + (uVar9 - 1));
                }
                *(byte *)((int)param_5 + (int)puVar21) = (byte)uVar9;
                uVar12 = bVar10 >> 2 & 0xffffff03;
                if ((bVar10 >> 2 & 3) != 0) {
                  uVar12 = (uint)*(byte *)((int)puVar18 + (uVar12 - 1));
                }
                ((byte *)((int)param_5 + 1))[(int)puVar21] = (byte)uVar12;
                uVar12 = bVar10 >> 4 & 0xffffff03;
                if ((bVar10 >> 4 & 3) != 0) {
                  uVar12 = (uint)*(byte *)((int)puVar18 + (uVar12 - 1));
                }
                ((byte *)((int)param_5 + 2))[(int)puVar21] = (byte)uVar12;
                uVar12 = (uint)(bVar10 >> 6);
                if (bVar10 >> 6 != 0) {
                  uVar12 = (uint)*(byte *)((int)puVar18 + (uVar12 - 1));
                }
                ((byte *)((int)param_5 + 3))[(int)puVar21] = (byte)uVar12;
                puVar18 = (uint *)((int)puVar18 + 7);
              }
              else {
                uVar4 = *(ushort *)((int)puVar18 + 2);
                *(byte *)puVar21 = *(byte *)((uint)((uVar4 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + 1) =
                     *(byte *)((uint)((uVar4 >> 1 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + 2) =
                     *(byte *)((uint)((uVar4 >> 2 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + 3) =
                     *(byte *)((uint)((uVar4 >> 3 & 1) != 0) + (int)puVar18);
                *(byte *)(uVar6 + (int)puVar21) =
                     *(byte *)((uint)((uVar4 >> 4 & 1) != 0) + (int)puVar18);
                *(byte *)(uVar6 + 1 + (int)puVar21) =
                     *(byte *)((uint)((uVar4 >> 5 & 1) != 0) + (int)puVar18);
                *(byte *)(uVar6 + 2 + (int)puVar21) =
                     *(byte *)((uint)((uVar4 >> 6 & 1) != 0) + (int)puVar18);
                *(byte *)(uVar6 + 3 + (int)puVar21) =
                     *(byte *)((uint)((uVar4 >> 7 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + uVar6 * 2) =
                     *(byte *)((uint)((uVar4 >> 8 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + uVar6 * 2 + 1) =
                     *(byte *)((uint)((uVar4 >> 9 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + uVar6 * 2 + 2) =
                     *(byte *)((uint)((uVar4 >> 10 & 1) != 0) + (int)puVar18);
                *(byte *)((int)puVar21 + uVar6 * 2 + 3) =
                     *(byte *)((uint)((uVar4 >> 0xb & 1) != 0) + (int)puVar18);
                *(byte *)((int)param_5 + (int)puVar21) =
                     *(byte *)((uint)((uVar4 >> 0xc & 1) != 0) + (int)puVar18);
                ((byte *)((int)param_5 + 1))[(int)puVar21] =
                     *(byte *)((uint)((uVar4 >> 0xd & 1) != 0) + (int)puVar18);
                ((byte *)((int)param_5 + 2))[(int)puVar21] =
                     *(byte *)((uint)((uVar4 >> 0xe & 1) != 0) + (int)puVar18);
                bVar10 = *(byte *)((uint)((short)uVar4 < 0) + (int)puVar18);
                uVar12 = (uint)bVar10;
                ((byte *)((int)param_5 + 3))[(int)puVar21] = bVar10;
                puVar18 = puVar18 + 1;
              }
            }
            else if ((uVar12 & 1) == 0) {
              bVar10 = (byte)*puVar18;
              if (bVar10 == 0) {
                *puVar21 = *(uint *)((int)puVar18 + 1);
                *(undefined4 *)(uVar6 + (int)puVar21) = *(undefined4 *)((int)puVar18 + 5);
                puVar19 = (uint *)((int)puVar18 + 0xd);
                *(undefined4 *)((int)puVar21 + uVar6 * 2) = *(undefined4 *)((int)puVar18 + 9);
                puVar18 = (uint *)((int)puVar18 + 0x11);
                uVar12 = *puVar19;
                *(uint *)((int)param_5 + (int)puVar21) = uVar12;
              }
              else {
                uVar12 = CONCAT22(CONCAT11(bVar10,bVar10),CONCAT11(bVar10,bVar10));
                *puVar21 = uVar12;
                *(uint *)(uVar6 + (int)puVar21) = uVar12;
                *(uint *)((int)puVar21 + uVar6 * 2) = uVar12;
                *(uint *)((int)param_5 + (int)puVar21) = uVar12;
                puVar18 = (uint *)((int)puVar18 + 1);
              }
            }
            else {
              uVar12 = 0;
              *puVar21 = 0;
              pbVar5 = (byte *)(uVar6 + (int)puVar21);
              pbVar5[0] = 0;
              pbVar5[1] = 0;
              pbVar5[2] = 0;
              pbVar5[3] = 0;
              pbVar5 = (byte *)((int)puVar21 + uVar6 * 2);
              pbVar5[0] = 0;
              pbVar5[1] = 0;
              pbVar5[2] = 0;
              pbVar5[3] = 0;
              pbVar5 = (byte *)((int)param_5 + (int)puVar21);
              pbVar5[0] = 0;
              pbVar5[1] = 0;
              pbVar5[2] = 0;
              pbVar5[3] = 0;
            }
            lVar3 = CONCAT44(uVar6,uVar12);
            puVar21 = puVar21 + 1;
            uVar9 = CONCAT31((int3)((uint)*(undefined4 *)(puVar15 + -0x10) >> 8),
                             (byte)*(undefined4 *)(puVar15 + -0x10) >> 2);
            uVar14 = uVar14 - 1;
          } while (uVar14 != 0);
          puVar21 = (uint *)((int)puVar21 + (int)param_5);
          uVar14 = *(int *)(puVar15 + -0xc) - 1;
          uVar4 = 0;
        } while (uVar14 != 0);
      }
      uRam000f0004 = uVar4;
      uRam000f0000 = CONCAT11(0x3e,(char)lVar3);
      uRam000f0014 = 0x21;
      sRam000f0002 = DAT_00005cc0;
      uRam000f0006 = (undefined2)((ulonglong)lVar3 >> 0x20);
      uRam000f0008 = SUB42(puVar18,0);
      uRam000f000a = (ushort)puVar21;
      uRam000f000c = SUB42(param_5,0);
      uRam000f000e = 0x1c00;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      uVar6 = (uint)uRam000f0000;
      uVar12 = (uint)uRam000f0004;
      puVar18 = (uint *)CONCAT22((short)((uint)puVar18 >> 0x10),uRam000f0008);
      puVar19 = (uint *)(uint)uRam000f000a;
      param_3 = uRam000f0006;
      if (!(bool)uRam000f0012) {
        *(undefined4 *)(puVar16 + -4) = 0x6869;
        FUN_00005f6c(puVar18);
        return DAT_00000937;
      }
    }
  }
  else {
    DAT_00005578 = (uint)(ushort)((*(short *)puVar18 + 1) - _DAT_00005594);
    puVar18 = (uint *)(param_6 + 6);
    _DAT_00005596 = (short)param_1;
    uVar4 = (*(short *)(param_6 + 4) + 1) - _DAT_00005596;
    DAT_0000557c = (uint)uVar4;
    uVar6 = *(ushort *)(param_4 + 0x42) - DAT_00005578;
    _DAT_00005596 = param_1;
    param_3 = param_3;
    if (DAT_00005578 <= *(ushort *)(param_4 + 0x42)) {
      lVar3 = (ulonglong)DAT_00005578 * (ulonglong)DAT_0000557c;
      uVar7 = 0;
      puVar8 = (undefined8 *)lVar3;
      DAT_00000937 = puVar8;
      _DAT_00005590 = uVar6;
      if (DAT_0000556c != (uint *)0x0) {
        puVar21 = (uint *)(param_4 + 0x80);
        uVar12 = DAT_0000557c;
        puVar19 = DAT_0000556c;
        do {
          param_5 = (uint *)(DAT_00005578 + _DAT_00005590 + (int)puVar19);
          *(uint *)(puVar15 + -4) = uVar12;
          puVar18 = puVar21;
          do {
            while( true ) {
              if (DAT_00005568 <= puVar18) {
                *(undefined4 *)(puVar15 + -8) = 0x651f;
                puVar8 = FUN_00005fbe(puVar8,0,0,(short)param_5,puVar18,(short)puVar19,
                                      *(undefined2 *)(puVar15 + -8));
                return puVar8;
              }
              puVar21 = (uint *)((int)puVar18 + 1);
              bVar10 = (byte)*puVar18;
              uVar11 = (undefined3)((uint)puVar8 >> 8);
              puVar8 = (undefined8 *)CONCAT31(uVar11,bVar10);
              if (0xbf < bVar10) break;
              puVar20 = (uint *)((int)puVar19 + 1);
              *(byte *)puVar19 = bVar10;
              puVar18 = puVar21;
              puVar19 = puVar20;
              if (param_5 <= puVar20) goto LAB_0000654a;
            }
            puVar18 = (uint *)((int)puVar18 + 2);
            bVar1 = *(byte *)puVar21;
            puVar8 = (undefined8 *)CONCAT31(uVar11,bVar1);
            for (uVar12 = bVar10 & 0x3f; uVar12 != 0; uVar12 = uVar12 - 1) {
              *(byte *)puVar19 = bVar1;
              puVar19 = (uint *)((int)puVar19 + 1);
            }
            puVar21 = puVar18;
            puVar20 = puVar19;
          } while (puVar19 < param_5);
LAB_0000654a:
          puVar19 = (uint *)((int)puVar20 - _DAT_00005590);
          uVar12 = *(int *)(puVar15 + -4) - 1;
        } while (uVar12 != 0);
        if (DAT_00005568 <= puVar21) {
          *(undefined4 *)(puVar15 + -4) = 0x6560;
          puVar8 = FUN_00005fbe(puVar8,0,0,(short)param_5,puVar21,(short)puVar19,
                                *(undefined2 *)(puVar15 + -4));
        }
        puVar18 = (uint *)((int)puVar21 + 1);
        uVar6 = CONCAT31((int3)((uint)puVar8 >> 8),(byte)*puVar21);
        lVar3 = CONCAT44(uVar7,uVar6);
        param_3 = (short)uVar7;
        if ((byte)*puVar21 != 0xc) goto code_r0x00005e3a;
        iVar13 = 0xc0;
        puVar21 = puVar18;
        puVar20 = &DAT_00005598;
        do {
          if (DAT_00005568 <= puVar21) {
            *(undefined4 *)(puVar15 + -4) = 0x657b;
            puVar8 = FUN_00005fbe(uVar6,(short)iVar13,(char)uVar7,(short)param_5,puVar21,
                                  (short)puVar20,*(undefined2 *)(puVar15 + -4));
            return puVar8;
          }
          puVar19 = puVar20 + 1;
          puVar18 = puVar21 + 1;
          *puVar20 = *puVar21;
          iVar13 = iVar13 + -1;
          puVar21 = puVar18;
          puVar20 = puVar19;
          uVar4 = 0;
        } while (iVar13 != 0);
      }
      uRam000f0004 = uVar4;
      uRam000f0000 = CONCAT11(0x3e,(char)lVar3);
      uRam000f0014 = 0x21;
      sRam000f0002 = DAT_00005cc0;
      uRam000f0006 = (undefined2)((ulonglong)lVar3 >> 0x20);
      uRam000f0008 = SUB42(puVar18,0);
      uRam000f000a = (ushort)puVar19;
      uRam000f000c = SUB42(param_5,0);
      uRam000f000e = 0x1c00;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      uVar6 = (uint)uRam000f0000;
      uVar12 = (uint)uRam000f0004;
      puVar18 = (uint *)CONCAT22((short)((uint)puVar18 >> 0x10),uRam000f0008);
      puVar19 = (uint *)(uint)uRam000f000a;
      param_3 = uRam000f0006;
      if (!(bool)uRam000f0012) {
        *(undefined4 *)(puVar15 + -4) = 0x6594;
        FUN_00005f6c(puVar18);
        return DAT_00000937;
      }
    }
  }
code_r0x00005e3a:
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)uVar6);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0004 = (ushort)uVar12;
    uRam000f0008 = SUB42(puVar18,0);
    uRam000f000a = (ushort)puVar19;
    uRam000f000c = SUB42(param_5,0);
    uRam000f0006 = param_3;
    uRam000f000e = uVar22;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  puVar8 = (undefined8 *)thunk_FUN_00000f64(1);
  return puVar8;
}


/* ===== FUN_0000643c @ 0000643c ===== */

/* WARNING: Removing unreachable block (ram,0x0000685c) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

undefined4 __allregs
FUN_0000643c(uint *param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,int param_5,
            undefined4 param_6,uint *param_7)

{
  char *pcVar1;
  char cVar2;
  ushort uVar3;
  longlong lVar4;
  uint uVar5;
  undefined4 uVar6;
  undefined8 *puVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  byte bVar12;
  uint uVar13;
  undefined2 uVar14;
  int *piVar15;
  uint *puVar16;
  uint *puVar17;
  undefined2 uVar18;
  uint *puVar19;
  undefined2 unaff_ES;
  undefined2 uVar20;
  undefined1 in_CF;
  undefined2 uStack_6;
  
  uVar20 = 0x1c00;
  DAT_0000556c = param_1;
  uVar5 = FUN_00005cc2(0x3d00,param_2,param_4,(short)param_5,param_6,(short)param_7);
  uVar14 = (undefined2)param_5;
  uVar18 = (short)param_7;
  if (!(bool)in_CF) {
    uVar6 = FUN_00005f17(uVar5);
    DAT_00005568 = (uint *)((int)DAT_00005568 + -0x12);
    piVar15 = (int *)(PTR_LAB_00005560 + DAT_00005564);
    FUN_00005fbe(uVar6,param_2,(char)param_3,(short)param_5,piVar15,(short)param_7,0x646d);
    if (*piVar15 != 0x31434c4b) {
      uVar6 = FUN_00005e3a((char)*piVar15,param_2,param_3,(short)param_5,(short)(piVar15 + 1),
                           (short)param_7);
      return uVar6;
    }
    DAT_00005578 = piVar15[1];
    puVar17 = (uint *)(piVar15 + 3);
    DAT_0000557c = piVar15[2];
    lVar4 = (ulonglong)DAT_0000557c * (ulonglong)DAT_00005578;
    DAT_00000937 = (undefined4)lVar4;
    if (DAT_0000556c != (uint *)0x0) {
      iVar9 = 0xc0;
      puVar16 = puVar17;
      puVar19 = &DAT_00005598;
      do {
        uVar6 = (undefined4)((ulonglong)lVar4 >> 0x20);
        if (DAT_00005568 <= puVar16) {
          puVar7 = FUN_00005fbe((int)lVar4,(short)iVar9,(char)((ulonglong)lVar4 >> 0x20),
                                (short)param_5,puVar16,(short)puVar19,0x65d5);
          lVar4 = CONCAT44(uVar6,puVar7);
        }
        uVar5 = (uint)lVar4;
        puVar17 = puVar16 + 1;
        *puVar19 = *puVar16;
        iVar9 = iVar9 + -1;
        puVar16 = puVar17;
        puVar19 = puVar19 + 1;
      } while (iVar9 != 0);
      param_5 = DAT_00005578 * 3;
      uVar10 = DAT_0000557c >> 2;
      uVar8 = (uint)piVar15 & 0xff;
      uVar11 = DAT_00005578;
      param_7 = DAT_0000556c;
      do {
        uVar18 = (undefined2)uVar10;
        uStack_6 = (undefined2)(uVar10 >> 0x10);
        uVar10 = uVar11 >> 2;
        do {
          if (DAT_00005568 <= puVar17) {
            FUN_00005fbe(uVar5,(short)uVar10,(char)uVar11,(short)param_5,puVar17,(short)param_7,
                         0x6606);
          }
          uVar13 = uVar8 & 0xffff03ff;
          if ((uVar8 & 0x300) == 0) {
            uVar13 = CONCAT31((int3)(uVar13 >> 8),(char)*puVar17);
            puVar17 = (uint *)((int)puVar17 + 1);
          }
          if ((uVar13 & 2) == 0) {
            if ((uVar13 & 1) == 0) {
              uVar5 = *(uint *)((int)puVar17 + 3);
              uVar8 = uVar5 & 3;
              if ((uVar5 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)param_7 = (char)uVar8;
              uVar8 = uVar5 >> 2 & 3;
              if ((uVar5 >> 2 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + 1) = (char)uVar8;
              uVar8 = uVar5 >> 4 & 3;
              if ((uVar5 >> 4 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + 2) = (char)uVar8;
              uVar8 = uVar5 >> 6 & 3;
              if ((uVar5 >> 6 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + 3) = (char)uVar8;
              uVar8 = uVar5 >> 8 & 3;
              if ((uVar5 & 0x300) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)(uVar11 + (int)param_7) = (char)uVar8;
              uVar8 = uVar5 >> 10 & 3;
              if ((uVar5 >> 10 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)(uVar11 + 1 + (int)param_7) = (char)uVar8;
              uVar8 = uVar5 >> 0xc & 3;
              if ((uVar5 >> 0xc & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)(uVar11 + 2 + (int)param_7) = (char)uVar8;
              uVar8 = uVar5 >> 0xe & 3;
              if ((uVar5 >> 0xe & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)(uVar11 + 3 + (int)param_7) = (char)uVar8;
              uVar8 = uVar5 >> 0x10 & 3;
              if ((uVar5 & 0x30000) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + uVar11 * 2) = (char)uVar8;
              uVar8 = uVar5 >> 0x12 & 3;
              if ((uVar5 >> 0x12 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + uVar11 * 2 + 1) = (char)uVar8;
              uVar8 = uVar5 >> 0x14 & 3;
              if ((uVar5 >> 0x14 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + uVar11 * 2 + 2) = (char)uVar8;
              uVar8 = uVar5 >> 0x16 & 3;
              if ((uVar5 >> 0x16 & 3) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)((int)param_7 + uVar11 * 2 + 3) = (char)uVar8;
              bVar12 = (byte)(uVar5 >> 0x18);
              uVar8 = bVar12 & 0xffffff03;
              if ((uVar5 & 0x3000000) != 0) {
                uVar8 = (uint)*(byte *)((int)puVar17 + (uVar8 - 1));
              }
              *(char *)(param_5 + (int)param_7) = (char)uVar8;
              uVar5 = bVar12 >> 2 & 0xffffff03;
              if ((bVar12 >> 2 & 3) != 0) {
                uVar5 = (uint)*(byte *)((int)puVar17 + (uVar5 - 1));
              }
              *(char *)(param_5 + 1 + (int)param_7) = (char)uVar5;
              uVar5 = bVar12 >> 4 & 0xffffff03;
              if ((bVar12 >> 4 & 3) != 0) {
                uVar5 = (uint)*(byte *)((int)puVar17 + (uVar5 - 1));
              }
              *(char *)(param_5 + 2 + (int)param_7) = (char)uVar5;
              uVar5 = (uint)(bVar12 >> 6);
              if (bVar12 >> 6 != 0) {
                uVar5 = (uint)*(byte *)((int)puVar17 + (uVar5 - 1));
              }
              *(char *)(param_5 + 3 + (int)param_7) = (char)uVar5;
              puVar17 = (uint *)((int)puVar17 + 7);
            }
            else {
              uVar3 = *(ushort *)((int)puVar17 + 2);
              *(char *)param_7 = *(char *)((uint)((uVar3 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + 1) = *(char *)((uint)((uVar3 >> 1 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + 2) = *(char *)((uint)((uVar3 >> 2 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + 3) = *(char *)((uint)((uVar3 >> 3 & 1) != 0) + (int)puVar17);
              *(char *)(uVar11 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 4 & 1) != 0) + (int)puVar17);
              *(char *)(uVar11 + 1 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 5 & 1) != 0) + (int)puVar17);
              *(char *)(uVar11 + 2 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 6 & 1) != 0) + (int)puVar17);
              *(char *)(uVar11 + 3 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 7 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + uVar11 * 2) =
                   *(char *)((uint)((uVar3 >> 8 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + uVar11 * 2 + 1) =
                   *(char *)((uint)((uVar3 >> 9 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + uVar11 * 2 + 2) =
                   *(char *)((uint)((uVar3 >> 10 & 1) != 0) + (int)puVar17);
              *(char *)((int)param_7 + uVar11 * 2 + 3) =
                   *(char *)((uint)((uVar3 >> 0xb & 1) != 0) + (int)puVar17);
              *(char *)(param_5 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 0xc & 1) != 0) + (int)puVar17);
              *(char *)(param_5 + 1 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 0xd & 1) != 0) + (int)puVar17);
              *(char *)(param_5 + 2 + (int)param_7) =
                   *(char *)((uint)((uVar3 >> 0xe & 1) != 0) + (int)puVar17);
              bVar12 = *(byte *)((uint)((short)uVar3 < 0) + (int)puVar17);
              uVar5 = (uint)bVar12;
              *(byte *)(param_5 + 3 + (int)param_7) = bVar12;
              puVar17 = puVar17 + 1;
            }
          }
          else if ((uVar13 & 1) == 0) {
            cVar2 = (char)*puVar17;
            if (cVar2 == '\0') {
              *param_7 = *(uint *)((int)puVar17 + 1);
              *(undefined4 *)(uVar11 + (int)param_7) = *(undefined4 *)((int)puVar17 + 5);
              puVar16 = (uint *)((int)puVar17 + 0xd);
              *(undefined4 *)((int)param_7 + uVar11 * 2) = *(undefined4 *)((int)puVar17 + 9);
              puVar17 = (uint *)((int)puVar17 + 0x11);
              uVar5 = *puVar16;
              *(uint *)(param_5 + (int)param_7) = uVar5;
            }
            else {
              uVar5 = CONCAT22(CONCAT11(cVar2,cVar2),CONCAT11(cVar2,cVar2));
              *param_7 = uVar5;
              *(uint *)(uVar11 + (int)param_7) = uVar5;
              *(uint *)((int)param_7 + uVar11 * 2) = uVar5;
              *(uint *)(param_5 + (int)param_7) = uVar5;
              puVar17 = (uint *)((int)puVar17 + 1);
            }
          }
          else {
            uVar5 = 0;
            *param_7 = 0;
            pcVar1 = (char *)(uVar11 + (int)param_7);
            pcVar1[0] = '\0';
            pcVar1[1] = '\0';
            pcVar1[2] = '\0';
            pcVar1[3] = '\0';
            pcVar1 = (char *)((int)param_7 + uVar11 * 2);
            pcVar1[0] = '\0';
            pcVar1[1] = '\0';
            pcVar1[2] = '\0';
            pcVar1[3] = '\0';
            pcVar1 = (char *)(param_5 + (int)param_7);
            pcVar1[0] = '\0';
            pcVar1[1] = '\0';
            pcVar1[2] = '\0';
            pcVar1[3] = '\0';
          }
          lVar4 = CONCAT44(uVar11,uVar5);
          param_7 = param_7 + 1;
          uVar8 = (uint)CONCAT11((char)(uVar13 >> 8) + '\x01',(byte)uVar13 >> 2);
          uVar10 = uVar10 - 1;
        } while (uVar10 != 0);
        param_7 = (uint *)((int)param_7 + param_5);
        uVar10 = CONCAT22(uStack_6,uVar18) - 1;
        param_2 = 0;
      } while (uVar10 != 0);
    }
    uRam000f0004 = param_2;
    uRam000f0000 = CONCAT11(0x3e,(char)lVar4);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0006 = (undefined2)((ulonglong)lVar4 >> 0x20);
    uRam000f0008 = SUB42(puVar17,0);
    uRam000f000a = SUB42(param_7,0);
    uVar14 = (undefined2)param_5;
    uRam000f000e = 0x1c00;
    uRam000f000c = uVar14;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar5 = (uint)uRam000f0000;
    param_6 = CONCAT22((short)((uint)puVar17 >> 0x10),uRam000f0008);
    param_2 = uRam000f0004;
    param_3 = uRam000f0006;
    uVar18 = uRam000f000a;
    if (!(bool)uRam000f0012) {
      FUN_00005f6c(param_6);
      return DAT_00000937;
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)uVar5);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (undefined2)param_6;
    uRam000f0004 = param_2;
    uRam000f0006 = param_3;
    uRam000f000a = uVar18;
    uRam000f000c = uVar14;
    uRam000f000e = uVar20;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  uVar6 = thunk_FUN_00000f64(1);
  return uVar6;
}


/* ===== FUN_00006481 @ 00006481 ===== */

/* WARNING: Removing unreachable block (ram,0x00006587) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * __allregs
FUN_00006481(uint param_1,ushort param_2,undefined2 param_3,int param_4,undefined2 param_5,
            byte *param_6,undefined4 *param_7)

{
  byte bVar1;
  byte bVar2;
  longlong lVar3;
  ushort uVar4;
  undefined8 *puVar5;
  undefined3 uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined4 uVar10;
  byte *pbVar11;
  byte *pbVar12;
  byte *pbVar13;
  undefined2 uVar14;
  byte *pbVar15;
  undefined4 *puVar17;
  undefined2 unaff_ES;
  undefined2 uVar18;
  byte *pbVar16;
  
  uVar18 = 0x1c00;
  uVar14 = (short)param_7;
  if (*(char *)(param_4 + 0x41) == '\x01') {
    _DAT_00005594 = *(short *)param_6;
    _DAT_00005596 = *(short *)(param_6 + 2);
    pbVar12 = param_6 + 6;
    DAT_00005578 = (uint)(ushort)((*(short *)(param_6 + 4) + 1) - _DAT_00005594);
    param_6 = param_6 + 8;
    uVar4 = (*(short *)pbVar12 + 1) - _DAT_00005596;
    uVar8 = (uint)uVar4;
    param_1 = *(ushort *)(param_4 + 0x42) - DAT_00005578;
    DAT_0000557c = uVar8;
    if (DAT_00005578 <= *(ushort *)(param_4 + 0x42)) {
      lVar3 = (ulonglong)DAT_00005578 * (ulonglong)uVar8;
      uVar10 = 0;
      puVar5 = (undefined8 *)lVar3;
      DAT_00000937 = puVar5;
      _DAT_00005590 = param_1;
      if (DAT_0000556c != (byte *)0x0) {
        pbVar12 = (byte *)(param_4 + 0x80);
        pbVar15 = DAT_0000556c;
        do {
          pbVar11 = pbVar15 + DAT_00005578 + _DAT_00005590;
          pbVar13 = pbVar12;
          do {
            while( true ) {
              if (DAT_00005568 <= pbVar13) {
                puVar5 = FUN_00005fbe(puVar5,0,0,(short)pbVar11,pbVar13,(short)pbVar15,0x651f);
                return puVar5;
              }
              pbVar12 = pbVar13 + 1;
              bVar1 = *pbVar13;
              uVar6 = (undefined3)((uint)puVar5 >> 8);
              puVar5 = (undefined8 *)CONCAT31(uVar6,bVar1);
              if (0xbf < bVar1) break;
              pbVar16 = pbVar15 + 1;
              *pbVar15 = bVar1;
              pbVar13 = pbVar12;
              pbVar15 = pbVar16;
              if (pbVar11 <= pbVar16) goto LAB_0000654a;
            }
            pbVar13 = pbVar13 + 2;
            bVar2 = *pbVar12;
            puVar5 = (undefined8 *)CONCAT31(uVar6,bVar2);
            for (uVar7 = bVar1 & 0x3f; uVar7 != 0; uVar7 = uVar7 - 1) {
              *pbVar15 = bVar2;
              pbVar15 = pbVar15 + 1;
            }
            pbVar12 = pbVar13;
            pbVar16 = pbVar15;
          } while (pbVar15 < pbVar11);
LAB_0000654a:
          pbVar15 = pbVar16 + -_DAT_00005590;
          uVar8 = uVar8 - 1;
        } while (uVar8 != 0);
        if (DAT_00005568 <= pbVar12) {
          puVar5 = FUN_00005fbe(puVar5,0,0,(short)pbVar11,pbVar12,(short)pbVar15,0x6560);
        }
        param_5 = SUB42(pbVar11,0);
        param_3 = (undefined2)uVar10;
        param_2 = (ushort)uVar8;
        param_6 = pbVar12 + 1;
        param_1 = CONCAT31((int3)((uint)puVar5 >> 8),*pbVar12);
        lVar3 = CONCAT44(uVar10,param_1);
        uVar14 = (short)pbVar15;
        if (*pbVar12 != 0xc) goto FUN_00005e3a;
        iVar9 = 0xc0;
        pbVar12 = param_6;
        puVar17 = &DAT_00005598;
        do {
          if (DAT_00005568 <= pbVar12) {
            puVar5 = FUN_00005fbe(param_1,(short)iVar9,(char)uVar10,param_5,pbVar12,(short)puVar17,
                                  0x657b);
            return puVar5;
          }
          param_7 = puVar17 + 1;
          param_6 = pbVar12 + 4;
          *puVar17 = *(undefined4 *)pbVar12;
          iVar9 = iVar9 + -1;
          pbVar12 = param_6;
          puVar17 = param_7;
          uVar4 = 0;
        } while (iVar9 != 0);
      }
      uRam000f0004 = uVar4;
      uRam000f0000 = CONCAT11(0x3e,(char)lVar3);
      uRam000f0014 = 0x21;
      sRam000f0002 = DAT_00005cc0;
      uRam000f0006 = (undefined2)((ulonglong)lVar3 >> 0x20);
      uRam000f0008 = SUB42(param_6,0);
      uRam000f000a = SUB42(param_7,0);
      uRam000f000e = 0x1c00;
      uRam000f000c = param_5;
      uRam000f0010 = unaff_ES;
      (*(code *)0xf0100)();
      param_1 = (uint)uRam000f0000;
      param_6 = (byte *)CONCAT22((short)((uint)param_6 >> 0x10),uRam000f0008);
      param_2 = uRam000f0004;
      param_3 = uRam000f0006;
      uVar14 = uRam000f000a;
      if (!(bool)uRam000f0012) {
        FUN_00005f6c(param_6);
        return DAT_00000937;
      }
    }
  }
FUN_00005e3a:
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)param_1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = SUB42(param_6,0);
    uRam000f0004 = param_2;
    uRam000f0006 = param_3;
    uRam000f000a = uVar14;
    uRam000f000c = param_5;
    uRam000f000e = uVar18;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  puVar5 = (undefined8 *)thunk_FUN_00000f64(1);
  return puVar5;
}


/* ===== FUN_00006542 @ 00006542 ===== */

/* WARNING: Instruction at (ram,0x00006543) overlaps instruction at (ram,0x00006542)
    */
/* WARNING: Removing unreachable block (ram,0x00006587) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * __allregs
FUN_00006542(undefined8 *param_1,int param_2,undefined2 param_3,undefined4 *param_4,byte *param_5,
            byte *param_6,int param_7)

{
  byte bVar1;
  undefined4 *puVar2;
  char cVar3;
  undefined8 *puVar4;
  undefined3 uVar6;
  uint uVar5;
  undefined2 uVar7;
  int iVar8;
  BADSPACEBASE *in_ESP;
  undefined2 uVar9;
  byte *pbVar10;
  byte *pbVar11;
  undefined2 uVar12;
  byte *pbVar13;
  undefined4 *puVar14;
  undefined2 unaff_ES;
  undefined2 uVar15;
  undefined4 uStack_8;
  undefined4 *puStack_4;
  
  uVar15 = 0x1c00;
  puVar14 = &puStack_4;
  pbVar10 = (byte *)&puStack_4;
  cVar3 = '\t';
  puVar2 = param_4;
  do {
    puVar2 = puVar2 + -1;
    puVar14 = puVar14 + -1;
    *puVar14 = *puVar2;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  do {
    pbVar11 = param_5;
    pbVar13 = param_6;
    puStack_4 = param_4;
    if (param_6 < pbVar10) goto LAB_00006517;
LAB_0000654a:
    uVar9 = SUB42(pbVar10,0);
    pbVar13 = param_6 + -_DAT_00005590;
    param_7 = param_7 + -1;
    if (param_7 == 0) {
      if (DAT_00005568 <= pbVar11) {
        param_1 = FUN_00005fbe(param_1,0,(char)param_3,uVar9,pbVar11,(short)pbVar13,0x6560);
      }
      pbVar10 = pbVar11 + 1;
      uVar5 = CONCAT31((int3)((uint)param_1 >> 8),*pbVar11);
      uVar7 = (short)param_7;
      uVar12 = (short)pbVar13;
      if (*pbVar11 == 0xc) {
        iVar8 = 0xc0;
        puVar14 = &DAT_00005598;
        do {
          if (DAT_00005568 <= pbVar10) {
            puVar4 = FUN_00005fbe(uVar5,(short)iVar8,(char)param_3,uVar9,pbVar10,(short)puVar14,
                                  0x657b);
            return puVar4;
          }
          puVar2 = puVar14 + 1;
          pbVar11 = pbVar10 + 4;
          *puVar14 = *(undefined4 *)pbVar10;
          iVar8 = iVar8 + -1;
          pbVar10 = pbVar11;
          puVar14 = puVar2;
        } while (iVar8 != 0);
        uRam000f0014 = 0x21;
        uRam000f0000 = 0x3e0c;
        sRam000f0002 = DAT_00005cc0;
        uRam000f0004 = 0;
        uRam000f0008 = SUB42(pbVar11,0);
        uRam000f000a = SUB42(puVar2,0);
        uRam000f000e = 0x1c00;
        uRam000f0006 = param_3;
        uRam000f000c = uVar9;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        uVar5 = (uint)uRam000f0000;
        pbVar10 = (byte *)CONCAT22((short)((uint)pbVar11 >> 0x10),uRam000f0008);
        uVar7 = uRam000f0004;
        param_3 = uRam000f0006;
        uVar12 = uRam000f000a;
        if (!(bool)uRam000f0012) {
          FUN_00005f6c(pbVar10);
          return DAT_00000937;
        }
      }
      if (DAT_00005cc0 != 0) {
        uRam000f0000 = CONCAT11(0x3e,(char)uVar5);
        uRam000f0014 = 0x21;
        sRam000f0002 = DAT_00005cc0;
        uRam000f0008 = SUB42(pbVar10,0);
        uRam000f0004 = uVar7;
        uRam000f0006 = param_3;
        uRam000f000a = uVar12;
        uRam000f000c = uVar9;
        uRam000f000e = uVar15;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        DAT_00005cc0 = 0;
      }
      puVar4 = (undefined8 *)thunk_FUN_00000f64(1);
      return puVar4;
    }
    pbVar10 = pbVar13 + DAT_00005578 + _DAT_00005590;
    param_2 = 0;
    param_5 = pbVar11;
LAB_00006517:
    while( true ) {
      if (DAT_00005568 <= param_5) {
        puVar4 = FUN_00005fbe(param_1,(short)param_2,(char)param_3,(short)pbVar10,param_5,
                              (short)pbVar13,0x651f);
        return puVar4;
      }
      pbVar11 = param_5 + 1;
      bVar1 = *param_5;
      uVar6 = (undefined3)((uint)param_1 >> 8);
      param_1 = (undefined8 *)CONCAT31(uVar6,bVar1);
      if (0xbf < bVar1) break;
      param_6 = pbVar13 + 1;
      *pbVar13 = bVar1;
      param_5 = pbVar11;
      pbVar13 = param_6;
      if (pbVar10 <= param_6) goto LAB_0000654a;
    }
    uVar5 = CONCAT31(uVar6,bVar1) & 0xffffff3f;
    param_1._0_1_ = (undefined1)uVar5;
    param_2._1_3_ = (undefined3)((uint)param_2 >> 8);
    param_5 = param_5 + 2;
    bVar1 = *pbVar11;
    param_1 = (undefined8 *)CONCAT31((int3)(uVar5 >> 8),bVar1);
    param_6 = pbVar13;
    for (param_2 = CONCAT31(param_2._1_3_,param_1._0_1_); param_2 = 0, param_2 != 0;
        param_2 = param_2 + -1) {
      *param_6 = bVar1;
      param_6 = param_6 + 1;
    }
  } while( true );
}


/* ===== FUN_0000686f @ 0000686f ===== */

void __allregs FUN_0000686f(void)

{
  undefined2 uVar1;
  undefined2 uVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  uVar1 = CONCAT11((undefined1)DAT_000028a5,(undefined1)DAT_000028a5);
  uVar2 = CONCAT11((undefined1)DAT_000028a5,(undefined1)DAT_000028a5);
  puVar4 = DAT_00003918;
  for (uVar3 = DAT_0000391c >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *puVar4 = CONCAT22(uVar1,uVar2);
    puVar4 = puVar4 + 1;
  }
  return;
}


/* ===== FUN_00006877 @ 00006877 ===== */

void __allregs FUN_00006877(void)

{
  undefined2 uVar1;
  undefined2 uVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  uVar1 = CONCAT11(LAB_000085c8,LAB_000085c8);
  uVar2 = CONCAT11(LAB_000085c8,LAB_000085c8);
  puVar4 = DAT_00003918;
  for (uVar3 = DAT_0000391c >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *puVar4 = CONCAT22(uVar1,uVar2);
    puVar4 = puVar4 + 1;
  }
  return;
}


/* ===== FUN_00006977 @ 00006977 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00006977(void)

{
  byte bVar1;
  int *piVar2;
  byte bVar3;
  undefined1 uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  int iVar14;
  int *piVar15;
  undefined1 *puVar16;
  undefined4 *puVar17;
  
  if (DAT_000090c0 != DAT_000090c4) {
    DAT_000090c4 = DAT_000090c0;
    FUN_0000395e();
  }
  iVar14 = (DAT_000090e0 & 0x7fffff) << 8;
  uVar12 = ~(DAT_000090e0 << 9) >> 1;
  iVar5 = (DAT_000090e0 >> 0x17) * 4;
  DAT_000090fc = (int)((ulonglong)
                       ((longlong)*(int *)(FUN_0000944b + iVar5 + 5) * (longlong)(int)uVar12) >>
                      0x20) +
                 (int)((ulonglong)((longlong)*(int *)(iVar5 + 0x9454) * (longlong)iVar14) >> 0x20);
  DAT_000090fc = DAT_000090fc * 2;
  DAT_00009100 = (int)((ulonglong)
                       ((longlong)*(int *)(&DAT_00009650 + iVar5) * (longlong)(int)uVar12) >> 0x20)
                 + (int)((ulonglong)((longlong)*(int *)(&DAT_00009654 + iVar5) * (longlong)iVar14)
                        >> 0x20);
  DAT_00009100 = DAT_00009100 * 2;
  _uRam00006add = CONCAT31((int3)((uint)LAB_00003909 >> 8),-(byte)((uint)DAT_000090dc >> 0x19));
  puVar13 = &DAT_00004e60;
  _uRam00006b5d = _uRam00006add;
  for (uVar12 = DAT_000038f1 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *puVar13 = 0;
    puVar13 = puVar13 + 1;
  }
  piVar15 = &DAT_00004a60;
  uVar12 = DAT_000038f1 >> 2;
  iVar5 = DAT_00003918;
  do {
    *piVar15 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15[1] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar2 = piVar15 + 3;
    piVar15[2] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15 = piVar15 + 4;
    *piVar2 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    uVar12 = uVar12 - 1;
  } while (uVar12 != 0);
  uVar12 = 0;
LAB_00006a60:
  iVar14 = DAT_000085bc;
  uVar6 = *(uint *)(uVar12 * 4 + 0x4624) >> 1;
  uVar10 = DAT_000090d8 +
           (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_00009100) >> 0x20) * -2;
  uVar9 = DAT_000090d4 +
          (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar5 = *(int *)(uVar12 * 4 + 0x4224);
  uRam00006ad2 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
  uRam00006ad8 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
  iVar5 = ram0x000038fd;
  uVar6 = uVar9;
  uVar11 = uVar10;
  uRam00006b52 = uRam00006ad2;
  uRam00006b58 = uRam00006ad8;
  do {
    uVar7 = (uVar11 >> 0x16) << 10 | uVar6 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar7) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar5) < bVar3) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar5);
      *(byte *)((int)&DAT_00004e60 + iVar5) = bVar3;
      UNLOCK();
      uVar4 = *(undefined1 *)(iVar14 + 0x100000 + uVar7);
      puVar16 = (undefined1 *)(&DAT_00004a60)[iVar5];
      for (uVar7 = (uint)(byte)(bVar3 - bVar1); uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar16 = uVar4;
        puVar16 = puVar16 + 1;
      }
      (&DAT_00004a60)[iVar5] = puVar16;
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    else {
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    iVar5 = iVar5 + 1;
    uVar6 = uVar6 + 0x7fffffff;
    uVar11 = uVar11 + 0x80000001;
  } while( true );
  while( true ) {
    LOCK();
    bVar1 = (&DAT_00004e5f)[iVar8];
    (&DAT_00004e5f)[iVar8] = bVar3;
    UNLOCK();
    uVar4 = *(undefined1 *)(iVar14 + 0x100000 + uVar6);
    puVar16 = *(undefined1 **)(&DAT_00004a5c + iVar8 * 4);
    for (uVar6 = (uint)(byte)(bVar3 - bVar1); uVar6 != 0; uVar6 = uVar6 - 1) {
      *puVar16 = uVar4;
      puVar16 = puVar16 + 1;
    }
    *(undefined1 **)(&DAT_00004a5c + iVar8 * 4) = puVar16;
    iVar8 = iVar8 + -1;
    if (iVar8 == DAT_00003901) break;
LAB_00006b50:
    uVar9 = uVar9 + 0x80000001;
    uVar10 = uVar10 + 0x7fffffff;
    uVar6 = (uVar10 >> 0x16) << 10 | uVar9 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar6) + -1);
    if (bVar3 <= (byte)(&DAT_00004e5f)[iVar8]) {
      iVar8 = iVar8 + -1;
      if (iVar8 == DAT_00003901) break;
      goto LAB_00006b50;
    }
  }
  uRam00006add._1_2_ = uRam00006add._1_2_ + 1;
  uRam00006b5d._1_2_ = uRam00006b5d._1_2_ + 1;
  puVar13 = &DAT_00004e60;
  puVar17 = (undefined4 *)(uVar12 * 0x100 + _DAT_0000390d);
  for (uVar6 = DAT_000038f1 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *puVar17 = *puVar13;
    puVar13 = puVar13 + 1;
    puVar17 = puVar17 + 1;
  }
  uVar12 = uVar12 + 1;
  if (_FUN_00003a20 <= uVar12) {
    return;
  }
  goto LAB_00006a60;
}


/* ===== FUN_00006980 @ 00006980 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00006980(void)

{
  byte bVar1;
  int *piVar2;
  byte bVar3;
  undefined1 uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  int iVar14;
  int *piVar15;
  undefined1 *puVar16;
  undefined4 *puVar17;
  
  if (DAT_000090c0 != DAT_000090c4) {
    DAT_000090c4 = DAT_000090c0;
    FUN_0000395e();
  }
  iVar14 = (DAT_000090e0 & 0x7fffff) << 8;
  uVar12 = ~(DAT_000090e0 << 9) >> 1;
  iVar5 = (DAT_000090e0 >> 0x17) * 4;
  DAT_000090fc = (int)((ulonglong)
                       ((longlong)*(int *)(FUN_0000944b + iVar5 + 5) * (longlong)(int)uVar12) >>
                      0x20) +
                 (int)((ulonglong)((longlong)*(int *)(iVar5 + 0x9454) * (longlong)iVar14) >> 0x20);
  DAT_000090fc = DAT_000090fc * 2;
  DAT_00009100 = (int)((ulonglong)
                       ((longlong)*(int *)(&DAT_00009650 + iVar5) * (longlong)(int)uVar12) >> 0x20)
                 + (int)((ulonglong)((longlong)*(int *)(&DAT_00009654 + iVar5) * (longlong)iVar14)
                        >> 0x20);
  DAT_00009100 = DAT_00009100 * 2;
  _uRam00006add = CONCAT31((int3)((uint)LAB_00003909 >> 8),-(byte)((uint)DAT_000090dc >> 0x19));
  puVar13 = &DAT_00004e60;
  _uRam00006b5d = _uRam00006add;
  for (uVar12 = DAT_000038f1 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *puVar13 = 0;
    puVar13 = puVar13 + 1;
  }
  piVar15 = &DAT_00004a60;
  uVar12 = DAT_000038f1 >> 2;
  iVar5 = DAT_00003918;
  do {
    *piVar15 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15[1] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar2 = piVar15 + 3;
    piVar15[2] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15 = piVar15 + 4;
    *piVar2 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    uVar12 = uVar12 - 1;
  } while (uVar12 != 0);
  uVar12 = 0;
LAB_00006a60:
  iVar14 = DAT_000085bc;
  uVar6 = *(uint *)(uVar12 * 4 + 0x4624) >> 1;
  uVar10 = DAT_000090d8 +
           (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_00009100) >> 0x20) * -2;
  uVar9 = DAT_000090d4 +
          (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar5 = *(int *)(uVar12 * 4 + 0x4224);
  uRam00006ad2 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
  uRam00006ad8 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
  iVar5 = ram0x000038fd;
  uVar6 = uVar9;
  uVar11 = uVar10;
  uRam00006b52 = uRam00006ad2;
  uRam00006b58 = uRam00006ad8;
  do {
    uVar7 = (uVar11 >> 0x16) << 10 | uVar6 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar7) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar5) < bVar3) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar5);
      *(byte *)((int)&DAT_00004e60 + iVar5) = bVar3;
      UNLOCK();
      uVar4 = *(undefined1 *)(iVar14 + 0x100000 + uVar7);
      puVar16 = (undefined1 *)(&DAT_00004a60)[iVar5];
      for (uVar7 = (uint)(byte)(bVar3 - bVar1); uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar16 = uVar4;
        puVar16 = puVar16 + 1;
      }
      (&DAT_00004a60)[iVar5] = puVar16;
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    else {
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    iVar5 = iVar5 + 1;
    uVar6 = uVar6 + 0x7fffffff;
    uVar11 = uVar11 + 0x80000001;
  } while( true );
  while( true ) {
    LOCK();
    bVar1 = (&DAT_00004e5f)[iVar8];
    (&DAT_00004e5f)[iVar8] = bVar3;
    UNLOCK();
    uVar4 = *(undefined1 *)(iVar14 + 0x100000 + uVar6);
    puVar16 = *(undefined1 **)(&DAT_00004a5c + iVar8 * 4);
    for (uVar6 = (uint)(byte)(bVar3 - bVar1); uVar6 != 0; uVar6 = uVar6 - 1) {
      *puVar16 = uVar4;
      puVar16 = puVar16 + 1;
    }
    *(undefined1 **)(&DAT_00004a5c + iVar8 * 4) = puVar16;
    iVar8 = iVar8 + -1;
    if (iVar8 == DAT_00003901) break;
LAB_00006b50:
    uVar9 = uVar9 + 0x80000001;
    uVar10 = uVar10 + 0x7fffffff;
    uVar6 = (uVar10 >> 0x16) << 10 | uVar9 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar6) + -1);
    if (bVar3 <= (byte)(&DAT_00004e5f)[iVar8]) {
      iVar8 = iVar8 + -1;
      if (iVar8 == DAT_00003901) break;
      goto LAB_00006b50;
    }
  }
  uRam00006add._1_2_ = uRam00006add._1_2_ + 1;
  uRam00006b5d._1_2_ = uRam00006b5d._1_2_ + 1;
  puVar13 = &DAT_00004e60;
  puVar17 = (undefined4 *)(uVar12 * 0x100 + _DAT_0000390d);
  for (uVar6 = DAT_000038f1 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *puVar17 = *puVar13;
    puVar13 = puVar13 + 1;
    puVar17 = puVar17 + 1;
  }
  uVar12 = uVar12 + 1;
  if (_FUN_00003a20 <= uVar12) {
    return;
  }
  goto LAB_00006a60;
}


/* ===== FUN_00006b03 @ 00006b03 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00006b03(byte param_1,uint param_2,int param_3,uint param_4,uint param_5,int param_6,
            uint param_7,uint param_8,uint param_9)

{
  byte bVar1;
  byte bVar2;
  undefined1 uVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined1 *puVar7;
  undefined4 *puVar8;
  
LAB_00006b10:
  LOCK();
  bVar1 = *(byte *)((int)&DAT_00004e60 + param_3);
  *(byte *)((int)&DAT_00004e60 + param_3) = param_1;
  UNLOCK();
  uVar3 = *(undefined1 *)(param_6 + 0x100000 + param_2);
  puVar7 = (undefined1 *)(&DAT_00004a60)[param_3];
  for (uVar4 = (uint)(byte)(param_1 - bVar1); uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar7 = uVar3;
    puVar7 = puVar7 + 1;
  }
  (&DAT_00004a60)[param_3] = puVar7;
  param_3 = param_3 + 1;
  iVar5 = ram0x000038fd;
  if (param_3 != DAT_00003905) goto LAB_00006ad0;
LAB_00006b50:
  do {
    param_8 = param_8 + 0x80000001;
    param_7 = param_7 + 0x7fffffff;
    uVar4 = (param_7 >> 0x16) << 10 | param_8 >> 0x16;
    bVar1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + uVar4) + -1);
    if ((byte)(&DAT_00004e5f)[iVar5] < bVar1) goto LAB_00006b90;
    iVar5 = iVar5 + -1;
  } while (iVar5 != DAT_00003901);
  goto LAB_00006bbd;
LAB_00006b90:
  LOCK();
  bVar2 = (&DAT_00004e5f)[iVar5];
  (&DAT_00004e5f)[iVar5] = bVar1;
  UNLOCK();
  uVar3 = *(undefined1 *)(param_6 + 0x100000 + uVar4);
  puVar7 = *(undefined1 **)(&DAT_00004a5c + iVar5 * 4);
  for (uVar4 = (uint)(byte)(bVar1 - bVar2); uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar7 = uVar3;
    puVar7 = puVar7 + 1;
  }
  *(undefined1 **)(&DAT_00004a5c + iVar5 * 4) = puVar7;
  iVar5 = iVar5 + -1;
  if (iVar5 == DAT_00003901) {
LAB_00006bbd:
    sRam00006ade = sRam00006ade + 1;
    sRam00006b5e = sRam00006b5e + 1;
    puVar6 = &DAT_00004e60;
    puVar8 = (undefined4 *)(param_9 * 0x100 + _DAT_0000390d);
    for (uVar4 = DAT_000038f1 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar8 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar8 = puVar8 + 1;
    }
    param_9 = param_9 + 1;
    if (_FUN_00003a20 <= param_9) {
      return;
    }
    uVar4 = *(uint *)(param_9 * 4 + 0x4624) >> 1;
    param_5 = DAT_000090d8 +
              (int)((ulonglong)((longlong)(int)uVar4 * (longlong)DAT_00009100) >> 0x20) * -2;
    param_4 = DAT_000090d4 +
              (int)((ulonglong)((longlong)(int)uVar4 * (longlong)DAT_000090fc) >> 0x20) * -2;
    iVar5 = *(int *)(param_9 * 4 + 0x4224);
    uRam00006ad2 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
    uRam00006ad8 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
    param_3 = ram0x000038fd;
    param_6 = DAT_000085bc;
    uRam00006b52 = uRam00006ad2;
    uRam00006b58 = uRam00006ad8;
    param_7 = param_5;
    param_8 = param_4;
    while( true ) {
      param_2 = (param_5 >> 0x16) << 10 | param_4 >> 0x16;
      param_1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + param_2) + -1);
      if (*(byte *)((int)&DAT_00004e60 + param_3) < param_1) goto LAB_00006b10;
      param_3 = param_3 + 1;
      iVar5 = ram0x000038fd;
      if (param_3 == DAT_00003905) break;
LAB_00006ad0:
      param_4 = param_4 + 0x7fffffff;
      param_5 = param_5 + 0x80000001;
    }
  }
  goto LAB_00006b50;
}


/* ===== FUN_00006b83 @ 00006b83 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00006b83(byte param_1,uint param_2,int param_3,uint param_4,uint param_5,int param_6,
            uint param_7)

{
  byte bVar1;
  byte bVar2;
  undefined1 uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined1 *puVar9;
  undefined4 *puVar10;
  
LAB_00006b90:
  LOCK();
  bVar2 = (&DAT_00004e5f)[param_3];
  (&DAT_00004e5f)[param_3] = param_1;
  UNLOCK();
  uVar3 = *(undefined1 *)(param_6 + 0x100000 + param_2);
  puVar9 = *(undefined1 **)(&DAT_00004a5c + param_3 * 4);
  for (uVar5 = (uint)(byte)(param_1 - bVar2); uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar9 = uVar3;
    puVar9 = puVar9 + 1;
  }
  *(undefined1 **)(&DAT_00004a5c + param_3 * 4) = puVar9;
  param_3 = param_3 + -1;
  if (param_3 != DAT_00003901) goto LAB_00006b50;
LAB_00006bbd:
  sRam00006ade = sRam00006ade + 1;
  sRam00006b5e = sRam00006b5e + 1;
  puVar8 = &DAT_00004e60;
  puVar10 = (undefined4 *)(param_7 * 0x100 + _DAT_0000390d);
  for (uVar5 = DAT_000038f1 >> 2; param_6 = DAT_000085bc, uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar10 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar10 = puVar10 + 1;
  }
  param_7 = param_7 + 1;
  if (_FUN_00003a20 <= param_7) {
    return;
  }
  uVar5 = *(uint *)(param_7 * 4 + 0x4624) >> 1;
  param_5 = DAT_000090d8 +
            (int)((ulonglong)((longlong)(int)uVar5 * (longlong)DAT_00009100) >> 0x20) * -2;
  param_4 = DAT_000090d4 +
            (int)((ulonglong)((longlong)(int)uVar5 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar6 = *(int *)(param_7 * 4 + 0x4224);
  uRam00006ad2 = (undefined4)((ulonglong)((longlong)iVar6 * (longlong)DAT_00009100) >> 0x20);
  uRam00006ad8 = (undefined4)((ulonglong)((longlong)iVar6 * (longlong)DAT_000090fc) >> 0x20);
  iVar6 = ram0x000038fd;
  uVar5 = param_4;
  uVar7 = param_5;
  uRam00006b52 = uRam00006ad2;
  uRam00006b58 = uRam00006ad8;
  do {
    uVar4 = (uVar7 >> 0x16) << 10 | uVar5 >> 0x16;
    bVar2 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + uVar4) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar6) < bVar2) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar6);
      *(byte *)((int)&DAT_00004e60 + iVar6) = bVar2;
      UNLOCK();
      uVar3 = *(undefined1 *)(param_6 + 0x100000 + uVar4);
      puVar9 = (undefined1 *)(&DAT_00004a60)[iVar6];
      for (uVar4 = (uint)(byte)(bVar2 - bVar1); uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar9 = uVar3;
        puVar9 = puVar9 + 1;
      }
      (&DAT_00004a60)[iVar6] = puVar9;
      param_3 = ram0x000038fd;
      if (iVar6 + 1 == DAT_00003905) break;
    }
    else {
      param_3 = ram0x000038fd;
      if (iVar6 + 1 == DAT_00003905) break;
    }
    iVar6 = iVar6 + 1;
    uVar5 = uVar5 + 0x7fffffff;
    uVar7 = uVar7 + 0x80000001;
  } while( true );
LAB_00006b50:
  do {
    param_4 = param_4 + 0x80000001;
    param_5 = param_5 + 0x7fffffff;
    param_2 = (param_5 >> 0x16) << 10 | param_4 >> 0x16;
    param_1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + param_2) + -1);
    if ((byte)(&DAT_00004e5f)[param_3] < param_1) goto LAB_00006b90;
    param_3 = param_3 + -1;
  } while (param_3 != DAT_00003901);
  goto LAB_00006bbd;
}


/* ===== FUN_00006bf7 @ 00006bf7 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00006bf7(void)

{
  byte bVar1;
  int *piVar2;
  byte bVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  int iVar14;
  int *piVar15;
  char *pcVar16;
  undefined4 *puVar17;
  
  if (DAT_000090c0 != DAT_000090c4) {
    DAT_000090c4 = DAT_000090c0;
    FUN_0000395e();
  }
  iVar14 = (DAT_000090e0 & 0x7fffff) << 8;
  uVar12 = ~(DAT_000090e0 << 9) >> 1;
  iVar5 = (DAT_000090e0 >> 0x17) * 4;
  DAT_000090fc = (int)((ulonglong)
                       ((longlong)*(int *)(FUN_0000944b + iVar5 + 5) * (longlong)(int)uVar12) >>
                      0x20) +
                 (int)((ulonglong)((longlong)*(int *)(iVar5 + 0x9454) * (longlong)iVar14) >> 0x20);
  DAT_000090fc = DAT_000090fc * 2;
  DAT_00009100 = (int)((ulonglong)
                       ((longlong)*(int *)(&DAT_00009650 + iVar5) * (longlong)(int)uVar12) >> 0x20)
                 + (int)((ulonglong)((longlong)*(int *)(&DAT_00009654 + iVar5) * (longlong)iVar14)
                        >> 0x20);
  DAT_00009100 = DAT_00009100 * 2;
  _uRam00006d5d = CONCAT31((int3)((uint)LAB_00003909 >> 8),-(byte)((uint)DAT_000090dc >> 0x19));
  puVar13 = &DAT_00004e60;
  _uRam00006ddd = _uRam00006d5d;
  for (uVar12 = DAT_000038f1 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *puVar13 = 0;
    puVar13 = puVar13 + 1;
  }
  piVar15 = &DAT_00004a60;
  uVar12 = DAT_000038f1 >> 2;
  iVar5 = DAT_00003918;
  do {
    *piVar15 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15[1] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar2 = piVar15 + 3;
    piVar15[2] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15 = piVar15 + 4;
    *piVar2 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    uVar12 = uVar12 - 1;
  } while (uVar12 != 0);
  uVar12 = 0;
LAB_00006ce0:
  iVar14 = DAT_000085bc;
  uVar6 = *(uint *)(uVar12 * 4 + 0x4624) >> 1;
  uVar10 = DAT_000090d8 +
           (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_00009100) >> 0x20) * -2;
  uVar9 = DAT_000090d4 +
          (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar5 = *(int *)(uVar12 * 4 + 0x4224);
  uRam00006d52 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
  uRam00006d58 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
  iVar5 = ram0x000038fd;
  uVar6 = uVar9;
  uVar11 = uVar10;
  uRam00006dd2 = uRam00006d52;
  uRam00006dd8 = uRam00006d58;
  do {
    uVar7 = (uVar11 >> 0x16) << 10 | uVar6 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar7) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar5) < bVar3) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar5);
      *(byte *)((int)&DAT_00004e60 + iVar5) = bVar3;
      UNLOCK();
      cVar4 = *(char *)(iVar14 + 0x100000 + uVar7);
      pcVar16 = (char *)(&DAT_00004a60)[iVar5];
      for (uVar7 = (uint)(byte)(bVar3 - bVar1); uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar16 = ((byte)(cVar4 + 0xb0U) >> 2) + 0x50;
        pcVar16 = pcVar16 + 1;
      }
      (&DAT_00004a60)[iVar5] = pcVar16;
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    else {
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    iVar5 = iVar5 + 1;
    uVar6 = uVar6 + 0x7fffffff;
    uVar11 = uVar11 + 0x80000001;
  } while( true );
  while( true ) {
    LOCK();
    bVar1 = (&DAT_00004e5f)[iVar8];
    (&DAT_00004e5f)[iVar8] = bVar3;
    UNLOCK();
    cVar4 = *(char *)(iVar14 + 0x100000 + uVar6);
    pcVar16 = *(char **)(&DAT_00004a5c + iVar8 * 4);
    for (uVar6 = (uint)(byte)(bVar3 - bVar1); uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar16 = ((byte)(cVar4 + 0xb0U) >> 2) + 0x50;
      pcVar16 = pcVar16 + 1;
    }
    *(char **)(&DAT_00004a5c + iVar8 * 4) = pcVar16;
    iVar8 = iVar8 + -1;
    if (iVar8 == DAT_00003901) break;
LAB_00006dd0:
    uVar9 = uVar9 + 0x80000001;
    uVar10 = uVar10 + 0x7fffffff;
    uVar6 = (uVar10 >> 0x16) << 10 | uVar9 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar6) + -1);
    if (bVar3 <= (byte)(&DAT_00004e5f)[iVar8]) {
      iVar8 = iVar8 + -1;
      if (iVar8 == DAT_00003901) break;
      goto LAB_00006dd0;
    }
  }
  uRam00006d5d._1_2_ = uRam00006d5d._1_2_ + 1;
  uRam00006ddd._1_2_ = uRam00006ddd._1_2_ + 1;
  puVar13 = &DAT_00004e60;
  puVar17 = (undefined4 *)(uVar12 * 0x100 + _DAT_0000390d);
  for (uVar6 = DAT_000038f1 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *puVar17 = *puVar13;
    puVar13 = puVar13 + 1;
    puVar17 = puVar17 + 1;
  }
  uVar12 = uVar12 + 1;
  if (_FUN_00003a20 <= uVar12) {
    return;
  }
  goto LAB_00006ce0;
}


/* ===== FUN_00006c00 @ 00006c00 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00006c00(void)

{
  byte bVar1;
  int *piVar2;
  byte bVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  int iVar14;
  int *piVar15;
  char *pcVar16;
  undefined4 *puVar17;
  
  if (DAT_000090c0 != DAT_000090c4) {
    DAT_000090c4 = DAT_000090c0;
    FUN_0000395e();
  }
  iVar14 = (DAT_000090e0 & 0x7fffff) << 8;
  uVar12 = ~(DAT_000090e0 << 9) >> 1;
  iVar5 = (DAT_000090e0 >> 0x17) * 4;
  DAT_000090fc = (int)((ulonglong)
                       ((longlong)*(int *)(FUN_0000944b + iVar5 + 5) * (longlong)(int)uVar12) >>
                      0x20) +
                 (int)((ulonglong)((longlong)*(int *)(iVar5 + 0x9454) * (longlong)iVar14) >> 0x20);
  DAT_000090fc = DAT_000090fc * 2;
  DAT_00009100 = (int)((ulonglong)
                       ((longlong)*(int *)(&DAT_00009650 + iVar5) * (longlong)(int)uVar12) >> 0x20)
                 + (int)((ulonglong)((longlong)*(int *)(&DAT_00009654 + iVar5) * (longlong)iVar14)
                        >> 0x20);
  DAT_00009100 = DAT_00009100 * 2;
  _uRam00006d5d = CONCAT31((int3)((uint)LAB_00003909 >> 8),-(byte)((uint)DAT_000090dc >> 0x19));
  puVar13 = &DAT_00004e60;
  _uRam00006ddd = _uRam00006d5d;
  for (uVar12 = DAT_000038f1 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *puVar13 = 0;
    puVar13 = puVar13 + 1;
  }
  piVar15 = &DAT_00004a60;
  uVar12 = DAT_000038f1 >> 2;
  iVar5 = DAT_00003918;
  do {
    *piVar15 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15[1] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar2 = piVar15 + 3;
    piVar15[2] = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    piVar15 = piVar15 + 4;
    *piVar2 = iVar5;
    iVar5 = iVar5 + DAT_000038ed;
    uVar12 = uVar12 - 1;
  } while (uVar12 != 0);
  uVar12 = 0;
LAB_00006ce0:
  iVar14 = DAT_000085bc;
  uVar6 = *(uint *)(uVar12 * 4 + 0x4624) >> 1;
  uVar10 = DAT_000090d8 +
           (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_00009100) >> 0x20) * -2;
  uVar9 = DAT_000090d4 +
          (int)((ulonglong)((longlong)(int)uVar6 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar5 = *(int *)(uVar12 * 4 + 0x4224);
  uRam00006d52 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
  uRam00006d58 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
  iVar5 = ram0x000038fd;
  uVar6 = uVar9;
  uVar11 = uVar10;
  uRam00006dd2 = uRam00006d52;
  uRam00006dd8 = uRam00006d58;
  do {
    uVar7 = (uVar11 >> 0x16) << 10 | uVar6 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar7) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar5) < bVar3) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar5);
      *(byte *)((int)&DAT_00004e60 + iVar5) = bVar3;
      UNLOCK();
      cVar4 = *(char *)(iVar14 + 0x100000 + uVar7);
      pcVar16 = (char *)(&DAT_00004a60)[iVar5];
      for (uVar7 = (uint)(byte)(bVar3 - bVar1); uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar16 = ((byte)(cVar4 + 0xb0U) >> 2) + 0x50;
        pcVar16 = pcVar16 + 1;
      }
      (&DAT_00004a60)[iVar5] = pcVar16;
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    else {
      iVar8 = ram0x000038fd;
      if (iVar5 + 1 == DAT_00003905) break;
    }
    iVar5 = iVar5 + 1;
    uVar6 = uVar6 + 0x7fffffff;
    uVar11 = uVar11 + 0x80000001;
  } while( true );
  while( true ) {
    LOCK();
    bVar1 = (&DAT_00004e5f)[iVar8];
    (&DAT_00004e5f)[iVar8] = bVar3;
    UNLOCK();
    cVar4 = *(char *)(iVar14 + 0x100000 + uVar6);
    pcVar16 = *(char **)(&DAT_00004a5c + iVar8 * 4);
    for (uVar6 = (uint)(byte)(bVar3 - bVar1); uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar16 = ((byte)(cVar4 + 0xb0U) >> 2) + 0x50;
      pcVar16 = pcVar16 + 1;
    }
    *(char **)(&DAT_00004a5c + iVar8 * 4) = pcVar16;
    iVar8 = iVar8 + -1;
    if (iVar8 == DAT_00003901) break;
LAB_00006dd0:
    uVar9 = uVar9 + 0x80000001;
    uVar10 = uVar10 + 0x7fffffff;
    uVar6 = (uVar10 >> 0x16) << 10 | uVar9 >> 0x16;
    bVar3 = *(byte *)CONCAT31(0x7fffff,*(char *)(iVar14 + uVar6) + -1);
    if (bVar3 <= (byte)(&DAT_00004e5f)[iVar8]) {
      iVar8 = iVar8 + -1;
      if (iVar8 == DAT_00003901) break;
      goto LAB_00006dd0;
    }
  }
  uRam00006d5d._1_2_ = uRam00006d5d._1_2_ + 1;
  uRam00006ddd._1_2_ = uRam00006ddd._1_2_ + 1;
  puVar13 = &DAT_00004e60;
  puVar17 = (undefined4 *)(uVar12 * 0x100 + _DAT_0000390d);
  for (uVar6 = DAT_000038f1 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *puVar17 = *puVar13;
    puVar13 = puVar13 + 1;
    puVar17 = puVar17 + 1;
  }
  uVar12 = uVar12 + 1;
  if (_FUN_00003a20 <= uVar12) {
    return;
  }
  goto LAB_00006ce0;
}


/* ===== FUN_00006d83 @ 00006d83 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00006d83(byte param_1,uint param_2,int param_3,uint param_4,uint param_5,int param_6,
            uint param_7,uint param_8,uint param_9)

{
  byte bVar1;
  byte bVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  char *pcVar7;
  undefined4 *puVar8;
  
LAB_00006d90:
  LOCK();
  bVar1 = *(byte *)((int)&DAT_00004e60 + param_3);
  *(byte *)((int)&DAT_00004e60 + param_3) = param_1;
  UNLOCK();
  cVar3 = *(char *)(param_6 + 0x100000 + param_2);
  pcVar7 = (char *)(&DAT_00004a60)[param_3];
  for (uVar4 = (uint)(byte)(param_1 - bVar1); uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar7 = ((byte)(cVar3 + 0xb0U) >> 2) + 0x50;
    pcVar7 = pcVar7 + 1;
  }
  (&DAT_00004a60)[param_3] = pcVar7;
  param_3 = param_3 + 1;
  iVar5 = ram0x000038fd;
  if (param_3 != DAT_00003905) goto LAB_00006d50;
LAB_00006dd0:
  do {
    param_8 = param_8 + 0x80000001;
    param_7 = param_7 + 0x7fffffff;
    uVar4 = (param_7 >> 0x16) << 10 | param_8 >> 0x16;
    bVar1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + uVar4) + -1);
    if ((byte)(&DAT_00004e5f)[iVar5] < bVar1) goto LAB_00006e10;
    iVar5 = iVar5 + -1;
  } while (iVar5 != DAT_00003901);
  goto LAB_00006e44;
LAB_00006e10:
  LOCK();
  bVar2 = (&DAT_00004e5f)[iVar5];
  (&DAT_00004e5f)[iVar5] = bVar1;
  UNLOCK();
  cVar3 = *(char *)(param_6 + 0x100000 + uVar4);
  pcVar7 = *(char **)(&DAT_00004a5c + iVar5 * 4);
  for (uVar4 = (uint)(byte)(bVar1 - bVar2); uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar7 = ((byte)(cVar3 + 0xb0U) >> 2) + 0x50;
    pcVar7 = pcVar7 + 1;
  }
  *(char **)(&DAT_00004a5c + iVar5 * 4) = pcVar7;
  iVar5 = iVar5 + -1;
  if (iVar5 == DAT_00003901) {
LAB_00006e44:
    sRam00006d5e = sRam00006d5e + 1;
    sRam00006dde = sRam00006dde + 1;
    puVar6 = &DAT_00004e60;
    puVar8 = (undefined4 *)(param_9 * 0x100 + _DAT_0000390d);
    for (uVar4 = DAT_000038f1 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar8 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar8 = puVar8 + 1;
    }
    param_9 = param_9 + 1;
    if (_FUN_00003a20 <= param_9) {
      return;
    }
    uVar4 = *(uint *)(param_9 * 4 + 0x4624) >> 1;
    param_5 = DAT_000090d8 +
              (int)((ulonglong)((longlong)(int)uVar4 * (longlong)DAT_00009100) >> 0x20) * -2;
    param_4 = DAT_000090d4 +
              (int)((ulonglong)((longlong)(int)uVar4 * (longlong)DAT_000090fc) >> 0x20) * -2;
    iVar5 = *(int *)(param_9 * 4 + 0x4224);
    uRam00006d52 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_00009100) >> 0x20);
    uRam00006d58 = (undefined4)((ulonglong)((longlong)iVar5 * (longlong)DAT_000090fc) >> 0x20);
    param_3 = ram0x000038fd;
    param_6 = DAT_000085bc;
    uRam00006dd2 = uRam00006d52;
    uRam00006dd8 = uRam00006d58;
    param_7 = param_5;
    param_8 = param_4;
    while( true ) {
      param_2 = (param_5 >> 0x16) << 10 | param_4 >> 0x16;
      param_1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + param_2) + -1);
      if (*(byte *)((int)&DAT_00004e60 + param_3) < param_1) goto LAB_00006d90;
      param_3 = param_3 + 1;
      iVar5 = ram0x000038fd;
      if (param_3 == DAT_00003905) break;
LAB_00006d50:
      param_4 = param_4 + 0x7fffffff;
      param_5 = param_5 + 0x80000001;
    }
  }
  goto LAB_00006dd0;
}


/* ===== FUN_00006e03 @ 00006e03 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00006e03(byte param_1,uint param_2,int param_3,uint param_4,uint param_5,int param_6,
            uint param_7)

{
  byte bVar1;
  byte bVar2;
  char cVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  char *pcVar9;
  undefined4 *puVar10;
  
LAB_00006e10:
  LOCK();
  bVar2 = (&DAT_00004e5f)[param_3];
  (&DAT_00004e5f)[param_3] = param_1;
  UNLOCK();
  cVar3 = *(char *)(param_6 + 0x100000 + param_2);
  pcVar9 = *(char **)(&DAT_00004a5c + param_3 * 4);
  for (uVar5 = (uint)(byte)(param_1 - bVar2); uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar9 = ((byte)(cVar3 + 0xb0U) >> 2) + 0x50;
    pcVar9 = pcVar9 + 1;
  }
  *(char **)(&DAT_00004a5c + param_3 * 4) = pcVar9;
  param_3 = param_3 + -1;
  if (param_3 != DAT_00003901) goto LAB_00006dd0;
LAB_00006e44:
  sRam00006d5e = sRam00006d5e + 1;
  sRam00006dde = sRam00006dde + 1;
  puVar8 = &DAT_00004e60;
  puVar10 = (undefined4 *)(param_7 * 0x100 + _DAT_0000390d);
  for (uVar5 = DAT_000038f1 >> 2; param_6 = DAT_000085bc, uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar10 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar10 = puVar10 + 1;
  }
  param_7 = param_7 + 1;
  if (_FUN_00003a20 <= param_7) {
    return;
  }
  uVar5 = *(uint *)(param_7 * 4 + 0x4624) >> 1;
  param_5 = DAT_000090d8 +
            (int)((ulonglong)((longlong)(int)uVar5 * (longlong)DAT_00009100) >> 0x20) * -2;
  param_4 = DAT_000090d4 +
            (int)((ulonglong)((longlong)(int)uVar5 * (longlong)DAT_000090fc) >> 0x20) * -2;
  iVar6 = *(int *)(param_7 * 4 + 0x4224);
  uRam00006d52 = (undefined4)((ulonglong)((longlong)iVar6 * (longlong)DAT_00009100) >> 0x20);
  uRam00006d58 = (undefined4)((ulonglong)((longlong)iVar6 * (longlong)DAT_000090fc) >> 0x20);
  iVar6 = ram0x000038fd;
  uVar5 = param_4;
  uVar7 = param_5;
  uRam00006dd2 = uRam00006d52;
  uRam00006dd8 = uRam00006d58;
  do {
    uVar4 = (uVar7 >> 0x16) << 10 | uVar5 >> 0x16;
    bVar2 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + uVar4) + -1);
    if (*(byte *)((int)&DAT_00004e60 + iVar6) < bVar2) {
      LOCK();
      bVar1 = *(byte *)((int)&DAT_00004e60 + iVar6);
      *(byte *)((int)&DAT_00004e60 + iVar6) = bVar2;
      UNLOCK();
      cVar3 = *(char *)(param_6 + 0x100000 + uVar4);
      pcVar9 = (char *)(&DAT_00004a60)[iVar6];
      for (uVar4 = (uint)(byte)(bVar2 - bVar1); uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar9 = ((byte)(cVar3 + 0xb0U) >> 2) + 0x50;
        pcVar9 = pcVar9 + 1;
      }
      (&DAT_00004a60)[iVar6] = pcVar9;
      param_3 = ram0x000038fd;
      if (iVar6 + 1 == DAT_00003905) break;
    }
    else {
      param_3 = ram0x000038fd;
      if (iVar6 + 1 == DAT_00003905) break;
    }
    iVar6 = iVar6 + 1;
    uVar5 = uVar5 + 0x7fffffff;
    uVar7 = uVar7 + 0x80000001;
  } while( true );
LAB_00006dd0:
  do {
    param_4 = param_4 + 0x80000001;
    param_5 = param_5 + 0x7fffffff;
    param_2 = (param_5 >> 0x16) << 10 | param_4 >> 0x16;
    param_1 = *(byte *)CONCAT31(0x7fffff,*(char *)(param_6 + param_2) + -1);
    if ((byte)(&DAT_00004e5f)[param_3] < param_1) goto LAB_00006e10;
    param_3 = param_3 + -1;
  } while (param_3 != DAT_00003901);
  goto LAB_00006e44;
}


/* ===== FUN_00006e95 @ 00006e95 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00006e95(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined4 param_5,undefined2 param_6)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  
  uVar4 = param_5;
  FUN_00006f17(param_1,param_2,param_3,param_4,param_5,param_6);
  FUN_00003322(&LAB_0000bc98,uVar4,0x6ea6);
  iVar1 = FUN_000036bf(1,0,&LAB_0000bc98,1,0x6ebd);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),64000,&DAT_00006e88,4,0x6ed4);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x300,&DAT_00006e90,4,0x6eeb);
  uVar2 = 0x6e80;
  uVar3 = 4;
  FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0,&DAT_00006e80,4,0x6eff);
  DAT_00006e94 = 0;
  _DAT_00006e84 = 0;
  FUN_0000701e(uVar2,uVar3,param_4,param_5,param_6);
  return;
}


/* ===== FUN_00006f17 @ 00006f17 ===== */

void __allregs
FUN_00006f17(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined4 param_5,undefined2 param_6)

{
  FUN_0000706b(param_1,param_2,param_3,param_4,(short)param_5,param_6);
  FUN_00003376(&DAT_00006e80,param_5,0x6f27);
  FUN_00003376(&DAT_00006e88,param_5,0x6f32);
  FUN_00003376(&DAT_00006e90,param_5,0x6f3d);
  return;
}


/* ===== FUN_00006f3e @ 00006f3e ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs FUN_00006f3e(undefined2 param_1,int *param_2,int *param_3)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 in_CF;
  
  while( true ) {
    while( true ) {
      FUN_0000708b(param_1,param_2,(short)param_3);
      if ((bool)in_CF) {
        return 0xffffffff;
      }
      iVar2 = *DAT_00006e80;
      if (iVar2 != 0x4b445678) break;
      DAT_00005578 = DAT_00006e80[2];
      DAT_0000557c = DAT_00006e80[3];
      _DAT_00006e84 = DAT_00006e80[4];
      in_CF = 0;
    }
    if (iVar2 != 0x4b445670) break;
    in_CF = (int *)0xfffffff7 < DAT_00006e80;
    param_2 = DAT_00006e80 + 2;
    param_3 = DAT_00006e90;
    for (iVar2 = 0xc0; iVar2 != 0; iVar2 = iVar2 + -1) {
      *param_3 = *param_2;
      param_2 = param_2 + 1;
      param_3 = param_3 + 1;
    }
    DAT_00006e94 = '\x01';
  }
  if (iVar2 == 0x4b445669) {
    FUN_00007135((char)(DAT_00006e80 + 2),DAT_00006e80 + 2,DAT_00006e88);
    if (DAT_00006e94 == '\x01') {
      FUN_0000746b();
      DAT_00006e94 = '\0';
    }
    FUN_00007120();
    return 0;
  }
  uVar1 = thunk_FUN_00000f64(FUN_00006fcf);
  return uVar1;
}


/* ===== FUN_00006fcf @ 00006fcf ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Stack frame is not setup normally: Input value of stackpointer is not used */
/* WARNING: Removing unreachable block (ram,0x00007053) */

void __allregs
FUN_00006fcf(int param_1,int param_2,undefined2 param_3,undefined1 *param_4,undefined1 *param_5)

{
  byte *pbVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  uint uVar4;
  char *pcVar5;
  int iVar6;
  undefined2 uVar7;
  undefined4 uVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  bool in_CF;
  
  out(*param_4,param_3);
  if (!in_CF && param_2 != 1) {
    uVar2 = in(param_3);
    *param_5 = uVar2;
    puVar9 = (undefined4 *)(*(int *)(param_1 + 0x44) * 0x74696769);
    puVar10 = (undefined4 *)puVar9[1];
    uVar8 = puVar9[5];
    iVar6 = puVar9[6];
    uVar3 = puVar9[7];
    uVar7 = (undefined2)uVar8;
    uVar2 = in(uVar7);
    *(undefined1 *)*puVar9 = uVar2;
    pbVar1 = (byte *)((int)puVar10 + 0x69);
    *pbVar1 = *pbVar1 & (byte)uVar8;
    out(*puVar10,uVar7);
    *(byte *)(iVar6 + 0x74) = *(byte *)(iVar6 + 0x74) & (byte)uVar3;
    *(byte *)(puVar9[0xc] + 0x74) = *(byte *)(puVar9[0xc] + 0x74) & (byte)puVar9[0xd];
    puVar10 = (undefined4 *)puVar9[0x11];
    uVar8 = puVar9[0x15];
    uVar4 = puVar9[0x17];
    uVar7 = (undefined2)uVar8;
    uVar3 = in(uVar7);
    *(undefined4 *)puVar9[0x10] = uVar3;
    pcVar5 = (char *)(uVar4 | 0xcfba240a);
    out(*puVar10,uVar7);
    *pcVar5 = *pcVar5 + (char)pcVar5;
    thunk_FUN_00000f64(uVar8);
    return;
  }
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_00007010 @ 00007010 ===== */

/* WARNING: Instruction at (ram,0x00007011) overlaps instruction at (ram,0x00007010)
    */

void __allregs FUN_00007010(undefined4 *param_1,byte *param_2,byte *param_3)

{
  byte bVar1;
  byte bVar2;
  byte *pbVar3;
  char cVar4;
  uint uVar5;
  BADSPACEBASE *in_ESP;
  byte *pbVar6;
  undefined4 uStack_8;
  byte abStack_4 [4];
  
  pbVar6 = abStack_4;
  cVar4 = '\t';
  do {
    param_1 = param_1 + -1;
    pbVar6 = (byte *)((int)pbVar6 + -4);
    *(undefined4 *)pbVar6 = *param_1;
    cVar4 = cVar4 + -1;
  } while ('\0' < cVar4);
  do {
    pbVar6 = param_2;
    if (abStack_4 <= param_3) {
      return;
    }
    while( true ) {
      bVar1 = *pbVar6;
      if (0xbf < bVar1) break;
      pbVar3 = param_3 + 1;
      *param_3 = bVar1;
      pbVar6 = pbVar6 + 1;
      param_3 = pbVar3;
      if (abStack_4 <= pbVar3) {
        return;
      }
    }
    param_2 = pbVar6 + 2;
    bVar2 = pbVar6[1];
    for (uVar5 = (uint)(bVar1 & 0x3f); uVar5 != 0; uVar5 = uVar5 - 1) {
      *param_3 = bVar2;
      param_3 = param_3 + 1;
    }
  } while( true );
}


/* ===== FUN_0000701e @ 0000701e ===== */

/* WARNING: Removing unreachable block (ram,0x00005e48) */

void __allregs
FUN_0000701e(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined4 param_4,
            undefined2 param_5)

{
  undefined1 uVar1;
  undefined2 uVar2;
  undefined2 unaff_ES;
  bool bVar3;
  
  bVar3 = false;
  DAT_00005570 = 0;
  uVar2 = 0x33;
  uVar1 = FUN_00005cc2(0x4e00,0x33,param_2,param_3,param_4,param_5);
  if (!bVar3) {
    DAT_00000937 = *(undefined4 *)(DAT_00000927 + 0x1a);
    uVar1 = FUN_00005cc2(0x3d00,uVar2,(short)DAT_00000927,param_3,param_4,param_5);
    if (!bVar3) {
      DAT_0000701c = DAT_00005cc0;
      DAT_00005cc0 = 0;
      return;
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,uVar1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (undefined2)param_4;
    uRam000f000e = 0x1c00;
    uRam000f0004 = uVar2;
    uRam000f0006 = param_1;
    uRam000f000a = param_5;
    uRam000f000c = param_3;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  thunk_FUN_00000f64(1);
  return;
}


/* ===== FUN_0000706b @ 0000706b ===== */

/* WARNING: Removing unreachable block (ram,0x00007079) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

void __allregs
FUN_0000706b(undefined1 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
            undefined2 param_5,undefined2 param_6)

{
  undefined2 unaff_ES;
  undefined2 uVar1;
  
  uVar1 = 0x1c00;
  if (DAT_0000701c != 0) {
    uRam000f0000 = CONCAT11(0x3e,param_1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_0000701c;
    uRam000f000e = 0x1c00;
    uRam000f0004 = param_2;
    uRam000f0006 = param_3;
    uRam000f0008 = param_5;
    uRam000f000a = param_6;
    uRam000f000c = param_4;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    if ((bool)uRam000f0012) {
      if (DAT_00005cc0 != 0) {
        uRam000f0000 = CONCAT11(0x3e,(char)uRam000f0000);
        uRam000f0014 = 0x21;
        sRam000f0002 = DAT_00005cc0;
        uRam000f000c = param_4;
        uRam000f000e = uVar1;
        uRam000f0010 = unaff_ES;
        (*(code *)0xf0100)();
        DAT_00005cc0 = 0;
      }
      thunk_FUN_00000f64(1);
      return;
    }
    DAT_0000701c = 0;
  }
  return;
}


/* ===== FUN_0000708b @ 0000708b ===== */

/* WARNING: Removing unreachable block (ram,0x0000710d) */
/* WARNING: Removing unreachable block (ram,0x000070c4) */
/* WARNING: Removing unreachable block (ram,0x00005e48) */

int __allregs FUN_0000708b(undefined2 param_1,undefined4 param_2,undefined2 param_3)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined2 unaff_ES;
  undefined2 uVar4;
  undefined2 extraout_var;
  
  uVar4 = 0x1c00;
  if (DAT_00000937 < DAT_00005570 + 8U) {
    return 0;
  }
  iVar1 = FUN_0000345c(8,&DAT_00006e80,param_2,0x70b0);
  uRam000f0000 = CONCAT11(0x3f,(char)iVar1);
  uRam000f0014 = 0x21;
  sRam000f0002 = DAT_0000701c;
  sRam000f0004 = 8;
  sRam000f0006 = (short)DAT_00006e80;
  uRam000f0008 = (ushort)param_2;
  uRam000f000e = 0x1c00;
  uRam000f000a = param_3;
  uRam000f000c = param_1;
  uRam000f0010 = unaff_ES;
  (*(code *)0xf0100)();
  uVar2 = CONCAT22(extraout_var,uRam000f0000);
  uVar3 = CONCAT22((short)((uint)param_2 >> 0x10),uRam000f0008);
  if (!(bool)uRam000f0012) {
    DAT_00005574 = *(int *)(DAT_00006e80 + 4);
    iVar1 = FUN_00003439(uVar2,DAT_00005574,&DAT_00006e80,DAT_00006e80,uVar3,0x70e6);
    DAT_00005570 = DAT_00005570 + DAT_00005574;
    sRam000f0006 = (short)DAT_00006e80 + 8;
    sRam000f0004 = (short)DAT_00005574 + -8;
    uRam000f0000 = CONCAT11(0x3f,(char)iVar1);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_0000701c;
    uRam000f0008 = (ushort)uVar3;
    uRam000f000c = param_1;
    uRam000f000e = uVar4;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    uVar2 = (uint)uRam000f0000;
    uVar3 = (uint)uRam000f0008;
    if (!(bool)uRam000f0012) {
      return DAT_00005574;
    }
  }
  if (DAT_00005cc0 != 0) {
    uRam000f0000 = CONCAT11(0x3e,(char)uVar2);
    uRam000f0014 = 0x21;
    sRam000f0002 = DAT_00005cc0;
    uRam000f0008 = (ushort)uVar3;
    uRam000f000c = param_1;
    uRam000f000e = uVar4;
    uRam000f0010 = unaff_ES;
    (*(code *)0xf0100)();
    DAT_00005cc0 = 0;
  }
  iVar1 = thunk_FUN_00000f64(1);
  return iVar1;
}


/* ===== FUN_00007120 @ 00007120 ===== */

void __allregs FUN_00007120(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = DAT_00006e88;
  puVar3 = DAT_00000917;
  for (iVar1 = 16000; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  return;
}


/* ===== FUN_00007135 @ 00007135 ===== */

void __allregs FUN_00007135(byte param_1,char *param_2,char *param_3)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  char cVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  byte bVar8;
  uint uVar9;
  uint uVar10;
  char *pcVar11;
  
  uVar3 = DAT_00005578;
  iVar2 = DAT_00005578 * 3;
  uVar6 = DAT_0000557c >> 2;
  uVar10 = (uint)param_1;
  do {
    uVar7 = uVar3 >> 2;
    do {
      uVar9 = uVar10 & 0xffff03ff;
      pcVar11 = param_2;
      if ((uVar10 & 0x300) == 0) {
        uVar9 = CONCAT31((int3)(uVar9 >> 8),*param_2);
        pcVar11 = param_2 + 1;
      }
      if ((uVar9 & 2) == 0) {
        if ((uVar9 & 1) == 0) {
          uVar10 = *(uint *)(pcVar11 + 3);
          if ((uVar10 & 3) != 0) {
            *param_3 = pcVar11[(uVar10 & 3) - 1];
          }
          if ((uVar10 >> 2 & 3) != 0) {
            param_3[1] = pcVar11[(uVar10 >> 2 & 3) - 1];
          }
          if ((uVar10 >> 4 & 3) != 0) {
            param_3[2] = pcVar11[(uVar10 >> 4 & 3) - 1];
          }
          if ((uVar10 >> 6 & 3) != 0) {
            param_3[3] = pcVar11[(uVar10 >> 6 & 3) - 1];
          }
          if ((uVar10 & 0x300) != 0) {
            param_3[uVar3] = pcVar11[(uVar10 >> 8 & 3) - 1];
          }
          if ((uVar10 >> 10 & 3) != 0) {
            param_3[uVar3 + 1] = pcVar11[(uVar10 >> 10 & 3) - 1];
          }
          if ((uVar10 >> 0xc & 3) != 0) {
            param_3[uVar3 + 2] = pcVar11[(uVar10 >> 0xc & 3) - 1];
          }
          if ((uVar10 >> 0xe & 3) != 0) {
            param_3[uVar3 + 3] = pcVar11[(uVar10 >> 0xe & 3) - 1];
          }
          if ((uVar10 & 0x30000) != 0) {
            param_3[uVar3 * 2] = pcVar11[(uVar10 >> 0x10 & 3) - 1];
          }
          if ((uVar10 >> 0x12 & 3) != 0) {
            param_3[uVar3 * 2 + 1] = pcVar11[(uVar10 >> 0x12 & 3) - 1];
          }
          if ((uVar10 >> 0x14 & 3) != 0) {
            param_3[uVar3 * 2 + 2] = pcVar11[(uVar10 >> 0x14 & 3) - 1];
          }
          if ((uVar10 >> 0x16 & 3) != 0) {
            param_3[uVar3 * 2 + 3] = pcVar11[(uVar10 >> 0x16 & 3) - 1];
          }
          bVar8 = (byte)(uVar10 >> 0x18);
          if ((uVar10 & 0x3000000) != 0) {
            param_3[iVar2] = pcVar11[(bVar8 & 0xffffff03) - 1];
          }
          if ((bVar8 >> 2 & 3) != 0) {
            param_3[iVar2 + 1] = pcVar11[(bVar8 >> 2 & 0xffffff03) - 1];
          }
          if ((bVar8 >> 4 & 3) != 0) {
            param_3[iVar2 + 2] = pcVar11[(bVar8 >> 4 & 0xffffff03) - 1];
          }
          if (bVar8 >> 6 != 0) {
            param_3[iVar2 + 3] = pcVar11[(bVar8 >> 6) - 1];
          }
          param_2 = pcVar11 + 7;
        }
        else {
          uVar1 = *(ushort *)(pcVar11 + 2);
          if (pcVar11[(uVar1 & 1) != 0] != '\0') {
            *param_3 = pcVar11[(uVar1 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 1 & 1) != 0] != '\0') {
            param_3[1] = pcVar11[(uVar1 >> 1 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 2 & 1) != 0] != '\0') {
            param_3[2] = pcVar11[(uVar1 >> 2 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 3 & 1) != 0] != '\0') {
            param_3[3] = pcVar11[(uVar1 >> 3 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 4 & 1) != 0] != '\0') {
            param_3[uVar3] = pcVar11[(uVar1 >> 4 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 5 & 1) != 0] != '\0') {
            param_3[uVar3 + 1] = pcVar11[(uVar1 >> 5 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 6 & 1) != 0] != '\0') {
            param_3[uVar3 + 2] = pcVar11[(uVar1 >> 6 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 7 & 1) != 0] != '\0') {
            param_3[uVar3 + 3] = pcVar11[(uVar1 >> 7 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 8 & 1) != 0] != '\0') {
            param_3[uVar3 * 2] = pcVar11[(uVar1 >> 8 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 9 & 1) != 0] != '\0') {
            param_3[uVar3 * 2 + 1] = pcVar11[(uVar1 >> 9 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 10 & 1) != 0] != '\0') {
            param_3[uVar3 * 2 + 2] = pcVar11[(uVar1 >> 10 & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 0xb & 1) != 0] != '\0') {
            param_3[uVar3 * 2 + 3] = pcVar11[(uVar1 >> 0xb & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 0xc & 1) != 0] != '\0') {
            param_3[iVar2] = pcVar11[(uVar1 >> 0xc & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 0xd & 1) != 0] != '\0') {
            param_3[iVar2 + 1] = pcVar11[(uVar1 >> 0xd & 1) != 0];
          }
          if (pcVar11[(uVar1 >> 0xe & 1) != 0] != '\0') {
            param_3[iVar2 + 2] = pcVar11[(uVar1 >> 0xe & 1) != 0];
          }
          if (pcVar11[(short)uVar1 < 0] != '\0') {
            param_3[iVar2 + 3] = pcVar11[(short)uVar1 < 0];
          }
          param_2 = pcVar11 + 4;
        }
      }
      else {
        param_2 = pcVar11;
        if ((uVar9 & 1) == 0) {
          param_2 = pcVar11 + 1;
          cVar4 = *pcVar11;
          if (cVar4 == '\0') {
            uVar5 = *(undefined4 *)param_2;
            if ((char)uVar5 != '\0') {
              *param_3 = (char)uVar5;
            }
            cVar4 = (char)((uint)uVar5 >> 8);
            if (cVar4 != '\0') {
              param_3[1] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x10);
            if (cVar4 != '\0') {
              param_3[2] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x18);
            if (cVar4 != '\0') {
              param_3[3] = cVar4;
            }
            uVar5 = *(undefined4 *)(pcVar11 + 5);
            if ((char)uVar5 != '\0') {
              param_3[uVar3] = (char)uVar5;
            }
            cVar4 = (char)((uint)uVar5 >> 8);
            if (cVar4 != '\0') {
              param_3[uVar3 + 1] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x10);
            if (cVar4 != '\0') {
              param_3[uVar3 + 2] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x18);
            if (cVar4 != '\0') {
              param_3[uVar3 + 3] = cVar4;
            }
            uVar5 = *(undefined4 *)(pcVar11 + 9);
            if ((char)uVar5 != '\0') {
              param_3[uVar3 * 2] = (char)uVar5;
            }
            cVar4 = (char)((uint)uVar5 >> 8);
            if (cVar4 != '\0') {
              param_3[uVar3 * 2 + 1] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x10);
            if (cVar4 != '\0') {
              param_3[uVar3 * 2 + 2] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x18);
            if (cVar4 != '\0') {
              param_3[uVar3 * 2 + 3] = cVar4;
            }
            param_2 = pcVar11 + 0x11;
            uVar5 = *(undefined4 *)(pcVar11 + 0xd);
            if ((char)uVar5 != '\0') {
              param_3[iVar2] = (char)uVar5;
            }
            cVar4 = (char)((uint)uVar5 >> 8);
            if (cVar4 != '\0') {
              param_3[iVar2 + 1] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x10);
            if (cVar4 != '\0') {
              param_3[iVar2 + 2] = cVar4;
            }
            cVar4 = (char)((uint)uVar5 >> 0x18);
            if (cVar4 != '\0') {
              param_3[iVar2 + 3] = cVar4;
            }
          }
          else {
            uVar5 = CONCAT22(CONCAT11(cVar4,cVar4),CONCAT11(cVar4,cVar4));
            *(undefined4 *)param_3 = uVar5;
            *(undefined4 *)(param_3 + uVar3) = uVar5;
            *(undefined4 *)(param_3 + uVar3 * 2) = uVar5;
            *(undefined4 *)(param_3 + iVar2) = uVar5;
          }
        }
      }
      param_3 = param_3 + 4;
      uVar10 = (uint)CONCAT11((char)(uVar9 >> 8) + '\x01',(byte)uVar9 >> 2);
      uVar7 = uVar7 - 1;
    } while (uVar7 != 0);
    param_3 = param_3 + iVar2;
    uVar6 = uVar6 - 1;
  } while (uVar6 != 0);
  return;
}


/* ===== FUN_000073ad @ 000073ad ===== */

void __allregs
FUN_000073ad(uint param_1,uint param_2,int param_3,char *param_4,char *param_5,uint param_6,
            int param_7)

{
  ushort uVar1;
  char cVar2;
  undefined4 uVar3;
  byte bVar4;
  uint uVar5;
  char *pcVar6;
  
  do {
    uVar3 = *(undefined4 *)param_4;
    if ((char)uVar3 != '\0') {
      *param_5 = (char)uVar3;
    }
    cVar2 = (char)((uint)uVar3 >> 8);
    if (cVar2 != '\0') {
      param_5[1] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x10);
    if (cVar2 != '\0') {
      param_5[2] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x18);
    if (cVar2 != '\0') {
      param_5[3] = cVar2;
    }
    uVar3 = *(undefined4 *)(param_4 + 4);
    if ((char)uVar3 != '\0') {
      param_5[param_2] = (char)uVar3;
    }
    cVar2 = (char)((uint)uVar3 >> 8);
    if (cVar2 != '\0') {
      param_5[param_2 + 1] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x10);
    if (cVar2 != '\0') {
      param_5[param_2 + 2] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x18);
    if (cVar2 != '\0') {
      param_5[param_2 + 3] = cVar2;
    }
    uVar3 = *(undefined4 *)(param_4 + 8);
    if ((char)uVar3 != '\0') {
      param_5[param_2 * 2] = (char)uVar3;
    }
    cVar2 = (char)((uint)uVar3 >> 8);
    if (cVar2 != '\0') {
      param_5[param_2 * 2 + 1] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x10);
    if (cVar2 != '\0') {
      param_5[param_2 * 2 + 2] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x18);
    if (cVar2 != '\0') {
      param_5[param_2 * 2 + 3] = cVar2;
    }
    pcVar6 = param_4 + 0x10;
    uVar3 = *(undefined4 *)(param_4 + 0xc);
    if ((char)uVar3 != '\0') {
      param_5[param_3] = (char)uVar3;
    }
    cVar2 = (char)((uint)uVar3 >> 8);
    if (cVar2 != '\0') {
      param_5[param_3 + 1] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x10);
    if (cVar2 != '\0') {
      param_5[param_3 + 2] = cVar2;
    }
    cVar2 = (char)((uint)uVar3 >> 0x18);
    param_4 = pcVar6;
    if (cVar2 != '\0') {
      param_5[param_3 + 3] = cVar2;
    }
    while( true ) {
      do {
        while( true ) {
          param_5 = param_5 + 4;
          param_1 = param_1 - 1;
          if (param_1 == 0) {
            param_5 = param_5 + param_3;
            param_7 = param_7 + -1;
            if (param_7 == 0) {
              return;
            }
            param_1 = param_2 >> 2;
          }
          uVar5 = CONCAT11((char)(param_6 >> 8),(byte)param_6 >> 2) & 0xffff03ff;
          pcVar6 = param_4;
          if ((param_6 & 0x300) == 0) {
            uVar5 = CONCAT31((int3)(uVar5 >> 8),*param_4);
            pcVar6 = param_4 + 1;
          }
          param_6 = (uint)CONCAT11((char)(uVar5 >> 8) + '\x01',(char)uVar5);
          if ((uVar5 & 2) != 0) break;
          if ((uVar5 & 1) == 0) {
            uVar5 = *(uint *)(pcVar6 + 3);
            if ((uVar5 & 3) != 0) {
              *param_5 = pcVar6[(uVar5 & 3) - 1];
            }
            if ((uVar5 >> 2 & 3) != 0) {
              param_5[1] = pcVar6[(uVar5 >> 2 & 3) - 1];
            }
            if ((uVar5 >> 4 & 3) != 0) {
              param_5[2] = pcVar6[(uVar5 >> 4 & 3) - 1];
            }
            if ((uVar5 >> 6 & 3) != 0) {
              param_5[3] = pcVar6[(uVar5 >> 6 & 3) - 1];
            }
            if ((uVar5 & 0x300) != 0) {
              param_5[param_2] = pcVar6[(uVar5 >> 8 & 3) - 1];
            }
            if ((uVar5 >> 10 & 3) != 0) {
              param_5[param_2 + 1] = pcVar6[(uVar5 >> 10 & 3) - 1];
            }
            if ((uVar5 >> 0xc & 3) != 0) {
              param_5[param_2 + 2] = pcVar6[(uVar5 >> 0xc & 3) - 1];
            }
            if ((uVar5 >> 0xe & 3) != 0) {
              param_5[param_2 + 3] = pcVar6[(uVar5 >> 0xe & 3) - 1];
            }
            if ((uVar5 & 0x30000) != 0) {
              param_5[param_2 * 2] = pcVar6[(uVar5 >> 0x10 & 3) - 1];
            }
            if ((uVar5 >> 0x12 & 3) != 0) {
              param_5[param_2 * 2 + 1] = pcVar6[(uVar5 >> 0x12 & 3) - 1];
            }
            if ((uVar5 >> 0x14 & 3) != 0) {
              param_5[param_2 * 2 + 2] = pcVar6[(uVar5 >> 0x14 & 3) - 1];
            }
            if ((uVar5 >> 0x16 & 3) != 0) {
              param_5[param_2 * 2 + 3] = pcVar6[(uVar5 >> 0x16 & 3) - 1];
            }
            bVar4 = (byte)(uVar5 >> 0x18);
            if ((uVar5 & 0x3000000) != 0) {
              param_5[param_3] = pcVar6[(bVar4 & 0xffffff03) - 1];
            }
            if ((bVar4 >> 2 & 3) != 0) {
              param_5[param_3 + 1] = pcVar6[(bVar4 >> 2 & 0xffffff03) - 1];
            }
            if ((bVar4 >> 4 & 3) != 0) {
              param_5[param_3 + 2] = pcVar6[(bVar4 >> 4 & 0xffffff03) - 1];
            }
            if (bVar4 >> 6 != 0) {
              param_5[param_3 + 3] = pcVar6[(bVar4 >> 6) - 1];
            }
            param_4 = pcVar6 + 7;
          }
          else {
            uVar1 = *(ushort *)(pcVar6 + 2);
            if (pcVar6[(uVar1 & 1) != 0] != '\0') {
              *param_5 = pcVar6[(uVar1 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 1 & 1) != 0] != '\0') {
              param_5[1] = pcVar6[(uVar1 >> 1 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 2 & 1) != 0] != '\0') {
              param_5[2] = pcVar6[(uVar1 >> 2 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 3 & 1) != 0] != '\0') {
              param_5[3] = pcVar6[(uVar1 >> 3 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 4 & 1) != 0] != '\0') {
              param_5[param_2] = pcVar6[(uVar1 >> 4 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 5 & 1) != 0] != '\0') {
              param_5[param_2 + 1] = pcVar6[(uVar1 >> 5 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 6 & 1) != 0] != '\0') {
              param_5[param_2 + 2] = pcVar6[(uVar1 >> 6 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 7 & 1) != 0] != '\0') {
              param_5[param_2 + 3] = pcVar6[(uVar1 >> 7 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 8 & 1) != 0] != '\0') {
              param_5[param_2 * 2] = pcVar6[(uVar1 >> 8 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 9 & 1) != 0] != '\0') {
              param_5[param_2 * 2 + 1] = pcVar6[(uVar1 >> 9 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 10 & 1) != 0] != '\0') {
              param_5[param_2 * 2 + 2] = pcVar6[(uVar1 >> 10 & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 0xb & 1) != 0] != '\0') {
              param_5[param_2 * 2 + 3] = pcVar6[(uVar1 >> 0xb & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 0xc & 1) != 0] != '\0') {
              param_5[param_3] = pcVar6[(uVar1 >> 0xc & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 0xd & 1) != 0] != '\0') {
              param_5[param_3 + 1] = pcVar6[(uVar1 >> 0xd & 1) != 0];
            }
            if (pcVar6[(uVar1 >> 0xe & 1) != 0] != '\0') {
              param_5[param_3 + 2] = pcVar6[(uVar1 >> 0xe & 1) != 0];
            }
            if (pcVar6[(short)uVar1 < 0] != '\0') {
              param_5[param_3 + 3] = pcVar6[(short)uVar1 < 0];
            }
            param_4 = pcVar6 + 4;
          }
        }
        param_4 = pcVar6;
      } while ((uVar5 & 1) != 0);
      param_4 = pcVar6 + 1;
      cVar2 = *pcVar6;
      if (cVar2 == '\0') break;
      uVar3 = CONCAT22(CONCAT11(cVar2,cVar2),CONCAT11(cVar2,cVar2));
      *(undefined4 *)param_5 = uVar3;
      *(undefined4 *)(param_5 + param_2) = uVar3;
      *(undefined4 *)(param_5 + param_2 * 2) = uVar3;
      *(undefined4 *)(param_5 + param_3) = uVar3;
    }
  } while( true );
}


/* ===== FUN_0000746b @ 0000746b ===== */

byte __allregs FUN_0000746b(void)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  
  out(0x3c8,0);
  iVar2 = 0x300;
  pbVar3 = DAT_00006e90;
  do {
    bVar1 = *pbVar3;
    out(0x3c9,bVar1 >> 2);
    iVar2 = iVar2 + -1;
    pbVar3 = pbVar3 + 1;
  } while (iVar2 != 0);
  return bVar1 >> 2;
}


/* ===== FUN_000076fd @ 000076fd ===== */

void __allregs FUN_000076fd(undefined4 param_1,undefined4 param_2)

{
  undefined3 uVar1;
  
  DAT_000077e1 = (char)param_1;
  uVar1 = (undefined3)((uint)param_1 >> 8);
  if (DAT_000077e1 == '\0') {
    FUN_000023ec((short)((uint)param_1 >> 8) << 8,param_2);
  }
  else if (DAT_000077e1 == '\x01') {
    if ((DAT_000077e0 == '\x01') && (DAT_00002293 == '\0')) {
      FUN_000023ec((short)CONCAT31(uVar1,1),param_2);
    }
    puRam00002716 = (undefined *)0x1e93;
  }
  else if (DAT_000077e1 == '\x02') {
    if ((DAT_000077e0 == '\x01') && (DAT_00002293 == '\0')) {
      FUN_000023ec((short)CONCAT31(uVar1,1),param_2);
    }
    puRam00002716 = &DAT_00002093;
  }
  return;
}


/* ===== FUN_00007893 @ 00007893 ===== */

/* WARNING: Instruction at (ram,0x00007894) overlaps instruction at (ram,0x00007893)
    */

void __allregs
FUN_00007893(uint param_1,int param_2,undefined1 param_3,byte param_4,undefined4 *param_5,
            ushort *param_6)

{
  undefined4 *puVar1;
  char cVar2;
  BADSPACEBASE *in_ESP;
  undefined4 **ppuVar3;
  ushort *puVar4;
  undefined4 *apuStack_8 [2];
  
  ppuVar3 = apuStack_8 + 1;
  cVar2 = '\x05';
  puVar1 = param_5;
  do {
    puVar1 = puVar1 + -1;
    ppuVar3 = ppuVar3 + -1;
    *ppuVar3 = (undefined4 *)*puVar1;
    cVar2 = cVar2 + -1;
  } while ('\0' < cVar2);
  while( true ) {
    puVar4 = param_6 + 1;
    param_2 = param_2 + -1;
    if (param_2 < 0) break;
    param_6 = puVar4 + param_1 * 8;
    param_1 = (uint)*param_6;
  }
  apuStack_8[1] = param_5;
  if (-1 < (char)param_4) {
    FUN_000022ab(puVar4,param_5,param_3,param_4);
    return;
  }
  FUN_00002377(puVar4,param_5,param_3,param_4 & 0x7f);
  return;
}


/* ===== FUN_000078ba @ 000078ba ===== */

void __allregs FUN_000078ba(byte param_1)

{
  if (-1 < (char)param_1) {
    FUN_000022ab(&DAT_0000156b,0,0,param_1);
    return;
  }
  FUN_00002377(&DAT_0000156b,0,0,param_1 & 0x7f);
  return;
}


/* ===== FUN_000078da @ 000078da ===== */

void __allregs FUN_000078da(ushort param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  uVar1 = FUN_000023ec(param_1 & 0xff00,param_2);
  FUN_000012ac((short)uVar1,param_2);
  return;
}


/* ===== FUN_000078e7 @ 000078e7 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000078e7(void)

{
  DAT_000090ec = (uint)*(ushort *)(DAT_00000c93 + 4) + (uint)*(ushort *)(DAT_00000c93 + 2) * 0x10 +
                 ram0x00000807;
  DAT_000090f0 = (uint)*(ushort *)(DAT_00000c93 + 0x1e);
  _DAT_000090f4 = (uint)(*(ushort *)(DAT_00000c93 + 0x1e) >> 1);
  DAT_000090f8 = (uint)*(ushort *)(DAT_00000c93 + 0x22);
  FUN_000085d0();
  FUN_000093c0();
  return;
}


/* ===== FUN_000078f0 @ 000078f0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000078f0(void)

{
  DAT_000090ec = (uint)*(ushort *)(DAT_00000c93 + 4) + (uint)*(ushort *)(DAT_00000c93 + 2) * 0x10 +
                 ram0x00000807;
  DAT_000090f0 = (uint)*(ushort *)(DAT_00000c93 + 0x1e);
  _DAT_000090f4 = (uint)(*(ushort *)(DAT_00000c93 + 0x1e) >> 1);
  DAT_000090f8 = (uint)*(ushort *)(DAT_00000c93 + 0x22);
  FUN_000085d0();
  FUN_000093c0();
  return;
}


/* ===== FUN_00007940 @ 00007940 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00007940(undefined4 param_1)

{
  undefined2 uVar1;
  int iVar2;
  
  iVar2 = (uint)*(ushort *)(DAT_00000c93 + 2) * 0x10;
  DAT_000090ec = (uint)*(ushort *)(DAT_00000c93 + 4) + iVar2 + ram0x00000807;
  DAT_000090f0 = (uint)*(ushort *)(DAT_00000c93 + 0x1e);
  _DAT_000090f4 = (uint)(*(ushort *)(DAT_00000c93 + 0x1e) >> 1);
  DAT_000090f8 = (uint)*(ushort *)(DAT_00000c93 + 0x22);
  FUN_000085d0();
  uVar1 = (undefined2)iVar2;
  FUN_00008fa0();
  FUN_000079d7(param_1,uVar1);
  return;
}


/* ===== FUN_000079d7 @ 000079d7 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000079d7(undefined4 param_1,undefined2 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined *puVar4;
  
  if ((*(byte *)(DAT_00000c93 + 0x58) & 1) != 0) {
    DAT_00007f1e = 0x40;
    if (DAT_000090f0 - 6 < 0x40) {
      DAT_00007f1e = DAT_000090f0 - 6 & 0xfffffffe;
    }
    _DAT_00007f2e = 0;
    _DAT_00007f22 = (DAT_000090f0 >> 1) - 2;
    DAT_00007f26 = 7;
    DAT_00007f2a = 5;
    FUN_00007f32(5,4,&DAT_00007efb);
    _DAT_00007f2e = ~DAT_000090e0 + 0x1000000 >> 0x19;
    DAT_00007f2a = DAT_00007f1e;
    _DAT_00007f22 = DAT_000090f0 - DAT_00007f1e >> 1;
    DAT_00007f26 = 2;
    FUN_00007f32(DAT_00007b24,DAT_00007b28,&DAT_00007b2c);
    if ((*(byte *)(DAT_00000c93 + 0x58) & 2) != 0) {
      _DAT_00007f2e = 0;
      iVar2 = -(DAT_00007f1e >> 1);
      iVar3 = (DAT_00007f1e >> 1) - 1;
      iVar1 = (int)(~((uint)*(ushort *)(DAT_00000c93 + 0x56) * -0x10000 - DAT_000090e0) + 0x1000000)
              >> 0x19;
      if (iVar1 <= iVar2) {
        iVar1 = iVar2;
      }
      if (iVar3 <= iVar1) {
        iVar1 = iVar3;
      }
      _DAT_00007f22 = iVar1 + ((DAT_000090f0 >> 1) - 2);
      DAT_00007f26 = 8;
      puVar4 = &DAT_00007eec;
      if ((*(byte *)(DAT_00000c93 + 0x58) & 4) != 0) {
        puVar4 = &DAT_00007f0f;
      }
      DAT_00007f2a = 5;
      FUN_00007f32(5,3,puVar4);
    }
  }
  return;
}


/* ===== FUN_00007f32 @ 00007f32 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00007f32(int param_1,int param_2,int param_3)

{
  char cVar1;
  undefined3 uVar3;
  undefined4 uVar2;
  int iVar4;
  int iVar5;
  int iVar6;
  char *pcVar7;
  char *pcVar8;
  
  pcVar8 = (char *)(DAT_000090ec + DAT_00007f26 * DAT_000090f0 + _DAT_00007f22);
  pcVar7 = (char *)(param_3 + _DAT_00007f2e);
  iVar6 = param_1 - DAT_00007f2a;
  iVar5 = DAT_000090f0 - DAT_00007f2a;
  uVar2 = DAT_0000bc90;
  iVar4 = DAT_00007f2a;
  do {
    while( true ) {
      cVar1 = *pcVar7;
      uVar3 = (undefined3)((uint)uVar2 >> 8);
      pcVar7 = pcVar7 + 1;
      uVar2 = CONCAT31(uVar3,cVar1);
      if (cVar1 != '\0') break;
LAB_00007f77:
      pcVar8 = pcVar8 + 1;
      iVar4 = iVar4 + -1;
      if (iVar4 == 0) {
        pcVar7 = pcVar7 + iVar6;
        pcVar8 = pcVar8 + iVar5;
        param_2 = param_2 + -1;
        iVar4 = DAT_00007f2a;
        if (param_2 == 0) {
          return;
        }
      }
    }
    if (-1 < cVar1) {
      *pcVar8 = cVar1;
      goto LAB_00007f77;
    }
    uVar2 = CONCAT31(uVar3,*(char *)CONCAT31(uVar3,*pcVar8));
    *pcVar8 = *(char *)CONCAT31(uVar3,*pcVar8);
    pcVar8 = pcVar8 + 1;
    iVar4 = iVar4 + -1;
    if (iVar4 == 0) {
      pcVar7 = pcVar7 + iVar6;
      pcVar8 = pcVar8 + iVar5;
      param_2 = param_2 + -1;
      iVar4 = DAT_00007f2a;
      if (param_2 == 0) {
        return;
      }
    }
  } while( true );
}


/* ===== FUN_00007fa0 @ 00007fa0 ===== */

void __allregs FUN_00007fa0(void)

{
  return;
}


/* ===== FUN_00008120 @ 00008120 ===== */

void __allregs FUN_00008120(void)

{
  longlong lVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  DAT_000090e4 = (uint)*(ushort *)(DAT_00000c93 + 0x3c) * 0x10000;
  uVar2 = DAT_000090e4 >> 0x16;
  if ((int)uVar2 < 0) {
    uVar2 = -uVar2;
  }
  uVar3 = 0xfa - DAT_000090f8 >> 1;
  iVar4 = uVar3 - uVar2;
  if (uVar3 < uVar2) {
    if (-1 < DAT_000090e4) {
      iVar4 = -iVar4;
    }
    DAT_000090e4 = DAT_000090e4 + iVar4 * -0x400000;
    iVar4 = 0;
  }
  iVar5 = (int)((uint)*(ushort *)(DAT_00000c93 + 0x3a) << 0x10) >> 0x16;
  if (iVar4 <= iVar5) {
    iVar5 = iVar4;
  }
  if (iVar5 <= -iVar4) {
    iVar5 = -iVar4;
  }
  DAT_000090e8 = iVar5 << 0x16;
  iVar5 = (DAT_000090e4 + 0x40000000U & 0x7fffff) << 8;
  uVar2 = (uint)~(DAT_000090e4 << 9) >> 1;
  iVar4 = (DAT_000090e4 + 0x40000000U >> 0x17) * 4;
  DAT_00009104 = (int)((ulonglong)
                       ((longlong)*(int *)(FUN_0000944b + iVar4 + 5) * (longlong)(int)uVar2) >> 0x20
                      ) + (int)((ulonglong)((longlong)*(int *)(iVar4 + 0x9454) * (longlong)iVar5) >>
                               0x20);
  DAT_00009104 = DAT_00009104 * 2;
  DAT_00009108 = (int)((ulonglong)((longlong)*(int *)(&DAT_00009650 + iVar4) * (longlong)(int)uVar2)
                      >> 0x20) +
                 (int)((ulonglong)((longlong)*(int *)(&DAT_00009654 + iVar4) * (longlong)iVar5) >>
                      0x20);
  DAT_00009108 = DAT_00009108 * 2;
  DAT_000090c0 = 0x1000000;
  DAT_000090d4 = (int)((ulonglong)((longlong)DAT_000090e8 * (longlong)DAT_00009104) >> 0x20) * 8 +
                 -0x80000000;
  DAT_000090d8 = (int)((ulonglong)((longlong)DAT_000090e8 * (longlong)DAT_00009108) >> 0x20) * -8 +
                 -0x80000000;
  iVar4 = (int)((ulonglong)((longlong)DAT_00009104 * 0x1000000) >> 0x20);
  iVar5 = (int)((ulonglong)((longlong)DAT_00009108 * 0x1000000) >> 0x20);
  lVar1 = (longlong)iVar5 * (longlong)DAT_000090b4;
  DAT_000090b8 = (uint)lVar1 >> 0x10 | (int)((ulonglong)lVar1 >> 0x20) << 0x10;
  lVar1 = (longlong)iVar4 * (longlong)DAT_000090b4;
  DAT_000090bc = (uint)lVar1 >> 0x10 | (int)((ulonglong)lVar1 >> 0x20) << 0x10;
  DAT_000090d4 = DAT_000090d4 - (DAT_000090f0 >> 1) * iVar5;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f0 >> 1) * iVar4;
  DAT_000090d4 = DAT_000090d4 + (DAT_000090f8 >> 1) * DAT_000090bc;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f8 >> 1) * DAT_000090b8;
  return;
}


/* ===== FUN_000082b8 @ 000082b8 ===== */

void __allregs FUN_000082b8(undefined4 param_1,undefined4 param_2)

{
  FUN_00008120();
  FUN_00009200(param_1,param_2);
  FUN_000082d0();
  return;
}


/* ===== FUN_000082d0 @ 000082d0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000082d0(void)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  
  _DAT_0000ba7b = (ushort)*(byte *)(DAT_00000c93 + 0xdb);
  iVar1 = *(int *)(DAT_00000c93 + 0xd2) - _DAT_000090cc;
  if ((iVar1 < 0x80001) && (-0x80001 < iVar1)) {
    iVar3 = *(int *)(DAT_00000c93 + 0xd6) - _DAT_000090d0;
    if ((iVar3 == -0x40000 || -iVar3 < 0x40000) && (-0x40001 < -iVar3)) {
      iVar6 = (int)((ulonglong)((longlong)DAT_00009100 * (longlong)(iVar3 * -0x1000)) >> 0x20) +
              (int)((ulonglong)((longlong)DAT_000090fc * (longlong)(iVar1 * 0x1000)) >> 0x20);
      if (((uint)-iVar6 < 0x1fffffff || iVar6 == -0x1fffffff) && (0xfffff < (uint)-iVar6)) {
        uVar2 = 0;
        uVar4 = _FUN_00003a20 >> 1;
        uVar5 = _FUN_00003a20;
        if (*(uint *)(uVar4 * 4 + 0x4620) < (uint)(iVar6 * -8)) {
          uVar2 = uVar4;
          uVar5 = uVar4;
        }
        do {
          if ((uint)(iVar6 * -8) <= *(uint *)(uVar2 * 4 + 0x4624)) {
            uVar5 = *(uint *)(uVar2 * 4 + 0x4224) >> 3;
            _DAT_0000ba6b = uVar2;
            if (((uVar5 != 0) &&
                (iVar6 = ((int)(char)-((DAT_000090dc._3_1_ >> 1) - *(byte *)(DAT_00000c93 + 0xdb))
                         << 0x12) / (int)uVar5 + 0x80, iVar6 < 0x801)) && (-0x801 < iVar6)) {
              DAT_0000ba79 = (short)iVar6;
              iVar1 = ((int)((ulonglong)((longlong)DAT_00009100 * (longlong)(iVar1 * 0x1000)) >>
                            0x20) -
                      (int)((ulonglong)((longlong)DAT_000090fc * (longlong)(iVar3 * -0x1000)) >>
                           0x20)) / (int)uVar5 + 0x80;
              if ((iVar1 < 0x801) && (-0x801 < iVar1)) {
                _DAT_0000ba77 = (undefined2)iVar1;
                uVar2 = ((DAT_000090f8 >> 1) - (DAT_0000ba79 + -0x80)) +
                        ((DAT_000090e8 << 2) >> 0x18);
                if ((-1 < (int)uVar2) && (uVar2 < DAT_000090f8)) {
                  *(short *)(DAT_00000c93 + 0xd6) = (short)uVar2;
                  return;
                }
              }
            }
            break;
          }
          uVar2 = uVar2 + 1;
          uVar5 = uVar5 - 1;
        } while (uVar5 != 0);
      }
    }
  }
  *(undefined2 *)(DAT_00000c93 + 0xd6) = 0xffff;
  return;
}


/* ===== FUN_00008480 @ 00008480 ===== */

undefined4 __allregs FUN_00008480(uint param_1,uint param_2,int param_3)

{
  uint uVar1;
  
  uVar1 = (param_1 >> 0x16) << 10 | param_2 >> 0x16;
  return CONCAT31((int3)(uVar1 >> 8),*(undefined1 *)(uVar1 + param_3));
}


/* ===== FUN_000084c0 @ 000084c0 ===== */

void __allregs FUN_000084c0(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)param_1 >> 8),1),0x10000,&DAT_0000bc90,0x10000,0x84d7);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x4000,&DAT_0000bc94,0x10000,0x84ee);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x100,&DAT_000085c0,0x100,0x8505);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x100,&LAB_000085c4,0x100,0x851c);
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x10000,&LAB_00003909,0x100,0x8536);
  FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0x10000,&DAT_0000390d,0x100,0x8550);
  DAT_0000391c = DAT_000038f1 * DAT_000038ed;
  iVar1 = FUN_000036bf(CONCAT31((int3)((uint)DAT_0000391c >> 8),1),DAT_0000391c,&DAT_00003918,0x100,
                       0x8576);
  DAT_000090c4 = 0;
  FUN_000036bf(CONCAT31((int3)((uint)iVar1 >> 8),1),0,&LAB_0000bc98,1,0x8594);
  *(undefined4 *)(DAT_00000c93 + 0x488) = 0;
  return;
}


/* ===== FUN_000085d0 @ 000085d0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000085d0(void)

{
  uint uVar1;
  
  DAT_0000395d = *(undefined1 *)(DAT_00000c93 + 0xcf);
  _DAT_000090cc = *(int *)(DAT_00000c93 + 0x2c);
  DAT_000090d4 = *(int *)(DAT_00000c93 + 0x2c) << 0xd;
  _DAT_000090d0 = *(int *)(DAT_00000c93 + 0x30);
  DAT_000090d8 = *(int *)(DAT_00000c93 + 0x30) * -0x2000;
  uVar1 = *(uint *)(DAT_00000c93 + 0x34);
  if (0x7f00 < uVar1) {
    uVar1 = 0x7f00;
  }
  DAT_000090dc = uVar1 << 0x11;
  DAT_000090e0 = (uint)*(ushort *)(DAT_00000c93 + 0x38) * -0x10000;
  DAT_000090c0 = (int)(0xffffffff / (ulonglong)(uint)*(ushort *)(DAT_00000c93 + 0x3e));
  _DAT_000090c8 = (uint)*(ushort *)(DAT_00000c93 + 0x3e);
  return;
}


/* ===== FUN_000089b0 @ 000089b0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000089b0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  undefined2 uVar6;
  undefined4 uVar7;
  uint uVar8;
  int iVar9;
  undefined1 uVar10;
  
  FUN_00003322(&LAB_0000bc98,param_2,0x89bb);
  uVar10 = 0;
  FUN_000036bf(1,0,&LAB_0000bc98,1,0x89d2);
  _DAT_00008490 = (uint)*(byte *)(DAT_00000c93 + 0x59);
  iVar3 = DAT_00000c93;
  FUN_00005c98((short)DAT_00000c93,(short)param_1,s_4_MEG_000084a0,(short)param_3);
  uVar5 = (undefined2)iVar3;
  if (!(bool)uVar10) {
    DAT_0000395c = '\x01';
    _DAT_00008490 = 9;
  }
  FUN_00005c98(uVar5,(short)param_1,s_8_MEG_000084a6,(short)param_3);
  if (!(bool)uVar10) {
    DAT_0000395c = '\x01';
    _DAT_00008490 = 10;
  }
  FUN_00005c98(uVar5,(short)param_1,s_16_MEG_000084ac,(short)param_3);
  if (!(bool)uVar10) {
    DAT_0000395c = '\x02';
    _DAT_00008490 = 0xb;
  }
  FUN_00005c98(uVar5,(short)param_1,s_40_MEG_000084b3,(short)param_3);
  if (!(bool)uVar10) {
    DAT_0000395c = '\x03';
    _DAT_00008490 = 0xc;
  }
  uVar5 = (undefined2)_DAT_00008490;
  bRam00006ae6 = (byte)_DAT_00008490;
  DAT_00008494 = 1 << (bRam00006ae6 & 0x1f);
  _DAT_0000849c = DAT_00008494 - 1;
  uVar4 = (undefined2)((ulonglong)DAT_00008494 * (ulonglong)DAT_00008494 >> 0x20);
  iRam00006b1d = (int)((ulonglong)DAT_00008494 * (ulonglong)DAT_00008494);
  DAT_000028a5 = (uint)*(byte *)(DAT_00000c93 + 0x54);
  PTR_s_RESOURCE_RES_0000622c = s_PAL_RES_000089a3;
  iVar3 = DAT_00000c93;
  bRam00006aea = bRam00006ae6;
  bRam00006b66 = bRam00006ae6;
  bRam00006b6a = bRam00006ae6;
  iRam00006b9d = iRam00006b1d;
  bRam00006d66 = bRam00006ae6;
  bRam00006d6a = bRam00006ae6;
  iRam00006d9d = iRam00006b1d;
  bRam00006de6 = bRam00006ae6;
  bRam00006dea = bRam00006ae6;
  iRam00006e1d = iRam00006b1d;
  bRam00007fc7 = bRam00006ae6;
  bRam00007fcb = bRam00006ae6;
  bRam00007fdc = bRam00006ae6;
  bRam00007fe0 = bRam00006ae6;
  bRam00007ff8 = bRam00006ae6;
  bRam00007ffc = bRam00006ae6;
  bRam0000800d = bRam00006ae6;
  bRam00008011 = bRam00006ae6;
  DAT_00008105 = bRam00006ae6;
  DAT_00008109 = bRam00006ae6;
  FUN_00008480 = bRam00006ae6;
  bRam00008489 = bRam00006ae6;
  DAT_00008498 = iRam00006b1d;
  DAT_00008f48 = bRam00006ae6;
  DAT_00008f4c = bRam00006ae6;
  DAT_00008f6a = bRam00006ae6;
  DAT_00008f6e = bRam00006ae6;
  bRam00009068 = bRam00006ae6;
  bRam0000906c = bRam00006ae6;
  bRam0000907f = bRam00006ae6;
  bRam00009083 = bRam00006ae6;
  bRam00009429 = bRam00006ae6;
  bRam00009434 = bRam00006ae6;
  FUN_00006032(&DAT_00005598,uVar4,(short)DAT_00000c93,(short)param_1,DAT_00000c93 + 0x9a,
               (short)param_3);
  uVar6 = (undefined2)iVar3;
  FUN_00009f70();
  DAT_0000ac68 = 0x3f;
  DAT_0000ac69 = 0x3f;
  DAT_0000ac6a = 0;
  FUN_0000ac70();
  DAT_0000aead = (undefined1)uVar6;
  uVar2 = FUN_0000643c(0,uVar5,uVar4,uVar6,param_1,DAT_00000c93 + 0x7a,param_3);
  uVar5 = 0x85bc;
  uVar7 = 4;
  iVar3 = DAT_00000937;
  FUN_000036bf(CONCAT31((int3)((uint)uVar2 >> 8),3),DAT_00000937,&DAT_000085bc,4,0x8baa);
  iVar9 = DAT_00000c93 + 0x7a;
  FUN_0000643c(DAT_000085bc,(short)iVar3,uVar5,(short)uVar7,param_1,iVar9,param_3);
  while (DAT_00005578 < DAT_00008494) {
    FUN_00002f95(&DAT_000085bc,0x8bd5);
    iVar3 = iVar3 << 2;
    FUN_0000345c(iVar3,&DAT_000085bc,iVar9,0x8be3);
    param_3 = DAT_000085bc;
    FUN_0000bc06(DAT_000085bc);
  }
  while (uVar5 = (undefined2)uVar7, DAT_00008494 < DAT_00005578) {
    param_3 = DAT_000085bc;
    FUN_0000bed2(DAT_000085bc);
  }
  uVar4 = 0x85bc;
  iVar3 = DAT_00008498;
  FUN_0000345c(DAT_00008498,&DAT_000085bc,iVar9,0x8c1b);
  uVar2 = FUN_0000643c(0,(short)iVar3,uVar4,uVar5,param_1,DAT_00000c93 + 0x8a,param_3);
  uVar5 = 0x85b8;
  uVar7 = 4;
  iVar3 = DAT_00000937;
  FUN_000036bf(CONCAT31((int3)((uint)uVar2 >> 8),3),DAT_00000937,&DAT_000085b8,4,0x8c46);
  FUN_0000643c(DAT_000085b8,(short)iVar3,uVar5,(short)uVar7,param_1,DAT_00000c93 + 0x8a,param_3);
  uVar10 = (undefined1)uVar7;
  FUN_00004a3c();
  iVar3 = DAT_00008498;
  uVar2 = DAT_000085b8;
  FUN_00009ec0(DAT_00008498,uVar10,DAT_000085b8);
  while (DAT_00005578 < DAT_00008494) {
    FUN_00002f95(&DAT_000085b8,0x8c8a);
    iVar3 = iVar3 << 2;
    FUN_0000345c(iVar3,&DAT_000085b8,uVar2,0x8c98);
    param_3 = DAT_000085b8;
    FUN_0000bdc4(DAT_000085b8);
  }
  while (DAT_00008494 < DAT_00005578) {
    param_3 = DAT_000085b8;
    FUN_0000bed2(DAT_000085b8);
  }
  iVar3 = FUN_0000345c(DAT_00008498,&DAT_000085b8,uVar2,0x8cd0);
  iVar9 = 0x400;
  cVar1 = DAT_00008490 - 10;
  if (9 < DAT_00008490 && cVar1 != '\0') {
    do {
      iVar9 = iVar9 << 2;
      cVar1 = cVar1 + -1;
    } while (cVar1 != '\0');
  }
  uVar4 = 0x8644;
  FUN_000036bf(CONCAT31((int3)((uint)iVar3 >> 8),3),iVar9,&DAT_00008644,4,0x8cf7);
  uVar5 = (undefined2)iVar9;
  uVar8 = _DAT_000038f5 >> 1;
  DAT_00003901 = ram0x000038fd - uVar8;
  DAT_00003905 = ram0x000038fd + uVar8;
  FUN_00003930();
  uVar6 = (undefined2)uVar8;
  FUN_000091e5();
  FUN_000092a1();
  FUN_00009119();
  DAT_00003915 = '\a';
  DAT_00003916 = '\n';
  if (DAT_0000395c != '\0') {
    uVar2 = FUN_0000643c(0,uVar5,uVar4,uVar6,param_1,DAT_00000c93 + 0xaa,param_3);
    uVar5 = 0x3911;
    uVar7 = 4;
    iVar3 = DAT_00000937;
    FUN_000036bf(CONCAT31((int3)((uint)uVar2 >> 8),1),DAT_00000937,&DAT_00003911,4,0x8d71);
    FUN_0000643c(DAT_00003911,(short)iVar3,uVar5,(short)uVar7,param_1,DAT_00000c93 + 0xaa,param_3);
    uVar10 = (undefined1)uVar7;
    FUN_00004a3c();
    iVar3 = DAT_00000937;
    uVar2 = DAT_00003911;
    FUN_00009ec0(DAT_00000937,uVar10,DAT_00003911);
    iVar3 = CONCAT31((int3)((uint)iVar3 >> 8),DAT_0000395c);
    while (cVar1 = (char)iVar3 + -1, iVar9 = CONCAT31((int3)((uint)iVar3 >> 8),cVar1), cVar1 != '\0'
          ) {
      iVar3 = iVar9;
      FUN_00002f95(&DAT_00003911,0x8db3);
      FUN_0000345c(iVar9 << 2,&DAT_00003911,uVar2,0x8dc1);
      FUN_0000bdc4(DAT_00003911);
      DAT_00003915 = DAT_00003915 + '\x01';
      DAT_00003916 = DAT_00003916 + '\x01';
    }
  }
  LAB_00006952_3 = DAT_00003915;
  LAB_0000695d_3 = DAT_00003915;
  return;
}


/* ===== FUN_00008ce2 @ 00008ce2 ===== */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00008ce2(int param_1,undefined4 param_2,undefined4 param_3,undefined4 *param_4,
            undefined4 param_5)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  char cVar4;
  undefined2 uVar5;
  undefined4 uVar6;
  int iVar7;
  undefined2 uVar9;
  undefined1 uVar10;
  undefined2 uVar11;
  uint uVar12;
  undefined4 uVar13;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar14;
  byte in_CF;
  undefined4 *apuStack_8 [2];
  int iVar8;
  
  puVar1 = apuStack_8 + 1;
  puVar14 = apuStack_8 + 1;
  cVar4 = '\f';
  puVar2 = param_4;
  do {
    puVar2 = puVar2 + -1;
    puVar1 = puVar1 + -1;
    *puVar1 = *puVar2;
    cVar4 = cVar4 + -1;
  } while ('\0' < cVar4);
  apuStack_8[1] = param_4;
  FUN_000036bf(CONCAT31((int3)((uint)(&DAT_00008644 + (uint)in_CF + param_1) >> 8),3),param_2,
               param_3,4,0x8cf7);
  uVar9 = (undefined2)param_3;
  uVar5 = (undefined2)param_2;
  uVar12 = _DAT_000038f5 >> 1;
  DAT_00003901 = ram0x000038fd - uVar12;
  DAT_00003905 = ram0x000038fd + uVar12;
  FUN_00003930();
  uVar11 = (undefined2)uVar12;
  FUN_000091e5();
  FUN_000092a1();
  FUN_00009119();
  DAT_00003915 = '\a';
  DAT_00003916 = '\n';
  if (DAT_0000395c != '\0') {
    uVar3 = FUN_0000643c(0,uVar5,uVar9,uVar11,puVar14,DAT_00000c93 + 0xaa,param_5);
    uVar5 = 0x3911;
    uVar13 = 4;
    uVar6 = DAT_00000937;
    FUN_000036bf(CONCAT31((int3)((uint)uVar3 >> 8),1),DAT_00000937,&DAT_00003911,4,0x8d71);
    FUN_0000643c(DAT_00003911,(short)uVar6,uVar5,(short)uVar13,puVar14,DAT_00000c93 + 0xaa,param_5);
    uVar10 = (undefined1)uVar13;
    FUN_00004a3c();
    uVar6 = DAT_00000937;
    uVar3 = DAT_00003911;
    FUN_00009ec0(DAT_00000937,uVar10,DAT_00003911);
    iVar7 = CONCAT31((int3)((uint)uVar6 >> 8),DAT_0000395c);
    while (cVar4 = (char)iVar7 + -1, iVar8 = CONCAT31((int3)((uint)iVar7 >> 8),cVar4), cVar4 != '\0'
          ) {
      iVar7 = iVar8;
      FUN_00002f95(&DAT_00003911,0x8db3);
      FUN_0000345c(iVar8 << 2,&DAT_00003911,uVar3,0x8dc1);
      FUN_0000bdc4(DAT_00003911);
      DAT_00003915 = DAT_00003915 + '\x01';
      DAT_00003916 = DAT_00003916 + '\x01';
    }
  }
  LAB_00006952_3 = DAT_00003915;
  LAB_0000695d_3 = DAT_00003915;
  return;
}


/* ===== FUN_00008deb @ 00008deb ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00008deb(void)

{
  ushort uVar1;
  
  DAT_000090ec = (uint)*(ushort *)(DAT_00000c93 + 4) + (uint)*(ushort *)(DAT_00000c93 + 2) * 0x10 +
                 ram0x00000807;
  uVar1 = *(short *)(DAT_00000c93 + 0x1c) - *(short *)(DAT_00000c93 + 0x18);
  DAT_000090f0 = (uint)uVar1;
  _DAT_000090f4 = (uint)(uVar1 >> 1);
  DAT_000090f8 = (uint)(ushort)(*(short *)(DAT_00000c93 + 0x1a) - *(short *)(DAT_00000c93 + 0x16));
  DAT_000090a8 = (uint)(ushort)(*(short *)(DAT_00000c93 + 0x16) * 0x140 +
                               *(short *)(DAT_00000c93 + 0x18)) + DAT_00000917;
  _DAT_000090ac = 0x140 - (uint)uVar1;
  DAT_00009114 = (&PTR_DAT_0000748c)[*(byte *)(DAT_00000c93 + 0xcd)];
  return;
}


/* ===== FUN_00008df0 @ 00008df0 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00008df0(void)

{
  ushort uVar1;
  
  DAT_000090ec = (uint)*(ushort *)(DAT_00000c93 + 4) + (uint)*(ushort *)(DAT_00000c93 + 2) * 0x10 +
                 ram0x00000807;
  uVar1 = *(short *)(DAT_00000c93 + 0x1c) - *(short *)(DAT_00000c93 + 0x18);
  DAT_000090f0 = (uint)uVar1;
  _DAT_000090f4 = (uint)(uVar1 >> 1);
  DAT_000090f8 = (uint)(ushort)(*(short *)(DAT_00000c93 + 0x1a) - *(short *)(DAT_00000c93 + 0x16));
  DAT_000090a8 = (uint)(ushort)(*(short *)(DAT_00000c93 + 0x16) * 0x140 +
                               *(short *)(DAT_00000c93 + 0x18)) + DAT_00000917;
  _DAT_000090ac = 0x140 - (uint)uVar1;
  DAT_00009114 = (&PTR_DAT_0000748c)[*(byte *)(DAT_00000c93 + 0xcd)];
  return;
}


/* ===== FUN_00008fa0 @ 00008fa0 ===== */

void __allregs FUN_00008fa0(void)

{
  undefined1 *puVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  undefined1 *puVar10;
  
  DAT_000090fc = *(int *)(FUN_0000944b + ((uint)-DAT_000090e0 >> 0x17) * 4 + 5);
  DAT_00009100 = *(int *)(&DAT_00009650 + ((uint)-DAT_000090e0 >> 0x17) * 4);
  iVar4 = (int)((ulonglong)((longlong)DAT_000090c0 * (longlong)DAT_000090fc) >> 0x20);
  iVar5 = (int)((ulonglong)((longlong)DAT_000090c0 * (longlong)DAT_00009100) >> 0x20);
  DAT_000090b8 = (uint)((longlong)iVar5 * 0x10000) >> 0x10 |
                 (int)((ulonglong)((longlong)iVar5 * 0x10000) >> 0x20) << 0x10;
  DAT_000090bc = (uint)((longlong)iVar4 * 0x10000) >> 0x10 |
                 (int)((ulonglong)((longlong)iVar4 * 0x10000) >> 0x20) << 0x10;
  DAT_000090d4 = DAT_000090d4 - (DAT_000090f0 >> 1) * iVar5;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f0 >> 1) * iVar4;
  DAT_000090d4 = DAT_000090d4 + (DAT_000090f8 >> 1) * DAT_000090bc;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f8 >> 1) * DAT_000090b8;
  uVar3 = DAT_000090f8;
  uVar7 = DAT_000090d8;
  uVar9 = DAT_000090d4;
  puVar10 = DAT_000090ec;
  do {
    uVar2 = DAT_000090f0 >> 1;
    uVar6 = uVar7;
    uVar8 = uVar9;
    do {
      puVar1 = puVar10 + 1;
      *puVar10 = *(undefined1 *)(((uVar6 >> 0x16) << 10 | uVar8 >> 0x16) + DAT_000085b8);
      puVar10 = puVar10 + 2;
      *puVar1 = *(undefined1 *)
                 (((uVar6 + iVar4 >> 0x16) << 10 | uVar8 + iVar5 >> 0x16) + DAT_000085b8);
      uVar6 = uVar6 + iVar4 + iVar4;
      uVar8 = uVar8 + iVar5 + iVar5;
      uVar2 = uVar2 - 1;
    } while (uVar2 != 0);
    uVar9 = uVar9 - DAT_000090bc;
    uVar7 = uVar7 + DAT_000090b8;
    uVar3 = uVar3 - 1;
  } while (uVar3 != 0);
  return;
}


/* ===== FUN_00009119 @ 00009119 ===== */

void __allregs FUN_00009119(void)

{
  uRam0000918c = DAT_00003918;
  uRam000091a9 = DAT_00003918;
  return;
}


/* ===== FUN_00009129 @ 00009129 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00009129(int param_1,int param_2)

{
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  byte *pbVar10;
  
  iVar3 = 0;
  uVar7 = DAT_000090d8;
  uVar9 = DAT_000090d4;
  pbVar10 = DAT_000090a8;
  do {
    DAT_0000910c = (uint)*(byte *)(iVar3 + DAT_00009114);
    uVar6 = uVar7;
    uVar8 = uVar9;
    if (DAT_0000910c != 0) {
      pbVar10 = pbVar10 + DAT_0000910c;
      uVar4 = DAT_0000910c;
      do {
        uVar6 = uVar6 + param_2;
        uVar8 = uVar8 + param_1;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
    iVar5 = (DAT_000090f0 >> 1) - DAT_0000910c;
    do {
      bVar2 = *(byte *)(((uVar6 >> 0x18) << 8 | uVar8 >> 0x18) + 0x7fffffff);
      if (bVar2 < 0xb5) {
        bVar2 = (byte)(bVar2 + *pbVar10 + (bVar2 < 0xb4)) >> 1 |
                (CARRY1(bVar2,*pbVar10) || CARRY1(bVar2 + *pbVar10,bVar2 < 0xb4)) << 7;
      }
      pbVar1 = pbVar10 + 1;
      *pbVar10 = bVar2;
      bVar2 = *(byte *)(((uVar6 + param_2 >> 0x18) << 8 | uVar8 + param_1 >> 0x18) + 0x7fffffff);
      if (bVar2 < 0xb5) {
        bVar2 = (byte)(bVar2 + *pbVar1 + (bVar2 < 0xb4)) >> 1 |
                (CARRY1(bVar2,*pbVar1) || CARRY1(bVar2 + *pbVar1,bVar2 < 0xb4)) << 7;
      }
      pbVar10 = pbVar10 + 2;
      *pbVar1 = bVar2;
      uVar6 = uVar6 + param_2 + param_2;
      uVar8 = uVar8 + param_1 + param_1;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    pbVar10 = pbVar10 + DAT_0000910c + _DAT_000090ac;
    uVar9 = uVar9 - DAT_000090bc;
    uVar7 = uVar7 + DAT_000090b8;
    iVar3 = iVar3 + 1;
  } while (iVar3 != DAT_000090f8);
  return;
}


/* ===== FUN_00009168 @ 00009168 ===== */

/* WARNING: Instruction at (ram,0x00009169) overlaps instruction at (ram,0x00009168)
    */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00009168(uint param_1,uint param_2,uint param_3,undefined4 *param_4,int param_5,byte *param_6,
            uint param_7,uint param_8,int param_9)

{
  byte *pbVar1;
  char cVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar6;
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  puVar6 = &uStack_4;
  cVar2 = '\x02';
  do {
    param_4 = param_4 + -1;
    puVar6 = puVar6 + -1;
    *puVar6 = *param_4;
    cVar2 = cVar2 + -1;
  } while ('\0' < cVar2);
  while( true ) {
    iVar5 = (DAT_000090f0 >> 1) - param_1;
    do {
      bVar3 = *(byte *)(((param_2 >> 0x18) << 8 | param_3 >> 0x18) + 0x7fffffff);
      if (bVar3 < 0xb5) {
        bVar3 = (byte)(bVar3 + *param_6 + (bVar3 < 0xb4)) >> 1 |
                (CARRY1(bVar3,*param_6) || CARRY1(bVar3 + *param_6,bVar3 < 0xb4)) << 7;
      }
      pbVar1 = param_6 + 1;
      *param_6 = bVar3;
      bVar3 = *(byte *)(((param_2 + param_5 >> 0x18) << 8 | param_3 + (int)&uStack_4 >> 0x18) +
                       0x7fffffff);
      if (bVar3 < 0xb5) {
        bVar3 = (byte)(bVar3 + *pbVar1 + (bVar3 < 0xb4)) >> 1 |
                (CARRY1(bVar3,*pbVar1) || CARRY1(bVar3 + *pbVar1,bVar3 < 0xb4)) << 7;
      }
      param_6 = param_6 + 2;
      *pbVar1 = bVar3;
      param_2 = param_2 + param_5 + param_5;
      param_3 = param_3 + (int)&uStack_4 + (int)&uStack_4;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    param_6 = param_6 + DAT_0000910c + _DAT_000090ac;
    param_8 = param_8 - DAT_000090bc;
    param_7 = param_7 + DAT_000090b8;
    param_9 = param_9 + 1;
    if (param_9 == DAT_000090f8) break;
    param_1 = (uint)*(byte *)(param_9 + DAT_00009114);
    param_2 = param_7;
    param_3 = param_8;
    DAT_0000910c = param_1;
    if (param_1 != 0) {
      param_6 = param_6 + param_1;
      uVar4 = param_1;
      do {
        param_2 = param_2 + param_5;
        param_3 = param_3 + (int)&uStack_4;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
  }
  return;
}


/* ===== FUN_000091e5 @ 000091e5 ===== */

void __allregs FUN_000091e5(void)

{
  uRam0000925c = DAT_00003918;
  uRam00009271 = DAT_00003918;
  return;
}


/* ===== FUN_000091f5 @ 000091f5 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000091f5(int param_1,int param_2)

{
  undefined1 *puVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined1 *puVar9;
  
  iVar2 = 0;
  uVar6 = DAT_000090d8;
  uVar8 = DAT_000090d4;
  puVar9 = DAT_000090a8;
  do {
    DAT_0000910c = (uint)*(byte *)(iVar2 + DAT_00009114);
    uVar5 = uVar6;
    uVar7 = uVar8;
    if (DAT_0000910c != 0) {
      puVar9 = puVar9 + DAT_0000910c;
      uVar3 = DAT_0000910c;
      do {
        uVar5 = uVar5 + param_2;
        uVar7 = uVar7 + param_1;
        uVar3 = uVar3 - 1;
      } while (uVar3 != 0);
    }
    iVar4 = (DAT_000090f0 >> 1) - DAT_0000910c;
    do {
      puVar1 = puVar9 + 1;
      *puVar9 = *(undefined1 *)(((uVar5 >> 0x18) << 8 | uVar7 >> 0x18) + 0x7fffffff);
      puVar9 = puVar9 + 2;
      *puVar1 = *(undefined1 *)
                 (((uVar5 + param_2 >> 0x18) << 8 | uVar7 + param_1 >> 0x18) + 0x7fffffff);
      uVar5 = uVar5 + param_2 + param_2;
      uVar7 = uVar7 + param_1 + param_1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    puVar9 = puVar9 + DAT_0000910c + _DAT_000090ac;
    uVar8 = uVar8 - DAT_000090bc;
    uVar6 = uVar6 + DAT_000090b8;
    iVar2 = iVar2 + 1;
  } while (iVar2 != DAT_000090f8);
  return;
}


/* ===== FUN_00009200 @ 00009200 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00009200(int param_1,int param_2)

{
  undefined1 *puVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined1 *puVar9;
  
  iVar2 = 0;
  uVar6 = DAT_000090d8;
  uVar8 = DAT_000090d4;
  puVar9 = DAT_000090a8;
  do {
    DAT_0000910c = (uint)*(byte *)(iVar2 + DAT_00009114);
    uVar5 = uVar6;
    uVar7 = uVar8;
    if (DAT_0000910c != 0) {
      puVar9 = puVar9 + DAT_0000910c;
      uVar3 = DAT_0000910c;
      do {
        uVar5 = uVar5 + param_2;
        uVar7 = uVar7 + param_1;
        uVar3 = uVar3 - 1;
      } while (uVar3 != 0);
    }
    iVar4 = (DAT_000090f0 >> 1) - DAT_0000910c;
    do {
      puVar1 = puVar9 + 1;
      *puVar9 = *(undefined1 *)(((uVar5 >> 0x18) << 8 | uVar7 >> 0x18) + 0x7fffffff);
      puVar9 = puVar9 + 2;
      *puVar1 = *(undefined1 *)
                 (((uVar5 + param_2 >> 0x18) << 8 | uVar7 + param_1 >> 0x18) + 0x7fffffff);
      uVar5 = uVar5 + param_2 + param_2;
      uVar7 = uVar7 + param_1 + param_1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    puVar9 = puVar9 + DAT_0000910c + _DAT_000090ac;
    uVar8 = uVar8 - DAT_000090bc;
    uVar6 = uVar6 + DAT_000090b8;
    iVar2 = iVar2 + 1;
  } while (iVar2 != DAT_000090f8);
  return;
}


/* ===== FUN_00009238 @ 00009238 ===== */

/* WARNING: Instruction at (ram,0x00009239) overlaps instruction at (ram,0x00009238)
    */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00009238(uint param_1,uint param_2,uint param_3,undefined4 *param_4,int param_5,
            undefined1 *param_6,uint param_7,uint param_8,int param_9)

{
  undefined1 *puVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar5;
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  puVar5 = &uStack_4;
  cVar2 = '\x02';
  do {
    param_4 = param_4 + -1;
    puVar5 = puVar5 + -1;
    *puVar5 = *param_4;
    cVar2 = cVar2 + -1;
  } while ('\0' < cVar2);
  while( true ) {
    iVar4 = (DAT_000090f0 >> 1) - param_1;
    do {
      puVar1 = param_6 + 1;
      *param_6 = *(undefined1 *)(((param_2 >> 0x18) << 8 | param_3 >> 0x18) + 0x7fffffff);
      param_6 = param_6 + 2;
      *puVar1 = *(undefined1 *)
                 (((param_2 + param_5 >> 0x18) << 8 | param_3 + (int)&uStack_4 >> 0x18) + 0x7fffffff
                 );
      param_2 = param_2 + param_5 + param_5;
      param_3 = param_3 + (int)&uStack_4 + (int)&uStack_4;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    param_6 = param_6 + DAT_0000910c + _DAT_000090ac;
    param_8 = param_8 - DAT_000090bc;
    param_7 = param_7 + DAT_000090b8;
    param_9 = param_9 + 1;
    if (param_9 == DAT_000090f8) break;
    param_1 = (uint)*(byte *)(param_9 + DAT_00009114);
    param_2 = param_7;
    param_3 = param_8;
    DAT_0000910c = param_1;
    if (param_1 != 0) {
      param_6 = param_6 + param_1;
      uVar3 = param_1;
      do {
        param_2 = param_2 + param_5;
        param_3 = param_3 + (int)&uStack_4;
        uVar3 = uVar3 - 1;
      } while (uVar3 != 0);
    }
  }
  return;
}


/* ===== FUN_000092a1 @ 000092a1 ===== */

void __allregs FUN_000092a1(void)

{
  uRam0000934c = DAT_00003918;
  uRam00009372 = DAT_00003918;
  return;
}


/* ===== FUN_000092b1 @ 000092b1 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_000092b1(undefined4 param_1,int param_2)

{
  char *pcVar1;
  char cVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  byte bVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  char *pcVar12;
  
  uRam00009356 = *(undefined1 *)(DAT_00000c93 + 0xd0);
  iVar3 = 0;
  DAT_0000930c = DAT_0000930c + 1;
  uVar7 = DAT_000090d8;
  uVar9 = DAT_0000930c;
  uVar11 = DAT_000090d4;
  pcVar12 = DAT_000090a8;
  _DAT_0000932d = param_1;
  uRam00009362 = param_1;
  uRam0000937a = uRam00009356;
  uRam00009386 = param_1;
  do {
    DAT_0000910c = (uint)*(byte *)(iVar3 + DAT_00009114);
    uVar6 = uVar7;
    uVar10 = uVar11;
    if (DAT_0000910c != 0) {
      pcVar12 = pcVar12 + DAT_0000910c;
      uVar4 = DAT_0000910c;
      do {
        uVar6 = uVar6 + param_2;
        uVar10 = uVar10 + 0x7fffffff;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
    iVar5 = (DAT_000090f0 >> 1) - DAT_0000910c;
    do {
      cVar2 = *(char *)(((uVar6 >> 0x18) << 8 | uVar10 >> 0x18) + 0x7fffffff);
      uVar9 = uVar9 ^ uVar6;
      uVar9 = CONCAT31((int3)(uVar9 >> 8),(char)uVar9 + (char)(uVar9 >> 8)) & 0xffffff07;
      bVar8 = (byte)uVar9 ^ cVar2 >> 7;
      pcVar1 = pcVar12 + 1;
      *pcVar12 = cVar2 + bVar8;
      uVar6 = uVar6 + param_2;
      cVar2 = *(char *)(((uVar6 >> 0x18) << 8 | uVar10 + 0x7fffffff >> 0x18) + 0x7fffffff);
      uVar9 = (CONCAT31((int3)(uVar9 >> 8),bVar8) ^ (uint)CONCAT11((char)(uVar6 >> 0x18),cVar2)) &
              0xffffff07;
      bVar8 = (byte)uVar9 ^ cVar2 >> 7;
      uVar9 = CONCAT31((int3)(uVar9 >> 8),bVar8);
      pcVar12 = pcVar12 + 2;
      *pcVar1 = cVar2 + bVar8;
      uVar6 = uVar6 + param_2;
      uVar10 = uVar10 - 2;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    pcVar12 = pcVar12 + DAT_0000910c + _DAT_000090ac;
    uVar11 = uVar11 - DAT_000090bc;
    uVar7 = uVar7 + DAT_000090b8;
    iVar3 = iVar3 + 1;
  } while (iVar3 != DAT_000090f8);
  return;
}


/* ===== FUN_00009328 @ 00009328 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_00009328(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000933c @ 0000933c ===== */

/* WARNING: Instruction at (ram,0x0000933d) overlaps instruction at (ram,0x0000933c)
    */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_0000933c(int param_1,uint param_2,uint param_3,int param_4,char *param_5,uint param_6,
            undefined1 *param_7,int param_8)

{
  char *pcVar1;
  char cVar2;
  byte bVar3;
  uint uVar4;
  uint uVar5;
  BADSPACEBASE *in_ESP;
  undefined1 *puVar6;
  undefined1 auStack_4 [4];
  
  puVar6 = auStack_4;
  while( true ) {
    do {
      cVar2 = *(char *)(((param_2 >> 0x18) << 8 | (uint)puVar6 >> 0x18) + 0x7fffffff);
      uVar4 = param_3 ^ param_2;
      uVar5 = CONCAT31((int3)(uVar4 >> 8),(char)uVar4 + (char)(uVar4 >> 8)) & 0xffffff07;
      bVar3 = (byte)uVar5 ^ cVar2 >> 7;
      pcVar1 = param_5 + 1;
      *param_5 = cVar2 + bVar3;
      uVar4 = param_2 + param_4;
      cVar2 = *(char *)(((uVar4 >> 0x18) << 8 | (uint)(puVar6 + 0x7fffffff) >> 0x18) + 0x7fffffff);
      uVar5 = (CONCAT31((int3)(uVar5 >> 8),bVar3) ^ (uint)CONCAT11((char)(uVar4 >> 0x18),cVar2)) &
              0xffffff07;
      bVar3 = (byte)uVar5 ^ cVar2 >> 7;
      param_3 = CONCAT31((int3)(uVar5 >> 8),bVar3);
      param_5 = param_5 + 2;
      *pcVar1 = cVar2 + bVar3;
      param_2 = uVar4 + param_4;
      puVar6 = puVar6 + -2;
      param_1 = param_1 + -1;
    } while (param_1 != 0);
    param_5 = param_5 + DAT_0000910c + _DAT_000090ac;
    param_7 = param_7 + -DAT_000090bc;
    param_6 = param_6 + DAT_000090b8;
    param_8 = param_8 + 1;
    if (param_8 == DAT_000090f8) break;
    DAT_0000910c = (uint)*(byte *)(param_8 + DAT_00009114);
    param_2 = param_6;
    puVar6 = param_7;
    if (DAT_0000910c != 0) {
      param_5 = param_5 + DAT_0000910c;
      uVar4 = DAT_0000910c;
      do {
        param_2 = param_2 + param_4;
        puVar6 = puVar6 + 0x7fffffff;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
    param_1 = (DAT_000090f0 >> 1) - DAT_0000910c;
  }
  return;
}


/* ===== FUN_000093b5 @ 000093b5 ===== */

void __allregs FUN_000093b5(void)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  undefined2 *puVar8;
  undefined2 *puVar9;
  
  uVar6 = DAT_000090c0 >> 1;
  DAT_000090d4 = DAT_000090d4 - (DAT_000090f0 >> 1) * uVar6;
  uVar1 = DAT_000090d4;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f8 >> 1) * uVar6;
  uVar3 = DAT_000090f8;
  uVar4 = DAT_000090d8;
  puVar8 = DAT_000090ec;
  DAT_000090bc = uVar6;
  do {
    uVar2 = DAT_000090f0 >> 1;
    iVar7 = ((0 << (DAT_00008490 & 0x1f) | uVar4 >> 0x20 - (DAT_00008490 & 0x1f)) <<
            (DAT_00008490 & 0x1f)) + DAT_000085b8;
    uVar5 = uVar1;
    puVar9 = puVar8;
    do {
      puVar8 = puVar9 + 1;
      *puVar9 = CONCAT11(*(undefined1 *)((uVar5 + uVar6 >> 0x16) + iVar7),
                         *(undefined1 *)((uVar5 >> 0x16) + iVar7));
      uVar5 = uVar5 + uVar6 + uVar6;
      uVar2 = uVar2 - 1;
      puVar9 = puVar8;
    } while (uVar2 != 0);
    uVar4 = uVar4 + DAT_000090bc;
    uVar3 = uVar3 - 1;
  } while (uVar3 != 0);
  return;
}


/* ===== FUN_000093c0 @ 000093c0 ===== */

void __allregs FUN_000093c0(void)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  undefined2 *puVar8;
  undefined2 *puVar9;
  
  uVar6 = DAT_000090c0 >> 1;
  DAT_000090d4 = DAT_000090d4 - (DAT_000090f0 >> 1) * uVar6;
  uVar1 = DAT_000090d4;
  DAT_000090d8 = DAT_000090d8 - (DAT_000090f8 >> 1) * uVar6;
  uVar3 = DAT_000090f8;
  uVar4 = DAT_000090d8;
  puVar8 = DAT_000090ec;
  DAT_000090bc = uVar6;
  do {
    uVar2 = DAT_000090f0 >> 1;
    iVar7 = ((0 << (DAT_00008490 & 0x1f) | uVar4 >> 0x20 - (DAT_00008490 & 0x1f)) <<
            (DAT_00008490 & 0x1f)) + DAT_000085b8;
    uVar5 = uVar1;
    puVar9 = puVar8;
    do {
      puVar8 = puVar9 + 1;
      *puVar9 = CONCAT11(*(undefined1 *)((uVar5 + uVar6 >> 0x16) + iVar7),
                         *(undefined1 *)((uVar5 >> 0x16) + iVar7));
      uVar5 = uVar5 + uVar6 + uVar6;
      uVar2 = uVar2 - 1;
      puVar9 = puVar8;
    } while (uVar2 != 0);
    uVar4 = uVar4 + DAT_000090bc;
    uVar3 = uVar3 - 1;
  } while (uVar3 != 0);
  return;
}


/* ===== FUN_0000944b @ 0000944b ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Variable defined which should be unmapped: param_6 */

void __allregs
FUN_0000944b(char *param_1,char param_2,byte param_3,int param_4,int *param_5,int param_6)

{
  uint *puVar1;
  byte bVar2;
  undefined2 unaff_ES;
  
  *param_1 = *param_1 + (char)param_1;
  *param_1 = *param_1 + (char)param_1;
  DAT_2abf0192 = DAT_2abf0192 & param_3;
  *(int *)(param_4 + 0x47d97c04) = *(int *)(param_4 + 0x47d97c04) - param_4;
  param_6 = CONCAT22(param_6._2_2_,unaff_ES);
  puVar1 = (uint *)(param_6 + 0x1bc2e40c);
  bVar2 = param_2 - 1U & 0x1f;
  *puVar1 = *puVar1 >> bVar2 | *puVar1 << 0x20 - bVar2;
  *param_5 = ((uint)param_1 & 0xffffff03) + 0x5e0afb68;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_00009482 @ 00009482 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00009482(undefined4 param_1,undefined4 param_2,uint *param_3,uint param_4,undefined4 *param_5,
            char *param_6,uint *param_7)

{
  byte bVar1;
  char cVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar9;
  uint *puVar10;
  char *pcVar11;
  char *pcVar12;
  byte in_CF;
  bool bVar13;
  char in_AF;
  undefined1 auStack_b172 [45418];
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  puVar9 = &uStack_4;
  cVar2 = '\x15';
  do {
    param_5 = param_5 + -1;
    puVar9 = puVar9 + -1;
    *puVar9 = *param_5;
    cVar2 = cVar2 + -1;
  } while ('\0' < cVar2);
  bVar3 = (byte)param_1 + 0x44;
  bVar13 = 0xbb < (byte)param_1 || CARRY1(bVar3,in_CF);
  bVar3 = bVar3 + in_CF;
  bVar1 = bVar3 - 0x1e;
  iVar5 = CONCAT31((int3)((uint)param_1 >> 8),bVar1 + bVar13) + 0x176dd9de +
          (uint)(0x1d < bVar3 || CARRY1(bVar1,bVar13));
  *param_6 = *param_6 - (char)((uint)iVar5 >> 8);
  bVar1 = DAT_826a1a82;
  uVar8 = param_4 | _DAT_7b1d934f;
  bVar13 = *param_7 < uVar8 || *param_7 - uVar8 == 0;
  *param_7 = (*param_7 - uVar8) - 1;
  bVar3 = bVar1 + 0x90;
  bVar4 = bVar3 - bVar13;
  bVar3 = ((char)bVar4 < '\0') << 7 | (bVar4 == 0) << 6 | in_AF << 4 |
          ((POPCOUNT(bVar4) & 1U) == 0) << 2 | 2U | (bVar1 < 0x70 || bVar3 < bVar13);
  uVar7 = CONCAT22((short)((uint)iVar5 >> 0x10),CONCAT11(bVar3,bVar4));
  uVar6 = CONCAT22((short)((uint)param_2 >> 0x10),
                   CONCAT11((byte)((uint)param_2 >> 8) & bVar4,(char)param_2));
  pcVar11 = param_6 + 1;
  *(char *)param_7 = *param_6;
  puVar10 = (uint *)((uint)auStack_b172 & *param_3);
  if (0 < (int)puVar10) {
    DAT_26a82186 = DAT_26a82186 - bVar3;
    *(char *)(uVar6 + 0xc4852826) =
         *(char *)(uVar6 + 0xc4852826) -
         ((byte)(uVar8 >> 8) & (byte)((uint)*(undefined4 *)(param_6 + 0xe) >> 8));
    _DAT_1f34eb29 = uVar7;
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  uVar7 = uVar7 & 0xffffff32;
  pcVar12 = (char *)((int)param_7 + 1);
  do {
    *(short *)pcVar12 = (short)uVar7;
    uVar7 = uVar8 + (int)&uStack_4;
    uVar6 = uVar6 - 1;
    if (uVar6 == 0) {
      uVar7 = puVar10[1];
      uVar8 = *puVar10 + DAT_000090bc;
      if (puVar10[2] + -1 == 0) {
        return;
      }
      puVar10[2] = puVar10[2] + -1;
      puVar10[1] = uVar7;
      *puVar10 = uVar8;
      uVar6 = DAT_000090f0 >> 1;
      pcVar11 = (char *)(((0 << (DAT_00008490 & 0x1f) | uVar8 >> 0x20 - (DAT_00008490 & 0x1f)) <<
                         (DAT_00008490 & 0x1f)) + DAT_000085b8);
    }
    uVar8 = uVar7 + (int)&uStack_4;
    uVar7 = (uint)CONCAT11(pcVar11[uVar8 >> 0x16],pcVar11[uVar7 >> 0x16]);
    pcVar12 = pcVar12 + 2;
  } while( true );
}


/* ===== FUN_000097cc @ 000097cc ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_000097cc(undefined4 *param_1)

{
  char cVar1;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar2;
  undefined4 auStack_8 [2];
  
  puVar2 = auStack_8 + 1;
  cVar1 = '\x11';
  do {
    param_1 = param_1 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *param_1;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_00009822 @ 00009822 ===== */

int __allregs FUN_00009822(int param_1,undefined4 *param_2)

{
  char cVar1;
  BADSPACEBASE *in_ESP;
  undefined4 *puVar2;
  undefined4 auStack_8 [2];
  
  puVar2 = auStack_8 + 1;
  cVar1 = '\x0f';
  do {
    param_2 = param_2 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *param_2;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
  return param_1 + 0x1af054d9;
}


/* ===== FUN_00009830 @ 00009830 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x00009838) overlaps instruction at (ram,0x00009837)
    */

void __allregs FUN_00009830(char *param_1,int param_2,int param_3,int param_4,int param_5)

{
  byte bVar1;
  int iVar2;
  uint *puVar3;
  BADSPACEBASE *in_ESP;
  
  while( true ) {
    puVar3 = (uint *)(param_4 + -0x497f8f4);
    bVar1 = (byte)param_2 & 0x1f;
    *puVar3 = *puVar3 >> bVar1 | *puVar3 << 0x20 - bVar1;
    puVar3 = (uint *)CONCAT31((int3)((uint)param_2 >> 8),(byte)param_2 | *(byte *)(param_4 + -0x70))
    ;
    iVar2 = CONCAT31((int3)((uint)param_1 >> 8),DAT_7e07d95b);
    *(int *)(param_5 + 0x242ac004) = *(int *)(param_5 + 0x242ac004) - param_5;
    param_1 = (char *)((iVar2 + -0x10192) - (uint)CARRY4((uint)&stack0x00000004,*puVar3));
    *param_1 = *param_1 + (char)param_1;
    param_2 = (int)puVar3 + -1;
    if (param_2 == 0 || *param_1 != '\0') break;
    param_4 = param_4 + -1;
  }
  s_KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK_00000005._4_4_ = in((short)param_3);
  *(char *)(param_3 + -0x2b) = *(char *)(param_3 + -0x2b) + '\x01';
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_00009c82 @ 00009c82 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs
FUN_00009c82(undefined4 param_1,undefined4 param_2,uint *param_3,uint param_4,undefined4 *param_5,
            char *param_6,uint *param_7)

{
  uint *puVar1;
  undefined4 uVar2;
  byte bVar3;
  undefined4 *puVar4;
  char cVar5;
  byte bVar6;
  byte bVar7;
  byte bVar8;
  int iVar10;
  uint uVar11;
  BADSPACEBASE *in_ESP;
  int *piVar12;
  undefined2 unaff_ES;
  undefined2 unaff_CS;
  byte in_CF;
  bool bVar13;
  byte in_AF;
  undefined1 auStack_b172 [45418];
  undefined4 uStack_8;
  undefined1 auStack_4 [4];
  char *pcVar9;
  
  puVar4 = (undefined4 *)auStack_4;
  bVar6 = (char)&uStack_8 + 4;
  cVar5 = '\x15';
  do {
    param_5 = param_5 + -1;
    puVar4 = puVar4 + -1;
    *puVar4 = *param_5;
    cVar5 = cVar5 + -1;
  } while ('\0' < cVar5);
  bVar7 = (byte)param_1 + 0x44;
  bVar13 = 0xbb < (byte)param_1 || CARRY1(bVar7,in_CF);
  bVar7 = bVar7 + in_CF;
  bVar3 = bVar7 - 0x1e;
  iVar10 = CONCAT31((int3)((uint)param_1 >> 8),bVar3 + bVar13) + 0x176dd9de +
           (uint)(0x1d < bVar7 || CARRY1(bVar3,bVar13));
  *param_6 = *param_6 - (char)((uint)iVar10 >> 8);
  bVar3 = DAT_826a1a82;
  uVar11 = param_4 | _DAT_7b1d934f;
  bVar13 = *param_7 < uVar11 || *param_7 - uVar11 == 0;
  *param_7 = (*param_7 - uVar11) - 1;
  bVar7 = bVar3 + 0x90;
  bVar8 = bVar7 - bVar13;
  bVar7 = ((char)bVar8 < '\0') << 7 | (bVar8 == 0) << 6 | in_AF << 4 |
          ((POPCOUNT(bVar8) & 1U) == 0) << 2 | 2 | (bVar3 < 0x70 || bVar7 < bVar13);
  puVar1 = (uint *)(param_6 + 1);
  *(char *)param_7 = *param_6;
  piVar12 = (int *)((uint)auStack_b172 & *param_3);
  if (0 < (int)piVar12) {
    DAT_26a82186 = DAT_26a82186 - bVar7;
    pcVar9 = (char *)(CONCAT22((short)((uint)param_2 >> 0x10),
                               CONCAT11((byte)((uint)param_2 >> 8) & bVar8,(char)param_2)) +
                     -0x3b7ad7da);
    *pcVar9 = *pcVar9 - ((byte)(uVar11 >> 8) & (byte)((uint)*(undefined4 *)(param_6 + 0xe) >> 8));
    _DAT_1f34eb29 = CONCAT22((short)((uint)iVar10 >> 0x10),CONCAT11(bVar7,bVar8));
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  *(char *)((int)param_7 + -0x5b) = -*(char *)((int)param_7 + -0x5b);
  *puVar1 = *puVar1 & 0xe6a0f9b8;
  bVar7 = 9 < (bVar6 & 0xf) | in_AF;
  uVar11 = CONCAT31((int3)((uint)auStack_4 >> 8),bVar6 + bVar7 * -6) & 0xffffff0f;
  bVar6 = (char)uVar11 + ((char)((uint)auStack_4 >> 8) - bVar7) * -0x25;
  pcVar9 = (char *)CONCAT22((short)(uVar11 >> 0x10),(ushort)bVar6);
  uVar2 = in((short)param_3);
  *(undefined4 *)((int)param_7 + 1) = uVar2;
  *pcVar9 = *pcVar9 + '\x01';
  *pcVar9 = *pcVar9 + bVar6;
  *pcVar9 = *pcVar9;
  *(int *)(param_6 + 0x47d97c05) = *(int *)(param_6 + 0x47d97c05) - (int)puVar1;
  *(undefined2 *)piVar12 = unaff_ES;
  iVar10 = *piVar12;
  *piVar12 = 9;
  puVar1 = (uint *)(iVar10 + 0x1bc2e40c);
  bVar6 = (char)param_2 - 2U & 0x1f;
  *puVar1 = *puVar1 >> bVar6 | *puVar1 << 0x20 - bVar6;
  *(undefined2 *)(piVar12 + -1) = unaff_CS;
  *(uint *)((int)param_7 + 5) = ((uint)(pcVar9 + -0x2abf0192) & 0xffffff03) + 0x5e0afb68;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_00009e60 @ 00009e60 ===== */

void __allregs FUN_00009e60(undefined1 param_1,undefined1 *param_2,undefined1 *param_3)

{
  int iVar1;
  
  DAT_0000ac64 = DAT_000028a5;
  DAT_0000ac60 = 0xff;
  *param_3 = 0;
  iVar1 = 0xff;
  do {
    param_3 = param_3 + 1;
    DAT_0000ac68 = param_2[3];
    DAT_0000ac69 = param_2[4];
    DAT_0000ac6a = param_2[5];
    FUN_0000ac70();
    *param_3 = param_1;
    iVar1 = iVar1 + -1;
    param_2 = param_2 + 3;
  } while (iVar1 != 0);
  return;
}


/* ===== FUN_00009eb4 @ 00009eb4 ===== */

void __allregs FUN_00009eb4(uint param_1,undefined1 param_2,undefined2 *param_3)

{
  undefined2 *puVar1;
  uint uVar2;
  uint uVar3;
  undefined3 uVar5;
  undefined1 *puVar4;
  undefined2 *puVar6;
  
  DAT_00009eac = param_1;
  DAT_00009eb0 = param_3;
  FUN_00009e60(param_2,&DAT_00005598,DAT_000085c0);
  uVar2 = DAT_00009eac & 1;
  uVar3 = DAT_00009eac >> 1;
  puVar4 = DAT_000085c0;
  puVar6 = DAT_00009eb0;
  do {
    uVar5 = (undefined3)((uint)puVar4 >> 8);
    puVar4 = (undefined1 *)CONCAT31(uVar5,*(undefined1 *)((int)puVar6 + 1));
    puVar1 = puVar6 + 1;
    *puVar6 = CONCAT11(*puVar4,*(undefined1 *)CONCAT31(uVar5,*(undefined1 *)puVar6));
    uVar3 = uVar3 - 1;
    puVar6 = puVar1;
  } while (uVar3 != 0);
  if (uVar2 != 0) {
    *(undefined1 *)puVar1 = *(undefined1 *)CONCAT31(uVar5,*(undefined1 *)puVar1);
  }
  return;
}


/* ===== FUN_00009ec0 @ 00009ec0 ===== */

void __allregs FUN_00009ec0(uint param_1,undefined1 param_2,undefined2 *param_3)

{
  undefined2 *puVar1;
  uint uVar2;
  uint uVar3;
  undefined3 uVar5;
  undefined1 *puVar4;
  undefined2 *puVar6;
  
  DAT_00009eac = param_1;
  DAT_00009eb0 = param_3;
  FUN_00009e60(param_2,&DAT_00005598,DAT_000085c0);
  uVar2 = DAT_00009eac & 1;
  uVar3 = DAT_00009eac >> 1;
  puVar4 = DAT_000085c0;
  puVar6 = DAT_00009eb0;
  do {
    uVar5 = (undefined3)((uint)puVar4 >> 8);
    puVar4 = (undefined1 *)CONCAT31(uVar5,*(undefined1 *)((int)puVar6 + 1));
    puVar1 = puVar6 + 1;
    *puVar6 = CONCAT11(*puVar4,*(undefined1 *)CONCAT31(uVar5,*(undefined1 *)puVar6));
    uVar3 = uVar3 - 1;
    puVar6 = puVar1;
  } while (uVar3 != 0);
  if (uVar2 != 0) {
    *(undefined1 *)puVar1 = *(undefined1 *)CONCAT31(uVar5,*(undefined1 *)puVar1);
  }
  return;
}


/* ===== FUN_00009f05 @ 00009f05 ===== */

void __allregs FUN_00009f05(void)

{
  byte bVar1;
  undefined2 uVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  uint uVar7;
  uint uVar8;
  byte *pbVar9;
  byte *pbVar10;
  
  pbVar6 = (byte *)(DAT_000028a5 * 3);
  pbVar9 = pbVar6 + 0x5598;
  iVar4 = 0xff - DAT_000028a5;
  do {
    iVar5 = iVar4 + 1;
    uVar8 = 0x400;
    pbVar10 = pbVar9;
    do {
      uVar7 = (uint)*pbVar10 + (uint)pbVar10[1] * 2 + (uint)pbVar10[2];
      if (uVar7 < uVar8) {
        pbVar6 = pbVar10;
        uVar8 = uVar7;
      }
      pbVar10 = pbVar10 + 3;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    bVar3 = pbVar9[2];
    LOCK();
    uVar2 = *(undefined2 *)pbVar6;
    *(undefined2 *)pbVar6 = *(undefined2 *)pbVar9;
    UNLOCK();
    LOCK();
    bVar1 = pbVar6[2];
    pbVar6[2] = bVar3;
    UNLOCK();
    *(undefined2 *)pbVar9 = uVar2;
    pbVar9[2] = bVar1;
    pbVar9 = pbVar9 + 3;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return;
}


/* ===== FUN_00009f10 @ 00009f10 ===== */

void __allregs FUN_00009f10(void)

{
  byte bVar1;
  undefined2 uVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  uint uVar7;
  uint uVar8;
  byte *pbVar9;
  byte *pbVar10;
  
  pbVar6 = (byte *)(DAT_000028a5 * 3);
  pbVar9 = pbVar6 + 0x5598;
  iVar4 = 0xff - DAT_000028a5;
  do {
    iVar5 = iVar4 + 1;
    uVar8 = 0x400;
    pbVar10 = pbVar9;
    do {
      uVar7 = (uint)*pbVar10 + (uint)pbVar10[1] * 2 + (uint)pbVar10[2];
      if (uVar7 < uVar8) {
        pbVar6 = pbVar10;
        uVar8 = uVar7;
      }
      pbVar10 = pbVar10 + 3;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    bVar3 = pbVar9[2];
    LOCK();
    uVar2 = *(undefined2 *)pbVar6;
    *(undefined2 *)pbVar6 = *(undefined2 *)pbVar9;
    UNLOCK();
    LOCK();
    bVar1 = pbVar6[2];
    pbVar6[2] = bVar3;
    UNLOCK();
    *(undefined2 *)pbVar9 = uVar2;
    pbVar9[2] = bVar1;
    pbVar9 = pbVar9 + 3;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return;
}


/* ===== FUN_00009f25 @ 00009f25 ===== */

/* WARNING: Instruction at (ram,0x00009f26) overlaps instruction at (ram,0x00009f25)
    */
/* WARNING: Type propagation algorithm not settling */

void __allregs
FUN_00009f25(uint param_1,int param_2,ushort *param_3,uint param_4,undefined4 *param_5,
            ushort *param_6,ushort *param_7,int param_8)

{
  ushort uVar1;
  undefined4 *puVar2;
  char cVar3;
  ushort uVar4;
  ushort uVar5;
  int iVar6;
  BADSPACEBASE *in_ESP;
  undefined4 uStack_8;
  undefined1 auStack_4 [4];
  
  puVar2 = (undefined4 *)auStack_4;
  cVar3 = '\x1c';
  do {
    param_5 = param_5 + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *param_5;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  do {
    *(char *)(param_1 * 2) = *(char *)(param_1 * 2) + (char)param_1;
    *(char *)(param_4 + 0x1fb60ffe) = *(char *)(param_4 + 0x1fb60ffe) + (char)param_2;
    param_1 = (uint)(byte)param_7[1];
    if (param_4 + (uint)*(byte *)((int)param_7 + 1) * 2 + param_1 < auStack_4) {
      param_3 = param_7;
    }
    param_2 = param_2 + -1;
    param_7 = (ushort *)((int)param_7 + 3);
    if (param_2 == 0) {
      uVar4 = param_6[1];
      LOCK();
      uVar1 = *param_3;
      *param_3 = *param_6;
      param_1 = (uint)uVar1;
      UNLOCK();
      LOCK();
      uVar5 = param_3[1];
      *(char *)(param_3 + 1) = (char)uVar4;
      UNLOCK();
      *param_6 = uVar1;
      *(char *)(param_6 + 1) = (char)uVar5;
      param_6 = (ushort *)((int)param_6 + 3);
      iVar6 = param_8 + -1;
      param_2 = param_8;
      param_7 = param_6;
      param_8 = iVar6;
      if (iVar6 == 0) {
        return;
      }
    }
    param_4 = (uint)(byte)*param_7;
  } while( true );
}


/* ===== FUN_00009f66 @ 00009f66 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00009f66(void)

{
  int iVar1;
  int iVar2;
  byte *pbVar3;
  undefined1 uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  int iVar7;
  
  FUN_00009f10();
  puVar5 = &DAT_00005598;
  puVar6 = (undefined4 *)&DAT_00005260;
  for (iVar2 = 0xc0; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  puVar5 = (undefined4 *)&DAT_00005260;
  puVar6 = (undefined4 *)(DAT_00000c93 + 0xea);
  for (iVar2 = 0xc0; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  FUN_0000a033();
  DAT_0000ac64 = DAT_000028a5;
  DAT_0000ac60 = 0xff;
  FUN_0000bc9c();
  iVar2 = 0x100;
  pbVar3 = (byte *)(_LAB_000085c4 & 0xffffff00);
  iVar7 = 0;
  do {
    iVar1 = iVar7 * 3;
    uVar4 = 0x17;
    *pbVar3 = (byte)((&DAT_00005260)[iVar1] + (&DAT_00005261)[iVar1] + (&DAT_00005261)[iVar1] +
                    (&DAT_00005262)[iVar1]) / 0x17;
    pbVar3 = pbVar3 + 1;
    iVar7 = iVar7 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  DAT_0000ac64 = DAT_000028a5;
  DAT_0000ac60 = 0xff;
  DAT_0000ac68 = 0xf;
  DAT_0000ac69 = 0x20;
  DAT_0000ac6a = 0x3f;
  FUN_0000ac70();
  LAB_000085c8 = uVar4;
  return;
}


/* ===== FUN_00009f70 @ 00009f70 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs FUN_00009f70(void)

{
  int iVar1;
  int iVar2;
  byte *pbVar3;
  undefined1 uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  int iVar7;
  
  FUN_00009f10();
  puVar5 = &DAT_00005598;
  puVar6 = (undefined4 *)&DAT_00005260;
  for (iVar2 = 0xc0; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  puVar5 = (undefined4 *)&DAT_00005260;
  puVar6 = (undefined4 *)(DAT_00000c93 + 0xea);
  for (iVar2 = 0xc0; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  FUN_0000a033();
  DAT_0000ac64 = DAT_000028a5;
  DAT_0000ac60 = 0xff;
  FUN_0000bc9c();
  iVar2 = 0x100;
  pbVar3 = (byte *)(_LAB_000085c4 & 0xffffff00);
  iVar7 = 0;
  do {
    iVar1 = iVar7 * 3;
    uVar4 = 0x17;
    *pbVar3 = (byte)((&DAT_00005260)[iVar1] + (&DAT_00005261)[iVar1] + (&DAT_00005261)[iVar1] +
                    (&DAT_00005262)[iVar1]) / 0x17;
    pbVar3 = pbVar3 + 1;
    iVar7 = iVar7 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  DAT_0000ac64 = DAT_000028a5;
  DAT_0000ac60 = 0xff;
  DAT_0000ac68 = 0xf;
  DAT_0000ac69 = 0x20;
  DAT_0000ac6a = 0x3f;
  FUN_0000ac70();
  LAB_000085c8 = uVar4;
  return;
}


/* ===== FUN_0000a02e @ 0000a02e ===== */

void __allregs FUN_0000a02e(void)

{
  return;
}


/* ===== FUN_0000a033 @ 0000a033 ===== */

void __allregs FUN_0000a033(void)

{
  byte *pbVar1;
  byte *pbVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  
  pbVar4 = &DAT_00005260;
  pbVar5 = &DAT_00004f60;
  iVar3 = 0x100;
  do {
    *pbVar5 = *pbVar4 >> 1;
    pbVar1 = pbVar4 + 2;
    pbVar2 = pbVar5 + 2;
    pbVar5[1] = pbVar4[1] >> 1;
    pbVar4 = pbVar4 + 3;
    pbVar5 = pbVar5 + 3;
    *pbVar2 = *pbVar1 >> 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}


/* ===== FUN_0000a080 @ 0000a080 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x0000a08c) overlaps instruction at (ram,0x0000a08b)
    */

void __allregs FUN_0000a080(int param_1,char *param_2,undefined2 param_3,int param_4)

{
  byte bVar1;
  uint uVar2;
  uint *puVar3;
  
  puVar3 = (uint *)(param_1 + 1);
  LOCK();
  bVar1 = *(byte *)puVar3;
  *(byte *)puVar3 = *(byte *)puVar3 + (byte)puVar3;
  UNLOCK();
  *puVar3 = *puVar3 + param_4 + (uint)CARRY1(bVar1,(byte)puVar3);
  uVar2 = *puVar3;
  *puVar3 = (uint)(*puVar3 + (int)puVar3);
  if (!CARRY4(uVar2,(uint)puVar3) && *puVar3 != 0) {
    *puVar3 = (uint)(*puVar3 + (int)puVar3);
  }
  *param_2 = *param_2 + (char)((ushort)param_3 >> 8);
  *param_2 = '\0';
  param_2[0x7a] = param_2[0x7a] + (char)((uint)param_2 >> 8);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000a478 @ 0000a478 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000a478(undefined4 param_1)

{
  int *piVar1;
  
  piVar1 = (int *)CONCAT31((int3)((uint)param_1 >> 8),(char)param_1 + '\x04');
  *piVar1 = *piVar1 + (int)piVar1;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000a9b1 @ 0000a9b1 ===== */

void __allregs
FUN_0000a9b1(undefined4 param_1,char param_2,char *param_3,undefined2 param_4,undefined4 *param_5,
            char *param_6,int param_7,uint param_8)

{
  byte bVar1;
  short sVar2;
  char cVar3;
  byte bVar4;
  char cVar5;
  char *pcVar6;
  byte *pbVar7;
  ushort *puVar8;
  char cVar10;
  BADSPACEBASE *in_ESP;
  undefined4 **ppuVar11;
  uint *puVar12;
  int in_GS_OFFSET;
  char in_CF;
  char acStack_b [3];
  undefined4 *apuStack_8 [2];
  undefined2 uVar9;
  
  ppuVar11 = apuStack_8 + 1;
  apuStack_8[1] = param_5;
  cVar3 = '\x03';
  do {
    param_5 = param_5 + -1;
    ppuVar11 = ppuVar11 + -1;
    *ppuVar11 = (undefined4 *)*param_5;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  LOCK();
  cVar3 = *param_3;
  *param_3 = param_2;
  UNLOCK();
  param_3[0x4c] = param_3[0x4c] + cVar3 + in_CF;
  uVar9 = (undefined2)((uint)param_1 >> 0x10);
  cVar3 = (char)((uint)param_1 >> 8) * '\x02';
  pcVar6 = (char *)CONCAT22(uVar9,CONCAT11(cVar3,(char)param_1));
  puVar12 = (uint *)(param_7 + -1);
  acStack_b[(int)param_6 * 4] = acStack_b[(int)param_6 * 4] + cVar3;
  *pcVar6 = *pcVar6 + (char)param_3;
  bVar4 = (char)param_1 + 0x6b;
  pcVar6 = (char *)(CONCAT31((int3)((uint)pcVar6 >> 8),bVar4) + 0x4e);
  *pcVar6 = *pcVar6 + bVar4;
  pbVar7 = (byte *)CONCAT22(uVar9,CONCAT11(cVar3 + bVar4,bVar4));
  cVar10 = (char)param_4;
  pbVar7[-0x5bffa4dc] = pbVar7[-0x5bffa4dc] + cVar10;
  bVar1 = *pbVar7;
  *pbVar7 = *pbVar7 + bVar4;
  *(char *)puVar12 = *(char *)puVar12 + cVar10 + CARRY1(bVar1,bVar4);
  *(char *)puVar12 = *param_6;
  puVar8 = (ushort *)((uint)pbVar7 | 0x10900061);
  sVar2 = ((ushort)puVar8 & 3) - (*puVar8 & 3);
  *puVar8 = *puVar8 + (ushort)(0 < sVar2) * sVar2;
  pcVar6 = (char *)((int)puVar8 + in_GS_OFFSET + 0x26);
  cVar5 = (char)puVar8;
  *pcVar6 = *pcVar6 + cVar5;
  pcVar6 = (char *)segment(0x1c00,(short)(param_6 + 1));
  *pcVar6 = *pcVar6 + cVar5;
  *(char *)(param_7 + 0x6a) = *(char *)(param_7 + 0x6a) + cVar10 + (*puVar12 < param_8);
  param_6[param_8 * 2 + 0x6e] = param_6[param_8 * 2 + 0x6e] + cVar3 + bVar4;
  *(char *)puVar8 = (char)*puVar8 + cVar5;
  out(*(undefined4 *)(param_6 + 1),param_4);
  return;
}


/* ===== FUN_0000ab11 @ 0000ab11 ===== */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x0000abc3) overlaps instruction at (ram,0x0000abc0)
    */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Type propagation algorithm not settling */

void __allregs
FUN_0000ab11(char *param_1,int param_2,int param_3,undefined4 *param_4,undefined1 *param_5,
            undefined6 *param_6,int param_7,int param_8)

{
  byte *pbVar1;
  undefined6 uVar2;
  undefined6 uVar3;
  int iVar4;
  byte bVar5;
  undefined1 uVar6;
  byte bVar7;
  char cVar8;
  char cVar16;
  char *pcVar9;
  byte *pbVar10;
  undefined4 *puVar11;
  uint uVar12;
  char *pcVar13;
  int iVar14;
  byte bVar17;
  uint *puVar15;
  byte bVar18;
  int *piVar19;
  undefined4 uVar20;
  BADSPACEBASE *in_ESP;
  bool in_CF;
  bool in_ZF;
  char acStack_2e [26];
  undefined1 *puStack_14;
  undefined4 *puStack_8;
  undefined4 *puStack_4;
  undefined1 auStack_3 [3];
  
  puVar11 = &stack0xfffffffc;
  puStack_4 = param_4;
  cVar16 = '\x03';
  do {
    param_4 = param_4 + -1;
    puVar11 = puVar11 + -1;
    *puVar11 = *param_4;
    cVar16 = cVar16 + -1;
  } while ('\0' < cVar16);
  puStack_14 = (undefined1 *)&stack0xfffffffc;
  if (!in_CF && !in_ZF) {
    param_1 = *(char **)param_1;
  }
  pcVar9 = (char *)((int)auStack_3 * 4 + 0x4000043);
  cVar16 = (char)((uint)param_1 >> 8);
  *pcVar9 = *pcVar9 + cVar16;
  piVar19 = (int *)(param_3 + 1);
  *param_1 = *param_1 + cVar16;
  pcVar9 = param_1 + 1;
  param_1[0x44003ec2] = param_1[0x44003ec2] + (char)piVar19;
  DAT_3b944000 = DAT_3b944000 - (char)((uint)param_7 >> 8);
  (&stack0x7c100037)[(int)pcVar9] = (&stack0x7c100037)[(int)pcVar9] + (char)pcVar9;
  bVar5 = in(0xf7);
  puVar15 = (uint *)CONCAT31((int3)((uint)pcVar9 >> 8),bVar5);
  uVar12 = *puVar15;
  *(byte *)puVar15 = (byte)*puVar15 + bVar5;
  cVar16 = (char)((uint)pcVar9 >> 8);
  if ((char)(byte)*puVar15 < '\0') {
    pcVar9 = (char *)((int)param_6 * 9 + 0x33);
    *pcVar9 = *pcVar9 + cVar16;
    *(byte *)puVar15 = (byte)*puVar15 + (char)piVar19;
    iVar14 = *piVar19;
    *(char *)((int)&param_8 + param_7) = *(char *)((int)&param_8 + param_7) + bVar5;
    *puVar15 = *puVar15 ^ (uint)puVar15;
    pbVar10 = (byte *)((int)puVar15 + 1);
    puVar11 = (undefined4 *)
              CONCAT22((short)((uint)pbVar10 >> 0x10),
                       CONCAT11((char)((uint)pbVar10 >> 8) + (char)pbVar10,(char)pbVar10));
    *(char *)((int)puVar11 + -0x5bffd312) = *(char *)((int)puVar11 + -0x5bffd312) + '/';
    iVar4 = -param_8;
    acStack_2e[iVar4] = acStack_2e[iVar4] + (char)((int)&param_8 + iVar4);
    (&stack0xa68fffd0)[iVar14 * 8 + iVar4] =
         (&stack0xa68fffd0)[iVar14 * 8 + iVar4] + (char)((uint)((int)&param_8 + iVar4) >> 8);
    param_6 = (undefined6 *)*puVar11;
    param_5 = (undefined1 *)puVar11[1];
    uVar20 = puVar11[4];
    piVar19 = (int *)puVar11[5];
    uVar12 = puVar11[7];
    *(char *)(uVar12 + 0x22) = *(char *)(uVar12 + 0x22) + (char)uVar12;
    pcVar13 = (char *)(uVar12 & 0x23e80400);
    *pcVar13 = *pcVar13 + (char)piVar19;
    pcVar9 = (char *)(CONCAT31((int3)((uint)uVar20 >> 8),0x22) + 0x21 + (int)pcVar13 * 4);
    *pcVar9 = *pcVar9 + (char)((uint)pcVar13 >> 8);
    *pcVar13 = *pcVar13;
    param_2 = puVar11[8];
    *pcVar13 = '\0';
    uVar6 = in(0x33);
    pbVar10 = (byte *)CONCAT31((int3)((uint)pcVar13 >> 8),uVar6);
  }
  else {
    *(char *)(param_7 + -0x7c9bffde) =
         *(char *)(param_7 + -0x7c9bffde) + (char)((uint)piVar19 >> 8) + CARRY1((byte)uVar12,bVar5);
    *puVar15 = *puVar15 & (uint)puVar15;
    *(char *)(param_2 + 0x20) = *(char *)(param_2 + 0x20) + (char)param_7;
    pbVar10 = (byte *)CONCAT22((short)((uint)pcVar9 >> 0x10),CONCAT11(cVar16 * '\x02',bVar5));
  }
  bVar5 = *pbVar10;
  bVar18 = (byte)piVar19;
  *pbVar10 = *pbVar10 + bVar18;
  bVar7 = (char)pbVar10 + '\x1e' + CARRY1(bVar5,bVar18);
  piVar19 = (int *)CONCAT31((int3)((uint)pbVar10 >> 8),bVar7);
  pbVar10 = (byte *)(param_2 + -0x1bbfffe4 + (int)param_6 * 8);
  bVar5 = *pbVar10;
  *pbVar10 = *pbVar10 + bVar7;
  iVar14 = (int)piVar19 + (-(uint)CARRY1(bVar5,bVar7) - *piVar19);
  bVar7 = (byte)iVar14;
  bVar17 = bVar7 / 0x1a;
  bVar5 = bVar7 % 0x1a;
  piVar19 = (int *)CONCAT31((int3)(CONCAT22((short)((uint)iVar14 >> 0x10),CONCAT11(bVar17,bVar7)) >>
                                  8),bVar5);
  *(char *)((int)piVar19 + 0x240019c9) = *(char *)((int)piVar19 + 0x240019c9) + bVar18;
  uVar2 = *(undefined6 *)piVar19;
  *(byte *)piVar19 = (char)*piVar19 + bVar5;
  uVar3 = *param_6;
  pbVar10 = (byte *)uVar3;
  *(char *)(param_2 * 9) = *(char *)(param_2 * 9) + bVar17;
  cVar16 = (char)uVar3;
  *(char *)((int)piVar19 + 0x440015d3) = *(char *)((int)piVar19 + 0x440015d3) + cVar16;
  if (param_2 == 0) {
    pcVar9 = (char *)((int)piVar19 + (int)param_5 * 4 + 0xf);
    *pcVar9 = *pcVar9 + bVar17;
    *(char *)piVar19 = (char)*piVar19 + cVar16;
  }
  pbVar1 = (byte *)(piVar19 + -2);
  bVar7 = *pbVar1;
  *pbVar1 = *pbVar1 + bVar5;
  puVar15 = (uint *)((int)piVar19 + (uint)CARRY1(bVar7,bVar5) + *piVar19);
  uVar12 = *puVar15;
  pbVar1 = (byte *)((int)puVar15 + *puVar15);
  bVar5 = *pbVar10;
  bVar17 = (byte)((uint6)uVar3 >> 8);
  bVar7 = *pbVar10 + bVar17;
  *pbVar10 = bVar7 + CARRY4((uint)puVar15,uVar12);
  cVar8 = in(0x56);
  piVar19 = (int *)CONCAT31((int3)((uint)pbVar1 >> 8),cVar8);
  *piVar19 = (int)piVar19 +
             (uint)(CARRY1((byte)pbVar1,*pbVar1) ||
                   CARRY1((byte)pbVar1 + *pbVar1,
                          CARRY1(bVar5,bVar17) || CARRY1(bVar7,CARRY4((uint)puVar15,uVar12)))) +
             *piVar19;
  *(char *)(param_2 + -0x4f9bfff0) = *(char *)(param_2 + -0x4f9bfff0) + cVar8;
  LocalDescriptorTableRegister((short)*piVar19);
  uVar12 = in(0xe);
  pcVar9 = (char *)((int)param_6 + (int)uVar2);
  cVar8 = (char)uVar12;
  *pcVar9 = *pcVar9 + cVar8;
  *(char *)(uVar12 + 0x5e) = *(char *)(uVar12 + 0x5e) + cVar8;
  pcVar9 = (char *)(uVar12 | 0xca2c400);
  pcVar9[-0x5bfff414] = pcVar9[-0x5bfff414] + cVar16;
  *pcVar9 = *pcVar9 + cVar8;
  *(undefined1 *)param_6 = *param_5;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000ac70 @ 0000ac70 ===== */

void __allregs FUN_0000ac70(void)

{
  char *pcVar1;
  char *pcVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  char *pcVar6;
  uint uVar7;
  
  bRam0000acb6 = DAT_0000ac68 >> 1;
  bRam0000acc0 = DAT_0000ac69 >> 1;
  bRam0000acca = DAT_0000ac6a >> 1;
  iVar4 = (DAT_0000ac60 + 1) - DAT_0000ac64;
  pcVar6 = &DAT_00004f60 + DAT_0000ac64 * 3;
  uVar5 = 0xffffffff;
  do {
    while( true ) {
      pcVar1 = pcVar6 + 1;
      cVar3 = *pcVar6;
      pcVar2 = pcVar6 + 2;
      pcVar6 = pcVar6 + 3;
      uVar7 = *(int *)((uint)(byte)(cVar3 - 0xf) * 4 + 0xa060) +
              *(int *)(&DAT_0000a460 + (uint)(byte)(*pcVar1 - 0xf) * 4) +
              *(int *)(&DAT_0000a860 + (uint)(byte)(*pcVar2 - 0xf) * 4);
      if (uVar7 < uVar5) break;
      iVar4 = iVar4 + -1;
      if (iVar4 == 0) {
        return;
      }
    }
  } while ((uVar7 != 0) && (iVar4 = iVar4 + -1, uVar5 = uVar7, iVar4 != 0));
  return;
}


/* ===== FUN_0000ad1e @ 0000ad1e ===== */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

byte * __allregs FUN_0000ad1e(uint param_1,undefined4 param_2,byte *param_3,undefined2 param_4)

{
  longlong lVar1;
  short sVar2;
  int iVar3;
  char cVar4;
  undefined1 uVar5;
  byte bVar6;
  byte bVar7;
  byte bVar13;
  undefined *puVar8;
  byte *pbVar9;
  undefined3 uVar14;
  undefined1 *puVar10;
  undefined4 uVar11;
  undefined2 uVar15;
  uint uVar12;
  uint uVar16;
  uint uVar17;
  ushort uVar18;
  uint uVar19;
  uint uVar20;
  int iVar21;
  byte *pbVar22;
  uint uVar23;
  byte *pbVar24;
  byte *pbVar25;
  bool bVar26;
  
  iVar3 = DAT_00003918;
  DAT_0000ace6 = param_1 >> 8 & 0xff;
  DAT_0000ad1a = (byte)(param_1 >> 8);
  DAT_0000acfe = param_3 + (ushort)((ushort)DAT_0000ad1a * (ushort)(byte)param_1);
  DAT_0000ad02 = (uint)*(ushort *)(DAT_00000c93 + 0x486) * 0x100 + _DAT_0000390d;
  lVar1 = (0xffffffff /
          ((ulonglong)*(ushort *)(DAT_00000c93 + 0x480) *
          (ulonglong)*(ushort *)(DAT_00000c93 + 0xca))) *
          (ulonglong)*(uint *)((uint)*(ushort *)(DAT_00000c93 + 0x47e) * 4 + 0x4224);
  puVar8 = (undefined *)((uint)lVar1 >> 0x14 | (int)((ulonglong)lVar1 >> 0x20) << 0xc);
  if (puVar8 < &DAT_00001999) {
    puVar8 = &DAT_00001999;
  }
  _DAT_0000ad0a = (uint)puVar8 >> 0x10;
  _DAT_0000ad12 = _DAT_0000ad0a * DAT_0000ace6;
  _DAT_0000ad06 = (int)puVar8 << 0x10;
  _DAT_0000ad16 = (int)puVar8 << 0x11;
  _DAT_0000ad0e = ((uint)puVar8 >> 0xf) * DAT_0000ace6;
  DAT_0000acee = (byte *)(*(int *)(FUN_0000944b + (*(uint *)(DAT_00000c93 + 0x488) >> 0x17) * 4 + 5)
                         >> 0xf);
  sVar2 = (short)((int)((char)param_2 * 0x8000 + ((uint)puVar8 >> 1)) / (int)puVar8);
  lVar1 = (longlong)((int)sVar2 << 0x10) * (longlong)(int)DAT_0000acee;
  _DAT_0000acf2 = (byte *)((int)((ulonglong)lVar1 >> 0x20) << 0x10 | (uint)lVar1 >> 0x10);
  uVar16 = (uint)(ushort)(sVar2 + *(short *)(DAT_00000c93 + 0x476));
  uVar18 = (short)((int)((char)-(char)((uint)param_2 >> 8) * 0x10000 + ((uint)puVar8 >> 1)) /
                  (int)puVar8) + *(short *)(DAT_00000c93 + 0x478);
  DAT_0000ad1c = (char)*(ushort *)(DAT_00000c93 + 0x482);
  pbVar9 = (byte *)(uint)(*(ushort *)(DAT_00000c93 + 0x482) >> 8);
  DAT_0000acfa = param_3;
  DAT_0000ad1b = (byte)param_4;
  switch(pbVar9) {
  case (byte *)0x0:
    break;
  case (byte *)0x1:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar12 = 0;
      uVar20 = (uint)DAT_0000ad1a;
      pbVar9 = DAT_0000bc90;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar17 & 0xffff;
          if ((char)(uVar17 >> 8) == '\0') break;
          if (-1 < (short)uVar17) goto LAB_0000b703;
          uVar17 = uVar19 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar20 <= pbVar25) goto LAB_0000b703;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        pbVar22 = pbVar9;
        if (DAT_0000ad1d <= (byte)uVar19) {
          bVar6 = *pbVar25;
          pbVar22 = (byte *)CONCAT31((int3)((uint)pbVar9 >> 8),bVar6);
          if (bVar6 != 0) {
            puVar10 = (undefined1 *)
                      CONCAT22((short)((uint)pbVar9 >> 0x10),
                               CONCAT11(*(undefined1 *)(iVar21 + iVar3),bVar6));
            uVar14 = (undefined3)((uint)puVar10 >> 8);
            uVar5 = *(undefined1 *)CONCAT31(uVar14,*puVar10);
            pbVar22 = (byte *)CONCAT31(uVar14,uVar5);
            *(undefined1 *)(iVar21 + iVar3) = uVar5;
          }
        }
        uVar17 = uVar19 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
        pbVar9 = pbVar22;
      } while (pbVar25 < pbVar24 + uVar20);
LAB_0000b703:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x2:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar12 = 0;
      uVar20 = (uint)DAT_0000ad1a;
      pbVar9 = DAT_0000bc90;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar17 & 0xffff;
          if ((char)(uVar17 >> 8) == '\0') break;
          if (-1 < (short)uVar17) goto LAB_0000b813;
          uVar17 = uVar19 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar20 <= pbVar25) goto LAB_0000b813;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        pbVar22 = pbVar9;
        if (DAT_0000ad1d <= (byte)uVar19) {
          bVar6 = *pbVar25;
          pbVar22 = (byte *)CONCAT31((int3)((uint)pbVar9 >> 8),bVar6);
          if (bVar6 != 0) {
            puVar10 = (undefined1 *)
                      CONCAT22((short)((uint)pbVar9 >> 0x10),
                               CONCAT11(*(undefined1 *)(iVar21 + iVar3),bVar6));
            uVar5 = *puVar10;
            pbVar22 = (byte *)CONCAT31((int3)((uint)puVar10 >> 8),uVar5);
            *(undefined1 *)(iVar21 + iVar3) = uVar5;
          }
        }
        uVar17 = uVar19 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
        pbVar9 = pbVar22;
      } while (pbVar25 < pbVar24 + uVar20);
LAB_0000b813:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x3:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar12 = 0;
      uVar20 = (uint)DAT_0000ad1a;
      pbVar9 = DAT_0000bc90;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar17 & 0xffff;
          if ((char)(uVar17 >> 8) == '\0') break;
          if (-1 < (short)uVar17) goto LAB_0000b925;
          uVar17 = uVar19 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar20 <= pbVar25) goto LAB_0000b925;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        pbVar22 = pbVar9;
        if (DAT_0000ad1d <= (byte)uVar19) {
          bVar6 = *pbVar25;
          pbVar22 = (byte *)CONCAT31((int3)((uint)pbVar9 >> 8),bVar6);
          if (bVar6 != 0) {
            uVar15 = (undefined2)((uint)pbVar9 >> 0x10);
            puVar10 = (undefined1 *)
                      CONCAT22(uVar15,CONCAT11(*(undefined1 *)
                                                CONCAT22(uVar15,CONCAT11(*(undefined1 *)
                                                                          (iVar21 + iVar3),bVar6)),
                                               bVar6));
            uVar5 = *puVar10;
            pbVar22 = (byte *)CONCAT31((int3)((uint)puVar10 >> 8),uVar5);
            *(undefined1 *)(iVar21 + iVar3) = uVar5;
          }
        }
        uVar17 = uVar19 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
        pbVar9 = pbVar22;
      } while (pbVar25 < pbVar24 + uVar20);
LAB_0000b925:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x4:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar12 = 0;
      uVar20 = (uint)DAT_0000ad1a;
      pbVar9 = (byte *)CONCAT22((short)((uint)DAT_0000bc90 >> 0x10),
                                CONCAT11(DAT_0000ad1c,(char)DAT_0000bc90));
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar17 & 0xffff;
          if ((char)(uVar17 >> 8) == '\0') break;
          if (-1 < (short)uVar17) goto LAB_0000ba38;
          uVar17 = uVar19 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar20 <= pbVar25) goto LAB_0000ba38;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        if (DAT_0000ad1d <= (byte)uVar19) {
          uVar14 = (undefined3)((uint)pbVar9 >> 8);
          pbVar9 = (byte *)CONCAT31(uVar14,*pbVar25);
          if (*pbVar25 != 0) {
            bVar6 = *pbVar9;
            pbVar9 = (byte *)CONCAT31(uVar14,bVar6);
            *(byte *)(iVar21 + iVar3) = bVar6;
          }
        }
        uVar17 = uVar19 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
      } while (pbVar25 < pbVar24 + uVar20);
LAB_0000ba38:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x5:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = 0;
      uVar12 = (uint)DAT_0000ad1a;
      pbVar9 = DAT_0000bc94;
      uVar20 = (uint)uVar18;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar20 & 0xffff;
          if ((char)(uVar20 >> 8) == '\0') break;
          if (-1 < (short)uVar20) goto LAB_0000b4f1;
          uVar20 = uVar19 + 1;
          bVar26 = CARRY4(uVar17,_DAT_0000ad06);
          uVar17 = uVar17 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar12 <= pbVar25) goto LAB_0000b4f1;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        if ((DAT_0000ad1d <= (byte)uVar19) &&
           (pbVar9 = (byte *)CONCAT22((short)((uint)pbVar9 >> 0x10),CONCAT11(*pbVar25,(char)pbVar9))
           , *pbVar25 != 0)) {
          uVar14 = (undefined3)((uint)pbVar9 >> 8);
          uVar5 = *(undefined1 *)CONCAT31(uVar14,*(undefined1 *)(iVar21 + iVar3));
          pbVar9 = (byte *)CONCAT31(uVar14,uVar5);
          *(undefined1 *)(iVar21 + iVar3) = uVar5;
        }
        uVar20 = uVar19 + 1;
        bVar26 = CARRY4(uVar17,_DAT_0000ad06);
        uVar17 = uVar17 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
      } while (pbVar25 < pbVar24 + uVar12);
LAB_0000b4f1:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x6:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar12 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar20 = 0;
      pbVar9 = (byte *)(uint)DAT_0000ad1a;
      pbVar22 = pbVar24 + (int)pbVar9;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar17 = uVar12 & 0xffff;
          if ((char)(uVar12 >> 8) == '\0') break;
          if (-1 < (short)uVar12) goto LAB_0000b3f4;
          uVar12 = uVar17 + 1;
          bVar26 = CARRY4(uVar20,_DAT_0000ad06);
          uVar20 = uVar20 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar22 <= pbVar25) goto LAB_0000b3f4;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar17);
        if (DAT_0000ad1d <= (byte)uVar17) {
          bVar6 = *pbVar25;
          pbVar9 = (byte *)(uint)bVar6;
          if (bVar6 != 0) {
            *(byte *)(iVar21 + iVar3) = bVar6;
          }
        }
        uVar12 = uVar17 + 1;
        bVar26 = CARRY4(uVar20,_DAT_0000ad06);
        uVar20 = uVar20 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
      } while (pbVar25 < pbVar22);
LAB_0000b3f4:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x7:
    pbVar24 = param_3;
    if (((byte)param_4 & 1) != 0) {
      _DAT_0000ad0e = -_DAT_0000ad0e;
      DAT_0000ace6 = -DAT_0000ace6;
      pbVar24 = DAT_0000acfe + DAT_0000ace6;
    }
    uVar23 = 0;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar9;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar24 = pbVar24 + DAT_0000ace6;
        }
        pbVar24 = pbVar24 + _DAT_0000ad0e;
        if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
          return pbVar9;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (uVar16 & 0xff));
      uVar17 = 0;
      uVar12 = (uint)DAT_0000ad1a;
      pbVar9 = DAT_0000bc94;
      uVar20 = (uint)uVar18;
      pbVar25 = pbVar24;
      do {
        while( true ) {
          uVar19 = uVar20 & 0xffff;
          if ((char)(uVar20 >> 8) == '\0') break;
          if (-1 < (short)uVar20) goto LAB_0000b5f1;
          uVar20 = uVar19 + 1;
          bVar26 = CARRY4(uVar17,_DAT_0000ad06);
          uVar17 = uVar17 + _DAT_0000ad06;
          pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
          if (pbVar24 + uVar12 <= pbVar25) goto LAB_0000b5f1;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar19);
        if (DAT_0000ad1d <= (byte)uVar19) {
          bVar6 = *pbVar25;
          uVar15 = (undefined2)((uint)pbVar9 >> 0x10);
          pbVar9 = (byte *)CONCAT22(uVar15,CONCAT11(bVar6,(char)pbVar9));
          if (bVar6 != 0) {
            uVar14 = CONCAT21(uVar15,bVar6 + 0x20);
            uVar5 = *(undefined1 *)CONCAT31(uVar14,*(undefined1 *)(iVar21 + iVar3));
            pbVar9 = (byte *)CONCAT31(uVar14,uVar5);
            *(undefined1 *)(iVar21 + iVar3) = uVar5;
          }
        }
        uVar20 = uVar19 + 1;
        bVar26 = CARRY4(uVar17,_DAT_0000ad06);
        uVar17 = uVar17 + _DAT_0000ad06;
        pbVar25 = pbVar25 + bVar26 + _DAT_0000ad0a;
      } while (pbVar25 < pbVar24 + uVar12);
LAB_0000b5f1:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if (pbVar24 < DAT_0000acfa) {
        return pbVar9;
      }
      if (DAT_0000acfe <= pbVar24) {
        return pbVar9;
      }
    } while( true );
  case (byte *)0x8:
    uVar5 = (undefined1)*(undefined2 *)(DAT_00000c93 + 0x476);
    if ((char)((ushort)*(undefined2 *)(DAT_00000c93 + 0x476) >> 8) == '\0' &&
        (char)((ushort)*(undefined2 *)(DAT_00000c93 + 0x478) >> 8) == '\0') {
      bVar6 = (byte)*(undefined2 *)(DAT_00000c93 + 0x478);
      uVar14 = (undefined3)((uint)DAT_0000ad02 >> 8);
      pbVar9 = (byte *)CONCAT31(uVar14,uVar5);
      if (*pbVar9 <= bVar6) {
        cVar4 = DAT_0000ad1c;
        if (DAT_0000ad1c == '\0') {
          cVar4 = DAT_0000aead;
        }
        pbVar9 = (byte *)CONCAT31(uVar14,cVar4);
        *(char *)((uint)CONCAT11(uVar5,bVar6) + DAT_00003918) = cVar4;
      }
    }
    return pbVar9;
  default:
    pbVar9 = (byte *)FUN_00000f64(FUN_0000ae84);
    return pbVar9;
  }
  if (DAT_0000395d != '\0') {
    pbVar9 = param_3;
    if (((byte)param_4 & 1) != 0) {
      pbVar9 = DAT_0000acfe + -DAT_0000ace6;
      _DAT_0000ad0e = -_DAT_0000ad0e;
      _DAT_0000ad12 = -_DAT_0000ad12;
      DAT_0000ace6 = -DAT_0000ace6;
    }
    bVar6 = *(byte *)(DAT_00000c93 + 0x484);
    pbVar24 = (byte *)(uint)bVar6;
    if (bVar6 == 0) {
      uVar23 = 0;
      pbVar24 = (byte *)0x0;
      do {
        while ((char)(uVar16 >> 8) != '\0') {
          if (-1 < (short)uVar16) {
            return pbVar24;
          }
          uVar16 = uVar16 + 1;
          bVar26 = CARRY4(uVar23,_DAT_0000ad16);
          uVar23 = uVar23 + _DAT_0000ad16;
          if (bVar26) {
            pbVar9 = pbVar9 + DAT_0000ace6;
          }
          pbVar9 = pbVar9 + _DAT_0000ad0e;
          if ((pbVar9 < DAT_0000acfa) || (DAT_0000acfe <= pbVar9)) {
            return pbVar24;
          }
        }
        iVar21 = (uVar16 & 0xff) << 8;
        DAT_0000acea = pbVar9 + DAT_0000ad1a;
        uVar14 = (undefined3)((uint)DAT_0000ad02 >> 8);
        DAT_0000ad1d = *(byte *)CONCAT31(uVar14,(char)uVar16);
        pbVar24 = (byte *)CONCAT31(uVar14,DAT_0000ad1d);
        pbVar25 = pbVar9;
        if (CARRY4(uVar23,_DAT_0000ad06)) {
          pbVar25 = pbVar9 + DAT_0000ace6;
        }
        pbVar25 = pbVar25 + _DAT_0000ad12;
        if (DAT_0000acfe <= pbVar25) {
          return pbVar24;
        }
        if (pbVar25 < DAT_0000acfa) {
          return pbVar24;
        }
        uVar20 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
        _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
        uVar12 = 0;
        pbVar24 = DAT_0000acee;
        pbVar22 = pbVar9;
        do {
          while( true ) {
            uVar17 = uVar20 & 0xffff;
            if ((char)(uVar20 >> 8) == '\0') break;
            if (-1 < (short)uVar20) goto LAB_0000b2ea;
            uVar20 = uVar17 + 1;
            bVar26 = CARRY4(uVar12,_DAT_0000ad06);
            uVar12 = uVar12 + _DAT_0000ad06;
            pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
            if (DAT_0000acea <= pbVar22) goto LAB_0000b2ea;
          }
          iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar17);
          if (DAT_0000ad1d <= (byte)uVar17) {
            if (*pbVar22 == 0) {
              bVar6 = *(byte *)(iVar21 + iVar3);
            }
            else {
              bVar6 = ((byte)(*pbVar22 + 0xb0) >> 2) + 0x50;
            }
            if (pbVar22[(int)pbVar25 - (int)pbVar9] == 0) {
              bVar13 = *(byte *)(iVar21 + iVar3);
            }
            else {
              bVar13 = ((byte)(pbVar22[(int)pbVar25 - (int)pbVar9] + 0xb0) >> 2) + 0x50;
            }
            bVar7 = (byte)(bVar6 + bVar13) >> 1 | CARRY1(bVar6,bVar13) << 7;
            pbVar24 = (byte *)CONCAT31((int3)(CONCAT22((short)((uint)pbVar24 >> 0x10),
                                                       CONCAT11(bVar13,bVar6)) >> 8),bVar7);
            *(byte *)(iVar21 + iVar3) = bVar7;
          }
          uVar20 = uVar17 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
        } while (pbVar22 < DAT_0000acea);
LAB_0000b2ea:
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar9 = pbVar9 + DAT_0000ace6;
        }
        pbVar9 = pbVar9 + _DAT_0000ad0e;
        if (pbVar9 < DAT_0000acfa) {
          return pbVar24;
        }
        if (DAT_0000acfe <= pbVar9) {
          return pbVar24;
        }
      } while( true );
    }
    uVar23 = 0;
    bRam0000b171 = bVar6;
    bRam0000b184 = bVar6;
    do {
      while ((char)(uVar16 >> 8) != '\0') {
        if (-1 < (short)uVar16) {
          return pbVar24;
        }
        uVar16 = uVar16 + 1;
        bVar26 = CARRY4(uVar23,_DAT_0000ad16);
        uVar23 = uVar23 + _DAT_0000ad16;
        if (bVar26) {
          pbVar9 = pbVar9 + DAT_0000ace6;
        }
        pbVar9 = pbVar9 + _DAT_0000ad0e;
        if ((pbVar9 < DAT_0000acfa) || (DAT_0000acfe <= pbVar9)) {
          return pbVar24;
        }
      }
      iVar21 = (uVar16 & 0xff) << 8;
      DAT_0000acea = pbVar9 + DAT_0000ad1a;
      uVar14 = (undefined3)((uint)DAT_0000ad02 >> 8);
      DAT_0000ad1d = *(byte *)CONCAT31(uVar14,(char)uVar16);
      pbVar24 = (byte *)CONCAT31(uVar14,DAT_0000ad1d);
      pbVar25 = pbVar9;
      if (CARRY4(uVar23,_DAT_0000ad06)) {
        pbVar25 = pbVar9 + DAT_0000ace6;
      }
      pbVar25 = pbVar25 + _DAT_0000ad12;
      if (DAT_0000acfe <= pbVar25) {
        return pbVar24;
      }
      if (pbVar25 < DAT_0000acfa) {
        return pbVar24;
      }
      uVar20 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
      uVar12 = 0;
      pbVar24 = DAT_0000acee;
      pbVar22 = pbVar9;
      do {
        while( true ) {
          uVar17 = uVar20 & 0xffff;
          if ((char)(uVar20 >> 8) == '\0') break;
          if (-1 < (short)uVar20) goto LAB_0000b1a5;
          uVar20 = uVar17 + 1;
          bVar26 = CARRY4(uVar12,_DAT_0000ad06);
          uVar12 = uVar12 + _DAT_0000ad06;
          pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
          if (DAT_0000acea <= pbVar22) goto LAB_0000b1a5;
        }
        iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar17);
        if (DAT_0000ad1d <= (byte)uVar17) {
          bVar6 = *pbVar22;
          uVar14 = (undefined3)((uint)pbVar24 >> 8);
          if (bVar6 == 0) {
            uVar11 = CONCAT31(uVar14,*(undefined1 *)(iVar21 + iVar3));
          }
          else {
            uVar11 = CONCAT31(uVar14,bVar6 + 0x38);
            if (199 < bVar6) {
              uVar11 = CONCAT31(uVar14,0xff);
            }
          }
          bVar6 = pbVar22[(int)pbVar25 - (int)pbVar9];
          uVar15 = (undefined2)((uint)uVar11 >> 0x10);
          uVar5 = (undefined1)uVar11;
          if (bVar6 == 0) {
            uVar11 = CONCAT22(uVar15,CONCAT11(*(undefined1 *)(iVar21 + iVar3),uVar5));
          }
          else {
            uVar11 = CONCAT22(uVar15,CONCAT11(bVar6 + 0x38,uVar5));
            if (199 < bVar6) {
              uVar11 = CONCAT22(uVar15,CONCAT11(0xff,uVar5));
            }
          }
          bVar6 = (byte)((uint)uVar11 >> 8);
          bVar6 = (byte)((byte)uVar11 + bVar6) >> 1 | CARRY1((byte)uVar11,bVar6) << 7;
          pbVar24 = (byte *)CONCAT31((int3)((uint)uVar11 >> 8),bVar6);
          *(byte *)(iVar21 + iVar3) = bVar6;
        }
        uVar20 = uVar17 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
      } while (pbVar22 < DAT_0000acea);
LAB_0000b1a5:
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar9 = pbVar9 + DAT_0000ace6;
      }
      pbVar9 = pbVar9 + _DAT_0000ad0e;
      if (pbVar9 < DAT_0000acfa) {
        return pbVar24;
      }
      if (DAT_0000acfe <= pbVar9) {
        return pbVar24;
      }
    } while( true );
  }
  pbVar24 = param_3;
  if (((byte)param_4 & 1) != 0) {
    pbVar24 = DAT_0000acfe + -DAT_0000ace6;
    _DAT_0000ad0e = -_DAT_0000ad0e;
    _DAT_0000ad12 = -_DAT_0000ad12;
    DAT_0000ace6 = -DAT_0000ace6;
  }
  uVar23 = 0;
  do {
    while ((char)(uVar16 >> 8) != '\0') {
      if (-1 < (short)uVar16) {
        return pbVar9;
      }
      uVar16 = uVar16 + 1;
      bVar26 = CARRY4(uVar23,_DAT_0000ad16);
      uVar23 = uVar23 + _DAT_0000ad16;
      if (bVar26) {
        pbVar24 = pbVar24 + DAT_0000ace6;
      }
      pbVar24 = pbVar24 + _DAT_0000ad0e;
      if ((pbVar24 < DAT_0000acfa) || (DAT_0000acfe <= pbVar24)) {
        return pbVar9;
      }
    }
    iVar21 = (uVar16 & 0xff) << 8;
    DAT_0000acea = pbVar24 + DAT_0000ad1a;
    uVar14 = (undefined3)((uint)DAT_0000ad02 >> 8);
    DAT_0000ad1d = *(byte *)CONCAT31(uVar14,(char)uVar16);
    pbVar9 = (byte *)CONCAT31(uVar14,DAT_0000ad1d);
    pbVar25 = pbVar24;
    if (CARRY4(uVar23,_DAT_0000ad06)) {
      pbVar25 = pbVar24 + DAT_0000ace6;
    }
    pbVar25 = pbVar25 + _DAT_0000ad12;
    if (DAT_0000acfe <= pbVar25) {
      return pbVar9;
    }
    if (pbVar25 < DAT_0000acfa) {
      return pbVar9;
    }
    uVar20 = (uint)(ushort)(uVar18 + _DAT_0000acf4);
    _DAT_0000acf2 = _DAT_0000acf2 + (int)DAT_0000acee;
    uVar12 = 0;
    pbVar9 = DAT_0000bc90;
    pbVar22 = pbVar24;
    do {
      while( true ) {
        uVar17 = uVar20 & 0xffff;
        if ((char)(uVar20 >> 8) == '\0') break;
        if (-1 < (short)uVar20) goto LAB_0000b01e;
        uVar20 = uVar17 + 1;
        bVar26 = CARRY4(uVar12,_DAT_0000ad06);
        uVar12 = uVar12 + _DAT_0000ad06;
        pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
        if (DAT_0000acea <= pbVar22) goto LAB_0000b01e;
      }
      iVar21 = CONCAT31((int3)((uint)iVar21 >> 8),(byte)uVar17);
      if (DAT_0000ad1d <= (byte)uVar17) {
        bVar6 = *pbVar22;
        if (bVar6 == 0) {
          bVar6 = *(byte *)(iVar21 + iVar3);
        }
        bVar13 = pbVar22[(int)pbVar25 - (int)pbVar24];
        if (bVar13 == 0) {
          bVar13 = *(byte *)(iVar21 + iVar3);
        }
        puVar10 = (undefined1 *)CONCAT22((short)((uint)pbVar9 >> 0x10),CONCAT11(bVar13,bVar6));
        uVar5 = *puVar10;
        pbVar9 = (byte *)CONCAT31((int3)((uint)puVar10 >> 8),uVar5);
        *(undefined1 *)(iVar21 + iVar3) = uVar5;
      }
      uVar20 = uVar17 + 1;
      bVar26 = CARRY4(uVar12,_DAT_0000ad06);
      uVar12 = uVar12 + _DAT_0000ad06;
      pbVar22 = pbVar22 + bVar26 + _DAT_0000ad0a;
    } while (pbVar22 < DAT_0000acea);
LAB_0000b01e:
    uVar16 = uVar16 + 1;
    bVar26 = CARRY4(uVar23,_DAT_0000ad16);
    uVar23 = uVar23 + _DAT_0000ad16;
    if (bVar26) {
      pbVar24 = pbVar24 + DAT_0000ace6;
    }
    pbVar24 = pbVar24 + _DAT_0000ad0e;
    if (pbVar24 < DAT_0000acfa) {
      return pbVar9;
    }
    if (DAT_0000acfe <= pbVar24) {
      return pbVar9;
    }
  } while( true );
}


/* ===== FUN_0000ae84 @ 0000ae84 ===== */

/* WARNING: Instruction at (ram,0x0000af00) overlaps instruction at (ram,0x0000aeff)
    */
/* WARNING: Unable to track spacebase fully for stack */
/* WARNING: Removing unreachable block (ram,0x0000af00) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs FUN_0000ae84(undefined4 param_1,undefined2 param_2,undefined1 *param_3)

{
  byte *pbVar1;
  undefined1 uVar2;
  char cVar3;
  undefined4 *puVar4;
  char cVar7;
  undefined4 uVar8;
  undefined3 uVar9;
  uint uVar10;
  char *pcVar11;
  int iVar12;
  undefined4 uVar13;
  uint uVar14;
  int iVar15;
  short sVar16;
  BADSPACEBASE *in_ESP;
  uint uVar18;
  char *pcVar19;
  int iVar20;
  undefined1 *puVar21;
  bool bVar22;
  undefined1 *local_9;
  undefined1 uStack_5;
  undefined4 uStack_4;
  int *piVar5;
  uint *puVar6;
  int iVar17;
  
  local_9 = &uStack_5;
  uVar2 = in(param_2);
  param_3[-1] = uVar2;
  uVar2 = in(param_2);
  *param_3 = uVar2;
  sVar16 = (short)&local_9;
  puVar4 = (undefined4 *)segment(0x2ba9,sVar16);
  puVar21 = (undefined1 *)*puVar4;
  segment(0x2ba9,sVar16 + 4);
  segment(0x2ba9,sVar16 + 8);
  segment(0x2ba9,sVar16 + 0xc);
  piVar5 = (int *)segment(0x2ba9,sVar16 + 0x10);
  iVar15 = *piVar5;
  puVar4 = (undefined4 *)segment(0x2ba9,sVar16 + 0x14);
  uVar13 = *puVar4;
  puVar6 = (uint *)segment(0x2ba9,sVar16 + 0x18);
  uVar10 = *puVar6;
  puVar4 = (undefined4 *)segment(0x2ba9,sVar16 + 0x1c);
  uVar8 = *puVar4;
  iVar17 = CONCAT22((short)((uint)&local_9 >> 0x10),sVar16 + 0x20);
  uVar2 = in((short)uVar13);
  *puVar21 = uVar2;
  pbVar1 = (byte *)(iVar15 + 0x70);
  *pbVar1 = *pbVar1 & (byte)uVar13;
  if (*pbVar1 != 0) {
    pbVar1 = (byte *)(uVar10 + 0x70 + (int)(puVar21 + 1) * 2);
    *pbVar1 = *pbVar1 & (byte)uVar13;
    uStack_4 = param_1;
    uVar13 = FUN_00000f64(FUN_0000ae84);
    return uVar13;
  }
  *(undefined2 *)(iVar17 + -4) = 0x1c00;
  iVar15 = DAT_00003918;
  pcVar19 = DAT_0000acfe + -DAT_0000ace6;
  _DAT_0000ad0e = -_DAT_0000ad0e;
  _DAT_0000ad12 = -_DAT_0000ad12;
  DAT_0000ace6 = -DAT_0000ace6;
  uVar18 = 0;
  do {
    while ((char)(uVar10 >> 8) != '\0') {
      if (-1 < (short)uVar10) {
        return uVar8;
      }
      uVar10 = uVar10 + 1;
      bVar22 = CARRY4(uVar18,_DAT_0000ad16);
      uVar18 = uVar18 + _DAT_0000ad16;
      if (bVar22) {
        pcVar19 = pcVar19 + DAT_0000ace6;
      }
      pcVar19 = pcVar19 + _DAT_0000ad0e;
      if ((pcVar19 < DAT_0000acfa) || (DAT_0000acfe <= pcVar19)) {
        return uVar8;
      }
    }
    iVar20 = (uVar10 & 0xff) << 8;
    DAT_0000acea = pcVar19 + DAT_0000ad1a;
    *(undefined4 *)(iVar17 + -8) = uVar13;
    *(char **)(iVar17 + -0xc) = pcVar19;
    *(uint *)(iVar17 + -0x10) = uVar10;
    *(uint *)(iVar17 + -0x14) = uVar18;
    uVar9 = (undefined3)((uint)DAT_0000ad02 >> 8);
    DAT_0000ad1d = *(byte *)CONCAT31(uVar9,(char)uVar10);
    uVar8 = CONCAT31(uVar9,DAT_0000ad1d);
    pcVar11 = pcVar19;
    if (CARRY4(uVar18,_DAT_0000ad06)) {
      pcVar11 = pcVar19 + DAT_0000ace6;
    }
    pcVar11 = pcVar11 + _DAT_0000ad12;
    if (DAT_0000acfe <= pcVar11) {
      return uVar8;
    }
    if (pcVar11 < DAT_0000acfa) {
      return uVar8;
    }
    iVar12 = (int)pcVar11 - (int)pcVar19;
    uVar10 = (uint)(ushort)((short)uVar13 + _DAT_0000acf4);
    _DAT_0000acf2 = _DAT_0000acf2 + DAT_0000acee;
    uVar18 = 0;
    uVar8 = DAT_0000bc90;
    do {
      while( true ) {
        uVar14 = uVar10 & 0xffff;
        if ((char)(uVar10 >> 8) == '\0') break;
        if (-1 < (short)uVar10) goto LAB_0000b01e;
        uVar10 = uVar14 + 1;
        bVar22 = CARRY4(uVar18,_DAT_0000ad06);
        uVar18 = uVar18 + _DAT_0000ad06;
        pcVar19 = pcVar19 + (uint)bVar22 + _DAT_0000ad0a;
        if (DAT_0000acea <= pcVar19) goto LAB_0000b01e;
      }
      iVar20 = CONCAT31((int3)((uint)iVar20 >> 8),(byte)uVar14);
      if (DAT_0000ad1d <= (byte)uVar14) {
        cVar7 = *pcVar19;
        if (cVar7 == '\0') {
          cVar7 = *(char *)(iVar20 + iVar15);
        }
        cVar3 = pcVar19[iVar12];
        if (cVar3 == '\0') {
          cVar3 = *(char *)(iVar20 + iVar15);
        }
        puVar21 = (undefined1 *)CONCAT22((short)((uint)uVar8 >> 0x10),CONCAT11(cVar3,cVar7));
        uVar2 = *puVar21;
        uVar8 = CONCAT31((int3)((uint)puVar21 >> 8),uVar2);
        *(undefined1 *)(iVar20 + iVar15) = uVar2;
      }
      uVar10 = uVar14 + 1;
      bVar22 = CARRY4(uVar18,_DAT_0000ad06);
      uVar18 = uVar18 + _DAT_0000ad06;
      pcVar19 = pcVar19 + (uint)bVar22 + _DAT_0000ad0a;
    } while (pcVar19 < DAT_0000acea);
LAB_0000b01e:
    iVar20 = *(int *)(iVar17 + -0xc);
    uVar13 = *(undefined4 *)(iVar17 + -8);
    uVar10 = *(int *)(iVar17 + -0x10) + 1;
    uVar18 = *(uint *)(iVar17 + -0x14) + _DAT_0000ad16;
    if (CARRY4(*(uint *)(iVar17 + -0x14),_DAT_0000ad16)) {
      iVar20 = iVar20 + DAT_0000ace6;
    }
    pcVar19 = (char *)(iVar20 + _DAT_0000ad0e);
    if (pcVar19 < DAT_0000acfa) {
      return uVar8;
    }
    if (DAT_0000acfe <= pcVar19) {
      return uVar8;
    }
  } while( true );
}


/* ===== caseD_0 @ 0000aee9 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __allregs switchD_0000ae56::caseD_0(uint param_1,uint param_2,short param_3,byte *param_4)

{
  char cVar1;
  int iVar2;
  undefined1 uVar3;
  byte bVar4;
  byte bVar5;
  byte bVar8;
  undefined3 uVar9;
  undefined1 *puVar6;
  undefined4 uVar7;
  undefined2 uVar10;
  byte *pbVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  uint uVar16;
  uint uVar17;
  byte *pbVar18;
  bool bVar19;
  
  iVar2 = DAT_00003918;
  if (DAT_0000395d != '\0') {
    if ((DAT_0000ad1b & 1) != 0) {
      param_4 = DAT_0000acfe + -DAT_0000ace6;
      _DAT_0000ad0e = -_DAT_0000ad0e;
      _DAT_0000ad12 = -_DAT_0000ad12;
      DAT_0000ace6 = -DAT_0000ace6;
    }
    cVar1 = *(char *)(DAT_00000c93 + 0x484);
    uVar15 = CONCAT31((int3)(param_1 >> 8),cVar1);
    if (cVar1 == '\0') {
      uVar12 = 0;
      do {
        while ((char)(param_2 >> 8) != '\0') {
          if (-1 < (short)param_2) {
            return uVar15;
          }
          param_2 = param_2 + 1;
          bVar19 = CARRY4(uVar12,_DAT_0000ad16);
          uVar12 = uVar12 + _DAT_0000ad16;
          if (bVar19) {
            param_4 = param_4 + DAT_0000ace6;
          }
          param_4 = param_4 + _DAT_0000ad0e;
          if ((param_4 < DAT_0000acfa) || (DAT_0000acfe <= param_4)) {
            return uVar15;
          }
        }
        iVar14 = (param_2 & 0xff) << 8;
        DAT_0000acea = param_4 + DAT_0000ad1a;
        uVar9 = (undefined3)((uint)DAT_0000ad02 >> 8);
        DAT_0000ad1d = *(byte *)CONCAT31(uVar9,(char)param_2);
        uVar15 = CONCAT31(uVar9,DAT_0000ad1d);
        pbVar11 = param_4;
        if (CARRY4(uVar12,_DAT_0000ad06)) {
          pbVar11 = param_4 + DAT_0000ace6;
        }
        pbVar11 = pbVar11 + _DAT_0000ad12;
        if (DAT_0000acfe <= pbVar11) {
          return uVar15;
        }
        if (pbVar11 < DAT_0000acfa) {
          return uVar15;
        }
        uVar16 = (uint)(ushort)(param_3 + _DAT_0000acf4);
        _DAT_0000acf2 = _DAT_0000acf2 + DAT_0000acee;
        uVar17 = 0;
        uVar15 = DAT_0000acee;
        pbVar18 = param_4;
        do {
          while( true ) {
            uVar13 = uVar16 & 0xffff;
            if ((char)(uVar16 >> 8) == '\0') break;
            if (-1 < (short)uVar16) goto LAB_0000b2ea;
            uVar16 = uVar13 + 1;
            bVar19 = CARRY4(uVar17,_DAT_0000ad06);
            uVar17 = uVar17 + _DAT_0000ad06;
            pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
            if (DAT_0000acea <= pbVar18) goto LAB_0000b2ea;
          }
          iVar14 = CONCAT31((int3)((uint)iVar14 >> 8),(byte)uVar13);
          if (DAT_0000ad1d <= (byte)uVar13) {
            if (*pbVar18 == 0) {
              bVar4 = *(byte *)(iVar14 + iVar2);
            }
            else {
              bVar4 = ((byte)(*pbVar18 + 0xb0) >> 2) + 0x50;
            }
            if (pbVar18[(int)pbVar11 - (int)param_4] == 0) {
              bVar8 = *(byte *)(iVar14 + iVar2);
            }
            else {
              bVar8 = ((byte)(pbVar18[(int)pbVar11 - (int)param_4] + 0xb0) >> 2) + 0x50;
            }
            bVar5 = (byte)(bVar4 + bVar8) >> 1 | CARRY1(bVar4,bVar8) << 7;
            uVar15 = CONCAT31((int3)(CONCAT22((short)(uVar15 >> 0x10),CONCAT11(bVar8,bVar4)) >> 8),
                              bVar5);
            *(byte *)(iVar14 + iVar2) = bVar5;
          }
          uVar16 = uVar13 + 1;
          bVar19 = CARRY4(uVar17,_DAT_0000ad06);
          uVar17 = uVar17 + _DAT_0000ad06;
          pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
        } while (pbVar18 < DAT_0000acea);
LAB_0000b2ea:
        param_2 = param_2 + 1;
        bVar19 = CARRY4(uVar12,_DAT_0000ad16);
        uVar12 = uVar12 + _DAT_0000ad16;
        if (bVar19) {
          param_4 = param_4 + DAT_0000ace6;
        }
        param_4 = param_4 + _DAT_0000ad0e;
        if (param_4 < DAT_0000acfa) {
          return uVar15;
        }
        if (DAT_0000acfe <= param_4) {
          return uVar15;
        }
      } while( true );
    }
    uVar12 = 0;
    cRam0000b171 = cVar1;
    cRam0000b184 = cVar1;
    do {
      while ((char)(param_2 >> 8) != '\0') {
        if (-1 < (short)param_2) {
          return uVar15;
        }
        param_2 = param_2 + 1;
        bVar19 = CARRY4(uVar12,_DAT_0000ad16);
        uVar12 = uVar12 + _DAT_0000ad16;
        if (bVar19) {
          param_4 = param_4 + DAT_0000ace6;
        }
        param_4 = param_4 + _DAT_0000ad0e;
        if ((param_4 < DAT_0000acfa) || (DAT_0000acfe <= param_4)) {
          return uVar15;
        }
      }
      iVar14 = (param_2 & 0xff) << 8;
      DAT_0000acea = param_4 + DAT_0000ad1a;
      uVar9 = (undefined3)((uint)DAT_0000ad02 >> 8);
      DAT_0000ad1d = *(byte *)CONCAT31(uVar9,(char)param_2);
      uVar15 = CONCAT31(uVar9,DAT_0000ad1d);
      pbVar11 = param_4;
      if (CARRY4(uVar12,_DAT_0000ad06)) {
        pbVar11 = param_4 + DAT_0000ace6;
      }
      pbVar11 = pbVar11 + _DAT_0000ad12;
      if (DAT_0000acfe <= pbVar11) {
        return uVar15;
      }
      if (pbVar11 < DAT_0000acfa) {
        return uVar15;
      }
      uVar16 = (uint)(ushort)(param_3 + _DAT_0000acf4);
      _DAT_0000acf2 = _DAT_0000acf2 + DAT_0000acee;
      uVar17 = 0;
      uVar15 = DAT_0000acee;
      pbVar18 = param_4;
      do {
        while( true ) {
          uVar13 = uVar16 & 0xffff;
          if ((char)(uVar16 >> 8) == '\0') break;
          if (-1 < (short)uVar16) goto LAB_0000b1a5;
          uVar16 = uVar13 + 1;
          bVar19 = CARRY4(uVar17,_DAT_0000ad06);
          uVar17 = uVar17 + _DAT_0000ad06;
          pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
          if (DAT_0000acea <= pbVar18) goto LAB_0000b1a5;
        }
        iVar14 = CONCAT31((int3)((uint)iVar14 >> 8),(byte)uVar13);
        if (DAT_0000ad1d <= (byte)uVar13) {
          bVar4 = *pbVar18;
          uVar9 = (undefined3)(uVar15 >> 8);
          if (bVar4 == 0) {
            uVar7 = CONCAT31(uVar9,*(undefined1 *)(iVar14 + iVar2));
          }
          else {
            uVar7 = CONCAT31(uVar9,bVar4 + 0x38);
            if (199 < bVar4) {
              uVar7 = CONCAT31(uVar9,0xff);
            }
          }
          bVar4 = pbVar18[(int)pbVar11 - (int)param_4];
          uVar10 = (undefined2)((uint)uVar7 >> 0x10);
          uVar3 = (undefined1)uVar7;
          if (bVar4 == 0) {
            uVar7 = CONCAT22(uVar10,CONCAT11(*(undefined1 *)(iVar14 + iVar2),uVar3));
          }
          else {
            uVar7 = CONCAT22(uVar10,CONCAT11(bVar4 + 0x38,uVar3));
            if (199 < bVar4) {
              uVar7 = CONCAT22(uVar10,CONCAT11(0xff,uVar3));
            }
          }
          bVar4 = (byte)((uint)uVar7 >> 8);
          bVar4 = (byte)((byte)uVar7 + bVar4) >> 1 | CARRY1((byte)uVar7,bVar4) << 7;
          uVar15 = CONCAT31((int3)((uint)uVar7 >> 8),bVar4);
          *(byte *)(iVar14 + iVar2) = bVar4;
        }
        uVar16 = uVar13 + 1;
        bVar19 = CARRY4(uVar17,_DAT_0000ad06);
        uVar17 = uVar17 + _DAT_0000ad06;
        pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
      } while (pbVar18 < DAT_0000acea);
LAB_0000b1a5:
      param_2 = param_2 + 1;
      bVar19 = CARRY4(uVar12,_DAT_0000ad16);
      uVar12 = uVar12 + _DAT_0000ad16;
      if (bVar19) {
        param_4 = param_4 + DAT_0000ace6;
      }
      param_4 = param_4 + _DAT_0000ad0e;
      if (param_4 < DAT_0000acfa) {
        return uVar15;
      }
      if (DAT_0000acfe <= param_4) {
        return uVar15;
      }
    } while( true );
  }
  if ((DAT_0000ad1b & 1) != 0) {
    param_4 = DAT_0000acfe + -DAT_0000ace6;
    _DAT_0000ad0e = -_DAT_0000ad0e;
    _DAT_0000ad12 = -_DAT_0000ad12;
    DAT_0000ace6 = -DAT_0000ace6;
  }
  uVar15 = 0;
  do {
    while ((char)(param_2 >> 8) != '\0') {
      if (-1 < (short)param_2) {
        return param_1;
      }
      param_2 = param_2 + 1;
      bVar19 = CARRY4(uVar15,_DAT_0000ad16);
      uVar15 = uVar15 + _DAT_0000ad16;
      if (bVar19) {
        param_4 = param_4 + DAT_0000ace6;
      }
      param_4 = param_4 + _DAT_0000ad0e;
      if ((param_4 < DAT_0000acfa) || (DAT_0000acfe <= param_4)) {
        return param_1;
      }
    }
    iVar14 = (param_2 & 0xff) << 8;
    DAT_0000acea = param_4 + DAT_0000ad1a;
    uVar9 = (undefined3)((uint)DAT_0000ad02 >> 8);
    DAT_0000ad1d = *(byte *)CONCAT31(uVar9,(char)param_2);
    uVar12 = CONCAT31(uVar9,DAT_0000ad1d);
    pbVar11 = param_4;
    if (CARRY4(uVar15,_DAT_0000ad06)) {
      pbVar11 = param_4 + DAT_0000ace6;
    }
    pbVar11 = pbVar11 + _DAT_0000ad12;
    if (DAT_0000acfe <= pbVar11) {
      return uVar12;
    }
    if (pbVar11 < DAT_0000acfa) {
      return uVar12;
    }
    uVar12 = (uint)(ushort)(param_3 + _DAT_0000acf4);
    _DAT_0000acf2 = _DAT_0000acf2 + DAT_0000acee;
    uVar16 = 0;
    param_1 = DAT_0000bc90;
    pbVar18 = param_4;
    do {
      while( true ) {
        uVar17 = uVar12 & 0xffff;
        if ((char)(uVar12 >> 8) == '\0') break;
        if (-1 < (short)uVar12) goto LAB_0000b01e;
        uVar12 = uVar17 + 1;
        bVar19 = CARRY4(uVar16,_DAT_0000ad06);
        uVar16 = uVar16 + _DAT_0000ad06;
        pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
        if (DAT_0000acea <= pbVar18) goto LAB_0000b01e;
      }
      iVar14 = CONCAT31((int3)((uint)iVar14 >> 8),(byte)uVar17);
      if (DAT_0000ad1d <= (byte)uVar17) {
        bVar4 = *pbVar18;
        if (bVar4 == 0) {
          bVar4 = *(byte *)(iVar14 + iVar2);
        }
        bVar8 = pbVar18[(int)pbVar11 - (int)param_4];
        if (bVar8 == 0) {
          bVar8 = *(byte *)(iVar14 + iVar2);
        }
        puVar6 = (undefined1 *)CONCAT22((short)(param_1 >> 0x10),CONCAT11(bVar8,bVar4));
        uVar3 = *puVar6;
        param_1 = CONCAT31((int3)((uint)puVar6 >> 8),uVar3);
        *(undefined1 *)(iVar14 + iVar2) = uVar3;
      }
      uVar12 = uVar17 + 1;
      bVar19 = CARRY4(uVar16,_DAT_0000ad06);
      uVar16 = uVar16 + _DAT_0000ad06;
      pbVar18 = pbVar18 + (uint)bVar19 + _DAT_0000ad0a;
    } while (pbVar18 < DAT_0000acea);
LAB_0000b01e:
    param_2 = param_2 + 1;
    bVar19 = CARRY4(uVar15,_DAT_0000ad16);
    uVar15 = uVar15 + _DAT_0000ad16;
    if (bVar19) {
      param_4 = param_4 + DAT_0000ace6;
    }
    param_4 = param_4 + _DAT_0000ad0e;
    if (param_4 < DAT_0000acfa) {
      return param_1;
    }
    if (DAT_0000acfe <= param_4) {
      return param_1;
    }
  } while( true );
}


/* ===== caseD_5 @ 0000b423 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __allregs switchD_0000ae56::caseD_5(uint param_1,uint param_2,char *param_3)

{
  undefined1 uVar1;
  uint3 uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  char *pcVar11;
  bool bVar12;
  
  iVar3 = DAT_00003918;
  if ((DAT_0000ad1b & 1) != 0) {
    _DAT_0000ad0e = -_DAT_0000ad0e;
    DAT_0000ace6 = -DAT_0000ace6;
    param_3 = DAT_0000acfe + DAT_0000ace6;
  }
  uVar10 = 0;
  do {
    while ((char)(param_1 >> 8) != '\0') {
      if (-1 < (short)param_1) {
        return;
      }
      param_1 = param_1 + 1;
      bVar12 = CARRY4(uVar10,_DAT_0000ad16);
      uVar10 = uVar10 + _DAT_0000ad16;
      if (bVar12) {
        param_3 = param_3 + DAT_0000ace6;
      }
      param_3 = param_3 + _DAT_0000ad0e;
      if ((param_3 < DAT_0000acfa) || (DAT_0000acfe <= param_3)) {
        return;
      }
    }
    iVar9 = (param_1 & 0xff) << 8;
    DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (param_1 & 0xff));
    uVar6 = 0;
    uVar4 = (uint)DAT_0000ad1a;
    iVar5 = DAT_0000bc94;
    uVar8 = param_2;
    pcVar11 = param_3;
    do {
      while( true ) {
        uVar7 = uVar8 & 0xffff;
        if ((char)(uVar8 >> 8) == '\0') break;
        if (-1 < (short)uVar8) goto LAB_0000b4f1;
        uVar8 = uVar7 + 1;
        bVar12 = CARRY4(uVar6,_DAT_0000ad06);
        uVar6 = uVar6 + _DAT_0000ad06;
        pcVar11 = pcVar11 + (uint)bVar12 + _DAT_0000ad0a;
        if (param_3 + uVar4 <= pcVar11) goto LAB_0000b4f1;
      }
      iVar9 = CONCAT31((int3)((uint)iVar9 >> 8),(byte)uVar7);
      if (DAT_0000ad1d <= (byte)uVar7) {
        uVar2 = CONCAT21((short)((uint)iVar5 >> 0x10),*pcVar11);
        iVar5 = (uint)uVar2 << 8;
        if (*pcVar11 != '\0') {
          uVar1 = *(undefined1 *)CONCAT31(uVar2,*(undefined1 *)(iVar9 + iVar3));
          iVar5 = CONCAT31(uVar2,uVar1);
          *(undefined1 *)(iVar9 + iVar3) = uVar1;
        }
      }
      uVar8 = uVar7 + 1;
      bVar12 = CARRY4(uVar6,_DAT_0000ad06);
      uVar6 = uVar6 + _DAT_0000ad06;
      pcVar11 = pcVar11 + (uint)bVar12 + _DAT_0000ad0a;
    } while (pcVar11 < param_3 + uVar4);
LAB_0000b4f1:
    param_1 = param_1 + 1;
    bVar12 = CARRY4(uVar10,_DAT_0000ad16);
    uVar10 = uVar10 + _DAT_0000ad16;
    if (bVar12) {
      param_3 = param_3 + DAT_0000ace6;
    }
    param_3 = param_3 + _DAT_0000ad0e;
    if (param_3 < DAT_0000acfa) {
      return;
    }
    if (DAT_0000acfe <= param_3) {
      return;
    }
  } while( true );
}


/* ===== caseD_2 @ 0000b732 ===== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __allregs
switchD_0000ae56::caseD_2(undefined4 param_1,uint param_2,short param_3,char *param_4)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  undefined1 *puVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  char *pcVar12;
  bool bVar13;
  
  iVar3 = DAT_00003918;
  if ((DAT_0000ad1b & 1) != 0) {
    _DAT_0000ad0e = -_DAT_0000ad0e;
    DAT_0000ace6 = -DAT_0000ace6;
    param_4 = DAT_0000acfe + DAT_0000ace6;
  }
  uVar11 = 0;
  do {
    while ((char)(param_2 >> 8) != '\0') {
      if (-1 < (short)param_2) {
        return param_1;
      }
      param_2 = param_2 + 1;
      bVar13 = CARRY4(uVar11,_DAT_0000ad16);
      uVar11 = uVar11 + _DAT_0000ad16;
      if (bVar13) {
        param_4 = param_4 + DAT_0000ace6;
      }
      param_4 = param_4 + _DAT_0000ad0e;
      if ((param_4 < DAT_0000acfa) || (DAT_0000acfe <= param_4)) {
        return param_1;
      }
    }
    iVar10 = (param_2 & 0xff) << 8;
    DAT_0000ad1d = *(byte *)(DAT_0000ad02 + (param_2 & 0xff));
    uVar9 = (uint)(ushort)(param_3 + _DAT_0000acf4);
    _DAT_0000acf2 = _DAT_0000acf2 + DAT_0000acee;
    uVar7 = 0;
    uVar4 = (uint)DAT_0000ad1a;
    param_1 = DAT_0000bc90;
    pcVar12 = param_4;
    do {
      while( true ) {
        uVar8 = uVar9 & 0xffff;
        if ((char)(uVar9 >> 8) == '\0') break;
        if (-1 < (short)uVar9) goto LAB_0000b813;
        uVar9 = uVar8 + 1;
        bVar13 = CARRY4(uVar7,_DAT_0000ad06);
        uVar7 = uVar7 + _DAT_0000ad06;
        pcVar12 = pcVar12 + (uint)bVar13 + _DAT_0000ad0a;
        if (param_4 + uVar4 <= pcVar12) goto LAB_0000b813;
      }
      iVar10 = CONCAT31((int3)((uint)iVar10 >> 8),(byte)uVar8);
      uVar5 = param_1;
      if (DAT_0000ad1d <= (byte)uVar8) {
        cVar1 = *pcVar12;
        uVar5 = CONCAT31((int3)((uint)param_1 >> 8),cVar1);
        if (cVar1 != '\0') {
          puVar6 = (undefined1 *)
                   CONCAT22((short)((uint)param_1 >> 0x10),
                            CONCAT11(*(undefined1 *)(iVar10 + iVar3),cVar1));
          uVar2 = *puVar6;
          uVar5 = CONCAT31((int3)((uint)puVar6 >> 8),uVar2);
          *(undefined1 *)(iVar10 + iVar3) = uVar2;
        }
      }
      uVar9 = uVar8 + 1;
      bVar13 = CARRY4(uVar7,_DAT_0000ad06);
      uVar7 = uVar7 + _DAT_0000ad06;
      pcVar12 = pcVar12 + (uint)bVar13 + _DAT_0000ad0a;
      param_1 = uVar5;
    } while (pcVar12 < param_4 + uVar4);
LAB_0000b813:
    param_2 = param_2 + 1;
    bVar13 = CARRY4(uVar11,_DAT_0000ad16);
    uVar11 = uVar11 + _DAT_0000ad16;
    if (bVar13) {
      param_4 = param_4 + DAT_0000ace6;
    }
    param_4 = param_4 + _DAT_0000ad0e;
    if (param_4 < DAT_0000acfa) {
      return param_1;
    }
    if (DAT_0000acfe <= param_4) {
      return param_1;
    }
  } while( true );
}


/* ===== FUN_0000bc06 @ 0000bc06 ===== */

void __allregs FUN_0000bc06(int param_1)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  byte *pbVar10;
  byte *pbVar11;
  
  iVar3 = DAT_00005578;
  pbVar8 = (byte *)(param_1 + DAT_00005578 * DAT_00005578);
  pbVar10 = (byte *)(param_1 + DAT_00005578 * DAT_00005578 * 2);
  iVar7 = DAT_00005578;
  do {
    pbVar9 = pbVar8 + -iVar3;
    pbVar8 = pbVar9 + iVar3;
    iVar5 = iVar3;
    pbVar11 = pbVar10;
    bVar1 = *pbVar9;
    do {
      pbVar8 = pbVar8 + -1;
      pbVar10 = pbVar11 + -2;
      bVar2 = *pbVar8;
      *pbVar10 = bVar2;
      pbVar11[-1] = (byte)(bVar1 + bVar2) >> 1 | CARRY1(bVar1,bVar2) << 7;
      iVar4 = DAT_00005578;
      iVar5 = iVar5 + -1;
      pbVar11 = pbVar10;
      bVar1 = bVar2;
    } while (iVar5 != 0);
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  DAT_00005578 = DAT_00005578 * 2;
  iVar3 = DAT_00005578;
  pbVar8 = (byte *)(param_1 + DAT_00005578);
  pbVar10 = (byte *)(param_1 + DAT_00005578);
  iVar7 = DAT_00005578;
  do {
    iVar5 = DAT_0000557c * iVar3;
    pbVar8 = pbVar8 + iVar5 + -1;
    pbVar10 = pbVar10 + iVar5 * 2 + -1;
    iVar6 = DAT_0000557c;
    bVar1 = pbVar8[-iVar5];
    do {
      pbVar8 = pbVar8 + iVar4 * -2;
      pbVar10 = pbVar10 + iVar4 * -4;
      bVar2 = *pbVar8;
      *pbVar10 = bVar2;
      pbVar10[iVar3] = (byte)(bVar1 + bVar2) >> 1 | CARRY1(bVar1,bVar2) << 7;
      iVar6 = iVar6 + -1;
      bVar1 = bVar2;
    } while (iVar6 != 0);
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  DAT_0000557c = DAT_0000557c << 1;
  return;
}


/* ===== FUN_0000bc5a @ 0000bc5a ===== */

/* WARNING: Instruction at (ram,0x0000bc5b) overlaps instruction at (ram,0x0000bc5a)
    */

void __allregs FUN_0000bc5a(int param_1,int param_2,undefined4 *param_3,byte *param_4,byte *param_5)

{
  byte bVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  BADSPACEBASE *in_ESP;
  byte *pbVar5;
  byte *pbVar6;
  undefined4 uStack_8;
  byte abStack_4 [4];
  
  pbVar5 = abStack_4;
  cVar3 = '\x02';
  do {
    param_3 = param_3 + -1;
    pbVar5 = (byte *)((int)pbVar5 + -4);
    *(undefined4 *)pbVar5 = *param_3;
    cVar3 = cVar3 + -1;
  } while ('\0' < cVar3);
  while( true ) {
    pbVar5 = param_4 + -1;
    pbVar6 = param_5 + -1;
    iVar4 = DAT_0000557c;
    bVar1 = pbVar5[-param_1];
    do {
      pbVar5 = pbVar5 + -(int)abStack_4;
      pbVar6 = pbVar6 + (int)abStack_4 * -2;
      bVar2 = *pbVar5;
      *pbVar6 = bVar2;
      abStack_4[(int)pbVar6] = (byte)(bVar1 + bVar2) >> 1 | CARRY1(bVar1,bVar2) << 7;
      iVar4 = iVar4 + -1;
      bVar1 = bVar2;
    } while (iVar4 != 0);
    param_2 = param_2 + -1;
    if (param_2 == 0) break;
    param_1 = DAT_0000557c * (int)abStack_4;
    param_4 = pbVar5 + param_1;
    param_5 = pbVar6 + param_1 * 2;
  }
  DAT_0000557c = DAT_0000557c << 1;
  return;
}


/* ===== FUN_0000bc9c @ 0000bc9c ===== */

void __allregs FUN_0000bc9c(void)

{
  byte bVar1;
  undefined2 uVar2;
  byte bVar3;
  byte bVar4;
  byte bVar5;
  uint uVar6;
  char cVar7;
  char cVar9;
  byte *pbVar8;
  undefined2 uVar10;
  byte bVar11;
  byte bVar12;
  
  uVar6 = DAT_0000bc90;
  do {
    uVar2 = *(undefined2 *)((int)&DAT_00005598 + (uVar6 & 0xff) * 3);
    bVar11 = (byte)uVar2;
    bVar1 = *(byte *)((int)&DAT_00005598 + (uVar6 & 0xff) * 3 + 2);
    pbVar8 = (byte *)CONCAT22((short)(uVar6 >> 0x10),CONCAT11((char)uVar6,(char)uVar6));
    do {
      uVar6 = (uint)pbVar8 >> 8 & 0xff;
      bVar12 = *(byte *)((int)&DAT_00005598 + uVar6 * 3 + 2);
      uVar10 = *(undefined2 *)((int)&DAT_00005598 + uVar6 * 3);
      bVar5 = (byte)uVar10;
      bVar3 = bVar5 + bVar11;
      bVar4 = (byte)((ushort)uVar10 >> 8);
      DAT_0000ac68 = (byte)(bVar3 + 1) >> 1 | (CARRY1(bVar5,bVar11) || 0xfe < bVar3) << 7;
      bVar5 = (byte)((ushort)uVar2 >> 8);
      bVar3 = bVar4 + bVar5;
      DAT_0000ac69 = (byte)(bVar3 + 1) >> 1 | (CARRY1(bVar4,bVar5) || 0xfe < bVar3) << 7;
      bVar3 = bVar12 + bVar1;
      DAT_0000ac6a = (byte)(bVar3 + 1) >> 1 | (CARRY1(bVar12,bVar1) || 0xfe < bVar3) << 7;
      bVar12 = bVar11;
      FUN_0000ac70();
      *pbVar8 = bVar12;
      cVar9 = (char)((uint)pbVar8 >> 8);
      cVar7 = (char)pbVar8;
      uVar10 = (undefined2)((uint)pbVar8 >> 0x10);
      *(byte *)CONCAT31(CONCAT21(uVar10,cVar7),cVar9) = bVar12;
      cVar9 = cVar9 + '\x01';
      pbVar8 = (byte *)CONCAT22(uVar10,CONCAT11(cVar9,cVar7));
    } while (cVar9 != '\0');
    uVar6 = CONCAT31((int3)((uint)pbVar8 >> 8),cVar7 + '\x01');
  } while ((char)(cVar7 + '\x01') != '\0');
  FUN_0000bd0e();
  FUN_0000bd62();
  return;
}


/* ===== FUN_0000bd0e @ 0000bd0e ===== */

void __allregs FUN_0000bd0e(void)

{
  int iVar1;
  undefined1 *puVar2;
  char cVar3;
  undefined1 *puVar4;
  byte bVar6;
  ushort uVar7;
  char cVar5;
  
  puVar2 = (undefined1 *)(DAT_0000bc94 & 0xffff0000);
  do {
    do {
      puVar4 = puVar2;
      cVar5 = (char)((uint)puVar4 >> 8);
      bVar6 = ~(cVar5 << 3);
      iVar1 = ((uint)puVar4 & 0xff) * 3;
      uVar7 = *(ushort *)((int)&DAT_00005598 + iVar1);
      DAT_0000ac68 = (undefined1)((ushort)((uVar7 & 0xff) * (ushort)bVar6) >> 8);
      DAT_0000ac69 = (undefined1)((ushort)((uVar7 >> 8) * (ushort)bVar6) >> 8);
      DAT_0000ac6a = (undefined1)
                     ((ushort)((ushort)*(byte *)((int)&DAT_00005598 + iVar1 + 2) * (ushort)bVar6) >>
                     8);
      FUN_0000ac70();
      *puVar4 = (char)uVar7;
      cVar3 = (char)puVar4 + '\x01';
      puVar2 = (undefined1 *)CONCAT31((int3)((uint)puVar4 >> 8),cVar3);
    } while (cVar3 != '\0');
    bVar6 = cVar5 + 1;
    puVar2 = (undefined1 *)((uint)CONCAT21((short)((uint)puVar4 >> 0x10),bVar6) << 8);
  } while (bVar6 < 0x20);
  return;
}


/* ===== FUN_0000bd62 @ 0000bd62 ===== */

void __allregs FUN_0000bd62(void)

{
  int iVar1;
  char cVar2;
  char cVar4;
  char cVar5;
  undefined2 uVar6;
  uint uVar3;
  byte bVar7;
  ushort uVar8;
  
  uVar3 = DAT_0000bc94 & 0xffff0000;
  do {
    do {
      cVar4 = (char)(uVar3 >> 8);
      bVar7 = ~(cVar4 << 3);
      cVar2 = (char)uVar3;
      cVar5 = cVar4 * '\x02';
      iVar1 = (CONCAT11(cVar5,cVar2) & 0xff) * 3;
      uVar8 = *(ushort *)((int)&DAT_00005598 + iVar1);
      DAT_0000ac68 = (char)((ushort)((uVar8 & 0xff) * (ushort)bVar7) >> 8) + cVar5;
      DAT_0000ac69 = (char)((ushort)((uVar8 >> 8) * (ushort)bVar7) >> 8) + cVar5;
      DAT_0000ac6a = (char)((ushort)((ushort)*(byte *)((int)&DAT_00005598 + iVar1 + 2) *
                                    (ushort)bVar7) >> 8) + cVar5;
      FUN_0000ac70();
      uVar6 = (undefined2)(uVar3 >> 0x10);
      *(char *)CONCAT22(uVar6,CONCAT11(cVar4 + ' ',cVar2)) = (char)uVar8;
      uVar3 = CONCAT31((int3)(uVar3 >> 8),cVar2 + '\x01');
    } while ((char)(cVar2 + '\x01') != '\0');
    uVar3 = (uint)CONCAT21(uVar6,cVar4 + 1U) << 8;
  } while ((byte)(cVar4 + 1U) < 0x20);
  return;
}


/* ===== FUN_0000bdc4 @ 0000bdc4 ===== */

void __allregs FUN_0000bdc4(int param_1)

{
  undefined1 uVar1;
  byte bVar2;
  undefined1 uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint uVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  undefined3 uVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  undefined2 uVar8;
  
  uVar12 = DAT_00005578;
  puVar14 = (undefined1 *)(param_1 + DAT_00005578 * DAT_0000557c);
  puVar15 = (undefined1 *)(param_1 + DAT_00005578 * DAT_0000557c * 2);
  iVar6 = DAT_0000bc90;
  iVar11 = DAT_0000557c;
  do {
    iVar6 = (uint)CONCAT21((short)((uint)iVar6 >> 0x10),puVar14[-uVar12]) << 8;
    puVar14 = puVar14 + -uVar12 + uVar12;
    uVar9 = uVar12;
    puVar16 = puVar15;
    do {
      puVar14 = puVar14 + -1;
      puVar15 = puVar16 + -2;
      uVar1 = *puVar14;
      *puVar15 = uVar1;
      puVar16[-1] = *(undefined1 *)CONCAT31((int3)((uint)iVar6 >> 8),uVar1);
      uVar5 = DAT_00005578;
      iVar6 = (uint)CONCAT21((short)((uint)iVar6 >> 0x10),uVar1) << 8;
      uVar9 = uVar9 - 1;
      puVar16 = puVar15;
    } while (uVar9 != 0);
    iVar11 = iVar11 + -1;
  } while (iVar11 != 0);
  DAT_00005578 = DAT_00005578 * 2;
  iVar6 = DAT_00005578;
  puVar14 = (undefined1 *)(param_1 + DAT_00005578);
  puVar15 = (undefined1 *)(param_1 + DAT_00005578);
  uVar12 = uVar5 & 0x7fffffff;
  iVar11 = -(DAT_00005578 + -1);
  do {
    iVar4 = DAT_0000557c * iVar6;
    puVar14 = puVar14 + iVar4 + -1;
    puVar15 = puVar15 + iVar4 * 2 + -1;
    uVar8 = (undefined2)((uint)DAT_0000bc90 >> 0x10);
    iVar7 = (uint)CONCAT21(uVar8,puVar14[-iVar4 + -1]) << 8;
    iVar10 = DAT_0000557c;
    if ((char)iVar11 == '\0') {
      uVar13 = CONCAT21(uVar8,puVar15[iVar4 * -2]);
    }
    else {
      uVar13 = CONCAT21(uVar8,puVar14[-iVar4 + iVar11]);
    }
    do {
      puVar14 = puVar14 + uVar5 * -2;
      puVar15 = puVar15 + uVar5 * -4;
      *puVar15 = *puVar14;
      if ((char)iVar11 == '\0') {
        bVar2 = puVar15[1];
      }
      else {
        bVar2 = puVar14[iVar11];
      }
      uVar3 = *(undefined1 *)CONCAT31((int3)((uint)iVar7 >> 8),bVar2);
      uVar1 = puVar14[-1];
      if ((bVar2 & 1) != 0) {
        uVar3 = *(undefined1 *)CONCAT31(uVar13,uVar1);
      }
      puVar15[iVar6] = uVar3;
      iVar7 = CONCAT22((short)((uint)iVar7 >> 0x10),CONCAT11(uVar1,bVar2));
      uVar13 = CONCAT21((short)((uint3)uVar13 >> 8),bVar2);
      iVar10 = iVar10 + -1;
    } while (iVar10 != 0);
    iVar11 = DAT_0000557c * iVar6;
    puVar14 = puVar14 + iVar11 + -1;
    puVar15 = puVar15 + iVar11 * 2 + -1;
    uVar13 = CONCAT21((short)((uint)DAT_0000bc90 >> 0x10),puVar14[-iVar11]);
    iVar11 = DAT_0000557c;
    do {
      puVar14 = puVar14 + uVar5 * -2;
      puVar15 = puVar15 + uVar5 * -4;
      uVar1 = *puVar14;
      *puVar15 = uVar1;
      puVar15[iVar6] = *(undefined1 *)CONCAT31(uVar13,uVar1);
      uVar13 = CONCAT21((short)((uint3)uVar13 >> 8),uVar1);
      iVar11 = iVar11 + -1;
    } while (iVar11 != 0);
    iVar11 = 0;
    uVar12 = uVar12 - 1;
  } while (uVar12 != 0);
  DAT_0000557c = DAT_0000557c << 1;
  return;
}


/* ===== FUN_0000bed2 @ 0000bed2 ===== */

void __allregs FUN_0000bed2(undefined1 *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  
  uVar1 = DAT_00005578;
  uVar2 = DAT_0000557c >> 1;
  puVar4 = param_1;
  do {
    uVar3 = uVar1 >> 1;
    puVar5 = puVar4;
    do {
      puVar4 = puVar5 + 1;
      *puVar5 = *param_1;
      param_1 = param_1 + 2;
      uVar3 = uVar3 - 1;
      puVar5 = puVar4;
    } while (uVar3 != 0);
    param_1 = param_1 + uVar1;
    uVar2 = uVar2 - 1;
  } while (uVar2 != 0);
  DAT_00005578 = DAT_00005578 >> 1;
  DAT_0000557c = DAT_0000557c >> 1;
  return;
}


/* ===== FUN_0000bf15 @ 0000bf15 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000bf15(undefined4 param_1,int param_2,int param_3,undefined4 param_4)

{
  byte bVar1;
  char cVar2;
  ushort uVar3;
  byte *pbVar4;
  char *pcVar6;
  byte bVar7;
  int *piVar5;
  
  pbVar4 = (byte *)FUN_0000bf84(param_1,param_4);
  bVar7 = *pbVar4;
  bVar1 = (byte)pbVar4;
  *pbVar4 = *pbVar4 + bVar1;
  cVar2 = (bVar1 + 0x41) - CARRY1(bVar7,bVar1);
  piVar5 = (int *)CONCAT31((int3)((uint)pbVar4 >> 8),cVar2);
  *(char *)piVar5 = (char)*piVar5 + cVar2;
  pcVar6 = (char *)((int)piVar5 - *piVar5);
  cVar2 = (char)pcVar6;
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
  bVar7 = (byte)((uint)(param_3 + -1) >> 8);
  DAT_00560000 = DAT_00560000 & bVar7;
  *pcVar6 = *pcVar6 + cVar2;
  bVar7 = bVar7 & DAT_00560000;
  *pcVar6 = *pcVar6 + cVar2;
  piVar5 = (int *)((uint)pcVar6 & 0xffffffbf);
  *(char *)piVar5 = (char)*piVar5 + (char)piVar5;
  pcVar6 = (char *)((int)piVar5 - *piVar5);
  cVar2 = (char)pcVar6;
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
  DAT_00560000 = DAT_00560000 - bVar7;
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
  piVar5 = (int *)CONCAT31((int3)((uint)pcVar6 >> 8),cVar2 + 'A');
  *(char *)piVar5 = (char)*piVar5 + cVar2 + 'A';
  pbVar4 = (byte *)((int)piVar5 - *piVar5);
  bVar7 = *pbVar4;
  bVar1 = (byte)pbVar4;
  *pbVar4 = *pbVar4 + bVar1;
  pbVar4 = (byte *)CONCAT31((int3)((uint)pbVar4 >> 8),bVar1 + *pbVar4 + CARRY1(bVar7,bVar1));
  bVar7 = *pbVar4;
  bVar1 = (byte)((uint)(param_2 + 1) >> 8);
  *pbVar4 = *pbVar4 + bVar1;
  uVar3 = (ushort)(pbVar4 + (-(uint)CARRY1(bVar7,bVar1) - *(int *)pbVar4));
  cVar2 = (char)(uVar3 / bRam002b0000);
  pcVar6 = (char *)CONCAT22((short)((uint)(pbVar4 + (-(uint)CARRY1(bVar7,bVar1) - *(int *)pbVar4))
                                   >> 0x10),CONCAT11((char)(uVar3 % (ushort)bRam002b0000),cVar2));
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
  *pcVar6 = *pcVar6 + cVar2;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000bf7c @ 0000bf7c ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000bf7c(char *param_1,int param_2,byte *param_3)

{
  byte bVar1;
  char cVar2;
  byte *pbVar4;
  byte bVar6;
  char in_CF;
  ushort uVar3;
  char *pcVar5;
  
  pbVar4 = (byte *)CONCAT31((int3)((uint)param_1 >> 8),(char)param_1 + *param_1 + in_CF);
  bVar1 = *pbVar4;
  bVar6 = (byte)((uint)(param_2 + 1) >> 8);
  *pbVar4 = *pbVar4 + bVar6;
  uVar3 = (ushort)(pbVar4 + (-(uint)CARRY1(bVar1,bVar6) - *(int *)pbVar4));
  cVar2 = (char)(uVar3 / *param_3);
  pcVar5 = (char *)CONCAT22((short)((uint)(pbVar4 + (-(uint)CARRY1(bVar1,bVar6) - *(int *)pbVar4))
                                   >> 0x10),CONCAT11((char)(uVar3 % (ushort)*param_3),cVar2));
  *pcVar5 = *pcVar5 + cVar2;
  *pcVar5 = *pcVar5 + cVar2;
  *pcVar5 = *pcVar5 + cVar2;
  *pcVar5 = *pcVar5 + cVar2;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


/* ===== FUN_0000bf84 @ 0000bf84 ===== */

/* WARNING: Control flow encountered bad instruction data */

void __allregs FUN_0000bf84(undefined4 param_1,byte *param_2)

{
  char cVar1;
  char *pcVar2;
  
  cVar1 = (char)((ushort)param_1 / (ushort)*param_2);
  pcVar2 = (char *)CONCAT22((short)((uint)param_1 >> 0x10),
                            CONCAT11((char)((ushort)param_1 % (ushort)*param_2),cVar1));
  *pcVar2 = *pcVar2 + cVar1;
  *pcVar2 = *pcVar2 + cVar1;
  *pcVar2 = *pcVar2 + cVar1;
  *pcVar2 = *pcVar2 + cVar1;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

