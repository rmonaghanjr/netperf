#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "../include/netutils.h"
#include "../include/traceroute.h"

#define BUFFER_SIZE 4096
#define HOP_LIMIT 64

void print_icmp_packet(struct icmp* header) {
    printf("type:%d\n", header->icmp_type);
    printf("code:%d\n", header->icmp_code);
}

unsigned short packet_checksum(unsigned short* buffer, int num_words) {
    unsigned long sum;
    for (sum = 0; num_words > 0; num_words--) {
        sum += *buffer++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

int traceroute(char* destination) {
    int curr_hop = 0;
    int done, result;
    while (!done && curr_hop < HOP_LIMIT) {
        result = send_icmp_packet(destination, curr_hop, &done);
        if (result < 0) {
            perror("send_icmp_packet");
            return -1;
        }
        curr_hop++;
    }
    return 0;
}

int send_icmp_packet(char* destination, int curr_hop, int* done) {
    char buffer[BUFFER_SIZE] = {0};
    char buffnr[BUFFER_SIZE] = {0};
    char* source;
    int sfd, one, ready;
    fd_set rfs;
    socklen_t len = sizeof(struct sockaddr_in);
    struct ip* ip_header;
    struct sockaddr_in addr, addrn;
    struct ih_idseq seq;
    struct icmp* icmp_header;
    struct icmp* icmp_headern;
    struct timeval timeout;

    source = src_ipaddr(); 
    sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    ip_header = (struct ip*) buffer;
    one = 1;

    if (setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        return 1;
    }    

    addr.sin_port = htons(7);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, destination, &(addr.sin_addr));

    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = 28;
    ip_header->ip_id = 10000;
    ip_header->ip_off = 0;
    ip_header->ip_p = IPPROTO_ICMP;
    // this goes in send icmp_packet
    ip_header->ip_ttl = curr_hop;
    inet_pton(AF_INET, source, &(ip_header->ip_src));
    inet_pton(AF_INET, destination, &(ip_header->ip_dst));
    ip_header->ip_sum = packet_checksum((unsigned short *) buffer, 9);

    seq.icd_id = 0;
    seq.icd_seq = curr_hop + 1;

    icmp_header = (struct icmp*) (buffer + 20);
    icmp_header->icmp_type = ICMP_ECHO;
    icmp_header->icmp_code = 0;
    icmp_header->icmp_cksum = 0;
    icmp_header->icmp_hun.ih_idseq = seq;
    icmp_header->icmp_cksum = packet_checksum((unsigned short *) (buffer + 20), 4);

    FD_ZERO(&rfs);
    FD_SET(sfd, &rfs);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    sendto(sfd, buffer, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr*) & addr, sizeof(addr));

    ready = select(sfd + 1, &rfs, NULL, NULL, &timeout);
    if (ready < 0) {
        perror("ready");
        free(source);
        return -1;
    } else if (ready == 0) {
        printf("timed out at hop %d...\n", curr_hop);
    } else {
        recvfrom(sfd, buffnr, sizeof(buffnr), 0, (struct sockaddr*) & addrn, &len);
        
        icmp_headern = (struct icmp*) (buffer + 20);
        printf("at addr %s at hop %d...\n", inet_ntoa(addrn.sin_addr), curr_hop);
        if (strcmp(inet_ntoa(addrn.sin_addr), destination) == 0) {
            *done = 1;
            printf("DONE!\n");
            free(source);
            return 0;
        }
    }

    free(source);
    return 1;
}
