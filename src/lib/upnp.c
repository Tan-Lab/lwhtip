/**
 * @file   upnp.c
 * @brief An UPNP communication library.
 *
 * A source file of an UPNP communication library.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "upnp.h"

int upnp_sock(void)
{
        char address[] = UPNP_MULTICAST_ADDR;
        int port, ssock, reuse;
        struct sockaddr_in saddr;
        struct ip_mreq mreq;

        memset(&saddr, 0, sizeof(saddr));
        port = UPNP_PORT;
        saddr.sin_port = htons(port);
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if ((ssock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                return -1;
        }

        if (bind(ssock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
                perror("bind");
                return -1;
        }

        mreq.imr_multiaddr.s_addr = inet_addr(address);
        mreq.imr_interface.s_addr = INADDR_ANY;

        if (setsockopt(ssock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                perror("setsockopt IP_ADD_MEMBERSHIP");
                return -1;
        }

        reuse = 1;

        if (setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
                perror("setsockopt SO_REUSEADDR");
                return -1;
        }

        return ssock;
}

void print_upnp(int sock)
{
        char buff[1024];
        int len, n;

        while (1) {
                n = recvfrom(sock, buff, sizeof(buff), 0, (struct sockaddr *) NULL, (socklen_t *) &len);

                if (n < 0)
                        break;

                printf("    recv UPnP\n");
                printf("%s\n", buff);
        }
}

int close_upnp(int sock)
{
        if (close(sock) < 0) {
                perror("close");
                return -1;
        }

        return 0;
}

int send_upnp_message(char *send_buf, int len, char *src_address, char *dst_address, int port)
{
        int reuse, sock;
        struct sockaddr_in addr;
        in_addr_t ipaddr;

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                return -1;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(dst_address);
        ipaddr = inet_addr(src_address);

        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &ipaddr, sizeof(ipaddr)) < 0) {
                perror("setsockopt");
                return -1;
        }

        reuse = 1;

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
                perror("setsockopt SO_REUSEADDR");
                return -1;
        }

        if (sendto(sock, send_buf, len, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("sendto");
                return -1;
        }

        if (close(sock) < 0) {
                perror("close");
                return -1;
        }

        return 0;
}
