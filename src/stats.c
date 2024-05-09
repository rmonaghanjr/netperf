#include <math.h>
#include <string.h>
#include <stdio.h>

#include "../include/stats.h"
#include "../include/defs.h"

void print_link_stats_table(struct traceroute_ll *ll) {
    int max_ip_strlen = 16;
    printf("|-----|-----------------|----------|-------------|----------|----------|\n");
    printf("| Hop | Link            | Avg (ms) | Stddev (ms) | Max (ms) | Min (ms) |\n");
    printf("|-----|-----------------|----------|-------------|----------|----------|\n");
    struct node* current = (*ll).head;
    while (current != NULL) {
        if ((*current).data->timed_out) {
            printf("|  %02d | ICMP_ECHO might be blocked on this router...TIMED OUT (5s)     |\n", (*current).data->hop);
            printf("|-----|-----------------|----------|-------------|----------|----------|\n");
            current = current->next;
            continue;
        }
        print_link_stats_row((*current).data);
        current = current->next;
    }
}

void print_link_stats_row(struct link_stats *ls) {
    int in_strlen = strlen(ls->ip);
    int max_ip_strlen = 15;
    char ipbuf[16] = {0};

    if (in_strlen < max_ip_strlen) {
        // right pad with spaces 
        strcpy(ipbuf, ls->ip); 
        for (int i = in_strlen; i < max_ip_strlen; i++) {
            ipbuf[i] = ' ';
        }
    } else {
        strcpy(ipbuf, ls->ip);
    }

    printf("| %c%02d | %s | %05.2f    | %05.2f       | %05.2f    | %05.2f    |\n", ls->is_dest ? '*' : ' ', ls->hop,  ipbuf, get_link_avg(ls), get_link_stddev(ls), get_link_max(ls), get_link_min(ls));
    printf("|-----|-----------------|----------|-------------|----------|----------|\n");
}

void print_link_stats_prominent(struct link_stats *ls) {
    int in_strlen = strlen(ls->ip);
    int max_ip_strlen = 15;
    char ipbuf[16] = {0};

    if (in_strlen < max_ip_strlen) {
        // right pad with spaces 
        strcpy(ipbuf, ls->ip); 
        for (int i = in_strlen; i < max_ip_strlen; i++) {
            ipbuf[i] = ' ';
        }
    } else {
        strcpy(ipbuf, ls->ip);
    }

    printf("\nBottleneck Link\n");
    printf("|-------------|-----------------|\n");
    printf("|        Link | %s |\n", ipbuf);
    printf("|-------------|-----------------|\n");
    printf("|    Avg (ms) | %05.2f           |\n", get_link_avg(ls));
    printf("|-------------|-----------------|\n");
    printf("| Stddev (ms) | %05.2f           |\n", get_link_stddev(ls));
    printf("|-------------|-----------------|\n");
    printf("|    Max (ms) | %05.2f           |\n", get_link_max(ls));
    printf("|-------------|-----------------|\n");
    printf("|    Min (ms) | %05.2f           |\n", get_link_min(ls));
    printf("|-------------|-----------------|\n");
}

float get_link_avg(struct link_stats* ls) {
    float sum = 0;
    for (int i = 0; i < TESTS; i++) {
        sum += ls->elapsed_ms[i];
    }
    return sum / TESTS;
}
    
float get_link_stddev(struct link_stats* ls) {
    if (ls->timed_out) return -1;
    float avg = get_link_avg(ls);
    float sum = 0;
    for (int i = 0; i < TESTS; i++) {
        sum += (ls->elapsed_ms[i] - avg) * (ls->elapsed_ms[i] - avg);
    }
    return sqrt(sum / TESTS);
}

float get_link_max(struct link_stats* ls) {
    if (ls->timed_out) return -1;
    float max_elapsed = 0;
    for (int i = 0; i < TESTS; i++) {
        if (ls->elapsed_ms[i] > max_elapsed) max_elapsed = ls->elapsed_ms[i];
    }
    return max_elapsed;
}

float get_link_min(struct link_stats* ls) {
    if (ls->timed_out) return -1;

    float min_elapsed = MAXFLOAT;
    for (int i = 0; i < TESTS; i++) {
        if (ls->elapsed_ms[i] < min_elapsed) min_elapsed = ls->elapsed_ms[i];
    }
    return min_elapsed;
}

