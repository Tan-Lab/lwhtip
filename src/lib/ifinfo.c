/**
 * @file   ifinfo.c
 * @brief A library handling network interface information.
 *
 * A source file of a library that handle network interface information.
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
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef __APPLE__
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if.h>
#endif /* __APPLE__ */

#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <sys/socket.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>
#endif /* __linux__ */

#include "ifinfo.h"
#include "datalink.h"

/* global */
/** A list of network interface information. */
struct ifinfo *ifinfo_list = NULL;
/** A number of network interface information in the list */
int ifinfo_list_num = IFINFO_LIST_INVALID;
/** A size of a list of network interface information */
int ifinfo_list_size = IFINFO_LIST_INVALID;

struct ifinfo *get_ifinfo_list(void)
{
        return ifinfo_list;
}

void set_ifinfo_list(void *p)
{
        ifinfo_list = (struct ifinfo *) p;
}

int get_ifinfo_list_num(void)
{
        return ifinfo_list_num;
}

int set_ifinfo_list_num(int num)
{
        if ((num < IFINFO_LIST_INVALID) || (num > IFINFO_LIST_MAX_SIZE)) {
                fprintf(stderr, "Specified number is invalid(%d).\n", num);
                return -1;
        }
        ifinfo_list_num = num;

        return 0;
}

int increment_ifinfo_list_num(void)
{
        int num = get_ifinfo_list_num();
        int size = get_ifinfo_list_size();

        if (num >= size) {
                fprintf(stderr, "network interface list is already full.\n");
                return -1;
        }
        ifinfo_list_num += 1;

        return 0;
}

int get_ifinfo_list_size(void)
{
        return ifinfo_list_size;
}

int set_ifinfo_list_size(int size)
{
        if ((size < IFINFO_LIST_INVALID) || (size > IFINFO_LIST_MAX_SIZE)) {
                fprintf(stderr, "Specified size is invalid(%d).\n", size);
                return -1;
        }
        ifinfo_list_size = size;

        return 0;
}

struct ifinfo *search_ifinfo_by_ifname(const char *ifname)
{
        struct ifinfo *p;
        int i, num = get_ifinfo_list_num();

        for (i = 0; i < num; i++) {
                p = get_ifinfo_list() + IFINFO_LEN * i;
                if (strcmp(ifname, p->ifname) == 0)
                        return p;
        }

        return NULL;
}

struct ifinfo *get_empty_ifinfo(void)
{
        struct ifinfo *p = get_ifinfo_list();
        int num = get_ifinfo_list_num();
        int size = get_ifinfo_list_size();

        if (num >= size) {
                fprintf(stderr, "ifinfo list is already full.\n");
                return NULL;
        }
        return p + IFINFO_LEN * num;
}

int set_ifinfo_list_ifname(char *ifname)
{
        struct ifinfo *p;

        if ((p = search_ifinfo_by_ifname(ifname)) != NULL) {
        	fprintf(stderr, "Specified network interface is already exist: %s\n", ifname);
                return -1;
        }

        if ((p = get_empty_ifinfo()) == NULL) {
                fprintf(stderr, "ifinfo_list is full.");
                return -1;
        }

        if (increment_ifinfo_list_num() == -1) {
                fprintf(stderr, "increment_ifinfo_list_num() failed.\n");
                return -1;
        }

        memcpy(p->ifname, ifname, IFNAMSIZ);

        return 0;
}

int set_ifinfo_addr(char *ifname, char *ipaddr, char *netmask)
{
        struct ifinfo *p;

        if ((p = search_ifinfo_by_ifname(ifname)) == NULL) {
                fprintf(stderr, "matching entry not found for ifname: %s\n", ifname);
                return -1;
        }
        memcpy(p->ipaddr, ipaddr, INET6_ADDRSTRLEN);
        memcpy(p->netmask, netmask, INET6_ADDRSTRLEN);

        return 0;
}

int set_ifinfo_hwaddr(char *ifname, u_char *macaddr)
{
        struct ifinfo *p;

        if ((p = search_ifinfo_by_ifname(ifname)) == NULL) {
                fprintf(stderr, "matching entry not found for ifname: %s\n", ifname);
                return -1;
        }

        memcpy((char *) p->macaddr, (char *) macaddr, ETHER_ADDR_LEN);

        return 0;

}

int set_ifinfo_iftype(char *ifname, int iftype)
{
        struct ifinfo *p;

        if ((p = search_ifinfo_by_ifname(ifname)) == NULL) {
                fprintf(stderr, "matching entry not found for ifname: %s\n", ifname);
                return -1;
        }
        p->iftype = iftype;

        return 0;
}

