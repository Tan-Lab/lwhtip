/**
 * @file   htip.c
 * @brief A library handling HTIP frame.
 *
 * A source file of a library that handle HTIP frame.
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
#include <sys/types.h>
#include <string.h>

#include "datalink.h"
#include "ifinfo.h"
#include "tlv.h"
#include "htip.h"
#include "fdb.h"

int send_htip_device_info(u_char *device_category, int device_category_len,
        u_char *manufacturer_code, u_char *model_name, int model_name_len,
        u_char *model_number, int model_number_len)
{
        u_int len, rlen;
        int n;
        u_char *payload;
        u_char dstaddr[] = HTIP_L2AGENT_DST_MACADDR;
        struct ifinfo *ifip;
        int i, num = get_ifinfo_list_num();

        for (i = 0; i < num; i++) {
                len = 0;
                rlen = 0;
                ifip = get_ifinfo_list() + IFINFO_LEN * i;

                if (ifip->fd < 0) {
                        continue;
                }

                if ((payload = malloc(ETH_DATA_LEN)) == NULL) {
                        perror("malloc");
                        return -1;
                }

                memset(payload, 0, ETH_DATA_LEN);
                len += create_lldp_tlv(payload, ifip->macaddr, ETHER_ADDR_LEN, (u_char *) ifip->ifname, strlen(ifip->ifname));

                if ((rlen = create_basic_htip_device_info_tlv(
                    payload + len, ifip->macaddr, ETHER_ADDR_LEN,
                    (u_char *) ifip->ifname, strlen(ifip->ifname),
                    device_category, device_category_len, manufacturer_code,
                    model_name, model_name_len, model_number,
                    model_number_len)) == 0) {
                        fprintf(stderr, "create_required_htip_device_info_tlv() failed\n");
                        return -1;
                }

                len += rlen;
                len += create_end_of_lldpdu_tlv(payload + len);
#ifdef DEBUG
                printf("  htip frame created: %d bytes using macaddr: %02x:%02x:%02x:%02x:%02x:%02x, ifname: %s.\n",
                        len, ifip->macaddr[0], ifip->macaddr[1], ifip->macaddr[2], ifip->macaddr[3], ifip->macaddr[4], ifip->macaddr[5], ifip->ifname);
#endif /* DEBUG */
                if ((n = write_frame(ifip->fd, ifip->ifname, dstaddr, ifip->macaddr, payload, len)) < 0) {
                        fprintf(stderr, "write_frame() failed on ifname: %s.\n", ifip->ifname);
                        return -1;
                }

                if (n != (len + sizeof(struct ether_header))) {
                        fprintf(stderr, "sent bytes: %d != htip frame bytes:%d\n", n, len);
                }

                free(payload);
        }

        return 0;
}

int send_htip_link_info(void)
{
        int link_info_tlv_len = 0, len = 0, rlen, n;
        u_char *payload, *link_info_payload;
        u_char dstaddr[] = HTIP_L2AGENT_DST_MACADDR;
        struct ifinfo *ifip;
        int i, num = get_ifinfo_list_num(), macaddr_num;
        u_int8_t *macaddrs[MAX_FDB_ENTRY_SIZE];

        for (i = 0; i < num; i++) {
                ifip = get_ifinfo_list() + IFINFO_LEN * i;
                memset(macaddrs, 0, sizeof(macaddrs));
                macaddr_num = get_remote_entry_num_by_macaddr(ifip->macaddr, macaddrs);

                if (macaddr_num == 0)
                        continue;

                link_info_tlv_len += get_htip_link_info_tlv_len(ETHER_ADDR_LEN, macaddr_num);
        }

        if ((link_info_payload = malloc(link_info_tlv_len)) == NULL) {
                perror("malloc");
                return -1;
        }

        for (i = 0; i < num; i++) {
                len = 0;
                rlen = 0;
                ifip = get_ifinfo_list() + IFINFO_LEN * i;

                if (ifip->fd < 0)
                        continue;

                memset(macaddrs, 0, sizeof(macaddrs));
                macaddr_num = get_remote_entry_num_by_macaddr(ifip->macaddr, macaddrs);

                if (macaddr_num == 0)
                        continue;

                len = get_basic_htip_link_info_len(ETHER_ADDR_LEN, strlen(ifip->ifname), link_info_tlv_len);

                if ((payload = malloc(ETH_DATA_LEN)) == NULL) {
                        perror("malloc");
                        return -1;
                }

                memset(payload, 0, ETH_DATA_LEN);
                len += create_lldp_tlv(payload, ifip->macaddr, ETHER_ADDR_LEN, (u_char *) ifip->ifname, strlen(ifip->ifname));

                if ((rlen = create_basic_htip_link_info_tlv(payload,
                        ifip->macaddr, ETHER_ADDR_LEN,
                        (u_char *) ifip->ifname, strlen(ifip->ifname),
                        link_info_payload, link_info_tlv_len)) == -1) {
                        fprintf(stderr, "create_basic_htip_link_info_tlv() failed\n");
                        return -1;
                }

                len += rlen;
                len += create_end_of_lldpdu_tlv(payload + len);
#ifdef DEBUG
                printf("  htip frame created: %d bytes using macaddr: %02x:%02x:%02x:%02x:%02x:%02x, ifname: %s.\n",
                        len, ifip->macaddr[0], ifip->macaddr[1], ifip->macaddr[2], ifip->macaddr[3], ifip->macaddr[4], ifip->macaddr[5], ifip->ifname);
#endif /* DEBUG */
                if ((n = write_frame(ifip->fd, ifip->ifname, dstaddr, ifip->macaddr, payload, len)) < 0) {
                        fprintf(stderr, "write_frame() failed on ifname: %s.\n", ifip->ifname);
                        return -1;
                }

                if (n != (len + sizeof(struct ether_header))) {
                        fprintf(stderr, "sent bytes: %d != htip frame bytes:%d\n", n, len);
                }

                free(payload);
        }

        free(link_info_payload);

        return 0;
}

