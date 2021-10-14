#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/fd.h>

int main(int ac, char **av)
{
    char *dev;
    int fd;

    if (ac > 2) {
	(void) fprintf(stderr, "Usage: %s [device]\n", av[0]);
	exit(EXIT_FAILURE);
    }
    dev = ac > 1? av[1]: "/dev/fd0";
    fd = open(dev, O_RDONLY | O_NDELAY);
    if (fd < 0) {
	perror(dev);
	exit(EXIT_FAILURE);
    }
    if (ioctl(fd, FDEJECT, NULL) < 0) {
	perror("floppy eject ioctl");
	exit(EXIT_FAILURE);
    }
    (void) close(fd);
    exit(EXIT_SUCCESS);
}