int set_ifinfo_portno(char *ifname, u_int16_t port_no)
{
        struct ifinfo *p;

        if ((p = search_ifinfo_by_ifname(ifname)) == NULL) {
                fprintf(stderr, "matching entry not found for ifname: %s\n", ifname);
                return -1;
        }
        p->port_no = port_no;

        return 0;
}

struct ifinfo *malloc_ifinfo_list(int size)
{
        void *p;
        struct ifinfo *ifip;
        int i;

        if (set_ifinfo_list_size(size) == -1) {
                fprintf(stderr, "set_ifinfo_list_size() failed.\n");
                return NULL;
        }

        if (set_ifinfo_list_num(0) == -1) {
                fprintf(stderr, "set_ifinfo_list_num() failed.\n");
                return NULL;
        }

        if ((p = malloc(IFINFO_LEN * size)) == NULL) {
                perror("malloc");
                return NULL;
        }

        memset(p, 0, IFINFO_LEN * size);
        set_ifinfo_list(p);

        return (struct ifinfo *) p;
}

void free_ifinfo_list(void)
{
        struct ifinfo *p;

        if (set_ifinfo_list_size(IFINFO_LIST_INVALID) == -1)
                fprintf(stderr, "set_ifinfo_list_size() failed.\n");

        if (set_ifinfo_list_num(IFINFO_LIST_INVALID) == -1)
                fprintf(stderr, "set_ifinfo_list_num() failed.\n");

        if ((p = get_ifinfo_list()) != NULL) {
                free(p);
                set_ifinfo_list(NULL);
        }
}

int open_netif(void)
{
        struct ifinfo *p;
        int i, num = get_ifinfo_list_num();

        for (i = 0; i < num; i++) {
                p = get_ifinfo_list() + IFINFO_LEN * i;
                if ((p->fd = set_promiscuous_mode(p->ifname)) < 0) {
                        fprintf(stderr, "set_promiscuous_mode() failed on net ifname: %s\n", p->ifname);
                        // return -1;
                }
        }

        return 0;
}

void close_netif(void)
{
        struct ifinfo *p = get_ifinfo_list();
        int i = 0, num = get_ifinfo_list_num();

        for (i = 0; i < num; i++) {
                p = get_ifinfo_list() + IFINFO_LEN * i;
                if (p->fd >= 0)
                        if (close(p->fd) < 0)
                                perror("close");
        }

        free_ifinfo_list();
}

void print_ifinfo(void)
{
        struct ifinfo *p;
        int i, num = get_ifinfo_list_num();
        char macaddr[MAC_BUF_SIZE];

        printf("  print ifinfo list num: %d, size: %d.\n", get_ifinfo_list_num(), get_ifinfo_list_size());
        for (i = 0; i < num; i++) {
                p = get_ifinfo_list() + IFINFO_LEN * i;
                ether_addr_str(p->macaddr, macaddr);
                printf("   ifname: %s, fd: %d, ip: %s, netmask: %s, mac: %s, type: %d, port: %d\n",
                        p->ifname, p->fd, p->ipaddr, p->netmask, macaddr, p->iftype, p->port_no);
        }
}

#ifdef __linux__
int read_ifinfo(void)
{
        int i, n, nifs = 0, sock;
        char ip[INET6_ADDRSTRLEN], netmask[INET6_ADDRSTRLEN];
        struct ifreq *ifr, ifbuf[IFINFO_LIST_MAX_SIZE];
        struct sockaddr *addr;
        struct ifconf ifc;
        struct ifinfo tmp, *p;
        struct ifaddrs *ifa, *ifa_list;
        struct ifreq ifrbuf;

        if ((n = getifaddrs(&ifa_list)) == -1) {
                perror("getifaddrs");
                return -1;
        }

        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                printf("  getifaddrs if: %s\n", ifa->ifa_name);
                if (is_valid_ifaddr(ifa) < 0)
                        continue;
                printf("  getifaddrs available if: %s\n", ifa->ifa_name);
                nifs += 1;
        }

#ifdef DEBUG
        printf("  getifaddrs ifs: %d\n", nifs);
