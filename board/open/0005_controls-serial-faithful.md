Type: feature
Title: All original input modes and the serial link are faithful

## Contract

The same input trace causes the same response through menus and gameplay in every original input
mode, including joystick variants and two-player serial play.

## Evidence

Menu mouse/keyboard paths are exercised. Gameplay control coverage is incomplete. Port 0x201 still
returns constant 0xf0; an idle controller is not analog-input emulation. The serial implementation
has not been located reliably; old 0x3f8 string-helper hits were data offsets, not UART evidence.

## Next

Use the bounded children: board:0030 (gameplay keyboard/mouse), board:0031 (analog joystick),
board:0032 (serial link). Start each from a recorded original trace and a reproducible port scenario.
Keep menu behavior covered while extending gameplay input.

## Accept

All children pass their oracle and cross-target checks, including calibration/settings variants.
A settings toggle or an idle device response alone does not verify the selected input mode.
