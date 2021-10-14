/* backlight.c - set backlight level
 *
 * Michael Schmitz <schmitz@biophys.uni-duesseldorf.de>
 *
 * based on: cannibalized trackpad.c
 *
 * Tool for setting the PowerBook trackpad options on linux
 *
 * by benh <bh40@calva.net>
 * 2/13/99
 *
 * Pieces from mousehack, from numerous contributors...
 *
 */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/cuda.h>
#include <linux/adb.h>
#include <linux/pmu.h>

#undef DEBUG_SCAN
#define DEBUG_REPLY
#undef DEBUG

/*
 * muh buffer overruns necessitate this
*/
#define BUFLEN 80


int fd;
int pmu_fd;

/*
 * we ignore write and read errors, or rather pass the error codes
 */
int
send(unsigned char *y, int len)
{
    int n;

#ifdef DEBUG
    printf("send: ");
    for (n=0; n < len; n++)
	printf("0x%02x ",y[n]);
    printf("\n");
#endif

    n = write(fd, y, (size_t) len);

    return n;
}

int
listen(unsigned char *y)
{
    int n;

    n = read(fd, y, BUFLEN);

#ifdef DEBUG
    printf("%d: ",n);
    if (n > 0) {
	int i;
	for (i=0; i < n; i++)
	    printf("0x%02x ",y[i]);
    }
    printf("\n");
#endif

    return n;
}


/*
 * see drivers/macintosh/via-pmu.c
 */
void
set_backlight_level(int id, int set)
{
	unsigned char buf[BUFLEN + 1]; /* for offset */

#ifdef DEBUG
	printf("level set to: %d\n", set);
#endif

	buf[0] = id;
	buf[1] = PMU_BACKLIGHT_BRIGHT;
	buf[2] = (set < 1 ? 0x7f : 0x4a - (set<<1) );
	send(buf, 3);
	listen(buf+1);

	buf[0] = id;
	buf[1] = PMU_POWER_CTRL;
	buf[2] = PMU_POW_BACKLIGHT | (set < 1 ? PMU_POW_OFF : PMU_POW_ON);
	send(buf, 3);
	listen(buf);
}

/*
 * heuristics for finding the PMU: the first device that responds to a
 * extended battery status request is assumed to be the PMU
 */
int
locate_pmu(void)
{
	int i, n;

	unsigned char buf[BUFLEN];

	for (i=1; i<16; i++) {
#ifdef DEBUG_SCAN
		printf("testing %d...\n", i);
#endif
		buf[0] = i;
		buf[1] = 0x6b;

		send(buf, 2);
		n = listen(buf);

		if (n >= 8)
		{
#ifdef DEBUG
			printf("found PMU at %d\n", id);
#ifdef DEBUG_REPLY
			printf("%d: ",n);
			if (n > 0) {
				int j;
				for (j=0; j < n; j++)
				printf("0x%02x ",buf[j]);
			}
    			printf("\n");
#endif
#endif
			return i;
		}
	}
	return -1;
}

int
main(int argc, char **argv)
{
	int id, level, use_adb, set_level, quiet, usage;
#ifdef TEST_ADBDEV
	char devname[64];
#endif
	int arg;

	id = -1;
	level = -1;
	use_adb = 0;
	set_level = 1;
	quiet = 0;
	usage = 0;

	while ((arg = getopt (argc, argv, "aghq")) != -1) {
		if (arg == 'h')
			usage = 1;
		else if (arg == 'q')
			quiet = 1;
		else if (arg == 'a')
			use_adb = 1;
		else if (arg == 'g') {
			level = 0;
			set_level = 0;
		}
	}
	if (optind && argv[optind] && set_level)
		level = atol (argv[optind]);

  	if (level < 0 || usage)
  	{
  		printf("usage: backlight [-h] | [-g] | [-a] [-q] <level> \n");
  		printf("       -h         print this usage information\n");
  		printf("       -q         quiet mode\n");
  		printf("       -a         set backlight level using /dev/adb\n");
  		printf("                  (default is to set using ioctl)\n");
  		printf("       -g         read backlight level. using ioctl\n");
  		printf("  *************         WARNING         ************\n");
  		printf("  **  backlight is obsolete, please use fblevel   **\n");
  		printf("  **************************************************\n");
  		return 0;
  	}

	printf("  *************         WARNING         ************\n");
	printf("  **  backlight is obsolete, please use fblevel   **\n");
	printf("  **************************************************\n");

	fd = open("/dev/adb", O_RDWR);
	if (fd <= 0) {
		perror("opening /dev/adb");
		exit(EXIT_FAILURE);
	}

	id = locate_pmu();
	if (id < 0)
	{
		printf("no PMU found !\n");
		return 0;
	}

#ifdef DEBUG_SCAN
	printf("PMU found at %d!\n", id);
#endif

#ifdef TEST_ADBDEV	/* needs kernel patch ... */
	close(fd);

	/*
	 * 'raw' device; doesn't work for devices with assigned input
	 * handler currently (bug in kernel driver)
	 * 'buffered' device, however, doesn't work for devices
	 * without handler attached ...
	 */
	sprintf(devname, "/dev/adb%d", id);

	fd = open(devname, O_RDWR);
	if (fd <= 0) {
		perror("opening device");
		printf("Could not open %s\n", devname);
		exit(EXIT_FAILURE);
	}
#endif
	if (!use_adb) {
		pmu_fd = open("/dev/pmu", O_RDWR);
		if (pmu_fd <= 0) {
			perror("opening device");
			printf("Could not open device /dev/pmu \n");
			exit(EXIT_FAILURE);
		}
	}

	if (set_level) {
		if (use_adb) {
			/* use /dev/adb write to talk to PMU */
			set_backlight_level(id, level);
		} else {
			/* use /dev/pmu ioctl to talk to PMU */
			if (ioctl(pmu_fd, PMU_IOC_SET_BACKLIGHT, &level) < 0)
				perror("PMU_IOC_SET_BACKLIGHT ioctl");
		}
		if (!quiet)
			printf("Backlight set to level: %d\n", level);
	} else {
		/* use /dev/pmu ioctl to talk to PMU */
		if (ioctl(pmu_fd, PMU_IOC_GET_BACKLIGHT, &level) < 0)
			perror("PMU_IOC_GET_BACKLIGHT ioctl");

		printf("Backlight level: %d\n", level);
	}

	close(pmu_fd);
	close(fd);
	return 0;
}
