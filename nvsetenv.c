#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define NVSTART	0x1800
#define NVSIZE	0x800
#define MXSTRING        128
#define N_NVVARS	(int)(sizeof(nvvars) / sizeof(nvvars[0]))

static int nvfd, nvstr_used;
static char nvstrbuf[NVSIZE];

struct nvram {
    unsigned short magic;		/* 0x1275 */
    unsigned char  char2;
    unsigned char  char3;
    unsigned short cksum;
    unsigned short end_vals;
    unsigned short start_strs;
    unsigned short word5;
    unsigned long  bits;
    unsigned long  vals[1];
};

union {
    struct nvram nv;
    char c[NVSIZE];
    unsigned short s[NVSIZE/2];
} nvbuf;


enum nvtype {
    boolean,
    word,
    string
};

struct nvvar {
    const char *name;
    enum nvtype type;
} nvvars[] = {
    {"little-endian?", boolean},
    {"real-mode?", boolean},
    {"auto-boot?", boolean},
    {"diag-switch?", boolean},
    {"fcode-debug?", boolean},
    {"oem-banner?", boolean},
    {"oem-logo?", boolean},
    {"use-nvramrc?", boolean},
    {"real-base", word},
    {"real-size", word},
    {"virt-base", word},
    {"virt-size", word},
    {"load-base", word},
    {"pci-probe-list", word},
    {"screen-#columns", word},
    {"screen-#rows", word},
    {"selftest-#megs", word},
    {"boot-device", string},
    {"boot-file", string},
    {"diag-device", string},
    {"diag-file", string},
    {"input-device", string},
    {"output-device", string},
    {"oem-banner", string},
    {"oem-logo", string},
    {"nvramrc", string},
    {"boot-command", string},
};

union nvval {
    unsigned long word_val;
    char *str_val;
} nvvals[32];

int
nvcsum( void )
{
    int i;
    unsigned c;
    
    c = 0;
    for (i = 0; i < NVSIZE/2; ++i)
	c += nvbuf.s[i];
    c = (c & 0xffff) + (c >> 16);
    c += (c >> 16);
    return c & 0xffff;
}

void
nvload( void )
{
    int s;

    if (lseek(nvfd, NVSTART, 0) < 0
	|| read(nvfd, &nvbuf, NVSIZE) != NVSIZE) {
	perror("Error reading /dev/nvram");
	exit(EXIT_FAILURE);
    }
    s = nvcsum();
    if (s != 0xffff)
       (void) fprintf(stderr, "Warning: checksum error (%x) on nvram\n", 
		      s ^ 0xffff);
}

void
nvstore(void)
{
    if (lseek(nvfd, NVSTART, 0) < 0
	|| write(nvfd, &nvbuf, NVSIZE) != NVSIZE) {
	perror("Error writing /dev/nvram");
	exit(EXIT_FAILURE);
    }
}

void nvunpack( void )
{
    int i;
    unsigned long bmask;
    int vi, off, len;
    
    nvstr_used = 0;	
    bmask = 0x80000000;
    vi = 0;
    for (i = 0; i < N_NVVARS; ++i) {
	switch (nvvars[i].type) {
	case boolean:
	    nvvals[i].word_val = (nvbuf.nv.bits & bmask) ? 1 : 0;
	    bmask >>= 1;
	    break;
	case word:
	    nvvals[i].word_val = nvbuf.nv.vals[vi++];
	    break;
 	case string:
	    off = nvbuf.nv.vals[vi] >> 16;
	    len = nvbuf.nv.vals[vi++] & 0xffff;
	    nvvals[i].str_val = nvstrbuf + nvstr_used;
	    memcpy(nvvals[i].str_val, nvbuf.c + off - NVSTART, (size_t) len);
	    nvvals[i].str_val[len] = (char) 0;
	    nvstr_used += len + 1;
	    break;
	}
    }
}

void
nvpack( void )
{
    int     i, vi;
    size_t  off, len;
    unsigned long bmask;
	
    bmask = 0x80000000;
    vi = 0;
    off = NVSIZE;
    nvbuf.nv.bits = 0;
    for (i = 0; i < N_NVVARS; ++i) {
	switch (nvvars[i].type) {
	case boolean:
	    if (nvvals[i].word_val != 0)
		nvbuf.nv.bits |= bmask;
	    bmask >>= 1;
	    break;
	case word:
	    nvbuf.nv.vals[vi++] = nvvals[i].word_val;
	    break;
	case string:
	    len = strlen(nvvals[i].str_val);
	    off -= len;
	    memcpy(nvbuf.c + off, nvvals[i].str_val, len);
	    nvbuf.nv.vals[vi++] = ((off + NVSTART) << 16) + len;
	    break;
	}
    }
    nvbuf.nv.magic = 0x1275;
    nvbuf.nv.cksum = 0;
    nvbuf.nv.end_vals = NVSTART + (unsigned) &nvbuf.nv.vals[vi]
	- (unsigned) &nvbuf;
    nvbuf.nv.start_strs = (unsigned short int) (off + NVSTART);
    memset(&nvbuf.c[nvbuf.nv.end_vals - NVSTART], 0,
	   (size_t) (nvbuf.nv.start_strs - nvbuf.nv.end_vals) );
    nvbuf.nv.cksum = (unsigned short int) (~nvcsum());
}

void
print_var(int i, int indent)
{
    char *p;

    switch (nvvars[i].type) {
    case boolean:
	(void) printf("%s", nvvals[i].word_val != 0 ? "true": "false");
	break;
    case word:
	(void) printf("0x%lx", nvvals[i].word_val);
	break;
    case string:
	for (p = nvvals[i].str_val; *p != (char) 0; ++p)
	    if (*p != (char) '\r')
		(void) putchar(*p);
	    else
		(void) printf("\n%*s", indent, "");
	break;
    }
    (void) printf("\n");
}

void
parse_val(int i, char *str)
{
    char *endp;

    switch (nvvars[i].type) {
    case boolean:
	if (strcmp(str, "true") == 0)
	    nvvals[i].word_val = 1;
	else if (strcmp(str, "false") == 0)
	    nvvals[i].word_val = 0;
	else {
	    (void) fprintf(stderr, "bad boolean value '%s' for %s\n", str,
			   nvvars[i].name);
	    exit(EXIT_FAILURE);
	}
	break;
    case word:
	nvvals[i].word_val = strtoul(str, &endp, 16);
	if (str == endp) {
	    (void) fprintf(stderr, "bad hexadecimal value '%s' for %s\n", str,
			   nvvars[i].name);
	    exit(EXIT_FAILURE);
	}
	break;
    case string:
	nvvals[i].str_val = str;
	break;
    }
}

int 
main(int ac, char **av)
{
    int i = 0, l, print;

    l = (int) strlen(av[0]);
    print = (int) (ac <= 2 || 
		   (l > 8 && strcmp(&av[0][l-8], "printenv") == 0));
    if (print != 0 && ac > 2) {
	(void) fprintf(stderr, "Usage: %s [variable]\n", av[0]);
	exit(EXIT_FAILURE);
    }
    if (ac > 3) {
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

    nvfd = open("/dev/nvram", ac <= 2 ? O_RDONLY: O_RDWR);
    if (nvfd < 0) {
	perror("Couldn't open /dev/nvram");
	exit(EXIT_FAILURE);
    }
    nvload();
    nvunpack();

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
	parse_val(i, av[2]);
	nvpack();
	nvstore();
	break;
    }

    (void) close(nvfd);
    exit(EXIT_SUCCESS);
}
