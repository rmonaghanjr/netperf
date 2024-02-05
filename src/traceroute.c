#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "../include/traceroute.h"

#define BUFFER_SIZE 4096

unsigned short packet_checksum(unsigned short* buffer, int num_words) {
    unsigned long sum;
    for (sum = 0; num_words > 0; num_words--) {
        sum += *buffer++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

int traceroute(char* destination, int hop_limit) {
    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    char buffer[4096];
    struct ip* ip_header = (struct ip*) buffer;
    int curr_hop = 0;

    int one = 1;
    int* val = &one;
    if (setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt");
        return 1;
    }    

    struct sockaddr_in addr;
    addr.sin_port = htons(7);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, destination, &(addr.sin_addr));

    while (1) {
        ip_header->ip_hl = 5;
        ip_header->ip_v = 4;
        ip_header->ip_tos = 0;
        ip_header->ip_len = 20 + 8;
        ip_header->ip_id = 10000;
        ip_header->ip_off = 0;
        ip_header->ip_ttl = curr_hop;
        ip_header->ip_p = IPPROTO_ICMP;

        inet_pton(AF_INET, "192.168.1.168", &(ip_header->ip_src));
        inet_pton(AF_INET, destination, &(ip_header->ip_dst));
        ip_header->ip_sum = packet_checksum((unsigned short *) buffer, 9);

        struct ih_idseq seq;
        seq.icd_id = 0;
        seq.icd_seq = curr_hop + 1;

        struct icmp* icmp_header = (struct icmp*) (buffer + 20);
        icmp_header->icmp_type = ICMP_ECHO;
        icmp_header->icmp_code = 0;
        icmp_header->icmp_cksum = 0;
        icmp_header->icmp_hun.ih_idseq = seq;
        icmp_header->icmp_cksum = packet_checksum((unsigned short *) (buffer + 20), 4);
        sendto(sfd, buffer, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr*) & addr, sizeof(addr));


    }

    return 0;
}

