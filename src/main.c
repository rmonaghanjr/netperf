#include <stdio.h>
#include <stdlib.h>

#include "../include/netutils.h"
#include "../include/traceroute.h"

#define VERSION "1.0"

int main(int argc, char* argv[]) {
    printf("netperf v%s\n", VERSION);
    if (argc != 2) {
        printf("Usage: %s <ipaddr>\n", argv[0]);
        return 1;
    } 

    char* ip = resolve_address(argv[1]);
    printf("collecting statistics for connection to %s (%s)...\n", ip, argv[1]);

    int result = traceroute(ip, 64);

    free(ip);
    return 0;
}
