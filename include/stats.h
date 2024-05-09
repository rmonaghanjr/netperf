#include "ll.h"

void print_link_stats_table(struct traceroute_ll *ll);
void print_link_stats_row(struct link_stats *ls);
void print_link_stats_prominent(struct link_stats *ls);

float get_link_avg(struct link_stats* ls);
float get_link_stddev(struct link_stats* ls);
float get_link_max(struct link_stats* ls);
float get_link_min(struct link_stats* ls);
