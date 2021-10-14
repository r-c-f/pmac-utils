/* mousemode.c
 *
 * A program for linux-pmac by jonh Tue Feb 18 00:46:10 EST 1997
 *
 * which feeds the right things to /dev/adb to reconfigure
 * Apple Desktop Bus mice. It sets a mouse's ADB register 3 to the
 * command line argument, which tells the mouse to invoke a different
 * handler. Handler 4 on my Logitech mouse says to return extended data.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int fd;	/* put this here where everybody can see it. Hey, it's not so	*/
	/* much a global as it is an "object variable," where this	*/
	/* program is the object. Yeah, that's it! Ahem.		*/

static void setmouse( int );
static int showmouse( void );
static void send( char *, int );
static void listen( char * );

int main(int argc, char **argv)
{
	int mode;

	fd = open("/dev/adb", O_RDWR);
	if (fd <= 0) {
		perror("opening /dev/adb");
		exit(EXIT_FAILURE);
	}

	if (argc >= 2) {
   	   mode = atoi(argv[1]);
	   if ((argc == 2 && (mode <= 0 || mode >= 0x0fd)) || argc >= 3) {
	      (void) printf("usage: mousemode [n]\n");
	      (void) printf("  Configures mouse at ADB address 3 to use handler ID n.\n");
	      (void) printf("  If n is omitted, prints value of current handler ID.\n");
	      exit(EXIT_FAILURE);
	   }

	   if (argc == 2) {
	      setmouse(mode);
	   }
	}

	(void) printf("%d\n", showmouse());

	(void) close(fd);
	exit(EXIT_SUCCESS);
}

static void setmouse(int mode)
{
	char y[15];

	/* curious parties should read Inside Mac/Devices/ADB Manager,	*/
	/* looking at page 5-11. Inside Mac is available as pdf files	*/
	/* from Apple's site.						*/

	/* CUDA device 0? (the clock seems to be at 1) */
	y[0]=(char) 0x000;
	/* mouse (0x30) listen (0x08) reg 3 (0x03) */
	y[1]=(char) 0x03b;
	/* service request enable (0x20), device addr 3 (0x03) */
	y[2]=(char) 0x023;
	/* device handler ID == mode */
	y[3]=(char) mode;

	send(y, 4);
	listen(y);
}

static int showmouse()
{
	char y[15];

	y[0]=(char) 0x000;	/* Cuda device 0 */
	y[1]=(char) 0x03f;	/* mouse talk reg 3 */

	send(y, 2);
	listen(y);

					/* make sure reply is from: */
	if (y[0] == (char) 0		/* cuda device 0 */
		&& y[1] == (char) 0	/* no status/error bits (I'm guessing) */
		&& y[2] == (char) 0x03f) {	/* mouse talk reg 3 */
	  /* skip 1st byte of reg 3, and return handler ID */
		return( (int) y[4] );
	} else {
		return -1;
	}
}

static void send(char *y, int len)
{
	int n;

	n = (int) write(fd, y, (size_t) len);
	if (n < len) {
		perror("writing /dev/adb");
		(void) close(fd);
		exit (EXIT_FAILURE);
	}
}

static void listen(char *y)
{
	int n;

	do {
		n = (int) read(fd, y, (size_t) 80);
		if (n > 0) {
			y += (char) n;
		} else if (n<0) {
			perror("reading /dev/adb");
			(void) close(fd);
			exit(EXIT_FAILURE);
		}
	} while (n > 0);
}
