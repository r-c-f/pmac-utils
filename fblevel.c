/*
 * ----------------------------------------------------------------------------
 * fblevel - sets the fb device brightness level on Apple Powerbooks
 *
 * Copyright 2000 Stephan Leemburg <stephan@jvc.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 * ----------------------------------------------------------------------------
 * $Log: fblevel.c,v $
 * Revision 1.1.1.1  2001/12/07 11:31:50  sleemburg
 * Initial CVS import of the unreleased pmud-0.8 to apmud (new project name
 * because of a name clash at sourceforge.net).
 *
 * Revision 1.3  2001/11/09 10:55:29  stephan
 * use "pmu.h"
 *
 * Revision 1.2  2000/10/09 14:16:33  stephan
 * use /dev/fb0 in stead of /dev/fb
 *
 * Revision 1.1  2000/05/11 14:48:46  stephan
 * Initial revision
 *
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/fb.h>
#include <linux/pmu.h>

#ifndef FBIOBLANK
#define FBIOBLANK      0x4611          /* 0 or vesa-level+1 */
#endif

static void fbon(int on);
static int set_fblevel(int level);
static int get_fblevel(void);
static void usage(char *program);

int main(int argc, char **argv)
{
	char *program = strrchr(*argv, '/');

	program = program ? program + 1 : *argv;

	switch(argc)
	{
	case 1:
		printf("%d\n", get_fblevel());
		break;
	case 2:
		if(!strcasecmp(*++argv, "on")
		|| !strcasecmp(*argv, "off"))
			fbon(strcasecmp(*argv, "off"));
		else
		{
			register char *p;

			for(p = *argv; *p && isdigit(*p); p++)
				;
			if(*p)
			{
				usage(program);
				return 1;
			}
			if(!set_fblevel(atoi(*argv)))
				return 1;
		}
		break;
	default:
		usage(program);
		return 1;

	}
	return 0;
}

static void fbon(int on)
{
	static int fd = -1;
	if(fd<0)
	{
		fd = open("/dev/fb0", O_RDONLY);
		if(fd<0)
		{
			perror("/dev/fb0");
			return;
		}
	}
	(void)ioctl(fd, FBIOBLANK, !on);
}

static int set_fblevel(int level)
{
	int fd, ret=0;
	if ( (fd = open("/dev/pmu", O_RDONLY)) < 0 ) {
		perror("/dev/pmu");
		goto out;
	}
	if(ioctl(fd, PMU_IOC_SET_BACKLIGHT, &level) == -1) {
		perror("ioctl PMU_IOC_SET_BACKLIGHT");
		goto out;
	}
	ret = 1;	/* success */
out:
	close(fd);
	return ret;
}

static int get_fblevel(void)
{
	int fd, level=0;
	if ( (fd = open("/dev/pmu", O_RDONLY)) < 0 ) {
		perror("/dev/pmu");
		goto out;
	}
	if(ioctl(fd, PMU_IOC_GET_BACKLIGHT, &level) == -1) {
		perror("ioctl PMU_IOC_GET_BACKLIGHT");
		goto out;
	}
out:
	close(fd);
	return level;
}

static void usage(char *program)
{
	fprintf(stderr, 
		"usage: %s [on|off|<level>]\n"
		"       on     : powers display on\n"
		"       off    : powers display off\n"
		"       <level>: set display brightness to <level>\n"
		"                (level is an integer)\n"
		"no argument prints the current brightness level\n",
                program
		);
}
