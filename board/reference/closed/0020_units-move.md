Type: bug
Title: Mobile units integrate their own positions
Parent: 0017

This engine can advance mobile-unit positions from their actual velocity fields.

## Evidence

The recorded AZER1 trace moved X/Y from 584582/1141637 at t=314 to 585000/1141125 at t=338 and
585090/1141020 at t=343. In the latter interval, five ticks × velocity (18,-21) = (90,-105),
matching the position delta. Live terrain following is recorded separately in board:0018.

## Preserve

`FIST_DUMP_REG` fires once at the first mission pump; varying only FIST_DUMPTICK compares the same
snapshot. Use tick-gated `FIST_SIMTRACE` or ordered step dumps for motion. This closure establishes
integration, not complete mission fidelity; board:0017 owns that requirement.
