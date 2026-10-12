#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "net/tun.h"

int create_tun(const char *requested_name, char *actual_name,
               size_t actual_name_len) {
    struct ifreq ifr;
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "net>open(/dev/net/tun) failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (requested_name && requested_name[0] != '\0') {
        strncpy(ifr.ifr_name, requested_name, IFNAMSIZ - 1);
    }

    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
        fprintf(stderr, "net>ioctl(TUNSETIFF) failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (actual_name && actual_name_len > 0) {
        actual_name[0] = '\0';
        strncpy(actual_name, ifr.ifr_name, actual_name_len - 1);
        actual_name[actual_name_len - 1] = '\0';
    }

    return fd;
}

