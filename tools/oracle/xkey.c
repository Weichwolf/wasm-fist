/* xkey.c -- send a key chord (optionally with Ctrl) via XTest to the headless DOSBox.
 *   build: cc -O2 tools/oracle/xkey.c -o /tmp/xkey -lX11 -lXtst
 *   usage: /tmp/xkey <keysym> [ctrl]     e.g. /tmp/xkey F6 ctrl  (toggle DOSBox wave record)
 */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/* from libXtst -- declared inline to avoid the -dev-only X11/extensions/XTest.h */
extern int XTestFakeKeyEvent(Display *, unsigned int keycode, int is_press, unsigned long delay);
int main(int argc,char**argv){
  if(argc<2) return 2;
  Display*d=XOpenDisplay(NULL); if(!d) return 1;
  KeySym ks=XStringToKeysym(argv[1]);
  KeyCode kc=XKeysymToKeycode(d,ks);
  int ctrl = (argc>2 && !strcmp(argv[2],"ctrl"));
  KeyCode cc=XKeysymToKeycode(d,XK_Control_L);
  if(ctrl){ XTestFakeKeyEvent(d,cc,True,CurrentTime); XFlush(d); usleep(30000);}
  XTestFakeKeyEvent(d,kc,True,CurrentTime); XFlush(d); usleep(60000);
  XTestFakeKeyEvent(d,kc,False,CurrentTime); XFlush(d); usleep(30000);
  if(ctrl){ XTestFakeKeyEvent(d,cc,False,CurrentTime); XFlush(d);}
  XCloseDisplay(d); return 0;
}
