/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <fli at shapeshifter dot se> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * Fredrik Lindberg
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xacpim.h"

int
init_win(struct xw_t *xw)
{
	XGCValues gvc;

	if ((xw->disp = XOpenDisplay(xw->display)) == 0)
	{
		fprintf(stderr, "Could not open display %s\n", xw->display);
		return 1;
	}
	xw->screen = DefaultScreen(xw->disp);
	xw->win = RootWindow(xw->disp, xw->screen);

	gvc.graphics_exposures = True;
	xw->winGC = XCreateGC(xw->disp, xw->win, GCBackground, &gvc);
	XMapWindow(xw->disp, xw->win);

	XSelectInput(xw->disp, xw->win, ExposureMask|FocusChangeMask);
	return 0;
}

void
close_win(struct xw_t *xw)
{

	XSync(xw->disp, False);
	clear_win(xw);
	XSync(xw->disp, False);
	XCloseDisplay(xw->disp);
}

XColor
get_color(struct xw_t *xw, char *color)
{
	XColor c;
	XWindowAttributes atrb;

	XGetWindowAttributes(xw->disp, xw->win, &atrb);
	XParseColor(xw->disp, atrb.colormap, color, &c);
	XAllocColor(xw->disp, atrb.colormap, &c);
	return c;
}

void
redraw(struct xw_t *xw, struct val_t *values)
{
	char buffer[128];

	sprintf(buffer, "Running on %s", (values->bat_acstat == 0) ? "battery" : "AC-power");

	if ((values->bat_acstat == 0) && (values->bat_time > -1)) {
		sprintf(buffer, "%s, remaining %d:%2.2d", buffer, values->bat_time / 60, values->bat_time % 60);
	}

	if (values->bat_stat > 0) {
		sprintf(buffer, "%s (%scharging)", buffer, (values->bat_stat == 1) ? "dis" : "");
	}

	if ((values->temperature > -1) && (xw->opt_temperature == 1)) {
		sprintf(buffer, "%s Temperature %0.1f°C", buffer, KELVTOC(values->temperature));
	}


	if (xw->opt_shadow == 1) {
		XSetForeground(xw->disp, xw->winGC, xw->c_shadow.pixel);
		XDrawString(xw->disp, xw->win, xw->winGC, xw->x + SHW_OFFSET, 
			xw->y + xw->f_height + SHW_OFFSET, buffer, strlen(buffer));
	}

	XSetForeground(xw->disp, xw->winGC, xw->color.pixel);
	XDrawString(xw->disp, xw->win, xw->winGC, xw->x, xw->y + xw->f_height, buffer, strlen(buffer));
}

void
clear_win(struct xw_t *xw)
{

	XClearArea(xw->disp, xw->win, xw->x, xw->y, xw->width, xw->height, True);
}

