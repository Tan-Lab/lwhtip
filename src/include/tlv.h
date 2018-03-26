/**
 * @file   tlv.h
 * @brief A library handling LLDP and HTIP TLV.
 *
 * A header file of a library that handle LLDP and HTIP TLV.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef TLV_H
#define TLV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <net/ethernet.h>
#ifdef __APPLE__
#include <machine/endian.h>
#endif /* __APPLE__ */

struct tlv_header {
#if BYTE_ORDER == LITTLE_ENDIAN
        u_int tlv_len1:1;
        u_int tlv_type:7;
#endif
#if BYTE_ORDER == BIG_ENDIAN
        u_int tlv_type:7;
        u_int tlv_len1:1;
#endif
        u_int tlv_len2:8;
};
#define MAX_TLV_LEN 0x1FF
#define TLV_HEADER_LEN 2

struct chassis_id_tlv_header {
        u_char chassis_id_subtype;
        u_char chassis_id[];
};

struct port_id_tlv_header {
        u_char port_id_subtype;
        u_char port_id[];
};

#define TTL_TLV_HEADER_LEN 2
struct ttl_tlv_header {
        u_int16_t ttl;
};

struct port_description_tlv_header {
};

#define HTIP_TLV_HEADER_LEN 4
struct htip_tlv_header {
        u_char ttc_oui[3];
        u_char ttc_subtype;
};

#define HTIP_DEVICE_INFO_HEADER_LEN 2
#define HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN 6
struct htip_device_info_header {
        u_char device_info_id;
        u_char device_info_len;
        u_char device_info[];
};

#define HTIP_LINK_INFO_IFTYPE_LEN sizeof(u_int8_t)
#define HTIP_LINK_INFO_PORTNO_LEN sizeof(u_int8_t)
struct htip_link_info_header {
        u_char iftype_len;
        u_int8_t iftype;
        u_char portno_len;
        u_int8_t portno;
        u_char macaddr_num;
};
#define HTIP_LINK_INFO_HEADER_LEN sizeof(struct htip_link_info_header)

/* Mandatory TLV */
#define END_OF_LLDPDU_TLV               0
#define CHASSIS_ID_TLV                  1
#define PORT_ID_TLV                     2
#define TIME_TO_LIVE_TLV                3

/* Optional TLV */
#define PORT_DESCRIPTION_TLV            4
#define SYSTEM_NAME_TLV                 5
#define SYSTEM_DESCRIPTION_TLV          6
#define SYSTEM_CAPABILITIES_TLV         7
#define MANAGEMENT_ADDRESS_TLV          8
#define ORGANIZATIONALLY_SPECIFIC_TLV   127

/* chassis ID subtype enumeration */
#define CHASSIS_ID_SUBTYPE_RESERVED             0
#define CHASSIS_ID_SUBTYPE_CHASSIS_COMPONENT    1
#define CHASSIS_ID_SUBTYPE_INTERFACE_ALIAS      2
#define CHASSIS_ID_SUBTYPE_PORT_COMPONENT       3
#define CHASSIS_ID_SUBTYPE_MAC_ADDRESS          4
#define CHASSIS_ID_SUBTYPE_NETWORK_ADDRESS      5
#define CHASSIS_ID_SUBTYPE_INTERFACE_NAME       6
#define CHASSIS_ID_SUBTYPE_LOCALLY_ASSIGNED     7

/* port ID subtype enumeration */
#define PORT_ID_SUBTYPE_RESERVED                0
#define PORT_ID_SUBTYPE_INTERFACE_ALIAS         1
#define PORT_ID_SUBTYPE_PORT_COMPONENT          2
#define PORT_ID_SUBTYPE_MAC_ADDRESS             3
#define PORT_ID_SUBTYPE_NETWORK_ADDRESS         4
#define PORT_ID_SUBTYPE_INTERFACE_NAME          5
#define PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID        6
#define PORT_ID_SUBTYPE_LOCALLY_ASSIGNED        7

/* HTIP TTC Subtype */
#define HTIP_TTC_SUBTYPE_DEVICE_INFO            1
#define HTIP_TTC_SUBTYPE_LINK_INFO              2
#define HTIP_TTC_SUBTYPE_MAC_ADDRESS_LIST       3

