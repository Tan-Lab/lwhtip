/**
 * @file   datalink.c
 * @brief A data link level network library.
 *
 * A source file of a library that manage data link level network.
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
#include <errno.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <netinet/if_ether.h>
#include <net/if_types.h>
#include <fcntl.h>
#include <net/bpf.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#endif /* __APPLE__ */

#ifdef __linux__
#include <linux/if_packet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/wireless.h>
#include <linux/if_bridge.h>
#endif /* __linux__ */

#include "binary.h"
#include "fdb.h"
#include "ifinfo.h"
#include "tlv.h"
#include "datalink.h"


int set_promiscuous_mode(const char *interface_name)
{
        int sock;
#ifdef __linux__
        struct ifreq ifr;
        struct packet_mreq mreq;

        if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
                perror("socket");
                return -1;
        }

        /* set an interface index to ifr */
        memset(&ifr, 0, sizeof(struct ifreq));
        strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
                perror("ioctl SIOCGIFINDEX");
                close(sock);
                return -1;
        }

        /* set promiscuous mode, this requires to be root user */
        memset(&mreq, 0, sizeof(mreq));	
        mreq.mr_type = PACKET_MR_PROMISC;
        mreq.mr_ifindex = ifr.ifr_ifindex;
        if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (void *) &mreq,
                       sizeof(mreq)) < 0) {
                perror("setsockopt PACKET_ADD_MEMBERSHIP");
                close(sock);
                return -1;
        }
#endif
#ifdef __APPLE__
        if ((sock = open_devbpf()) < 0) {
                fprintf(stderr, "open_devbpf() failed on net ifname: %s\n", interface_name);
                return -1;
        }

        if (set_bpf_options(sock, interface_name) < 0) {
                fprintf(stderr, "set_bpf_options() failed on net ifname: %s\n", interface_name);
                return -1;
        }

        if (check_datalink_type(sock) < 0) {
                fprintf(stderr, "check_datalink_type() failed on net ifname: %s\n", interface_name);
                return -1;
        }

#endif
        return sock;
}

u_int32_t get_iftype(const char *ifname)
{
        int sock;
        u_int32_t dlt = IANAIFTYPE_OTHER;
        char path[SYSFS_PATH_MAX];
        struct stat sbuf;
#ifdef __linux__
        struct ifreq ifr;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("socket");
                dlt = HW_INVALID;
                goto switch_dlt;
        }

        strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
                perror("ioctl(SIOCGIFHWADDR");
                dlt = HW_INVALID;
                goto switch_dlt;
        }
        dlt = ifr.ifr_hwaddr.sa_family;

        if (ioctl(sock, SIOCGIWNAME, &ifr) == 0) {
                dlt = HW_IEEE80211;
        }
        if (snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/bridge", ifr.ifr_name) < 0) {
                fprintf(stderr, "snprintf() failed.\n");
                return -1;
        }
#endif /* __linux__ */
#ifdef __APPLE__
        struct ifmediareq ifmr = {};
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                perror("socket");
                dlt = HW_INVALID;
                goto switch_dlt;
        }

        strncpy(ifmr.ifm_name, ifname, sizeof(ifmr.ifm_name) - 1);
        if (ioctl(sock, SIOCGIFMEDIA, &ifmr) != -1) {
                if (ifmr.ifm_current & IFM_IEEE80211) {
                        dlt = HW_IEEE80211;
                } else if (ifmr.ifm_current & IFM_ETHER) {
                        dlt = HW_ETHER;
                }
        } else {
                /* Not error */
                // perror("ioctl(SIOCGIFMEDIA)");
                dlt = HW_INVALID;
        }
        if (snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/bridge", ifmr.ifm_name) < 0) {
                fprintf(stderr, "snprintf() failed.\n");
                return -1;
        }
#endif /* __APPLE__ */
        /* ignore bridge interface */
        if (stat(path, &sbuf) == 0) {
                if (S_ISDIR(sbuf.st_mode)) {
                        dlt = HW_INVALID;
                }
        }
switch_dlt:
        if ((sock != -1) && (close(sock) == -1)) {
                perror("close");
        }

        switch (dlt) {
                /* Currently support only ethernet and IEEE 802.11 */
                case HW_ETHER:
                		return IANAIFTYPE_ETHERNETCSMACD;
                case HW_IEEE80211:
                        return IANAIFTYPE_IEEE80211;
                default:
                        fprintf(stderr, "Unsupported datalink type:%u\n", dlt);
                        return IANAIFTYPE_OTHER;
        }
}