#endif /* DEBUG */
        n = nifs;

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                perror("socket");
                return -1;
        }

        if ((ifinfo_list = malloc_ifinfo_list(IFINFO_LIST_MAX_SIZE)) == NULL) {
                fprintf(stderr, "malloc_ifinfo_list() failed.\n");
                return -1;
        }

        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                if (is_valid_ifaddr(ifa) < 0)
                        continue;

                memset(&ifrbuf, 0, sizeof(ifrbuf));
                memset(ip, 0, INET6_ADDRSTRLEN);
                memset(netmask, 0, INET6_ADDRSTRLEN);
                strncpy(ifrbuf.ifr_name, ifa->ifa_name, IFNAMSIZ);
                ifr = &ifrbuf;
                printf("ifname: %s\n", ifr->ifr_name);
                addr = &(ifr->ifr_addr);

                if (ioctl(sock, SIOCGIFADDR, ifr) != -1) {
                        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ip, sizeof(ip)) == NULL) {
                                perror("inet_ntop");
                        }
                        addr = &(ifr->ifr_netmask);
                        if (ioctl(sock, SIOCGIFNETMASK, ifr) == -1) {
                                perror("ioctl(SIOCGIFNETMASK)");
                        }
                        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), netmask, sizeof(netmask)) == NULL) {
                                perror("inet_ntop");
                        }
                } else {
                        perror("ioctl(SIOCGIFADDR)");
                }

                if (ioctl(sock, SIOCGIFHWADDR, ifr) == -1) {
                        perror("ioctl(SIOCFIFHWADDR");
                        continue;;
                }

                if (set_ifinfo_list_ifname(ifr->ifr_name) < 0) {
                        fprintf(stderr, "set_ifinfo_ifname() failed\n");
                        return -1;
                }
                if (set_ifinfo_hwaddr(ifr->ifr_name, (u_char *) ifr->ifr_hwaddr.sa_data) < 0) {
                        fprintf(stderr, "set_ifinfo_hwaddr() failed.\n");
                        return -1;
                }

                if (set_ifinfo_addr(ifr->ifr_name, ip, netmask) < 0) {
                        fprintf(stderr, "set_ifinfo_addr() failed\n");
                        return -1;
                }
        }

        if (close(sock) < 0) {
                perror("close");
                return -1;
        }

        return 0;
}

int is_valid_netif(const struct ifinfo *p)
{
        if (is_valid_ifname(p->ifname) < 0)
                return -1;

        if (strncmp(p->ipaddr, IPv4_LOOPBACK, INET6_ADDRSTRLEN) == 0){
                fprintf(stderr, "loopback IPv4 addr\n");
                return -1;

        }
        if (memcmp(p->macaddr, HWADDR_LOOPBACK, ETHER_ADDR_LEN) == 0) {
                fprintf(stderr, "loopback MAC addr\n");
                return -1;
        }

        return 0;
}
#endif /* __linux__ */

int is_valid_ifname(const char *p)
{
        if (strncmp(p, IFNAME_LOOPBACK, IFNAMSIZ) == 0) {
                fprintf(stderr, "loopback ifname\n");
                return -1;
        }

        return 0;
}

int is_valid_ifaddr(struct ifaddrs *ifa)
{
        int i;
        char *p, empty[IFNAMSIZ] = {0};

        if (is_valid_ifname(ifa->ifa_name) == -1)
                return -1;

        /* Currently IPv4 support */
         fprintf(stderr, "    is_valid_ifaddr family: %x\n", ifa->ifa_addr->sa_family);
#ifdef __linux__
        if (ifa->ifa_addr->sa_family != PF_PACKET)
#endif /* __linux__ */
#ifdef __APPLE__
        if (ifa->ifa_addr->sa_family != AF_LINK)
#endif /* __APPLE__ */
                return -1;

        if (get_iftype(ifa->ifa_name) == IANAIFTYPE_OTHER)
                return -1;

        return 0;
}

int read_net_type(void)
{
        struct ifreq ifr;
#ifdef __linux__
        struct ethtool_cmd cmd;
#endif /* __linux__ */
        struct ifinfo *p;
        int sock, i, num = get_ifinfo_list_num();
        u_int32_t dlt;

        for (i = 0; i < num; i++) {
                p = get_ifinfo_list() + IFINFO_LEN * i;

                if ((dlt = get_iftype(p->ifname)) == HW_INVALID) {
                        fprintf(stderr, "get_iftype() failed.\n");
                        return -1;
                }

                if (set_ifinfo_iftype(p->ifname, dlt) == -1) {
                        fprintf(stderr, "set_ifinfo_iftype() failed.\n");
                        return -1;
                }
        }

        return 0;
}

