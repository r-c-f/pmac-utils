/*	nwnvsetnv.c

	used to set the Envenrioment variables in power macs NVram.
	This is for the NewWorld nvram only

	nwcode: Copyright (C) 2000	by Klaus Halfmann

	see README for details
*/
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// #define MXSTRING        128:
// #define N_NVVARS	(int)(sizeof(nvvars) / sizeof(nvvars[0]))
// #define NVMAGIC		0x1275

/* CHRP NVRAM header */
typedef struct {
  unsigned char		signature;
  unsigned char		cksum;
  unsigned short	len;
  char			name[12];
  // unsigned char data[0];
} chrp_header;

/* Check in proc/cpuinfo if this is a new world machine */

int checkNewWorld(void)
{
    FILE* cpuf = fopen("/proc/cpuinfo","r");
    char  buf[256]; // "pmac-generation    : NewWolrd|OldWorld 
    int	  result = 0, found = 0;

    if (!cpuf) {
	perror("Couldn't open /proc/cpuinfo");
	exit(EXIT_FAILURE);
    }

    while (NULL != fgets(buf, 255, cpuf))
    {
	if (!strncmp(buf, "pmac-generation" ,15))
	{
	    char* index = strchr(buf, ':') + 2;
	    if (!index) // ???
		continue;
	    result = !strncmp(index,"NewWorld",8);
	    found = 1;
	    break;
	}
    }
    fclose(cpuf);

    /* Some kernels don't have pmac-generation ( <= 2.2.15 and earlier 2.3.x ) */
    /* Look for AAPL in /proc/device-tree/compatible instead. */
    if (!found) {
	cpuf = fopen("/proc/device-tree/compatible", "r");
	if(!cpuf)
	    return 0;
	
	fgets(buf, 255, cpuf);
	fclose(cpuf);
	if (strncmp(buf, "AAPL", 4) != 0)
	    result = 1;
    }

    return result; // assume OldWorld
}

/* seek NVRAM until common (OF) part 
   return the lenght of the part in case of sucess,
   	  0 otherwise.
   chrph is set to the value of the "common" block eventually found
   *nvstart is set to the seekpoint where common block starts.
*/
   
int nvscan(int nvfd, chrp_header* chrph, int* nvstart)
{
    int		start = 0;
    
    while (read(nvfd, chrph, sizeof(chrp_header)) == sizeof(chrp_header))
    {
	int size = chrph->len * 0x10 - sizeof(chrp_header);    	
	if (!strncmp(chrph->name, "common", 7))
	{
	    *nvstart = start;
	    return size; // seeked upto start
	}
	if (lseek(nvfd, size, SEEK_CUR) < 0)
	   break;
	start += size + sizeof(chrp_header);
    }
    fprintf(stderr,"No common Block found\n");
    exit(EXIT_FAILURE);
}

static char* nvload( int nvfd , int nvsize)
{
    char* nvbuf = malloc(nvsize);
    char *b = nvbuf;
    int i;

    if (!nvbuf) {
	perror("Error allocating buffer");
	exit(EXIT_FAILURE);
    }
    while ((i = read(nvfd, b, nvsize)) > 0) {
       b += i;
       nvsize -= i;
    }
    if (i == -1) {
	perror("Error reading /dev/nvram");
	exit(EXIT_FAILURE);
    }
    return nvbuf;
}

static void
print_vars(char* nvbuf, int nvsize)
{
    int i = 0;

    while (i < nvsize)
    {
 	int size = strnlen(nvbuf, nvsize);
	if (size == 0)
	    break;
	printf("%s\n",nvbuf);
	nvbuf += (size + 1);	// count 0-byte, too
    }
}

/* move memory around to insert the value.
 *
 * @param nvbufend 	byte AFTER the end of the buffer
 * @param varsize 	length of the variable name
 * @param buf		byte where varaible NAME starts
 * @param newval	new value to replace old one
 * @param foundsize 	lenght of varible + '=' + value
 * @param equalpos 	position relative to buf where '=' was found
 */
