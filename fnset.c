/*
 * fnset.c
 * Copyright 2001 Jimi Xenidis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Wouldn't have been able to figure out how to do this without trackpad.c
 * thanks to benh
 *
 * MSch 2004/02/12: added fallback device type (handler 1) for Lombard
*/


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/adb.h>
#include <linux/pmu.h>


#define PMU_GET_VERSION		0xea	/* get pmu firmware version # */
#define PMU_VERSION_LOMBARD	11
#define PMU_VERSION_KEYLARGO	12	/* another guess */
#define PMU_VERSION_IBOOK	PMU_VERSION_KEYLARGO

const char *prog;

int debug = 0;

void
usage(void)
{
    fprintf(stderr,
	    "Usage: %s [-hdfusb] [0|1]\n"
	    "  -h    Help, this message.\n"
	    "  -d    Turn debugging on.\n"
	    "  -f    Force search and setting of bits for an unknown PMU.\n"
	    "          WARNING: may damage hardware.\n"
	    "  -s    Set, function keys require <fn> modifier.\n"
	    "          Same as checking the box in MacOS.\n"
	    "  -u    Unset, hot keys require <fn> modifier.\n"
	    "          Same as unchecking the box in MacOS.\n"
	    "  -v    Verbose output.\n"
	    "  -b    Brief, output adb register bit value only.\n"
	    "  0     Set adb register bit to 0.\n"
	    "          May not be the same as the -u option.\n"
	    "  1     Set adb register bit to 1.\n"
	    "          May not be the same as the -s option.\n"
	    "No arguments displays the <fn> mode as if -v was used.\n",
	    prog);
}

int
put(int fd, unsigned char data[], size_t sz)
{
    int n;

    if (debug) {
	int i;
	fprintf(stderr, "writing %ld bytes: {", (long)sz);
	for (i = 0; i < sz; i++) {
	    fprintf(stderr, " 0x%02x", data[i]);
	}
	fputs(" }\n", stderr);
    }

    n = write(fd, data, sz);
    if (n != sz) {
	if (n == -1) {
	    fprintf(stderr, "%s: write(): %s\n",
		    prog, strerror(errno));
	} else {
	    fprintf(stderr, "%s: write(): expected %ld got %ld\n",
		    prog, (long)sz, (long)n);
	}
	return -1;
    }
    return n;
}

int
get(int fd, unsigned char data[], size_t sz)
{
    int n;

    n = read(fd, data, sz);
    if (n == -1) {
	fprintf(stderr, "%s: read(): %s\n",
		prog, strerror(errno));
	return -1;
    }

    if (debug) {
	int i;
	fprintf(stderr, "read %d bytes", n);
	if (n > 0) {
	    fputs(": {", stderr);
	    for (i = 0; i < n; i++) {
		fprintf(stderr, " 0x%02x", data[i]);
	    }
	    fputs(" }", stderr);
	}
	fputc('\n', stderr);
    }

    return n;
}

int
pmu_version(int fd)
{
    unsigned char data[32];
    int n;

    data[0] = PMU_PACKET;
    data[1] = PMU_GET_VERSION;

    n = put(fd, data, 2);
    if (n == -1) {
	return -1;
    }

    n = get(fd, data, sizeof (data));
    if (n == -1) {
	return -1;
    }
    if (n != 2) {
	fprintf(stderr, "%s: read(): expected 2 got %d\n",
		prog, n);
	return -1;
    }
    return data[1];
}

int
show_regs(int fd, int id)
{
    unsigned char data[16];
    int i;

    fprintf(stderr, "=== reading all registers for device %d. ===\n", id);

    for (i = 0; i < 4; i++) {
	data[0] = ADB_PACKET;
	data[1] = ADB_READREG(id, i);
	put(fd, data, 2);
	get(fd, data, sizeof(data));
    }

    fprintf(stderr, "=== done reading all registers for device %d. ===\n", id);

    return 1;
}


int
find_device(int fd, int type, int bitpos, int *bit)
{
    int i;
    int n;
    int ret = -1;
    unsigned char data[16];

    for (i = 1; i < 16; i++) {
	if (debug) {
	    fprintf(stderr, "probe adb device %d.\n", i);
	}
	data[0] = ADB_PACKET;
	data[1] = ADB_READREG(i, 1);
	n = put(fd, data, 2);
	if (n == -1) {
	    return -1;
	}

	n = get(fd, data, sizeof (data));
	if (n == -1) {
	    return -1;
	}

	if ((n > 0) && (data[1] == type)) {
	    if (ret > 0) {
		fprintf(stderr, "Ignoring other keyboard at %d\n", i);
	    } else {
		if (debug) {
		    fprintf(stderr, "Found keyboard at ADB id %d.\n", i);
		}
		ret = i;
		*bit = (data[2] & (1<<bitpos)) >> bitpos;
		if (debug) {
		    show_regs(fd, ret);
		}
	    }
	}
    }
    return ret;
}

