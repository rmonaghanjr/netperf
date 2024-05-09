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
#include "../include/ll.h"
#include "../include/defs.h"

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

int traceroute(char* destination, void* v_ll) {
    struct traceroute_ll* ll = (struct traceroute_ll*) v_ll;
    int curr_hop = 0;
    int done = 0;
    int result = 0;

    while (!done && curr_hop < HOP_LIMIT) {
        result = send_icmp_packet(destination, curr_hop, &done, ll);
        if (result < 0) {
            perror("send_icmp_packet");
            return -1;
        }
        curr_hop++;
    }
    return 0;
}

int send_icmp_packet(char* destination, int curr_hop, int* done, void* v_ll) {
    struct traceroute_ll* ll = (struct traceroute_ll*) v_ll;
    char buffer[BUFFER_SIZE] = {0};
    char buffnr[BUFFER_SIZE] = {0};
    char* source;
    int sfd, one, ready;
    fd_set rfs;
    socklen_t len = sizeof(struct sockaddr_in);
    clock_t start, end;
    float elapsed_ms;
    float average = 0;
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

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    struct link_stats* stats = (struct link_stats*) malloc(sizeof(struct link_stats));
    memset(stats, 0, sizeof(struct link_stats));
    // link destination
    stats->hop = curr_hop;
    stats->timed_out = 0;

    for (int i = 0; i < TESTS; i++) {
        start = clock();
        sendto(sfd, buffer, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr*) & addr, sizeof(addr));

        ready = select(sfd + 1, &rfs, NULL, NULL, &timeout);
        if (ready < 0) {
            perror("ready");
            free(source);
            return -1;
        } else if (ready == 0) {
            printf("timed out at hop %d...\n", curr_hop);
            stats->timed_out = 1;
            ll_append(ll, stats);
            return 0;
        } else {
            recvfrom(sfd, buffnr, sizeof(buffnr), 0, (struct sockaddr*) & addrn, &len);
            char* ip = (char*) malloc(16);
            strncpy(ip, inet_ntoa(addrn.sin_addr), 16);
            stats->ip = ip;
            end = clock();
            elapsed_ms = (((double) (end - start)) / CLOCKS_PER_SEC) * 100000;

            stats->elapsed_ms[i] = elapsed_ms;
            average += elapsed_ms;
        }

        usleep(100000);
    }

    icmp_headern = (struct icmp*) (buffer + 20);
    printf("at addr %s at hop %d...(%4.2fms)\n", inet_ntoa(addrn.sin_addr), curr_hop, average / TESTS);

    ll_append(ll, stats);

    if (strcmp(inet_ntoa(addrn.sin_addr), destination) == 0) {
        *done = 1;
        stats->is_dest = 1;
        printf("reached destination! collecting statistics...\n");
        free(source);
        return 0;
    }

    free(source);
    return 1;
}
