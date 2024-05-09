#pragma once

#include <stdint.h>
#include <sys/socket.h>

void print_icmp_packet(struct icmp* header);
unsigned short packet_checksum(unsigned short* buffer, int num_words);
int traceroute(char* destination, void* v_ll);
int send_icmp_packet(char* destination, int curr_hop, int* done, void* v_ll);
