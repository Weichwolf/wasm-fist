/* xclick.c -- move the X pointer to (x,y) and synthesize a left click via XTest.
 * Used by the headless (xvfb) DOSBox oracle scripts to drive the game's menus.
 *   build: cc -O2 tools/oracle/xclick.c -o /tmp/xclick -lX11 -lXtst
 *   usage: /tmp/xclick <x> <y>
 */
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc < 3) return 2;
	int x = atoi(argv[1]), y = atoi(argv[2]);
	Display *d = XOpenDisplay(NULL);
	if (!d) return 1;
	XTestFakeMotionEvent(d, -1, x, y, CurrentTime);
	XFlush(d); usleep(120000);
	XTestFakeButtonEvent(d, 1, True, CurrentTime);
	XFlush(d); usleep(80000);
	XTestFakeButtonEvent(d, 1, False, CurrentTime);
	XFlush(d);
	XCloseDisplay(d);
	return 0;
}
