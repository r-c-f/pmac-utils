#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define NVSTART	0x140f
#define NVSIZE	0x2

unsigned char nvbuf[NVSIZE];

int nvfd;

void
nvload( void )
{
    if (lseek(nvfd, NVSTART, 0) < 0
	|| read(nvfd, &nvbuf, NVSIZE) != NVSIZE) {
	perror("Error reading /dev/nvram");
	exit(EXIT_FAILURE);
    }
}

void
nvstore( void )
{
    if (lseek(nvfd, NVSTART, 0) < 0
	|| write(nvfd, &nvbuf, NVSIZE) != NVSIZE) {
	perror("Error writing /dev/nvram");
	exit(EXIT_FAILURE);
    }
}

struct nvvar {
    char *name;
} nvvars[] = {
    {"video-mode"},
    {"color-mode"},
};

#define N_NVVARS	(int) (sizeof(nvvars) / sizeof(nvvars[0]))

void
print_var(int i, /*@unused@*/ int indent)
{
    (void) printf("%d", (int) nvbuf[i]);
    (void) printf("\n");
}

int main(int ac, char **av)
{
    int i = 0;

    if (ac > 3 || 
        (ac == 2 && (strcmp(av[1], "-?" ) == 0 || strcmp( av[1], "-h" ) == 0 || strcmp( av[1] , "--help" ) == 0)) ) {
	(void) fprintf(stderr, "Usage: %s [variable [value]]\n", av[0]);
	exit(EXIT_FAILURE);
    }

    if (ac >= 2) {
	for (i = 0; i < N_NVVARS; ++i)
	    if (strcmp(av[1], nvvars[i].name) == 0)
		break;
	if (i >= N_NVVARS) {
	    (void) fprintf(stderr, "%s: no variable called '%s'\n", 
			   av[1], av[1]);
	    exit(EXIT_FAILURE);
	}
    }

    nvfd = open("/dev/nvram", ac <= 2? O_RDONLY: O_RDWR);
    if (nvfd < 0) {
	perror("Couldn't open /dev/nvram");
	exit(EXIT_FAILURE);
    }
    nvload();

    switch (ac) {
    case 1:
	for (i = 0; i < N_NVVARS; ++i) {
	    (void) printf("%-16s", nvvars[i].name);
	    print_var(i, 16);
	}
	break;

    case 2:
	print_var(i, 0);
	break;
	
    case 3:
        if (strcmp(av[1],nvvars[0].name)==0) 
	   nvbuf[0]=(unsigned char)strtoul(av[2], 0, 0);
        if (strcmp(av[1],nvvars[1].name)==0) 
	   nvbuf[1]=(unsigned char)strtoul(av[2], 0, 0);
        nvstore();
	break;
    }

    (void) close(nvfd);
    exit(EXIT_SUCCESS);
}
