#pragma once

#include <stdint.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

void print_icmp_packet(struct icmp* header);
unsigned short packet_checksum(unsigned short* buffer, int num_words);
int traceroute(char* destination);
int send_icmp_packet(char* destination, int curr_hop, int* done);
