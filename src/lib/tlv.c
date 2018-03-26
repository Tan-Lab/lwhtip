/**
 * @file   tlv.c
 * @brief A library handling LLDP and HTIP TLV.
 *
 * A source file of a library that handle LLDP and HTIP TLV.
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
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/types.h>

#include "binary.h"
#include "datalink.h"
#include "ifinfo.h"
#include "tlv.h"

u_int get_tlv_len(const struct tlv_header *th)
{
        return (th->tlv_len1 << 7) + th->tlv_len2;
}

void set_tlv_len(struct tlv_header *th, const u_int len)
{
        th->tlv_len1 = len >> 8;
        th->tlv_len2 = len & 0x0FF;
}

void print_tlvs(const char *buf, const size_t len)
{
        const char *p = NULL;
        u_int tlv_len = 0;

        p = buf;

        while (p < buf + len) {
                tlv_len = get_tlv_len((struct tlv_header *) p);
                print_tlv((struct tlv_header *) p, tlv_len);
                p += TLV_HEADER_LEN + tlv_len;
        }
}

void print_tlv(const struct tlv_header *th, const u_int len)
{
        printf("  TLV type: %x, len: %u\n", th->tlv_type, len);

        switch (th->tlv_type) {
        case END_OF_LLDPDU_TLV:
                printf("    End of LLDPDU\n");
                break;
        case CHASSIS_ID_TLV:
                printf("    Chassis ID TLV\n");
                print_chassis_id_tlv(th, len);
                break;
        case PORT_ID_TLV:
                printf("    Port ID TLV\n");
                print_port_id_tlv(th, len);
                break;
        case TIME_TO_LIVE_TLV:
                printf("    Time To Live ID TLV\n");
                print_ttl_tlv(th, len);
                break;
        case PORT_DESCRIPTION_TLV:
                printf("    Port description TLV\n");
                print_port_description_tlv(th, len);
                break;
        case ORGANIZATIONALLY_SPECIFIC_TLV:
                printf("    Organazationally Specific TLV\n");
                print_htip_tlv(th, len);
                break;
        default:
                break;
        }
}

void print_chassis_id_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        struct chassis_id_tlv_header *ch;
        u_int subtype = 0;

        p = (char *) th + TLV_HEADER_LEN;
        ch = (struct chassis_id_tlv_header *) p;
        subtype = ch->chassis_id_subtype & 0x0F;
        printf("      chassis ID subtype: %u\n", subtype);

        switch (subtype) {
        case CHASSIS_ID_SUBTYPE_RESERVED:
                break;
        case CHASSIS_ID_SUBTYPE_CHASSIS_COMPONENT:
                break;
        case CHASSIS_ID_SUBTYPE_INTERFACE_ALIAS:
                break;
        case CHASSIS_ID_SUBTYPE_PORT_COMPONENT:
                break;
        case CHASSIS_ID_SUBTYPE_MAC_ADDRESS:
                printf("      chassis ID subtype: MAC address ");
                print_hexlstr((char *) ch->chassis_id, len - 1);
                printf("\n");
                break;
        case CHASSIS_ID_SUBTYPE_NETWORK_ADDRESS:
                break;
        case CHASSIS_ID_SUBTYPE_INTERFACE_NAME:
                break;
        case CHASSIS_ID_SUBTYPE_LOCALLY_ASSIGNED:
                break;
        default:
                printf("      chassis ID subtype: Unknown ");
                print_hexlstr((char *) ch->chassis_id, len - 1);
                printf("\n");
                break;
        }
}

void print_port_id_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        struct port_id_tlv_header *ph;
        u_int subtype = 0;

        p = (char *) th + TLV_HEADER_LEN;
        ph = (struct port_id_tlv_header *) p;
        subtype = ph->port_id_subtype & 0x0F;
        printf("      port ID subtype: %u\n", subtype);

        switch (subtype) {
        case PORT_ID_SUBTYPE_RESERVED:
                break;
        case PORT_ID_SUBTYPE_INTERFACE_ALIAS:
                break;
        case PORT_ID_SUBTYPE_PORT_COMPONENT:
                break;
        case PORT_ID_SUBTYPE_MAC_ADDRESS:
                printf("      port ID subtype: MAC address ");
                print_hexlstr((char *) ph->port_id, len - 1);
                printf("\n");
                break;
        case PORT_ID_SUBTYPE_NETWORK_ADDRESS:
        case PORT_ID_SUBTYPE_INTERFACE_NAME:
        case PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID:
        case PORT_ID_SUBTYPE_LOCALLY_ASSIGNED:
        default:
                printf("      port ID subtype: Unknown ");
                print_hexlstr((char *) ph->port_id, len - 1);
                printf("\n");
                break;
        }

}

void print_ttl_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        struct ttl_tlv_header *ttlh;
        u_int ttl = 0;

        p = (char *) th + TLV_HEADER_LEN;
        ttlh = (struct ttl_tlv_header *) p;
        ttl = ntohs(ttlh->ttl);

        printf("      time to live: %u\n", ttl);
}

void print_port_description_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        char port_description[len + 1];

        p = (char *) th + TLV_HEADER_LEN;
        memset(port_description, 0, len);
        memcpy(port_description, p, len);
        //print_hexlstr(port_description, len);
        printf("      port description: %s\n", port_description);
}

void print_htip_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        struct htip_tlv_header *hh;
        u_int ttc_subtype;

        p = (char *) th + TLV_HEADER_LEN;
        hh = (struct htip_tlv_header *) p;

        ttc_subtype = hh->ttc_subtype;

        if (is_htip_tlv(th, len) < 0)
                return;

        printf("      htip ttc subtype: %u\n", ttc_subtype);
        printf("      htip ttc oui: %x %x %x\n", hh->ttc_oui[0], hh->ttc_oui[1], hh->ttc_oui[2]);

        switch (ttc_subtype) {
        case HTIP_TTC_SUBTYPE_DEVICE_INFO:
                printf("      Device information htip ttc subtype: %u\n", ttc_subtype);
                print_htip_device_info(p + HTIP_TLV_HEADER_LEN);
                break;
        case HTIP_TTC_SUBTYPE_LINK_INFO:
                printf("      Link information htip ttc subtype: %u\n", ttc_subtype);
                print_htip_link_info(p + HTIP_TLV_HEADER_LEN);
                break;
        case HTIP_TTC_SUBTYPE_MAC_ADDRESS_LIST:
                printf("      MAC address list htip ttc subtype: %u\n", ttc_subtype);
                break;
        default:
                printf("      Unknown htip ttc subtype: %u\n", ttc_subtype);
                break;
        }
}

void print_htip_link_info(char *p)
{
        int i, len;
        char maddr[MAC_BUF_SIZE];
        struct htip_link_info_header *hh;

        hh = (struct htip_link_info_header *) p;
        printf("        iftype len: %u, iftype: %d, port len: %u, port no: %d, mac num: %u, mac:",
                hh->iftype_len, hh->iftype, hh->portno_len, hh->portno, hh->macaddr_num);
        len = hh->macaddr_num;

        for (i = 0; i < len; i++) {
                ether_addr_str((u_int8_t *) hh + HTIP_LINK_INFO_HEADER_LEN + ETHER_ADDR_LEN * i, maddr);
                printf(" %s", maddr);
        }

        printf("\n");
}

void print_htip_device_info(char *p)
{
        int len;
        struct htip_device_info_header *hh;

        hh = (struct htip_device_info_header *) p;
        printf("        ID: %u, len: %u,", hh->device_info_id, hh->device_info_len);
        len = hh->device_info_len;
        print_hexlstr((char *) hh->device_info, len);
        printf(", device info: %.*s\n", len, hh->device_info);
}

int is_htip_tlv(const struct tlv_header *th, const u_int len)
{
        char *p;
        struct htip_tlv_header *hh;
        u_int ttc_subtype;

        p = (char *) th + TLV_HEADER_LEN;
        hh = (struct htip_tlv_header *) p;
        ttc_subtype = hh->ttc_subtype;

        if (hh->ttc_oui[0] != 0xE0 || hh->ttc_oui[1] != 0x27 || hh->ttc_oui[2] != 0x1A)
                return -1;

        switch (ttc_subtype) {
        case HTIP_TTC_SUBTYPE_DEVICE_INFO:
        case HTIP_TTC_SUBTYPE_LINK_INFO:
        case HTIP_TTC_SUBTYPE_MAC_ADDRESS_LIST:
                return 1;
        default:
                return -1;
        }
}

int create_end_of_lldpdu_tlv(u_char *p)
{
        struct tlv_header *th;

        th = (struct tlv_header *) p;
        th->tlv_type = END_OF_LLDPDU_TLV;
        set_tlv_len(th, 0);

        return TLV_HEADER_LEN;
}

int create_chassis_id_tlv(u_char *p, u_char *macaddr, u_int macaddr_len)
{
        struct tlv_header th;
        struct chassis_id_tlv_header ch;

        th.tlv_type = CHASSIS_ID_TLV;
        set_tlv_len(&th, 1 + macaddr_len);
        memcpy(p, &th, TLV_HEADER_LEN);

        ch.chassis_id_subtype = CHASSIS_ID_SUBTYPE_MAC_ADDRESS;
        memcpy(p + TLV_HEADER_LEN, &ch, 1);
        /* MAC address may be 6 octets or 8 octets. */
        memcpy(p + TLV_HEADER_LEN + 1, macaddr, macaddr_len);

        return TLV_HEADER_LEN + 1 + macaddr_len;
}

