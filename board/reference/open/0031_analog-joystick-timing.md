Type: feature
Title: The joystick port reproduces original strobe and axis-decay timing
Parent: 0005

## Contract

Reproduce the original joystick axis/button protocol and calibration on both targets, using
deterministic device state at the port boundary.

## Evidence

The current port 0x201 read returns constant 0xf0. The original fb29 strobes the port and counts
until axis bits clear; constant idle cannot represent axis movement or calibration.

## Next

1. Capture original strobe/read timing for center, axis extremes and button combinations in each
   supported joystick mode. Establish the axis conversion and calibration from engine asm/traces.
2. Recommendation: latch each axis deadline at the 0x201 strobe in the shared PIT timeline. Reads
   derive axis/button bits from that state; never use host polling speed as the analog value.
3. Map host joystick samples at the input boundary, then replay the same samples headlessly.
   Cover absent device, calibration, held buttons and the configured flightstick variants.

## Accept

Identical port-read sequences and engine calibration/control responses for matching inputs on both
targets and the oracle. Keyboard/mouse operation with an idle or absent joystick remains correct.
