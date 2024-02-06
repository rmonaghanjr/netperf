#pragma once

void print_icmp_packet(struct icmp* header);
unsigned short packet_checksum(unsigned short* buffer, int num_words);
int traceroute(char* start, int hop_limit);