int create_port_id_tlv(u_char *p, u_char *macaddr, u_int macaddr_len)
{
        struct tlv_header th;
        struct port_id_tlv_header ph;

        th.tlv_type = PORT_ID_TLV;
        set_tlv_len(&th, 1 + macaddr_len);
        memcpy(p, &th, TLV_HEADER_LEN);

        ph.port_id_subtype = PORT_ID_SUBTYPE_MAC_ADDRESS;
        memcpy(p + TLV_HEADER_LEN, &ph, 1);
        /* MAC address may be 6 octets or 8 octets. */
        memcpy(p + TLV_HEADER_LEN + 1, macaddr, macaddr_len);

        return TLV_HEADER_LEN + 1 + macaddr_len;
}

int create_ttl_tlv(u_char *p, u_int16_t ttl)
{
        struct tlv_header th;
        struct ttl_tlv_header ttlh;

        th.tlv_type = TIME_TO_LIVE_TLV;
        set_tlv_len(&th, TTL_TLV_HEADER_LEN);
        memcpy(p, &th, TLV_HEADER_LEN);

        if (ttl >= 65535) {
                fprintf(stderr, "ttl should be in range: 0 <= ttl <= 65535\n");
                return -1;
        }

        ttlh.ttl = htons(ttl);
        memcpy(p + TTL_TLV_HEADER_LEN, &ttlh, TTL_TLV_HEADER_LEN);

        return TLV_HEADER_LEN + TTL_TLV_HEADER_LEN;
}

