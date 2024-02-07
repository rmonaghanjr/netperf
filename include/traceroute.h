#pragma once

#include <stdint.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

struct sender_args {
    char done;
    int sfd;
    struct sockaddr_in* sendaddr;
    struct sockaddr_in* recvaddr;
    socklen_t len;
    char* sendbuff;
    char recvbuff[BUFFER_SIZE];
};

void print_icmp_packet(struct icmp* header);
struct ip* build_ip_packet(int curr_hop, char* source, char* dest);
struct icmp* build_icmp_packet(int curr_hop);
void* wait_on_packet(void* arg);
int send_icmp_packet(int timeout, uint64_t* time_taken, struct sender_args* args);
unsigned short packet_checksum(unsigned short* buffer, int num_words);
int traceroute(char* start, int hop_limit);
