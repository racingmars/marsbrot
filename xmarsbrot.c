/*
    X Windows interface to marsbrot.
    
	marsbrot, a Mandelbrot Set image rendered.
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
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Request POSIX.1-2008-compliant interfaces. */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

void quit(Widget w, XtPointer client, XtPointer call);
void draw(Widget w, XtPointer client, XtPointer call);
void handleclick(Widget w, XtPointer client_data, XEvent *event, 
                     Boolean *continue_to_dispatch);

int main(int argc, char **argv)
{
    XtAppContext app;
    Widget root, form, command, command2, lbl1, lbl2, lbl3, simple;
    Arg arglist[10];
    Cardinal arglc = 0;

    XtSetLanguageProc(NULL, NULL, NULL);
    root = XtOpenApplication(&app, "xmarsbrot", NULL, 0, &argc, argv, NULL, shellWidgetClass, NULL, 0);
    form = XtCreateManagedWidget("form", formWidgetClass, root, NULL, 0);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNborderWidth,  0); arglc++;
    lbl1 = XtCreateManagedWidget("My label:", labelWidgetClass, form, arglist, arglc);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNfromHoriz,  lbl1); arglc++;
    command = XtCreateManagedWidget("Quit", commandWidgetClass, form, arglist, arglc);
    XtAddCallback(command, XtNcallback, quit, app);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNfromHoriz,  command); arglc++;
    command2 = XtCreateManagedWidget("draw", commandWidgetClass, form, arglist, arglc);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNfromVert,  command); arglc++;
    XtSetArg(arglist[arglc], XtNborderWidth,  0); arglc++;
    lbl2 = XtCreateManagedWidget("Lab 2:", labelWidgetClass, form, arglist, arglc);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNfromHoriz,  lbl1); arglc++;
    XtSetArg(arglist[arglc], XtNborderWidth,  0); arglc++;
    XtSetArg(arglist[arglc], XtNfromVert,  command); arglc++;
    lbl3 = XtCreateManagedWidget("Lab 3", labelWidgetClass, form, arglist, arglc);

    arglc = 0;
    XtSetArg(arglist[arglc], XtNborderWidth,  0); arglc++;
    XtSetArg(arglist[arglc], XtNfromVert,  lbl2); arglc++;
    XtSetArg(arglist[arglc], XtNwidth,  500); arglc++;
    XtSetArg(arglist[arglc], XtNheight,  500); arglc++;
    XtSetArg(arglist[arglc], XtNborderWidth,  1); arglc++;
    simple = XtCreateManagedWidget("Simple", simpleWidgetClass, form, arglist, arglc);

    XtAddCallback(command2, XtNcallback, draw, simple);

    //XtAddCallback(simple, XtNcallback, handleclick, None);
    XtAddEventHandler(simple, ButtonPressMask | ExposureMask, False, handleclick, simple);

    XtRealizeWidget(root);
    XtAppMainLoop(app);
    printf("Done!\n");
    return 0;
}

void quit(Widget w, XtPointer client, XtPointer call)
{
    XtAppSetExitFlag(client);
}

void draw(Widget w, XtPointer client, XtPointer call)
{
    Display *display;
    Drawable window;
    GC gc;
    int x, y;

    display = XtDisplay(client);
    window = XtWindow(client);
    gc = XCreateGC(display, window, None, None);
    printf("Got GC\n");

    XSetForeground(display, gc, 0xFF0000);

    for (x=0; x<100; x++) {
        for (y = 0; y<100; y++) {
            XDrawPoint(display, window, gc, x, y);
        }
    }

    XSetForeground(display, gc, 0xFF44AA);

    for (x=100; x<200; x++) {
        for (y = 100; y<200; y++) {
            XDrawPoint(display, window, gc, x, y);
        }
    }
}

void changeText(Widget w, XtPointer client, XtPointer call)
{

}

void handleclick(Widget w, XtPointer client_data, XEvent *event, 
                     Boolean *continue_to_dispatch)
{
    XButtonPressedEvent *bp = (XButtonPressedEvent *) event;
    XExposeEvent *rr = (XExposeEvent *) event;
    if (event->type == ButtonPress)
    {
        printf("Got click (x, y) = (%d, %d)\n", bp->x, bp->y);
    }
    else if (event->type == Expose)
    {
        printf("Got resize\n");
        draw(None, client_data, None);
    }
}
