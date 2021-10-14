#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "awacs_defs.h"

static inline void eieio( void )
{
    asm volatile("eieio" : :);
}

static inline unsigned ld_rev(volatile unsigned *addr)
{
    unsigned val;

    asm volatile("lwbrx %0,0,%1" : "=r" (val) : "r" (addr));
    return val;
}

static inline void st_rev(volatile unsigned *addr, unsigned val)
{
    asm volatile("stwbrx %0,0,%1" : : "r" (val), "r" (addr) : "memory");
}

unsigned *awacs;

static inline void busy_wait( void )
{
    eieio();
    while ((ld_rev(&awacs[4]) & 0x1000000) != 0)
        ;
}

int main(int ac, char **av)
{
    int fd, i;
    int l, r;
    int hflag = 0, aflag = 0, cflag = 1, mflag = 0, testflag = 0;
	int mux_mask = MASK_ADDR_MUX;
	
    while ((i = getopt(ac, av, "hacmt")) != -1)
    {
        switch (i)
        {
            case 'h':
                hflag = 1;
                break;
            case 'a':
                aflag = 1;
                mux_mask |= MASK_MUX_AUDIN;
                break;
            case 'c':
                cflag = 1;
                mux_mask |= MASK_MUX_CD;
                break;
            case 'm':
                mflag = 1;
                mux_mask |= MASK_MUX_MIC;
                break;
            case 't':
                aflag = cflag = mflag = 0;
                testflag = 1;
                break;
            default:
                (void) fprintf(stderr, "Usage: %s [-hacm] [vol | lvol rvol]\n", av[0]);
                (void) fprintf(stderr, "\t-h       mute headphones\n");
                (void) fprintf(stderr, "\t-a       activate Audio In playthrough\n");
                (void) fprintf(stderr, "\t-c       activate Cdrom playthrough\n");
                (void) fprintf(stderr, "\t-m       activate Microphone playthrough\n");
                (void) fprintf(stderr, "\tvol      Master Volume\n");
                (void) fprintf(stderr, "\tlvol     Left Master Volume\n");
                (void) fprintf(stderr, "\trvol     Right Master Volume\n");
                exit(EXIT_FAILURE);
        }
    }

    if ((fd = open("/dev/mem", O_RDWR)) < 0)
    {
        perror("/dev/mem");
        exit(EXIT_FAILURE);
    }

    awacs = (unsigned *)
        mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (int) AUD_CONT);

    if ((long)awacs == -1)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

	mux_mask |= GAINRIGHT(4) | GAINLEFT(4);
	
    st_rev(&awacs[4], mux_mask);
    busy_wait();
    st_rev(&awacs[4], mux_mask | MASK_GAINLINE);
    busy_wait();
    
    st_rev(&awacs[4], (hflag? MASK_HDMUTE: 0) | MASK_ADDR1 | MASK_LOOPTHRU | MASK_PAROUT);
    busy_wait();

    if (optind < ac)
    {
        l = r = atoi(av[optind]) & 0xf;
        
        if (optind + 1 < ac)
            r = atoi(av[optind+1]) & 0xf;
        
        if (l == 0 && r == 0) {
            st_rev(&awacs[4], MASK_ADDR1 | MASK_HDMUTE | MASK_SPKMUTE | MASK_LOOPTHRU | MASK_RECALIBRATE);                /* mute the output */
            busy_wait();
        }
        else
        {
		    st_rev(&awacs[4], MASK_ADDR_VOLSPK | VOLLEFT(l) | VOLRIGHT(r));
    		busy_wait();
    		
    		st_rev(&awacs[4], MASK_ADDR_VOLHD | VOLLEFT(l) | VOLRIGHT(r));
    		busy_wait();
        }
    }

    exit(EXIT_SUCCESS);
}
