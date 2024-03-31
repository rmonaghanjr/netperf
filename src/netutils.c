#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/netutils.h"

#define BUFFER_SIZE 1024

char* src_ipaddr() {
    char hostbuffer[256];
    char* strbuf = (char*) malloc(BUFFER_SIZE);

    struct hostent *host_entry;
    int host_name;

    // To retrieve hostname
    host_name = gethostname(hostbuffer, sizeof(hostbuffer));
    if (host_name < 0) {
        perror("hostname");
        return NULL;
    }

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry < 0) {
        perror("gethostbyname");
        return NULL;
    }

    strcpy(strbuf, inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])));

    return strbuf;
}

char* resolve_address(char* domain) {
    char* resolved = (char*) malloc(BUFFER_SIZE);
    int error;
    struct addrinfo hints, *res, *result;
    void* ptr;
    memset(resolved, 0, BUFFER_SIZE);

    memset(&hints, 0, sizeof(hints));
    // PF_UNSPEC was causing some domains to resolve to ipv6 addrs, so we want to specify the right type here
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    error = getaddrinfo(domain, NULL, &hints, &result);
    if (error != 0) {
        perror("getaddrinfo");
        return NULL;
    }
    res = result;

    while (res) {
        inet_ntop(res->ai_family, res->ai_addr->sa_data, resolved, BUFFER_SIZE);

        switch (res->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop (res->ai_family, ptr, resolved, BUFFER_SIZE);
        res = res->ai_next;
    }

    freeaddrinfo(result);
    return resolved;
}
