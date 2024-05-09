#include <stdlib.h>

#include "../include/stats.h"

struct traceroute_ll* ll_init() {
    struct traceroute_ll* ll = (struct traceroute_ll*) malloc(sizeof(struct traceroute_ll));
    (*ll).size = 0;
    (*ll).head = NULL;

    return ll;
}

void ll_append(struct traceroute_ll *ll, struct link_stats *data) {
    struct node* new_node = (struct node*) malloc(sizeof(struct node));
    (*new_node).data = data;
    (*new_node).next = NULL;

    (*ll).size++;
    if ((*ll).head == NULL) {
        (*ll).head = new_node;
        return;
    }
    struct node* current = (*ll).head;
    while ((*current).next != NULL) {
        current = (*current).next;
    }
    (*current).next = new_node;
}

void ll_destroy(struct traceroute_ll *ll) {
    struct node* current = (*ll).head;
    struct node* next = NULL;
    while (current != NULL) {
        next = (*current).next;
        free((*current).data);
        free(current);
        current = next;
    }
    free(ll);
}

struct node* ll_get_bottleneck(struct traceroute_ll *ll) {
    struct node* current = (*ll).head;
    struct node* bottleneck = current;
    while (current != NULL) {
        float avg = get_link_avg((*current).data);
        float curr_avg = get_link_avg((*bottleneck).data);
        if (avg > curr_avg) {
            bottleneck = current;
        }
        current = (*current).next;
    }
    return bottleneck;
}


