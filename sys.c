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
#include <unistd.h>

#define RETRY 10	/* We try 10 times... */
#define SLEEP 5		/* ...and sleep 5 sec between each */

/* 
 * Wrapper function to malloc to handle errors 
 */
void
* _malloc(size_t size)
{
	void *p;
	int i = 0;
	while (((p = malloc(size)) == NULL) && (i++ < RETRY))
	{
		fprintf(stderr, "Could not allocate memory!\n"); 
		sleep(SLEEP);
	}

	if (p == NULL)
	{
		fprintf(stderr, "Memory allocation failed, giving up. help?\n");
		exit(EXIT_FAILURE);
	}
	else {
		return p;
	}
}

