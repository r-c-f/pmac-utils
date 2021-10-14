/* mousemode.c
 *
 * A program for linux-pmac by jonh Tue Feb 18 00:46:10 EST 1997
 * hacked mercilessly by warner@lothar.com: don't blame jonh for my bugs!
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
#include <linux/adb.h>
#include <linux/cuda.h>

int fd;	/* put this here where everybody can see it. Hey, it's not so	*/
	/* much a global as it is an "object variable," where this	*/
	/* program is the object. Yeah, that's it! Ahem.		*/


void
send(unsigned char *y, int len)
{
    int n;

#if 0
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

void
listen(unsigned char *y)
{
    int n;
    
    n = read(fd, y, 80);
#if 0
    printf("%d: ",n);
    if (n > 0) {
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
}

void
setmouse(int addr, int mode)
{
    unsigned char y[15];

    /* curious parties should read Inside Mac/Devices/ADB Manager,	*/
    /* looking at page 5-11. Inside Mac is available as pdf files	*/
    /* from Apple's site.						*/

    /* CUDA device 0? (the clock seems to be at 1) */
    y[0] = ADB_PACKET;
    /* mouse (0x30) listen (0x08) reg 3 (0x03) */
    y[1] = ADB_WRITEREG(addr, 3);
    /* service request enable (0x20), device addr 3 (0x03) */
    //y[2]=(char) 0x023;
    y[2] = 0x20 + addr;
    /* device handler ID == mode */
    y[3]= mode;

    send(y, 4);
    listen(y);
}

int 
showmouse(int addr)
{
    unsigned char y[15];
    
    y[0] = ADB_PACKET;
    y[1] = ADB_READREG(addr, 3);

    send(y, 2);
    listen(y);

    /* make sure reply is from: */
    if (y[0] == ADB_READREG(addr, 3)) {
	return (y[2]);
    } else {
	return -1;
    }
}

int
main(int argc, char **argv)
{
    int addr, mode;
    
    // argc==2: 'mousemode <addr>'
    // argc==3: 'mousemode <addr> <handler>'
    if (argc < 2 || argc > 3) {
	printf("usage: mousemode <addr> [<handler>]\n");
	printf(" Configures mouse at ADB address <addr> to use <handler>\n");
	printf(" without <handler>, print value of current handler\n");
	exit(EXIT_FAILURE);
    }
    
    fd = open("/dev/adb", O_RDWR);
    if (fd <= 0) {
	perror("opening /dev/adb");
	exit(EXIT_FAILURE);
    }
    
    addr = atoi(argv[1]);
    if (argc == 3) {
	mode = atoi(argv[2]);
	printf("handler for addr %d was %d\n", addr, showmouse(addr));
	printf("trying to set handler to %d...\n", mode);
	setmouse(addr, mode);
	printf("handler is now %d\n", showmouse(addr));
    } else {
	printf("handler for addr %d is %d\n", addr, showmouse(addr));
    }
    
    close(fd);
    return 0;
}
