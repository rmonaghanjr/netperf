#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#include "../include/netutils.h"
#include "../include/traceroute.h"

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

struct ip* build_ip_packet(int curr_hop, char* source, char* dest) {
    struct ip* packet = (struct ip*) malloc(sizeof(struct ip));
    memset(packet, 0, sizeof(struct ip));
    packet->ip_hl = 5;
    packet->ip_v = 4;
    packet->ip_len = 32;
    packet->ip_id = 10000;
    packet->ip_ttl = curr_hop;
    packet->ip_p = IPPROTO_ICMP;
    inet_pton(AF_INET, source, &(packet->ip_src));
    inet_pton(AF_INET, dest, &(packet->ip_dst));
    packet->ip_sum = packet_checksum((unsigned short *) packet, 10);
    return packet;
}

struct icmp* build_icmp_packet(int curr_hop) {
    struct icmp* packet = (struct icmp*) malloc(sizeof(struct icmp));
    memset(packet, 0, sizeof(struct icmp));

    struct ih_idseq seq;
    seq.icd_id = 0;
    seq.icd_seq = curr_hop + 1;

    packet->icmp_type = ICMP_ECHO;
    packet->icmp_code = 0;
    packet->icmp_cksum = 0;
    memcpy(&(packet->icmp_hun.ih_idseq), &seq, sizeof(struct ih_idseq));
    packet->icmp_cksum = packet_checksum((unsigned short *) packet, 4);

    return packet;
}

void* wait_on_packet(void* arg) {
    struct sender_args* sargs = (struct sender_args*) arg;

    sendto(sargs->sfd, sargs->sendbuff, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr*) &sargs->sendaddr, sizeof(*sargs->sendaddr));
    recvfrom(sargs->sfd, sargs->recvbuff, sizeof(sargs->recvbuff), 0, (struct sockaddr*) &sargs->recvaddr, &sargs->len);
    sargs->done = 1;

    return NULL;
}

int send_icmp_packet(int timeout, uint64_t* time_taken, struct sender_args* args) {
    clock_t start, end;
    double cpu_time_used;
    pthread_t id;
    start = clock();
    pthread_create(&id, NULL, &wait_on_packet, args);
    while (!args->done) {
        printf("%d\n", args->done);
        // wait
        end = clock();
        if ((((double) (end - start)) / CLOCKS_PER_SEC) * 1000 > timeout) {
            pthread_kill(id, 1);
            break;
        }
    }

    pthread_join(id, NULL);
    return 0;
}

int traceroute(char* destination, int hop_limit) {
    char* source = src_ipaddr(); 
    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    char buffer[4096] = {0};
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

    while (curr_hop < hop_limit) {
        // construct packet
        struct ip* ip_pckt = build_ip_packet(curr_hop, source, destination);
        struct icmp* icmp_pckt = build_icmp_packet(curr_hop);
        memcpy(buffer, ip_pckt, sizeof(struct ip));
        memcpy(buffer + sizeof(struct ip), icmp_pckt, sizeof(struct icmp));
        free(ip_pckt);
        free(icmp_pckt);

        struct sockaddr_in recvaddr;
        struct sender_args args;
        args.sfd = sfd;
        args.sendaddr = &addr;
        args.recvaddr = &recvaddr;
        args.len = sizeof(struct sockaddr_in);
        args.sendbuff = (char*) &buffer;

        uint64_t time_taken = 0;
        send_icmp_packet(5000, &time_taken, &args);

        printf("tt:%llu\n", time_taken);

        // send packet to router 
        printf("at addr %s at hop %d...\n", inet_ntoa(recvaddr.sin_addr), curr_hop);
        if (strcmp(inet_ntoa(recvaddr.sin_addr), destination) == 0) {
            printf("DONE!\n");
            free(source);
            return 0;
        }

        curr_hop++;
    }
    free(source);
    return 1;
}