int create_port_description_tlv(u_char *p, u_char *ifname, u_int ifname_len)
{
        struct tlv_header th;

        th.tlv_type = PORT_DESCRIPTION_TLV;
        set_tlv_len(&th, ifname_len);
        memcpy(p, &th, TLV_HEADER_LEN);
        memcpy(p + TLV_HEADER_LEN, ifname, ifname_len);

        return TLV_HEADER_LEN + ifname_len;
}

int create_lldp_tlv(u_char *p, u_char *macaddr,
        u_int macaddr_len, u_char *ifname, u_int ifname_len)
{
        u_int len = 0;

        len += create_chassis_id_tlv(p, macaddr, macaddr_len);
        len += create_port_id_tlv(p + len, macaddr, macaddr_len);
        len += create_ttl_tlv(p + len, TTL_DEFAULT);
        len += create_port_description_tlv(p + len, ifname, ifname_len);

        return len;
}

int get_lldp_tlv_len(u_int macaddr_len, u_int ifname_len)
{
        return TLV_HEADER_LEN + 1 + macaddr_len +       /* chassis ID TLV */
                TLV_HEADER_LEN + 1 + macaddr_len +      /* port ID TLV */
                TLV_HEADER_LEN + TTL_TLV_HEADER_LEN +   /* ttl TLV */
                TLV_HEADER_LEN + ifname_len;            /* port description TLV */
}

int create_tlv_header(u_char *p, u_int tlv_len)
{
        struct tlv_header th;

        th.tlv_type = ORGANIZATIONALLY_SPECIFIC_TLV;
        set_tlv_len(&th, tlv_len);
        memcpy(p, &th, TLV_HEADER_LEN);

        return TLV_HEADER_LEN;
}