/* HTIP device information ID */
#define HTIP_DEVICE_INFO_DEVICE_CATEGORY        1
#define HTIP_DEVICE_INFO_MANUFACTURER_CODE      2
#define HTIP_DEVICE_INFO_MODEL_NAME             3
#define HTIP_DEVICE_INFO_MODEL_NUMBER           4
#define HTIP_DEVICE_INFO_CHANNEL_USAGE_INFO                     20
#define HTIP_DEVICE_INFO_RADIO_SIGNAL_STRENGTH_INFO             21
#define HTIP_DEVICE_INFO_COMMUNICATION_ERROR_RATE_INFO          22
#define HTIP_DEVICE_INFO_STATUS_INFO                            50
#define HTIP_DEVICE_INFO_LLDPDU_TRANSMISSION_INTERVAL           80
#define HTIP_DEVICE_INFO_VENDOR_SPECIFIC_EXTENSION_FIELD        255

#define TTL_DEFAULT     60

/**
 * @brief Get a length of specified TLV header.
 *
 * The length is calculated by using the variables (tlv_len1, tlv_len2) of struct tlv_header.
 *
 * @param th A pointer to TLV header.
 * @return A length of specified TLV header.
 */
u_int get_tlv_len(const struct tlv_header *th);

/**
 * @brief Set a specified length to a TLV header.
 * @param th A pointer to TLV header.
 * @param len A set length to TLV header.
 */
void set_tlv_len(struct tlv_header *th, const u_int len);

/**
 * @brief Print TLV header for each buffer.
 * @param buf A pointer to a buffer.
 * @param len Byte length of TLV header in the buffer.
 */
void print_tlvs(const char *buf, const size_t len);

/**
 * @brief Print TLV type and length of a specified TLV header.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a chassis ID TLV.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_chassis_id_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a port ID TLV.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_port_id_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a ttl(time to live) TLV.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_ttl_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a port description TLV.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_port_description_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a TLV.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 */
void print_htip_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Print a HTIP device information.
 * @param p A pointer to a head of a header.
 */
void print_htip_device_info(char *p);

/**
 * @brief Print a HTIP link information.
 * @param p A pointer to a head of a header.
 */
void print_htip_link_info(char *p);

/**
 * @brief Check whether a specified TLV is HTIP TLV or not.
 * @param th A pointer to a TLV header.
 * @param len Byte length of a TLV header.
 * @return If it's HTIP TLV, it returns 1. If not, it returns -1.
 */
int is_htip_tlv(const struct tlv_header *th, const u_int len);

/**
 * @brief Create end of LLDPDU TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @return Bytes of created TLV.
 */
int create_end_of_lldpdu_tlv(u_char *p);

/**
 * @brief Create Chassis ID TLV in a specified pointer.
 *
 * Current implementation assumes that the chassis ID is statically MAC address. Please change chassis ID subtype and chassis ID as needed.
 *
 * @param p A head pointer to create TLV.
 * @param macaddr MAC address to set as chassis ID.
 * @param macaddr_len Bytes of MAC address.
 * @return Bytes of created TLV.
 */
int create_chassis_id_tlv(u_char *p, u_char *macaddr, u_int macaddr_len);

/**
 * @brief Create port ID TLV in a specified pointer.
 *
 * Current implementation assumes that the port ID is statically MAC address. Please change port ID subtype and port ID--as needed.
 *
 * @param p A head pointer to create TLV.
 * @param macaddr MAC address to set as port ID.
 * @param macaddr_len Bytes of MAC address.
 * @return Bytes of created TLV.
 */
int create_port_id_tlv(u_char *p, u_char *macaddr, u_int macaddr_len);

/**
 * @brief Create time to live TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param ttl Time to live seconds.
 * @return Bytes of created TLV.
 */
int create_ttl_tlv(u_char *p, u_int16_t ttl);

/**
 * @brief Create port description TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param ifname A network interface name.
 * @return Bytes of created TLV.
 */
int create_port_description_tlv(u_char *p, u_char *ifname, u_int ifname_len);

/**
 * @brief Create LLDP TLVs(chassis ID, port ID, ttl, port description) in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param macaddr A pointer to a MAC address.
 * @param macaddr_len A length of a MAC address.
 * @param ifname A pointer to a network interface name.
 * @param ifname_len A length of a network interface name.
 * @return Bytes of created TLV.
 */
int create_lldp_tlv(u_char *p, u_char *macaddr, u_int macaddr_len,
        u_char *ifname, u_int ifname_len);

/**
 * @brief Get a TLV length that includes LLDP TLVs(chassis ID, port ID, ttl, port description).
 * @param macaddr_len A length of MAC address.
 * @param ifname_len A length of network interface name.
 * @return A TLV length of basic LLDP TLVs.
 */
int get_lldp_tlv_len(u_int macaddr_len, u_int ifname_len);

/**
 * @brief Create TLV header.
 * @param p A pointer to create TLV.
 * @param tlv_len A information string length.
 * @return Bytes of created TLV.
 */
int create_tlv_header(u_char *p, u_int tlv_len);

/**
 * @brief Create HTIP TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param ttc_subtype A TTC sub type.
 * @return Bytes of created TLV.
 */
