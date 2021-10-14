/*
 * autoboot.c - set/clear Server Mode boot on the VIA-CUDA
 *
 *
 * Copyright (C) 2005 Rich Johnson.  All rights rserved.
 *
 * This code is modeled on pmacpow, written by Takashi Oe
 * and modified by Keith Keller.
 * 
 * To make: save file to autoboot.c
 * gcc -o autoboot  autoboot.c
 * strip autoboot
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Setting Server Mode causes the machine to boot automatically
 * upon restoration of power after an outage.
 * Clearing Server Mode will requires a manual boot.
 * Normal shutdown and sheduling a boot (see bootsched) clear
 * Server Mode.
 *
 * The server mode setting is good for the current OS session only
 * and is cleared when the machine boots.  For continuous
 * unattended operation autoboot should be invoked during the boot 
 * process.
 *
 * Implemented configurations are:
 *  CUDA machines:  2.4 and 2.6 series kernels
 *  PMU machine:  2.6 series kernels
 *
 * see also: bootsched.
 */

#include "autoboot-conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

unsigned char adb_cmd[3] =
	{ CUDA_PACKET, 0x13, 0 };

char pmu_cmd[] = "server_mode=0";

static int verbose = 0;
static int version  = 0;
static int synopsis  = 0;
static int servermode = 0;
static char *program_name;

static void usage(void) {
	fprintf(stderr,
		"%s [-v] on\n"
		"%s [-v] off\n"
		"%s -V\n"
		"%s -?\n"
		"\t-v: verbose operation\n"
		"\t-V: print version number and exit \n"
		"\t-?: print this synopsis\n"
		"\ton: enable server mode, boot whenever power restored\n"
		"\toff: disable server mode, boot manually\n",
		program_name, program_name, program_name, program_name );
	exit(1);
}


static void print_version( void ){
	fprintf( stderr, "%s: v1.1\n", program_name );
	exit(0);
	};

static int pmu_set_servermode( long is_server )
{
	int fd;
	int n;

	pmu_cmd[ strlen(pmu_cmd)-1 ]	= '0';

	if ((fd = open("/proc/pmu/options", O_WRONLY)) < 0) {
		return -1;
	}
	if( is_server )
		pmu_cmd[ strlen(pmu_cmd)-1 ]	= '1';
	n = write(fd, pmu_cmd, sizeof(pmu_cmd));
	close( fd );

	if (n != sizeof(pmu_cmd)) {
		fprintf(stderr, "%s: PMU write returned %d\n", program_name, n);
		return -1;
	}
	return 0;
}

static int adb_set_servermode( long is_server )
{
	int fd;
	int n;

	adb_cmd[2]	 = 0;
	if ((fd = open("/dev/adb", O_RDWR)) < 0) {
		return -1;
	}
	if( is_server != 0 )
		adb_cmd[2]	= 1;
	n = write(fd, adb_cmd, sizeof(adb_cmd));
	close(fd);

	if (n != sizeof(adb_cmd)) {
		fprintf(stderr, "%s: ADB write returned %d\n", program_name, n);
		return -1;
	}
	return 0;
}

static void report_result( long result, long is_server )
{
	if( result != 0 ){
		fprintf(stderr, "Automatic boot status unchanged\n");
	}
	  else {
		if( is_server ){
			fprintf(stderr, "Automatic boot enabled (Server Mode)\n");
		}
		  else{
		  	fprintf(stderr, "Automatic boot disabled \n");
		}
	}
}

extern int optind;

int main(int argc, char **argv)
{
	int arg;
	int result;

	program_name = (char *)basename(argv[0]);
	if (getuid()) {
		fprintf(stderr, "Sorry, must be root to set power up time\n");
		return 1;
	}
	while ((arg = getopt(argc, argv, "vV")) != EOF) {
		switch (arg) {
		case 0:
			break;
		case '?':
			synopsis = 1;
			break;
		case 'V':
			version = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}
	}

	if(synopsis)
		usage();
	if (version){
		if (optind != argc)
			usage();
		print_version();
	}
        if (optind != argc-1)
		usage();

	if( strcmp( argv[optind], "on" ) == 0 ){
		servermode = 1;
	}
	else if( strcmp( argv[optind], "off" ) == 0 ){
		servermode = 0;
	}
	else{
		usage();
	}
	
	/* first, assume a modern machine */
	result	= pmu_set_servermode( servermode );

	/* ...if not successful, see if we have an older ADB/CUDA machine */
	if( result ){
		result = adb_set_servermode( servermode );
		}

	/* ... and report the results */
	if( verbose ){
		report_result( result, servermode );
		}

	exit(result);
}
