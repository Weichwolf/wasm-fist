/* xclick.c -- move the X pointer to (x,y) and synthesize a left click via XTest.
 * Used by the headless (xvfb) DOSBox oracle scripts to drive the game's menus.
 *   build: cc -O2 tools/oracle/xclick.c -o /tmp/xclick -lX11 -lXtst
 *   usage: /tmp/xclick <x> <y>
 *
 * XTest prototypes are declared inline so this builds WITHOUT the libxtst-dev
 * header (X11/extensions/XTest.h) -- only libXtst.so (runtime) is needed, which
 * is present on hosts that lack the -dev package.  Link with -lXtst, or point at
 * the runtime soname directly (/usr/lib/.../libXtst.so.6) when the .so symlink
 * is absent.  (board:0002/0003 headless-oracle enablement.)
 */
#include <X11/Xlib.h>
#include <stdlib.h>
#include <unistd.h>

/* from libXtst -- declared here to avoid the -dev-only XTest.h dependency */
extern int XTestFakeMotionEvent(Display *, int screen, int x, int y, unsigned long delay);
extern int XTestFakeButtonEvent(Display *, unsigned int button, int is_press, unsigned long delay);

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