#ifdef __APPLE__
int get_bpf_buffer_len(int bpf)
{
        int buffer_len = 1, enable = 1;

        if (ioctl(bpf, BIOCIMMEDIATE, &enable) == -1) {
                perror("ioctl(BIOCIMMEDIATE");
                return -1;
        }

        if (ioctl(bpf, BIOCGBLEN, &buffer_len) == -1) {
                perror("ioctl(BIOCGBLEN)");
                return -1;
        }

        return buffer_len;
}

void print_bpf_packet(struct bpf_hdr *bh)
{
        printf("  BPF bh_hdrlen: %d, bh_caplen: %d, bh_datalen: %d\n",
                bh->bh_hdrlen, bh->bh_caplen, bh->bh_datalen);
}
#endif /* __APPLE__ */

void print_ether_header(struct ether_header *eh)
{
        char saddr[MAC_BUF_SIZE], daddr[MAC_BUF_SIZE];

        ether_addr_str(eh->ether_shost, saddr);
        ether_addr_str(eh->ether_dhost, daddr);
        printf("src mac: %s -> dst mac: %s [type=%u]\n",
                saddr, daddr, eh->ether_type);
}

void print_ether_type(struct ether_header *eh)
{
        switch(ntohs(eh->ether_type)) {
        case ETHERTYPE_IP:
                printf("  IPv4 packet\n");
                break;
        case ETHERTYPE_IPV6:
                printf("  IPv6 packet\n");
                break;
        case ETHERTYPE_ARP:
                printf("  ARP packet\n");
                break;
        default:
                printf("  Unknown: %u\n", eh->ether_type);
        }
}

int is_htip_frame(struct ether_header *eh)
{
        if (ntohs(eh->ether_type) != 0x88CC)
                return -1;
        if ((eh->ether_dhost[0] == 0xFF) && (eh->ether_dhost[1] == 0xFF) &&
                (eh->ether_dhost[2] == 0xFF) && (eh->ether_dhost[3] == 0xFF) &&
                (eh->ether_dhost[4] == 0xFF) && (eh->ether_dhost[5] == 0xFF)) {
                return 1;
        }
        return -1;
}

#ifdef __APPLE__
int open_devbpf()
{
        int fd = -1;
        int i = 0;
        char buf[16];

        for (i = 0; i < 255; ++i) {
                snprintf(buf, sizeof(buf), "/dev/bpf%u", i);
                fd = open(buf, O_RDWR);
                if (fd > -1) {
                        return fd;
                }
                switch (errno) {
                case EBUSY:
                        break;
                default:
                        return -1;
                }
        }
        errno = ENOENT;
        return -1;
}

int check_datalink_type(int fd)
{
        u_int32_t dlt = 0;

        if (ioctl(fd, BIOCGDLT, &dlt) < 0) {
                perror("ioctl BIOCGDLT");
                return -1;
        }

        switch (dlt) {
        /* Currently support only ethernet and IEEE 802.11 */
        case HW_ETHER:
        case HW_IEEE80211:
                return 0;
        default:
                fprintf(stderr, "Unsupported datalink type:%u\n", dlt);
                errno = EINVAL;
                return -1;
        }
}

int set_bpf_options(int fd, const char *ifr_name)
{
        struct ifreq ifr;
        u_int32_t enable = 1;

        /* Associate the bpf device with an interface */
        strlcpy(ifr.ifr_name, ifr_name, (unsigned long)(sizeof(ifr.ifr_name) - 1));

        if (ioctl(fd, BIOCSETIF, &ifr) < 0) {
                perror("ioctl BIOCSETIF");
                return -1;
        }

        /* Set header complete mode */
        if (ioctl(fd, BIOCSHDRCMPLT, &enable) < 0) {
                perror("ioctl BIOCSHDRCMPLT");
                return -1;
        }

        /* Monitor packets sent from our interface */
        if (ioctl(fd, BIOCSSEESENT, &enable) < 0)
                return -1;

        /* Return immediately when a packet received */
        if (ioctl(fd, BIOCIMMEDIATE, &enable) < 0) {
                perror("ioctl(BIOCIMMEDIATE");
                return -1;
        }

        return 0;
}

