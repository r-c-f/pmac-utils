/* set volume of the startup chime in the PowerMac nvram. 

   Written by Matteo Frigo <athena@fftw.org>.
   $Id: nvsetvol.c,v 1.1 2003/01/11 11:26:30 athena Exp $

   This program is in the public domain.
*/

/* The volume is stored as a byte at address 8 in the parameter ram. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/nvram.h>
#ifndef IOC_NVRAM_SYNC
#warning IOC_NVRAM_SYNC undefined -- update your headers
#define IOC_NVRAM_SYNC _IO('p', 0x43)
#endif

typedef struct {
     unsigned char sig;
     unsigned char cksum;
     unsigned short len;
     char name[12];
} header;

void die(const char *s) __attribute__((noreturn));
void die(const char *s)
{
     perror(s);
     exit(1);
}

void seek_pram(int fd, int offset)
{
     if (lseek(fd, offset, SEEK_SET) < 0)
	  die("lseek");
}

// MSch: only works for newworld - oldworld have XPRAM at offset 0x1300 fixed. 

int find_pram(int fd, int *size)
{
     header buf;
     int offset = 0;
     int rc = 0;
     while((rc = read(fd, &buf, sizeof(header))) == sizeof(header)) {
	  int sz = buf.len * 16;
	  // what's the sig in char?
	  // if (buf.sig == 0x1275) fprintf(stderr, "sig at offset %d rc %d buf.len %d buf.name %s \n", offset, rc, buf.len, buf.name);
	  // if (buf.sig == 0x5a) fprintf(stderr, "sig at offset %x rc %d buf.len %x buf.name %s \n", offset, rc, buf.len, buf.name);
	  // if (sz) fprintf(stderr, "offset %x rc %d sig %x buf.len %x buf.name %s \n", offset, rc, buf.sig, buf.len, buf.name);
	  if (!strncmp(buf.name, "APL,MacOS75", 11)) {
	       *size = sz - sizeof(header);
	       // fprintf(stderr, "XPRAM offset %x size %x\n", offset + sizeof(header), *size); 
	       return offset + sizeof(header);
	  }
	  if (sz)
	    offset += sz;
          else
            offset += sizeof(header);
	  seek_pram(fd, offset);
     }
     // no Core99 style PRAM found - just assume hard coded values as set in kernel:
     // XPRAM 0x1300
     // NR    0x1400
     // OF    0x1800
     // TODO: use ioctl to get PRAM offset
     // fprintf(stderr, "no NW PRAM found, assuming OW XPRAM offset 0x1300 size 0x100\n");
     *size = 0x100;
     return (0x1300);
     // NOTREACHED
     die("no PRAM found");
}

#define VOLADDR 8

int main(int argc, char *argv[])
{
     int fd, offset, size;
     char *buf;

     fd = open("/dev/nvram", O_RDWR);
     if (fd < 0) die("can't open /dev/nvram");
     
     offset = find_pram(fd, &size);
     buf = malloc(size);
     if (!buf) die("can't malloc()");

     seek_pram(fd, offset);
     if (read(fd, buf, size) != size)
	  die("error reading /dev/nvram");

     printf("current volume is %d\n", (unsigned char) buf[VOLADDR]);


#ifdef DEBUG_PRAM
     for (i=0; i < size; i++) {
          printf("cell %d : %d\n", i, (unsigned char) buf[i]);
     }
#endif
     
     if (argc > 1) {
	  buf[VOLADDR] = atoi(argv[1]);

	  seek_pram(fd, offset);
	  if (write(fd, buf, size) != size)
	       die("error writing /dev/nvram");
	  printf("new volume is %d\n", (unsigned char) buf[VOLADDR]);
     }
     ioctl(fd, IOC_NVRAM_SYNC);
     close(fd);
     return 0;
}
