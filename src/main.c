#include <stdio.h>
#include <stdlib.h>

#include "../include/netutils.h"
#include "../include/traceroute.h"
#include "../include/stats.h"
#include "../include/defs.h"

#define VERSION "1.0"

int main(int argc, char* argv[]) {
    printf("netperf v%s\n", VERSION);
    if (argc != 2) {
        printf("Usage: %s <ipaddr>\n", argv[0]);
        return 1;
    } 

    char* ip = resolve_address(argv[1]);
    struct traceroute_ll* ll = ll_init();
    printf("collecting statistics for connection to %s (%s)...\n", ip, argv[1]);

    int result = traceroute(ip, ll);

    if (result < 0) {
        printf("error: traceroute failed\n");
        return 1;
    }

    struct node* bottleneck = ll_get_bottleneck(ll);
    print_link_stats_table(ll);
    print_link_stats_prominent(bottleneck->data);
    free(ip);
    return 0;
}