int send_htip_device_link_info(u_char *device_category,
        int device_category_len, u_char *manufacturer_code, u_char *model_name,
        int model_name_len, u_char *model_number, int model_number_len)
{
        u_char dstaddr[] = HTIP_L2AGENT_DST_MACADDR;
        struct ifinfo *ifip;
        int i, num = get_ifinfo_list_num(), macaddr_num, n;
        u_int len, rlen, link_info_tlv_len = 0;
        u_char *payload, *link_info_payload;
        u_int8_t *macaddrs[MAX_FDB_ENTRY_SIZE];
        u_int16_t port_no;
        char tmp_macaddr[MAC_BUF_SIZE] = {0};

        for (i = 0; i < num; i++) {
                ifip = get_ifinfo_list() + IFINFO_LEN * i;
                memset(macaddrs, 0, sizeof(macaddrs));
                macaddr_num = get_remote_entry_num_by_macaddr(ifip->macaddr, macaddrs);
                link_info_tlv_len += get_htip_link_info_tlv_len(ETHER_ADDR_LEN, macaddr_num);
        }

        if ((link_info_payload = malloc(link_info_tlv_len)) == NULL) {
                perror("malloc");
                return -1;
        }

        len = 0;

        for (i = 0; i < num; i++) {
                ifip = get_ifinfo_list() + IFINFO_LEN * i;
                memset(macaddrs, 0, sizeof(macaddrs));
                macaddr_num = get_remote_entry_num_by_macaddr(ifip->macaddr, macaddrs);
#ifdef DEBUG
                printf("  HTIP link info try to create if: %s, iftype: %d, port: %d, mac_num: %d\n",
                        ifip->ifname, ifip->iftype, port_no, macaddr_num);
#endif /* DEBUG */

                if ((port_no = get_portno_by_macaddr(ifip->macaddr)) == FDB_ENTRY_PORT_INVALID) {
                        fprintf(stderr, "get_portno_by_macaddr() failed with IF: %s.\n", ifip->ifname);
                        fprintf(stderr, "This interface may not join bridge. Ignore to add FDB entry for this interface.\n");
                        continue;
                }

                rlen = create_htip_link_info_tlv(link_info_payload + len, ifip->iftype, port_no, macaddrs, macaddr_num);
                len += rlen;
#ifdef DEBUG
                printf("  HTIP link info create if: %s, iftype: %d, port: %d, mac_num: %d, len: %d\n",
                                 ifip->ifname, ifip->iftype, port_no, macaddr_num, rlen);
#endif /* DEBUG */
        }

        if (len != link_info_tlv_len) {
                fprintf(stderr, "calculated len: %d differ from created len: %d.\n", link_info_tlv_len, len);
        }

        for (i = 0; i < num; i++) {
                len = 0;
                rlen = 0;

                ifip = get_ifinfo_list() + IFINFO_LEN * i;

                if (ifip->fd < 0)
                        continue;

                memset(macaddrs, 0, sizeof(macaddrs));
                macaddr_num = get_remote_entry_num_by_macaddr(ifip->macaddr, macaddrs);

                if (macaddr_num == 0)
                        continue;

                if ((payload = malloc(ETH_DATA_LEN)) == NULL) {
                        perror("malloc");
                        return -1;
                }

                memset(payload, 0, ETH_DATA_LEN);

                len += create_lldp_tlv(payload, ifip->macaddr, ETHER_ADDR_LEN, (u_char *) ifip->ifname, strlen(ifip->ifname));

                if ((rlen = create_basic_htip_device_info_tlv(payload + len,
                        ifip->macaddr, ETHER_ADDR_LEN,
                        (u_char *) ifip->ifname, strlen(ifip->ifname),
                        device_category, device_category_len,
                        manufacturer_code, model_name, model_name_len,
                        model_number, model_number_len)) == 0) {
                        fprintf(stderr, "create_required_htip_device_info_tlv() failed\n");
                        return -1;
                }

                len += rlen;

                if ((rlen = create_basic_htip_link_info_tlv(payload + len,
                        ifip->macaddr, ETHER_ADDR_LEN,
                        (u_char *) ifip->ifname, strlen(ifip->ifname),
                        link_info_payload, link_info_tlv_len)) == -1) {
                        fprintf(stderr, "create_basic_htip_link_info_tlv() failed\n");
                        return -1;
                }

                len += rlen;
                len += create_end_of_lldpdu_tlv(payload + len);
#ifdef DEBUG
                printf("  htip frame created: %d bytes using macaddr: %02x:%02x:%02x:%02x:%02x:%02x, ifname: %s.\n",
                        len, ifip->macaddr[0], ifip->macaddr[1], ifip->macaddr[2], ifip->macaddr[3], ifip->macaddr[4], ifip->macaddr[5], ifip->ifname);
#endif /* DEBUG */

                if ((n = write_frame(ifip->fd, ifip->ifname, dstaddr, ifip->macaddr, payload, len)) < 0) {
                        fprintf(stderr, "write_frame() failed on ifname: %s.\n", ifip->ifname);
                        return -1;
                }

                if (n != (len + sizeof(struct ether_header)))
                        fprintf(stderr, "sent bytes: %d != htip frame bytes:%d\n", n, len);
#ifdef DEBUG
                fprintf(stderr, "\tsent htip bytes: %d\n", len);
#endif /* DEBUG */

                free(payload);
        }

        free(link_info_payload);

        return 0;
}
