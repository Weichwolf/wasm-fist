#!/usr/bin/env python3
# XTEST mouse click via ctypes.  Usage: xclick.py X Y [X2 Y2 ...]  (longer hold + settle for DOSBox)
import ctypes, sys, time
x11 = ctypes.CDLL("libX11.so.6"); xtst = ctypes.CDLL("libXtst.so.6")
x11.XOpenDisplay.restype = ctypes.c_void_p
dpy = x11.XOpenDisplay(None)
if not dpy: sys.exit("no display")
D = ctypes.c_void_p(dpy)
def click(x, y):
    xtst.XTestFakeMotionEvent(D, -1, int(x), int(y), 0); x11.XFlush(D); time.sleep(0.4)
    xtst.XTestFakeButtonEvent(D, 1, 1, 0); x11.XFlush(D); time.sleep(0.12)
    xtst.XTestFakeButtonEvent(D, 1, 0, 0); x11.XFlush(D); time.sleep(0.3)
a = sys.argv[1:]
for i in range(0, len(a), 2): click(a[i], a[i+1])
