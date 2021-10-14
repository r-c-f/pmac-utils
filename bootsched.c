/*
 * bootsched.c - set the next power up time on VIA-CUDA.
 *
 * Debian version of pmacpow.c (essentially, renamed and docs added)
 * 2005-07-06 MSch
 *
 * Copyright (C) 1999 Takashi Oe.  All rights reserved.
 * Parts of the code are from clock.c and bat.c
 * by Paul Mackerras.
 *
 * Modified 2002 Keith Keller.
 * 
 * To make: save file to pmacpow.c
 * gcc -o pmacpow pmacpow.c
 * strip pmacpow
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include "autoboot-conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

#ifdef HAVE_ASM_ADB_H
# include <asm/adb.h>
# else
#  ifdef HAVE_LINUX_ADB_H
#   include <linux/adb.h>
#  endif
#endif

#ifdef HAVE_ASM_CUDA_H
# include <asm/cuda.h>
# else
#  ifdef HAVE_LINUX_CUDA_H
#   include <linux/cuda.h>
#  endif
#endif

unsigned char reqt[2] =
	{ CUDA_PACKET, CUDA_GET_TIME};
unsigned char reqp[6] =
	{ CUDA_PACKET, CUDA_POWERUP_TIME, 0, 0, 0, 0};

unsigned char reply[16];

/* RTC on PowerMacs stores seconds since 1 Jan 1904 */
#define RTC_OFFSET	2082844800

static int week = 0;
static int day = -1;
static int abstime = 0;
static int quiet = 0;
static char *program_name;

void dump(unsigned char *buf, int n)
{
	int i;

	for (i = 0; i < n; ++i)
		fprintf(stderr, " %.2x", buf[i]);
	fprintf(stderr, "\n");
}

void usage(void) {
	fprintf(stderr,
		"%s [-q] [-w|-e|-d DAY] -t HH:MM\n"
		"%s [-q] +value[mhd]\n"
		"\t-w: set next power-up on a weekday\n"
		"\t-e: set next power-up on a weekend day\n"
		"\t-d DAY: set next power-up to DAY (0..6);"
			" 0 represents Sunday\n"
		"\t-q: quiet mode\n"
		"\t-t HH:MM: time of day to power-up\n"
		"\t+value[mhd]: time in secs, mins, hours, or days ahead of"
			" current time\n",
		program_name, program_name);
	exit(1);
}

int next_wday(int wday) {
	if (wday == 0)
		return 1;
	if (wday == 6)
		return 2;
	return 0;
}

int next_weday(int wday) {
	if ((wday >=1) && (wday < 6))
		return 6 - wday;
	return 0;
}

int set_powerup_time(int fd, long up)
{
	int n;

	*(long *)(reqp+2) = up;
	n = write(fd, reqp, sizeof(reqp));
	if (n != sizeof(reqp)) {
		fprintf(stderr, "%s: write returned %d\n", program_name, n);
		return -1;
	}
	if ((n = read(fd, reply, sizeof(reply))) < 0) {
		perror("read");
		return -1;
	}
	if (n != 3) {
		dump(reply, n);
		fprintf(stderr, "%s: command failed\n", program_name);
		return -1;
	}
	return 0;
}

long get_current_time(int fd)
{
	int n;

	n = write(fd, reqt, sizeof(reqt));
	if (n != sizeof(reqt)) {
		fprintf(stderr, "%s: write returned %d\n", program_name, n);
		return -1;
	}
	if ((n = read(fd, reply, sizeof(reply))) < 0) {
		perror("read");
		return -1;
	}
	if (n != 7) {
		dump(reply, n);
		fprintf(stderr, "%s: command failed\n", program_name);
		return -1;
	}
	return (long) ((reply[3] << 24) + (reply[4] << 16)
			+ (reply[5] << 8) + (long) reply[6]);
}

long calc_powerup_time(int hour, int min)
{
	struct tm tm, ctm;
	time_t systime;

	systime = time(NULL);
	ctm = *localtime(&systime);
	tm = ctm;
	if (!quiet)
		fprintf(stderr, "current time is %s", asctime(&tm));
	tm.tm_sec = 0;
	tm.tm_min = min;
	tm.tm_hour = hour;
	if (mktime(&tm) < (systime + 60)) {
		++tm.tm_mday;
		++tm.tm_wday;
		(void)mktime(&tm);
	}
	if (day == -1) {
		if (week == 1)
			tm.tm_mday += next_wday(tm.tm_wday);
		if (week == 2)
			tm.tm_mday += next_weday(tm.tm_wday);
	} else {
		if (day != tm.tm_wday) {
			if (day < tm.tm_wday)
				day += 7;
			tm.tm_mday += day - tm.tm_wday;
		}
	}
	return (long)(mktime(&tm) + RTC_OFFSET);
}

int main(int argc, char **argv)
{
	int fd, arg, hour = 0, min = 0;
	long sec, uptime;
	char *timestr = NULL, *tmp;
	extern int optind;

	program_name = (char *)basename(argv[0]);
	if (getuid()) {
		fprintf(stderr, "Sorry, must be root to set power up time\n");
		return 1;
	}
	while ((arg = getopt(argc, argv, "wed:tq")) != EOF) {
		switch (arg) {
		case 0:
			break;
		case 'w':
			week = 1;
			break;
		case 'e':
			week = 2;
			break;
		case 'd':
			day = atoi(optarg);
			if ((day < 0) || (day > 6))
				usage();
			break;
		case 't':
			abstime = 1;
			break;
		case 'q':
			quiet = 1;
			break;
		default:
			usage();
		}
	}
	if (argv[optind])
		timestr = argv[optind];
	else
		usage();
	tmp = strsep(&timestr, ":");
	if (tmp && abstime) {
		if (!tmp || !timestr)
			usage();
		hour = atoi(tmp);
		min = atoi(timestr);
		sec = 0;
	} else {
		timestr = argv[optind];
		sec = strtol(timestr, &tmp, 10);
		if (tmp) {
			switch (tolower(tmp[0])) {
			case 'd':
				sec *= 24;
			case 'h':
				sec *= 60;
			case 'm':
				sec *= 60;
				break;
			}
		}
	}

	if ((fd = open("/dev/adb", O_RDWR)) < 0) {
		perror("open");
		return 1;
	}
	if (sec > 0) {
		long cur = get_current_time(fd);

		if (cur == -1)
			return 1;
		uptime = cur + sec;
	} else
		uptime = calc_powerup_time(hour, min);
	if (set_powerup_time(fd, uptime) < 0)
		return 1;
	close(fd);
	if (!quiet) {
		uptime -= RTC_OFFSET;
		fprintf(stderr, "will be up again %s",
			asctime(localtime(&uptime)));
	}
	return 0;
}
