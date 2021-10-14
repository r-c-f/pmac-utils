#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/vt.h>
#include <asm/vc_ioctl.h>

int die(char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

int main(int ac, char **av)
{
    struct vc_mode vc;
    struct vt_consize vt;

    if (ioctl(1, VC_GETMODE, &vc) < 0)
	(void) die("VC_GETMODE");
    if (ac < 2) {
	(void) printf("%d %d\n", vc.mode, vc.depth);
	exit(EXIT_SUCCESS);
    }
    vc.mode = atoi(av[1]);
    if (ac > 2)
	vc.depth = atoi(av[2]);
    if (ioctl(1, VC_SETMODE, &vc) < 0)
	(void) die("VC_SETMODE");
    if (ioctl(1, VC_GETMODE, &vc) < 0)
	(void) die("VC_GETMODE");
    (void) memset(&vt, 0, sizeof(vt));
    vt.v_vlin = (unsigned short int) vc.height;
    vt.v_rows = (unsigned short int) (vc.height / 16);
    vt.v_vcol = (unsigned short int) vc.width;
    vt.v_cols = (unsigned short int) (vc.width / 8);
    if (ioctl(1, VT_RESIZEX, &vt) < 0)
	(void) die("VT_RESIZEX");
    (void) printf("\033[H\033[J");
    (void) fflush(stdout);
    exit(EXIT_SUCCESS);
}
