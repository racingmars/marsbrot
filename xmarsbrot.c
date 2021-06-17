/*
    X Windows interface to marsbrot.
    
    marsbrot, a Mandelbrot Set image renderer.
    Copyright (C) 2021 Matthew R. Wilson <mwilson@mattwilson.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/* Request POSIX.1-2001-compliant interfaces. */
#define _POSIX_C_SOURCE 200112L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

struct dim {
    int x, y;
};

#define STR_BUF_LEN 128

struct dim handleRedraw(Display *dpy, Window win, int screen);
void handleClick(XButtonPressedEvent *bp);

int main(int argc, char **argv) {
    Display *dpy;
    Window win;
    int screen;
    XEvent e;
    bool quit = false;
    struct dim mandRegion;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(dpy);
    win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 10, 10, 660,
        200, 1, BlackPixel(dpy, screen), WhitePixel(dpy, screen));
    XSelectInput(dpy, win, ExposureMask | ButtonPressMask |ButtonReleaseMask);
    XMapWindow(dpy, win);

    XStoreName(dpy, win, "XMarsbrot");

    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);

    while (!quit)
    {
        XNextEvent(dpy, &e);
        switch (e.type) {
            case Expose:
                mandRegion = handleRedraw(dpy, win, screen);
                break;
            case ButtonPress:
                handleClick((XButtonPressedEvent *)&e);
                break;
            case ClientMessage:
                if (e.xclient.data.l[0] == WM_DELETE_WINDOW)
                {
                    quit = true;
                }
                break;
        }
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}

struct dim handleRedraw(Display *dpy, Window win, int screen) {
    XWindowAttributes wa;
    GC gc;
    char buf[STR_BUF_LEN];
    struct dim mandRegion;

    gc = DefaultGC(dpy, screen);

    XGetWindowAttributes(dpy, win, &wa);
    snprintf(buf, STR_BUF_LEN, "Window size: %dx%d", wa.width, wa.height);

    XClearWindow(dpy, win);
    XDrawString(dpy, win, gc, 10, 20, buf, strlen(buf));

    mandRegion.x = wa.width;
    mandRegion.y = wa.height;
}

void handleClick(XButtonPressedEvent *bp) {
    printf("Got click (x, y) = (%d, %d)\n", bp->x, bp->y);
}