#ifdef __APPLE__
void print_netif(void)
{
        struct ifaddrs *ifa_list, *ifa;
        char addr[INET6_ADDRSTRLEN], netmask[INET6_ADDRSTRLEN];
        u_char *dl_addr;
        struct sockaddr_dl *dl;

        if (getifaddrs(&ifa_list) == -1) {
                perror("getifaddrs");
                return;
        }
        
        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                memset(addr, 0, sizeof(addr));
                memset(netmask, 0, sizeof(netmask));

                printf("  ifname: %s, ifa flags: 0x%.8x\n", ifa->ifa_name, ifa->ifa_flags);
                switch (ifa->ifa_addr->sa_family) {
                case AF_INET:
                        if (inet_ntop(AF_INET, &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr, addr, sizeof(addr)) == NULL) {
                                perror("inet_ntop");
                                return;
                        }
                        if (inet_ntop(AF_INET, &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr, netmask, sizeof(netmask)) == NULL) {
                                perror("inet_ntop");
                                return;
                        }
                        break;
                case AF_INET6:
                        if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr, addr, sizeof(addr)) == NULL) {
                                perror("inet_ntop");
                                return;
                        }
                        if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *) ifa->ifa_netmask)->sin6_addr, netmask, sizeof(netmask)) == NULL) {
                                perror("inet_ntop");
                                return;
                        }
                        break;
                default:
                        printf("  ifa sa_family: %u\n", ifa->ifa_addr->sa_family);
                        break;
                }
                printf("    addr: %s, netmask: %s\n", addr, netmask);
                dl = (struct sockaddr_dl *) ifa->ifa_addr;
                if (dl->sdl_family == AF_LINK && dl->sdl_type == IFT_ETHER) {
                        dl_addr = (unsigned char *) LLADDR(dl);
                        printf("    mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                dl_addr[0], dl_addr[1], dl_addr[2], dl_addr[3],
                                dl_addr[4], dl_addr[5]);
                }
        }
}

int read_ifinfo(void)
{
        struct ifinfo *p;
        struct ifaddrs *ifa_list, *ifa;
        char addr[INET6_ADDRSTRLEN], netmask[INET6_ADDRSTRLEN];
        u_char *dl_addr;
        struct sockaddr_dl *dl;
        int n;

        if (getifaddrs(&ifa_list) == -1) {
                perror("getifaddrs");
                return -1;
        }
        
        n = num_netif(ifa_list);

        if ((ifinfo_list = malloc_ifinfo_list(n)) == NULL) {
                fprintf(stderr, "mallocifinfo() failed.\n");
                freeifaddrs(ifa_list);
                return -1;
        }

        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                dl = (struct sockaddr_dl *) ifa->ifa_addr;
                if (is_available_ifaddr(ifa) == 0) {
                        if (set_ifinfo_list_ifname(ifa->ifa_name) < 0) {
                                fprintf(stderr, "set_ifinfo_ifname() failed\n");
                                return -1;
                        }
                        dl_addr = (unsigned char *) LLADDR(dl);
                        if (set_ifinfo_hwaddr(ifa->ifa_name, dl_addr) < 0) {
                                fprintf(stderr, "set_ifinfo_hwaddr() failed.\n");
                                return -1;
                        }
                }
        }
        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                if (search_ifinfo_by_ifname(ifa->ifa_name) == NULL)
                        continue;
                memset(addr, 0, sizeof(addr));
                memset(netmask, 0, sizeof(netmask));
                switch (ifa->ifa_addr->sa_family) {
                /* currently, ignore IPv6 */
                case AF_INET:
                        if (inet_ntop(AF_INET, &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr, addr, sizeof(addr)) == NULL) {
                                perror("inet_ntop");
                                return -1;
                        }
                        if (inet_ntop(AF_INET, &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr, netmask, sizeof(netmask)) == NULL) {
                                perror("inet_ntop");
                                return -1;
                        }
                        if (set_ifinfo_addr(ifa->ifa_name, addr, netmask) < 0) {
                                fprintf(stderr, "set_ifinfo_addr() failed\n");
                                return -1;
                        }
                        break;
                default:
                        break;
                }
        }
        freeifaddrs(ifa_list);

        return 0;
}

int num_netif(struct ifaddrs *ifa_list)
{
        struct ifaddrs *ifa;
        int n = 0;

        for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
                if (is_available_ifaddr(ifa) == 0)
                        n += 1;
        }

        return n;
}

int is_available_ifaddr(struct ifaddrs *ifa)
{
        struct ifinfo ifi;
        struct sockaddr_dl *dl = (struct sockaddr_dl *) ifa->ifa_addr;

        if (dl->sdl_family == AF_LINK && dl->sdl_type == IFT_ETHER) {
                /* check interface is up and support broadcast */
                if (((ifa->ifa_flags & IFF_UP) != IFF_UP) || ((ifa->ifa_flags & IFF_BROADCAST) != IFF_BROADCAST))
                        return -1;
                return 0;
        }

        return -1;
}
#endif /* __APPLE__ */
