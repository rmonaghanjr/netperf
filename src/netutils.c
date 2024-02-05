#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "../include/netutils.h"

#define BUFFER_SIZE 1024

char* resolve_address(char* domain) {
    char* resolved = (char*) malloc(BUFFER_SIZE);
    memset(resolved, 0, BUFFER_SIZE);

    int error;
    struct addrinfo hints, *res, *result;
    void* ptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
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
