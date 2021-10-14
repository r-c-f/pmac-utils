/* trackpad.c
 *
 * Tool for setting the PowerBook trackpad options on linux
 *
 * bye benh <bh40@calva.net>
 * 2/13/99
 *
 * 3/17/99 - Minor Fix: usage display used to leave /dev/adb open
 *
 * 10/12/99 - addition of 'show' option to aid in pmud debugging
 *
 * Pieces from mousehack, from numerous contributors...
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/cuda.h>
#include <linux/adb.h>

//#define DEBUG

int fd;

void
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
    if (n < len) {
	perror("writing /dev/adb");
	close(fd);
	exit (EXIT_FAILURE);
    }
}

int
listen(unsigned char *y)
{
    int n;
    
    n = read(fd, y, 80);

#ifdef DEBUG
    printf("listen (%d bytes): ",n);
    if (n >= 0) {
	int i;
	for (i=0; i < n; i++)
	    printf("0x%02x ",y[i]);
    }
    printf("\n");
#endif

    if (n < 0) {
	perror("reading /dev/adb");
	close(fd);
	exit(EXIT_FAILURE);
    }
    
    return n;
}

void
set_program_mode(int id, int set)
{
	unsigned char buf[16];
	int n;
	
#ifdef DEBUG
	printf("program mode set to: %d\n", set);
#endif
	
	buf[0] = ADB_PACKET;
	buf[1] = ADB_READREG(id, 1);
	send(buf, 2);
	n = listen(buf+1);
	buf[0] = ADB_PACKET;
	buf[1] = ADB_WRITEREG(id, 1);
	buf[8] = set ? 0x0d : 0x03;
	send(buf, 1+n);
	listen(buf);

	buf[0] = ADB_PACKET;
	buf[1] = ADB_READREG(id, 2);
	send(buf, 2);
	n = listen(buf+1);
}

enum
{
	mode_invalid = 0,
	mode_notap,
	mode_tap,
	mode_drag,
	mode_lock,
	mode_show
};


void
set_trackpad(int id, int mode)
{
	unsigned char buf[16];

	set_program_mode(id, 1);
	
#ifdef DEBUG		
	printf("setting mode: %d\n", mode);
#endif

	if (mode == mode_show) {
		int n;

		printf("READREG(%d, %d) ", id, 2);
		buf[0] = ADB_PACKET;
		buf[1] = ADB_READREG(id, 2);
		send(buf, 2);
		n = listen(buf);
#ifdef DEBUG
		printf("(reply %d bytes): ",n);
		if (n >= 0) {
			int ii;
			for (ii=0; ii < n; ii++)
				printf("0x%02x ",buf[ii]);
		}
		printf("\n");
#endif
		printf("trackpad settings %stap %sdrag %slock\n",
			buf[1] == 0x99 ? "" : "no",
			buf[2] == 0x94 ? "" : "no",
			buf[4] == 0xff ? "" : "no"
		);
	} else {
		buf[0] = ADB_PACKET;
		buf[1] = ADB_WRITEREG(id, 2);
		buf[2] = (mode < mode_tap) ? 0x19 : 0x99;
		buf[3] = (mode < mode_drag) ? 0x14 : 0x94;
		buf[4] = 0x19;
		buf[5] = (mode < mode_lock) ? 0xb2 : 0xff;
		buf[6] = 0xb2;
		buf[7] = 0x8a;
		buf[8] = 0x1b;
		buf[9] = 0x50;
		send(buf, 10);
		listen(buf);
	
#ifdef DEBUG
		{
		int i, n;
	
		for(i=0; i<4; i++)
		{
			printf("READREG(%d, %d) ", id, i);
			buf[0] = ADB_PACKET;
			buf[1] = ADB_READREG(id, i);
			send(buf, 2);
			n = listen(buf+1);
			printf("(reply %d bytes): ",n);
			if (n >= 0) {
				int ii;
				for (ii=1; ii <= n; ii++)
					printf("0x%02x ",buf[ii]);
			}
			printf("\n");
		}
		}
#endif
	}
	set_program_mode(id, 0);

}

int
locate_trackpad(void)
{
	int i, n;
	
	for (i=1; i<16; i++)
	{
		unsigned char buf[16];
		
#ifdef DEBUG		
		printf("testing %d...\n", i);
#endif		
		buf[0] = ADB_PACKET;
		buf[1] = ADB_READREG(i, 1);
		send(buf, 2);
		n = listen(buf);
		if ((n >= 4) && (buf[1] == 0x74) && (buf[2] == 0x70) &&
			(buf[3] == 0x61) && (buf[4] == 0x64))
		{
#ifdef DEBUG		
			printf("found trackpad at %d\n", i);
#endif
			return i;
		}
	}
	return -1;
}

int
main(int argc, char **argv)
{
	int id;
	int mode = mode_invalid;
	
  	if (argc >= 2) {
	  	if (strcmp(argv[1], "notap") == 0)
	  		mode = mode_notap;
	  	else if (strcmp(argv[1], "tap") == 0)
	  		mode = mode_tap;
	  	else if (strcmp(argv[1], "drag") == 0)
	  		mode = mode_drag;
	  	else if (strcmp(argv[1], "lock") == 0)
	  		mode = mode_lock;
	  	else if (strcmp(argv[1], "show") == 0)
	  		mode = mode_show;
	}  	
  	if (mode == mode_invalid)
  	{
  		printf("usage: trackpad notap|tap|drag|lock|show\n");
  		return 0;
  	}
  	
	fd = open("/dev/adb", O_RDWR);
	if (fd < 0) {
		perror("opening /dev/adb");
		exit(EXIT_FAILURE);
	}
  
	id = locate_trackpad();
	if (id < 0)
	{
		printf("no trackpad !\n");
		return 0;
	}
	
	set_trackpad(id, mode);
	
	close(fd);
	return 0;
}

