Type: bug
Title: Line-of-sight consumes the original unit-altitude fields
Parent: 0017

This engine can pass actual unit-altitude fields to LOS without the former default shim substitution.

## Evidence

The recovered ground-clamp service writes object+0x1d for vehicle types 0..3; their step methods
copy it to +0x0d. Recorded AZER1 samples had equal fields (type 00: 0x2c; type 01: 0x47).
The LOS TCB endpoints at +0xda/+0xe6 are passed through. The old stand-in remains an opt-in
A/B diagnostic (`FIST_LOS_STANDIN=1`), not the default behavior.

## Follow-up

The historical attrition match used captures at different write counts and was not tick-exact.
Board:0017 owns synchronized LOS/combat comparisons. Do not use the stand-in in acceptance runs.