int create_htip_tlv_header(u_char *p, u_char ttc_subtype);

/**
 * @brief Create HTIP device information TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param device_info_id A device information ID.
 * @param device_info A pointer to device information.
 * @param device_info_len Bytes of device information.
 * @return Bytes of created TLV.
 */
int create_htip_device_info_tlv(u_char *p, u_char device_info_id, u_char *device_info, u_int device_info_len);

/**
 * @brief Create HTIP device information TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param macaddr A pointer to a MAC address.
 * @param macaddr_len A length of a MAC address.
 * @param ifname A pointer to a network interface name.
 * @param ifname_len A length of a network interface name.
 * @param device_category A pointer to a device category.
 * @param device_category_len A length of a device category.
 * @param manufacturer_code A pointer to a manufacturer code. The length of it is determined in HTIP_DEVICE_INFO_MANUFACTURER_CODE_LEN.
 * @param model_name A pointer to a model name.
 * @param model_name_len A length of a model name.
 * @param model_number A pointer to a model number.
 * @param model_number_len A length of a model number.
 * @return Bytes of created TLV.
 */
int create_basic_htip_device_info_tlv(u_char *p, u_char *macaddr,
        u_int macaddr_len, u_char *ifname, u_int ifname_len,
        u_char *device_category, u_int device_category_len,
        u_char *manufacturer_code, u_char *model_name, u_int model_name_len,
        u_char *model_number, u_int model_number_len);

/**
 * @brief Get a TLV length of basic LLDP and HTIP TLVs.
 *
 * Get a TLV length that includes basic LLDP TLVs(chassis ID, port ID, ttl, port description) and required HTIP device information TLVs(device category, manufacturer code, model name, model number).
 *
 * @param macaddr_len A length of MAC address.
 * @param ifname_len A length of network interface name.
 * @param device_category_len A length of device category.
 * @param model_name_len A length of model name.
 * @param model_number_len A length of model number.
 * @return A TLV length of basic LLDP and HTIP TLVs.
 */
int get_basic_htip_device_info_len(u_int macaddr_len, u_int ifname_len,
        u_int device_category_len, u_int model_name_len,
        u_int model_number_len);

/**
 * @brief Create HTIP link information TLV in a specified pointer.
 * @param p A head pointer to create TLV.
 * @param iftype A network interface type(described in IANAifType).
 * @param port_no A port number in FDB.
 * @param macaddrs A pointer list of MAC address of FDB entries.
 * @param macaddr_num A number of MAC address for specified port number.
 * @return Bytes of created TLV.
 */
int create_htip_link_info_tlv(u_char *p, u_int32_t iftype, u_int16_t port_no,
        u_int8_t *macaddrs[], int macaddr_num);

/**
 * @brief Create basic LLDP and HTIP TLVs in a specified pointer.
 *
 * Create basic LLDP TLVs(chassis ID, port ID, ttl, port description) and a HTIP link information TLV in a specified pointer.
 *
 * @param p A head pointer to create TLV.
 * @param macaddr A pointer to a MAC address.
 * @param macaddr_len A length of a MAC address.
 * @param ifname A pointer to a network interface name.
 * @param ifname_len A length of a network interface name.
 * @return Bytes of created TLV.
 */
int create_basic_htip_link_info_tlv(u_char *p, u_char *macaddr,
        u_int macaddr_len, u_char *ifname, u_int ifname_len,
        u_char *link_info_tlv_payload, int link_info_tlv_len);

/**
 * @brief Get a TLV length of basic LLDP and HTIP TLVs.
 *
 * Get a TLV length that includes basic LLDP TLVs(chassis ID, port ID, ttl, port description) and HTIP link information TLV.
 *
 * @param macaddr_len A length of MAC address.
 * @param ifname_len A length of network interface name.
 * @param macaddr_num A number of MAC address contained in a link information.
 * @return A TLV length of basic LLDP and HTIP TLVs.
 */
int get_basic_htip_link_info_len(u_int macaddr_len, u_int ifname_len,
        int link_info_tlv_len);

/**
 * @brief Get a TLV length of basic LLDP and HTIP TLVs.
 *
 * Get a TLV length that includes basic LLDP TLVs(chassis ID, port ID, ttl, port description) and HTIP link information TLV.
 *
 * @param macaddr_len A length of MAC address.
 * @param ifname_len A length of network interface name.
 * @param macaddr_num A number of MAC address contained in a link information.
 * @return A TLV length of basic LLDP and HTIP TLVs.
 */
int get_htip_link_info_tlv_len(u_int macaddr_len, int macaddr_num);
#ifdef __cplusplus
}
#endif

#endif /* TLV_H */