static void 
insert_val (char* nvbufend, int varsize,   char* buf, 
	    char* newval,   int foundsize, int equalpos)
{
    int 	oldlen 	= foundsize - equalpos -1; // account for the '='
    int		newlen 	= strlen(newval);
    char* 	valpos 	= buf + varsize + 1;
    int 	delta 	= newlen - oldlen;

    if (delta > 0) // expand mem
	memmove(valpos + newlen,
		valpos + oldlen, (nvbufend - valpos - newlen));
    else if (delta < 0) // shrink mem
    {
	memmove(valpos + newlen, 
		valpos + oldlen, (nvbufend - valpos - oldlen));
	memset (nvbufend + delta, 0, -delta );
    }
    strncpy(valpos, newval, newlen);
}

/* return position where variable is found,
 * newval may be null.
 */

static char* set_var(char* nvbuf, int nvsize, char* varname, char* newval)
{
    int i =0;
    int varsize = strlen(varname);
    int equalpos;
    while (i < nvsize)
    {
	char* buf = &nvbuf[i];
 	int foundsize = strnlen(buf, nvsize -i);
	if (foundsize == 0)
	    break;
	equalpos = (int) (strchr(buf, '=') - buf);
	if (equalpos == varsize &&
	    !strncmp(buf, varname, equalpos))
	{
	    if (newval)	// set the value 
		insert_val(nvbuf + nvsize, varsize, buf, 
			   newval, foundsize, equalpos);
	    return buf;
	}
	i += foundsize + 1;	// count 0-byte, too
    }
    return NULL;
}

static void
print_var(char* nvbuf, int nvsize, char* varname)
{
    char* buf = set_var(nvbuf, nvsize, varname, NULL);
    if (buf)
	printf("%s\n",buf);
}

/* This fucntion is not used here, it is left
   her for the curious */

unsigned short chrp_checksum(chrp_header* hdr, char* nvbuf, int nvsize)
{
    unsigned char* 	ptr = (unsigned char*) &hdr->len;
    unsigned char* 	end = ptr + sizeof(chrp_header);
    unsigned short 	sum = hdr->signature;
	// this in fact skips the checksum
    for (; ptr < end; ptr++)
	sum += *ptr;
    while (sum > 0xFF)
	sum = (sum & 0xFF) + (sum>>8);
    return sum;
}                                                                                                 

static void
nvstore(int nvfd, chrp_header* chrph, char* nvbuf, int nvstart, int nvsize)
{
    // mmh, the checksum is calculated for the header only
    // since we did not modify the header we can just ignore it.
    ssize_t written;
    ssize_t seek =  nvstart + sizeof(chrp_header);
    written = lseek(nvfd, seek, SEEK_SET);
    if (written != seek)
    {
    	fprintf(stderr,"Error seeking /dev/nvram\n");
	exit(EXIT_FAILURE);
    }

    while ((written = write(nvfd, nvbuf, nvsize)) > 0)
    {
      nvsize -= written;
      nvbuf  += written;
    }
    if (written == -1)
    {
    	fprintf(stderr,"Error writing /dev/nvram %x %x %x\n", nvsize, seek, written);
	exit(EXIT_FAILURE);
    }
}

/* print / set the New World NVRAM */

void nvNew(int ac, char** av, int nvfd)
{
    int 		nvsize, nvstart;
    chrp_header		chrph;
    char*		nvbuf;

    nvsize = nvscan(nvfd, &chrph, &nvstart);
    nvbuf  = nvload(nvfd, nvsize);

    switch (ac) {
    case 1:
	print_vars(nvbuf, nvsize);
	break;

    case 2:
	print_var(nvbuf, nvsize, av[1]);
	break;
	
    case 3:
	set_var(nvbuf, nvsize, av[1], av[2]);
	nvstore(nvfd, &chrph, nvbuf, nvstart, nvsize);
	break;
    }
}
