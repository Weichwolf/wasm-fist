Type: discovery
Title: The original extender service table is located
Parent: 0009

This engine can use an identified original handler address for every extender service selector.

## Evidence

`re_out/fist_image.bin` offset 0xcb3 contains dword entries indexed by op/4 (32-bit code).
Independent anchors: 0x58→1103→8030 (LOS), 0x54→11a6 (height), 0x18→10ca (map load).

| Op | Handler | Op | Handler | Op | Handler | Op | Handler |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 00 | 0f77 | 04 | 10c9 | 08 | 10e0 | 0c | 10eb |
| 10 | 10f7 | 14 | 1089 | 18 | 10ca | 1c | 1109 |
| 20 | 0f8b | 24 | 82c0 | 28 | 8470 | 2c | 8460 |
| 30 | 11a5 | 34 | 11f2 | 38 | 11f3 | 3c | 11f4 |
| 40 | ad1e | 44 | 10da | 48 | 1265 | 4c | 123f |
| 50 | 1266 | 54 | 11a6 | 58 | 1103 | 5c | 10fd |
| 60 | 10f1 | 64 | 786a | 68 | 76fd | 6c | 77e2 |
| 70 | 11cb | 74 | 6f17 | 78 | 11dd | 7c | 77a4 |
| 80 | 7762 | 84 | 108b | | | | |

## Follow-up

Located does not mean implemented; board:0009 owns the current inventory. Ground-clamp ops 0x20
and 0x1c landed with the a19e caller fixes. Op 0x20 initializes object-list/DGROUP addresses during
mission setup and must not be gated on map load. Use live segment/page bases for oracle addresses.
