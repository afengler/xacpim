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
#include <sys/sysctl.h>

#include <errno.h>

/*
 * We gather the acpi data using sysctl, perhaps not the best way to do it
 * but it doesn't require uid 0 nor tampering with /dev/acpi
 */

#define CTL_BATTIME	"hw.acpi.battery.time"
#define CTL_BATSTATE	"hw.acpi.battery.state"
#define CTL_ACLINE	"hw.acpi.acline"
#define CTL_TEMP	"hw.acpi.thermal.tzX.temperature"
#define CTL_TEMP_LEN	36	/* Allocation length */
#define CTL_TEMP_OFST	18	/* Offset where to inject number in CTL_TEMP */

/* XXX: This prototype is for sys.c, and already defined in xacpim.h
 * but we don't want to include xacpim.h here because it contains X11 stuff
 */
void *	_malloc(size_t); 

int 	get_battime(); 
int 	get_batstat();
int 	get_acstat();
int	get_temperature(u_char); 

int 	_get_sysctl(char *, void *);

/*
 * Remaining battery time in minutes, error -1
 */
int
get_battime()
{
	int battime;

	if (_get_sysctl(CTL_BATTIME, &battime) != -1)
		return battime;
	else
		return -1;
}

/* Battery state,
 *  0 Charged
 *  1 Discharging
 *  2 Charging 
 * -1 Error
 */ 
int
get_batstat()
{
	int batstate;

	if (_get_sysctl(CTL_BATSTATE, &batstate) != -1)
		return batstate;
	else
		return -1;
}

/* 
 * AC state,
 *  0 Disconnected
 *  1 Connected
 * -1 Error
 */ 
int
get_acstat()
{
	int acstat;

	if (_get_sysctl(CTL_ACLINE, &acstat) != -1)
		return acstat;
	else
		return -1;
}

/* 
 * Temperature
 * Value is returned in kelvin * 10
 */
int
get_temperature(u_char zone)
{
	int temperature = -1;
	char *buffer;

	/* We only support 10 thermal zones, 0-9 */
	if (zone <= 9) { 
		buffer = _malloc(CTL_TEMP_LEN);
		memset(buffer, 0, CTL_TEMP_LEN); /* Make sure it's fully null'ed  */

		memcpy(buffer, CTL_TEMP, strlen(CTL_TEMP));
		buffer[CTL_TEMP_OFST] = zone + '0'; /* Need the ascii values */ 

		if (_get_sysctl(buffer, &temperature) == -1)
			temperature = -1;	

		free(buffer);
	}

	return temperature;
}

/*
 * sysctl wrapper-function to handle errors
 */ 
int
_get_sysctl(char *sysctl, void *res)
{
	size_t len = sizeof(res);

	if (sysctlbyname(sysctl, res, &len, NULL, 0) == -1) {
		fprintf(stderr, "sysctl: %s -> %s\n", sysctl, strerror(errno));
		return -1;
	}	
	return 0;
}