int create_htip_tlv_header(u_char *p, u_char ttc_subtype)
{
        struct htip_tlv_header hth;

        hth.ttc_oui[0] = 0xE0;
        hth.ttc_oui[1] = 0x27;
        hth.ttc_oui[2] = 0x1A;
        hth.ttc_subtype = ttc_subtype;
        memcpy(p, &hth, HTIP_TLV_HEADER_LEN);

        return HTIP_TLV_HEADER_LEN;
}

int create_htip_device_info_tlv(u_char *p, u_char device_info_id,
        u_char *device_info, u_int device_info_len)
{
        struct htip_device_info_header hdih;
        u_int len = 0;

        len += create_tlv_header(p, HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + device_info_len);
        len += create_htip_tlv_header(p + len, HTIP_TTC_SUBTYPE_DEVICE_INFO);

        switch (device_info_id) {
        case HTIP_DEVICE_INFO_DEVICE_CATEGORY:
                if (device_info_len > 255) {
                        fprintf(stderr, "HTIP device info category should be in range from 0 to 255 bytes. %d bytes are set.\n", device_info_len);
                }
                break;
        case HTIP_DEVICE_INFO_MANUFACTURER_CODE:
                if (device_info_len != HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN) {
                        fprintf(stderr, "HTIP device info must be %d bytes. %d bytes are set.\n", HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN, device_info_len);
                }
                break;
        case HTIP_DEVICE_INFO_MODEL_NAME:
                if (device_info_len > 31) {
                        fprintf(stderr, "HTIP device info model name should be in range from 0 to 31 bytes. %d bytes are set.\n", device_info_len);
                }
                break;
        case HTIP_DEVICE_INFO_MODEL_NUMBER:
                if (device_info_len > 31) {
                        fprintf(stderr, "HTIP device info model number should be in range from 0 to 31 bytes. %d bytes are set.\n", device_info_len);
                }
                break;
        default:
                break;
        }

        hdih.device_info_id = device_info_id;
        hdih.device_info_len = device_info_len;
        memcpy(p + TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN, &hdih, HTIP_DEVICE_INFO_HEADER_LEN);
        memcpy(p + TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN, device_info, device_info_len);

        return TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + device_info_len;
}

int create_basic_htip_device_info_tlv(u_char *p,
        u_char *macaddr, u_int macaddr_len, u_char *ifname, u_int ifname_len,
        u_char *device_category, u_int device_category_len,
        u_char *manufacturer_code,
        u_char *model_name, u_int model_name_len,
        u_char *model_number, u_int model_number_len)
{
        u_int len = 0;

        len += create_htip_device_info_tlv(p + len, HTIP_DEVICE_INFO_DEVICE_CATEGORY, device_category, device_category_len);
#ifdef DEBUG
        printf("    htip device info category len: %d\n", len);
#endif /* DEBUG */
        len += create_htip_device_info_tlv(p + len, HTIP_DEVICE_INFO_MODEL_NAME, model_name, model_name_len);
#ifdef DEBUG
        printf("    htip device info model name len: %d\n", len);
#endif /* DEBUG */
        len += create_htip_device_info_tlv(p + len, HTIP_DEVICE_INFO_MANUFACTURER_CODE, manufacturer_code, HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN);
#ifdef DEBUG
        printf("    htip device info manufacturer code len: %d\n", len);
#endif /* DEBUG */
        len += create_htip_device_info_tlv(p + len, HTIP_DEVICE_INFO_MODEL_NUMBER, model_number, model_number_len);
#ifdef DEBUG
        printf("    htip device info model name number: %d\n", len);
#endif /* DEBUG */

        return len;
}

int get_basic_htip_device_info_len(u_int macaddr_len, u_int ifname_len,
        u_int device_category_len, u_int model_name_len, u_int model_number_len)
{
        return get_lldp_tlv_len(macaddr_len, ifname_len) +
                TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + device_category_len +      /* htip device info device category */
                TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN +   /* htip device info manufacturer code */
                TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + model_name_len +   /* htip device info model name */
                TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_DEVICE_INFO_HEADER_LEN + model_number_len + /* htip device info model number */
                TLV_HEADER_LEN;         /* End of TLV */
}