void receive_all_frame(int fd)
{
        char *buf = NULL, *p = NULL;
        size_t blen = 0;
        ssize_t n = 0;
        struct bpf_hdr *bh = NULL;
        struct ether_header *eh = NULL;

        if (ioctl(fd, BIOCGBLEN, &blen) < 0)
                return;

        if ((buf = malloc(blen)) == NULL)
                return;

        printf("reading packets ...\n");

        for ( ; ; ) {
                memset(buf, '\0', blen);
                if ((n = read(fd, buf, blen)) <= 0)
                        return;
                p = buf;
                while (p < buf + n) {
                        /* bpf header */
                        bh = (struct bpf_hdr *)p;
                        /* ethernet header */
                        eh = (struct ether_header *)(p + bh->bh_hdrlen);

                        print_ether_header(eh);
                        print_bpf_packet(bh);
                        print_ether_type(eh);
                        is_htip_frame(eh);

                        p += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
                }
        }
}

void receive_htip_frame(int fd)
{
        char *buf = NULL;
        char *p = NULL;
        size_t blen = 0;
        ssize_t n = 0;
        struct bpf_hdr *bh = NULL;
        struct ether_header *eh = NULL;

        if (ioctl(fd, BIOCGBLEN, &blen) < 0)
                return;

        if ((buf = malloc(blen)) == NULL)
                return;

        printf("reading packets ...\n");

        for ( ; ; ) {
                memset(buf, '\0', blen);

                n = read(fd, buf, blen);

                if (n <= 0)
                        return;

                p = buf;
                while (p < buf + n) {
                        bh = (struct bpf_hdr *)p;

                        /* Start of ethernet frame */
                        eh = (struct ether_header *)(p + bh->bh_hdrlen);

                        if (is_htip_frame(eh) > 0) {
                                printf("\n");
                                print_ether_header(eh);
                                print_bpf_packet(bh);
                                print_hexdump((char *) eh, bh->bh_caplen);
                                print_tlvs((char *) eh + sizeof(struct ether_header), bh->bh_caplen - sizeof(struct ether_header));
                        }
                        printf(".");
                        fflush(stdout);

                        p += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
                }
        }
}
#endif /* __APPLE__ */

void ether_addr_str(const u_int8_t a[], char *p)
{
        if (snprintf(p, MAC_BUF_SIZE, MAC_FMT, a[0], a[1], a[2], a[3], a[4], a[5]) < 0)
                fprintf(stderr, "snprintf failed.\n");
}

int ether_addr_cmp(const u_int8_t a1[], const u_int8_t a2[])
{
        return ((a1[0] == a2[0]) && (a1[1] == a2[1]) && (a1[2] == a2[2]) &&
                (a1[3] == a2[3]) && (a1[4] == a2[4]) && (a1[5] == a2[5]));
}

int write_frame(int fd, char *ifname, u_char *dst_mac, u_char *src_mac,
        u_char *payload, u_int payload_len)
{
        struct ether_frame_t ef;
        ssize_t n;
#ifdef __linux__
        int sock;
        struct ifreq ifr;
        struct sockaddr_ll addr;
#endif /* __linux__ */

        memcpy(ef.eth_header.ether_dhost, dst_mac, ETHER_ADDR_LEN);
        memcpy(ef.eth_header.ether_shost, src_mac, ETHER_ADDR_LEN);
        memcpy(ef.payload, payload, payload_len);
        ef.eth_header.ether_type = htons(0x88cc);
        ef.len = ETHER_HDR_LEN + payload_len;

#ifdef __APPLE__
        if ((n = write(fd, &ef, ef.len)) == -1) {
                perror("write");
                return -1;
        }
#endif /* __APPLE__ */

#ifdef __linux__
        if ((sock = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
                perror("socket");
                return -1;
        }
        memset(&ifr, 0, sizeof(struct ifreq));
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
                perror("ioctl(SIOCGIFINDEX");
                return -1;
        }
        addr.sll_ifindex = ifr.ifr_ifindex;
        addr.sll_halen = ETH_ALEN;
        memcpy(addr.sll_addr, dst_mac, ETHER_ADDR_LEN);

        if ((n = sendto(sock, &ef, ef.len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll))) < 0) {
                fprintf(stderr, "sendto() failed\n");
                return -1;
        }

        if (close(sock) == -1) {
                perror("close");
                return -1;
        }
#endif /* __linux__ */

        return n;
}
