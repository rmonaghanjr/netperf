#include <stdint.h>

#include "defs.h"

struct link_stats {
    int hop;
    char* ip;
    uint8_t timed_out;
    uint8_t is_dest;
    float elapsed_ms[TESTS];
};

struct node {
    struct link_stats* data;
    struct node* next;
};

struct traceroute_ll {
    struct node *head;
    int size;
};

struct traceroute_ll* ll_init();
void ll_append(struct traceroute_ll *ll, struct link_stats *data);
void ll_destroy(struct traceroute_ll *ll);
struct node* ll_get_bottleneck(struct traceroute_ll *ll);