int create_htip_link_info_tlv(u_char *p, u_int32_t iftype, u_int16_t port_no, u_int8_t *macaddrs[], int macaddr_num)
{
        struct htip_link_info_header hlih;
        int i, j, fragment, max_macaddr_num, macaddr_num_tlv;
        u_int len = 0;

        /* max number of MAC address can be stored in TLV */
        max_macaddr_num = (MAX_TLV_LEN  - HTIP_TLV_HEADER_LEN - HTIP_LINK_INFO_HEADER_LEN) / ETHER_ADDR_LEN;
        /* A number of fragments that are separated in multiple TLVs */
        if (macaddr_num <= 0)
                fragment = 1;
        else
                fragment = macaddr_num / max_macaddr_num + 1;

#ifdef DEBUG
        printf("\tHTIP link info fragments: %d, msc mac: %d\n", fragment, max_macaddr_num);
#endif /* DEBUG */

        for (i = 0; i < fragment; i++) {
                macaddr_num_tlv = max_macaddr_num;
                if (i + 1 == fragment)
                        macaddr_num_tlv = macaddr_num - i * max_macaddr_num;

                len += create_tlv_header(p + len, HTIP_TLV_HEADER_LEN + HTIP_LINK_INFO_HEADER_LEN + ETHER_ADDR_LEN * macaddr_num_tlv);
                len += create_htip_tlv_header(p + len, HTIP_TTC_SUBTYPE_LINK_INFO);

                hlih.iftype_len = HTIP_LINK_INFO_IFTYPE_LEN;
                hlih.iftype = (u_int8_t) iftype;
                hlih.portno_len = HTIP_LINK_INFO_PORTNO_LEN;
                hlih.portno = (u_int8_t) port_no;
                hlih.macaddr_num = macaddr_num_tlv;
                memcpy(p + len, &hlih, HTIP_LINK_INFO_HEADER_LEN);
                len += HTIP_LINK_INFO_HEADER_LEN;

                for (j = 0; j < macaddr_num_tlv; j++) {
                        memcpy(p + len, macaddrs[i * max_macaddr_num + j], ETHER_ADDR_LEN);
                        len += ETHER_ADDR_LEN;
                }

#ifdef DEBUG
                printf("\t\tHTIP link info tlv len: %d, created len: %d, macaddr_num_tlv: %d, MAXTLVLEN: %d, HTIP_TLV_HEADER_LEN: %d, HTIP_LINK_INFO_HEADER_LEN: %d\n",
                        TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_LINK_INFO_HEADER_LEN + ETHER_ADDR_LEN * macaddr_num_tlv, len, macaddr_num_tlv, MAX_TLV_LEN,
                                                HTIP_TLV_HEADER_LEN, HTIP_LINK_INFO_HEADER_LEN);
#endif /* DEBUG */
        }

        return len;
}

int create_basic_htip_link_info_tlv(u_char *p, u_char *macaddr,
        u_int macaddr_len, u_char *ifname, u_int ifname_len,
        u_char *link_info_tlv_payload, int link_info_tlv_len)
{
        u_int len = 0;

        memcpy(p + len, link_info_tlv_payload, link_info_tlv_len);
        len += link_info_tlv_len;
#ifdef DEBUG
        printf("    htip link info tlv len: %d\n", len);
#endif /* DEBUG */

        return len;
}

int get_basic_htip_link_info_len(u_int macaddr_len, u_int ifname_len,
        int link_info_tlv_len)
{
        return get_lldp_tlv_len(macaddr_len, ifname_len) +
                link_info_tlv_len;       /* htip link info TLVs */
}

int get_htip_link_info_tlv_len(u_int macaddr_len, int macaddr_num)
{
        int i, j, fragment, max_macaddr_num, macaddr_num_tlv;

        max_macaddr_num = (MAX_TLV_LEN  - HTIP_TLV_HEADER_LEN - HTIP_LINK_INFO_HEADER_LEN) / ETHER_ADDR_LEN;
        if (macaddr_num <= 0)
                fragment = 1;
        else
                fragment = macaddr_num / max_macaddr_num + 1;

        return (TLV_HEADER_LEN + HTIP_TLV_HEADER_LEN + HTIP_LINK_INFO_HEADER_LEN) * fragment + macaddr_len * macaddr_num;
}
