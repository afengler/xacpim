/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <fli at shapeshifter dot se> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * Fredrik Lindberg
 * ----------------------------------------------------------------------------
 */
#ifndef XACPIM_H
#define XACPIM_H

#define VERSION		"0.1.1"

#define	DEF_FONT	"fixed"
#define DEF_COLOR	"white"

#define DEF_X		10		/* Default x position */
#define DEF_Y		10		/* Default y position */

#define DEF_INT		5		/* Default refresh interval */
#define DEF_TZ		0		/* Default thermal zone */
#define	SHW_OFFSET	1		/* Shadow offset in pixels */

/* We use 273.2 it's really 273.15 :/ */
#define KELVTOC(x)      ((x - 2732) / 10.0)

/* Internal datastructure */
struct xw_t {
	char	*display;
	Display	*disp;	
	Window	win;
	GC	winGC;
	int	screen;

	Font	font;
	XFontStruct *xfs;
	u_int	f_height;
	u_int	f_width;

	XColor	color;
	XColor	c_shadow;

	u_char	opt_temperature;
	u_char	opt_tzzone;
	u_char	opt_shadow;	
	u_int	interval;

	int	x;
	int	y;
	u_int	width;
	u_int	height;
};

struct opt_t {
	char    *fname;
	char	*color;
	char 	*s_color;
};

struct val_t {
	int	bat_time;
	int	bat_stat;
	int	bat_acstat;
	int	temperature;
};

void set_flg();

int     get_battime();
int     get_batstat();
int     get_acstat();
int     get_temperature(u_char);

void 	main_loop(struct xw_t *);
void 	usage(char *);

int 	init_win(struct xw_t *);
void 	close_win(struct xw_t *);
void 	redraw(struct xw_t *, struct val_t *);
void 	clear_win(struct xw_t *);
XColor 	get_color(struct xw_t *, char *);

void 	* _malloc(size_t);

#endif
