Type: feature
Area: input

Every input mode is faithful: keyboard, mouse, joystick (port 0x201), and the
serial-link two-player path produce the same engine response as the original for
the same input trace, each covered by a tools/verify.sh flow.
