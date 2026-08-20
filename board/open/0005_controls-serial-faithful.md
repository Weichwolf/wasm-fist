Type: feature
Area: input

Every input mode is faithful: keyboard, mouse, joystick (port 0x201), and the
serial-link two-player path produce the same engine response as the original for
the same input trace, each covered by a tools/verify.sh flow.

## Comments

Survey (baseline established): the input infrastructure splits into four modes
with sharply different port readiness.
  - MOUSE (INT 33h): handler captured (native_main.c:687, fn 0x14 mask 0x1f);
    already menu-bit-verified as part of the front-end.
  - KEYBOARD (INT 16h BIOS, fist_dos.c:505; INT 9): the injection path that
    drives menu navigation in the matrix works; ports 0x60/0x64 return idle at
    the port layer (fist_vga.c:111-113) with real keys arriving via INT 16h.
    In-mission control mapping (drive/turret/weapons) is reachable via the same
    injection but not yet driven by a matrix flow.
  - JOYSTICK (port 0x201): the engine has the REAL analog-timing protocol
    (fist_decomp.c ~33800: FUN_0000_fb29 strobes 0x201, a loop counts iterations
    until the axis bit clears). The SHIM stubs port 0x201 to a constant 0xf0
    (fist_vga.c:114) = "buttons up, timers already low" -> the timing loop reads
    zero immediately -> joystick presents as centered/idle. FAITHFUL for
    keyboard/mouse traces (joystick idle), but a REAL GAP for any joystick-input
    trace: the shim must emulate the 0x201 strobe->decay timing to feed real axis
    values. This is the one concrete, bounded shim task in 0005.
  - SERIAL LINK (two-player): NOT yet located. Grep found no INT 14h / UART /
    0x2f8 path; the earlier 0x3f8 hit was a FALSE POSITIVE -- FUN_1000_3f5c takes
    (char*,char*) and 0x3f8/0x62f are DGROUP data offsets, not COM port I/O. The
    real link I/O (likely a NovaLogic custom COM driver in the overlay/extender,
    or INT 14h) still needs locating before it can be scoped. Untouched.
Scope for 0005: (a) emulate the 0x201 analog-joystick timing in the shim (bounded,
concrete); (b) add a matrix flow driving in-mission keyboard control; (c) locate
+ port the serial-link I/O (open-ended, deferred until the link path is found).
