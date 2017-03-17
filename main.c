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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xacpim.h"

/* Should we run or not? */
u_char flg_run = 1;

int
main(int argc, char *argv[])
{
	int i, geo_mask = NoValue, opt_fork = 1;
	pid_t pid;
	struct xw_t xw;
	struct opt_t *opts;

	opts = _malloc(sizeof(struct opt_t));
	memset(opts, 0, sizeof(struct opt_t));

	xw.display 	= NULL;
	xw.x 		= DEF_X;
	xw.y 		= DEF_Y;
	xw.interval 	= DEF_INT;
	xw.opt_shadow 	= 0;
	xw.opt_temperature = 0;
	xw.opt_tzzone 	= 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0) {
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(argv[i], "-g") == 0) {
			geo_mask = XParseGeometry(argv[++i], &xw.x, &xw.y, NULL, NULL);
		}
		else if (strcmp(argv[i], "-display") == 0) {
			xw.display = argv[++i];
		}
		else if (strcmp(argv[i], "-f") == 0) {
			opts->fname = argv[++i];	
		}
		else if (strcmp(argv[i], "-c") == 0) {
			opts->color = argv[++i];
		}
		else if (strcmp(argv[i], "-cs") == 0) {
			xw.opt_shadow = 1;
			opts->s_color = argv[++i];
		}
		else if (strcmp(argv[i], "-n") == 0) {
			xw.interval = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-t") == 0) {
			xw.opt_temperature = 1;
		}
		else if (strcmp(argv[i], "-tz") == 0) {
			xw.opt_tzzone = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-nf") == 0) {
			opt_fork = 0;	
		}
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	/* Initialize window */
	if (init_win(&xw) != 0)
		exit(EXIT_FAILURE);

	xw.font = XLoadFont(xw.disp, (opts->fname != 0) ? opts->fname : DEF_FONT);
	XSetFont(xw.disp, xw.winGC, xw.font);
	xw.xfs = XQueryFont(xw.disp, xw.font);

	xw.x = (geo_mask & XValue) ? xw.x : DEF_X;
	xw.y = (geo_mask & YValue) ? xw.y : DEF_Y;	

	xw.x += (geo_mask & XNegative) ? DisplayWidth(xw.disp, xw.screen) : 0;
	xw.y += (geo_mask & YNegative) ? DisplayHeight(xw.disp, xw.screen) : 0;

	xw.f_height = xw.xfs->max_bounds.ascent + xw.xfs->max_bounds.descent;
	xw.f_width = xw.xfs->max_bounds.rbearing - xw.xfs->max_bounds.lbearing;

	xw.width = xw.f_width * 128;
	xw.height = xw.f_height * 2; 

	xw.color = get_color(&xw, (opts->color != 0) ? opts->color : DEF_COLOR);
	xw.c_shadow = get_color(&xw, (opts->s_color != 0) ? opts->s_color : DEF_COLOR);

	free(opts);	

	/* Fork into background */
	if (opt_fork == 1) {
		pid = fork();
		if (pid > 0)
			exit(EXIT_SUCCESS);
		else if (pid == -1)
			fprintf(stderr, "Unable to fork into background: %s\n", strerror(errno));
	}

	signal(SIGHUP,  (void *)set_flg);
	signal(SIGINT,  (void *)set_flg);
	signal(SIGKILL, (void *)set_flg);
	signal(SIGTERM, (void *)set_flg);
	signal(SIGQUIT, (void *)set_flg);

	/* Main program loop */
	main_loop(&xw);

	/* Cleanup before we quit */
	close_win(&xw);

	return 0;
}

void
main_loop(struct xw_t *xw)
{
	XEvent xev;
	char flg_update = 1;
	struct val_t values;
	time_t timeval = time(NULL);

	while(flg_run == 1) {

		/* It's time for an update, grab new data */
		if (flg_update == 1) {
			values.bat_time = get_battime();
			values.bat_stat = get_batstat();
			values.bat_acstat = get_acstat();
			values.temperature = get_temperature(xw->opt_tzzone);

			flg_update = 0;
			timeval = time(NULL);
			clear_win(xw);
		}

		while(XPending(xw->disp)) {
			XNextEvent(xw->disp, &xev);
			if (xev.type == Expose) {
				/* Did the exposure happen inside our drawing area? */
				if (((xev.xexpose.x >= xw->x) && (xev.xexpose.x <= (xw->x + xw->width))) ||
					((xev.xexpose.y >= xw->y) && (xev.xexpose.y <= (xw->y + xw->height))))
				{
					redraw(xw, &values);
					XSync(xw->disp, False);
				}
				/* The whole screen was exposed */
				else if (((xev.xexpose.x == 0) && (xev.xexpose.y == 0)) ||
						((xev.xexpose.x == DisplayWidth(xw->disp, xw->screen)) && 
						(xev.xexpose.y == DisplayHeight(xw->disp, xw->screen)))) 
				{
					redraw(xw, &values);
					XSync(xw->disp, False);
				}
			}
		}

		usleep(5000); /* Get some sleep */

		/* Is it time to grab new data from kernel? */
		if ((time(NULL) - timeval) >= xw->interval)
			flg_update = 1;
	}
}

void
set_flg()
{
	
	flg_run = 0;
}

void
usage(char *exec)
{

	printf("X ACPI Monitor %s\n", VERSION);
	printf("Usage %s [options]\n", exec);
	printf("Options:\n");
	printf(" -display disp \t Displayname\n");
	printf(" -g geometry \t [<width>{xX}<height>][{+-}<xoffset>{+-}<yoffset>] (width and height is ignored)\n");
	printf(" -f font \t Font name (default %s)\n", DEF_FONT);
	printf(" -c color \t Color (default %s)\n", DEF_COLOR);
	printf(" -cs color \t Shadow color\n");
	printf(" -n seconds \t Update interval (default %d)\n", DEF_INT);
	printf(" -t \t\t Display temperature\n");
	printf(" -tz zone \t Which thermal zone to gather temperature from (default %d)\n", DEF_TZ);
	printf(" -nf \t\t Don't fork into background\n");
}
