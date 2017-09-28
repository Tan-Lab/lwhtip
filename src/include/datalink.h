/**
 * @file   datalink.h
 * @brief A data link level network library.
 *
 * A header file of a library that manage data link level network.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef DATALINK_H
#define DATALINK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#define BPF_PATH_LEN    11
#include <net/bpf.h>
#include <ifaddrs.h>
#endif /* __APPLE__ */

#ifdef __linux__
#include <sys/socket.h>
#include <linux/if_arp.h>
#endif /* __linux__ */

#include <net/ethernet.h>

#ifdef __APPLE__
#define HW_ETHER DLT_EN10MB
#define HW_IEEE80211 DLT_IEEE802_11
#define HW_IEEE802154 DLT_IEEE802_15_4
#define HW_INVALID DLT_MATCHING_MAX
#endif /* __APPLE__ */

#ifdef __linux__
#define HW_ETHER ARPHRD_ETHER
#define HW_IEEE80211 ARPHRD_IEEE80211
#define HW_IEEE802154 ARPHRD_IEEE802154
#define HW_INVALID ARPHRD_NONE
#endif /* __linux__ */

#define IANAIFTYPE_ETHERNETCSMACD 6
#define IANAIFTYPE_IEEE80211 71
#define IANAIFTYPE_PLC 174
#define IANAIFTYPE_MOCAVERSION1 236
#define IANAIFTYPE_OTHER 1


/**
 * @brief Ethernet frame header to handle htip header.
 */
struct ether_frame_t {
        struct ether_header eth_header;        /**< Ethernet header **/
        unsigned char payload[ETHER_MAX_LEN - ETHER_HDR_LEN];	/**< Payload in ethernet header **/
        ssize_t len;	/**< A length of ethernet header **/
        ssize_t payload_len;	/**< A length of payload **/
};

/**
 * @brief Enable promiscuous for a specified network interface.
 * @param interface_name A network interface name.
 * @return If succeed, it returns an opened file descriptor. If failed, it returns -1.
 */
int set_promiscuous_mode(const char *interface_name);

/**
 * @brief Get a network interface type from a network interface name.
 * @param ifname A network interface name.
 * @return If succeeded, it returns a type value (such as HW_ETHER). If failed, it returns HW_INVALID.
 */
u_int32_t get_iftype(const char *ifname);

#ifdef __APPLE__
/**
 * @brief Get a BPF buffer length.
 * @param bpf A bpf file descriptor.
 * @return If succeed, it returns a BPF buffer length. If failed, it returns -1.
 */
int get_bpf_buffer_len(int bpf);

/**
 * @brief Print BPF header.
 * @param bh A pointer for a bpf_hdr.
 */
void print_bpf_packet(struct bpf_hdr *bh);
#endif /* __APPLE__ */

/**
 * @brief Print src and dst
 * @param eh A pointer to struct ether_header.
 * @detail Print source and destination MAC address from ether_header.
 */
void print_ether_header(struct ether_header *eh);

/**
 * @brief Print ether_header.
 * @param eh A pointer to struct ether_header.
 */
void print_ether_type(struct ether_header *eh);

/**
 * @brief Check whether a frame matches htip frame format or not.
 * @param eh A pointer to a ethernet header
 * @return If it's htip frame, it returns 1. Otherwise, it returns -1.
 */
int is_htip_frame(struct ether_header *eh);

#ifdef __APPLE__
/**
 * @brief Open an available bpf file descriptor.
 * @return If succeed, it returns bpf file descriptor. If fialed, it returns -1.
 */
int open_devbpf();

/**
 * @brief Check whether a data link type is htip available or not.
 * @param fd A bpf file descriptor
 * @return If it's htip available file descriptor, it returns 0. Otherwise, it returns -1.
 */
int check_datalink_type(int fd);

/**
 * @brief Set options to bpf file descriptor.
 * @param fd A bpf file descriptor
 * @param ifr_name A network interface name
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int set_bpf_options(int fd, const char *ifr_name);

/**
 * @brief Receive any ethernet frames and print them.
 * @param fd A file descriptor
 */
void receive_all_frame(int fd);

/**
 * @brief Receive htip ethernet frames and print them.
 * @param fd A file descriptor
 */
void receive_htip_frame(int fd);
#endif /* __APPLE__ */

#ifndef ETH_DATA_LEN
#define ETH_DATA_LEN 1500
#endif /* ETH_DATA_LEN */

/* These may are defined in linux/if_ether.h */
#ifndef MAC_FMT
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#endif /* MAC_FMT */
#ifndef MAC_BUF_SIZE
#define MAC_BUF_SIZE 18
#endif /* MAC_BUF_SIZE */

/**
 * @brief Store a MAC address string to a specified pointer.
 * @param a An array of MAC address.
 * @param p A pointer to store a MAC address string.
 */
void ether_addr_str(const u_int8_t a[], char *p);

/**
 * @brief Compare two MAC addresses.
 * @param a1 An array of MAC address
 * @param a2 An array of MAC address
 * @return If two addresses are not same, it returns 0. If same, it returns a value not 0.
 */
int ether_addr_cmp(const u_int8_t a1[], const u_int8_t a2[]);

/**
 * @brief Write a data to a file descriptor.
 * @param fd BPF file descriptor
 * @param dst_mac Destination MAC address
 * @param src_mac Source MAC address
 * @param payload Sent payload content
 * @param payload_len Length of payload
 * @return Sent payload bytes.
 * @detail Write a payload to a network interface specified by a file descriptor.
 */
int write_frame(int bpf, char *ifname, u_char *dst_mac, u_char *src_mac,
        u_char *payload, u_int payload_len);

#ifdef __cplusplus
}
#endif

#endif /* DATALINK_H */