int
fnset(int fd, int id, int bitpos, int bit)
{
    unsigned char data[4];
    int n;

    /* get current state */
    data[0] = ADB_PACKET;
    data[1] = ADB_READREG(id, 1);

    n = put(fd, data, 2);
    if (n == -1) {
	return 0;
    }

    n = get(fd, &data[1], sizeof (data) - 1);
    if (n == -1) {
	return 0;
    }

    /* write new state */
    data[0] = ADB_PACKET;
    data[1] = ADB_WRITEREG(id, 1);
    /* keep data[2] the same */
    data[3] = (data[3] & (~(1<<bitpos))) | (bit<<bitpos);

    n = put(fd, data, 1 + n);
    if (n == -1) {
	return 0;
    }
    n = get(fd, data, sizeof (data));
    if (n == -1) {
	return 0;
    }

    /* read in new state */
    data[0] = ADB_PACKET;
    data[1] = ADB_READREG(id, 1);
    n = put(fd, data, 2);
    if (n == -1) {
	return 0;
    }

    n = get(fd, data, sizeof (data));
    if (n == -1) {
	return 0;
    }

    if ((data[2] & (1<<bitpos)) != (bit<<bitpos)) {
	fprintf(stderr, "bit 0x%x did not get written.\n", (bit<<bitpos));
	return 0;
    }
    return 1;
}

int
main(int argc, char *argv[], char *envp[])
{
    const char *adb = "/dev/adb";
    const char *optlet = "dfvhsubvP:";

    extern char *optarg;
    extern int optind;
    int c;
    unsigned int bitpos = 0;
    int bit = -1;
    int force = 0;
    int val;
    int pmu;
    int fd;
    int id;
    int type, deftype = -1;
    int brief = 0;
    int verbose = 0;

    prog = argv[0];

    if (argc == 1) {
	/* if no arguments then be verbose */
	verbose = 1;
    } else {
	while((c = getopt(argc, argv, optlet)) != EOF) {
	    switch (c) {
	    case 'P':
		bitpos = strtol(optarg, (char **)NULL, 0);
		if (bitpos > 7) {
		    fprintf(stderr, "%s: bit position must be 0-8.\n", prog);
		    return 1;
		}
		break;
	    case 'd':
		debug = 1;
		break;
	    case 'f':
		force = 1;
		break;
	    case 'h':
		usage();
		return 0;
		break;
	    case 's':
		if (bit != -1) {
		    fprintf(stderr, "%s: conflicting arguments\n", prog);
		    usage();
		    return 1;
		}
		bit = 0;
		break;
	    case 'u':
		if (bit != -1) {
		    fprintf(stderr, "%s: conflicting arguments\n", prog);
		    usage();
		    return 1;
		}
		bit = 1;
		break;
	    case 'b':
		if (verbose) {
		    fprintf(stderr, "%s: conflicting arguments\n", prog);
		    usage();
		    return 1;
		}
		brief = 1 ;
		break;
	    case 'v':
		if (brief) {
		    fprintf(stderr, "%s: conflicting arguments\n", prog);
		    usage();
		    return 1;
		}
		verbose = 1 ;
		break;
	    case '?':
	    default:
		usage();
		return 1;
		break;
	    }
	}

	if (optind < argc) {
	    if (bit != -1) {
		fprintf(stderr, "%s: conflicting arguments\n", prog);
		usage();
		return 1;
	    }
	    if (argv[optind][0] == '1' && argv[optind][1] == '\0') {
		bit = 1;
	    } else if (argv[optind][0] == '0' && argv[optind][1] == '\0') {
		bit = 0;
	    }
	    if (debug) {
		fprintf(stderr, "set bit to: optarg: %u\n", bit);
	    }
	    ++optind;
	}

	if (optind != argc) {
	    fprintf(stderr,"%s: too many arguments\n", prog);
	    return 1;
	}
    }

    if (bitpos && !force) {
	fprintf(stderr,
		"%s: WARNING: use of bit offset other than 0 (-P option)"
		"requires force (-f option) since it may damage hardware!\n",
		prog);
    }

    fd = open(adb, O_RDWR);
    if (fd < 0) {
	fprintf(stderr, "%s: open(%s): %s\n",
		prog, adb, strerror(errno));
	return 1;
    }

    pmu = pmu_version(fd);
    switch (pmu) {
    case PMU_VERSION_IBOOK:
        type    = 0x04;
	deftype = 0x05; /* MSch: fallback needed for some iBooks */
	break;
    case PMU_VERSION_LOMBARD:
        type    = 0x05; /* btg: patched from 0x04 */
	deftype = 1;    /* MSch: fallback needed for my Lombard */
	break;
    default:
	fprintf(stderr, "unknown PMU version %d\n", pmu);
	if (force) {
	    type = 0x04;
	    fprintf(stderr, "forcing look up of type 0x%02x.\n", type);
	} else {
	    close(fd);
	    return 1;
	}
	break;
    case -1:
	close(fd);
	return 1;
	break;
    }

    id = find_device(fd, type, bitpos, &val);

    if (id == -1 && deftype != -1) {
	if (debug)
		fprintf(stderr, "Default device not found, falling back to type %x device ...\n", deftype);
    	id = find_device(fd, deftype, bitpos, &val);
    }

    if (id == -1) {
        fprintf(stderr, "Failed to find device\n");
	close(fd);
	return 1;
    }

    if (bit != -1) {
	if (bit != val) {
	    if (debug) {
		fprintf(stderr, "Changing value from %d to %d.\n", val, bit);
	    }
	    if (!fnset(fd, id, bitpos, bit)) {
		if (debug) {
		    fprintf(stderr, "Failed to set value to %d.\n", bit);
		}
		close(fd);
		return 1;
	    }
	    val = bit;
	} else {
	    if (debug) {
		fputs("Value already correct, nothing to do.\n", stderr);
	    }
	}
    }
    if (brief) {
	printf("%u\n", val);
    } else if (verbose) {
	if (val == 0) {
	    printf("Set: Function keys require <fn> modifier.\n");
	} else {
	    printf("Unset: Hot keys require <fn> modifier.\n");
	}
    }
    close(fd);
    return 0;
}
