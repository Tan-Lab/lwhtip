/**
 * @file   ifinfo.h
 * @brief A library handling network interface information.
 *
 * A header file of a library that handle network interface information.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef IFINFO_H
#define IFINFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <net/ethernet.h>
#ifdef __linux__
#include <linux/if.h>
#endif /* __linux__ */

#include <netinet/in.h>
#include <ifaddrs.h>

#ifdef __APPLE__
#include <ifaddrs.h>
#include <net/if.h>
#endif /* __APPLE__ */

/**
 * @brief A structure of a network interface information.
 */
struct ifinfo {
        /** A file descriptor */
        int fd;
        /** A name of network interface */
        char ifname[IFNAMSIZ];
        /** An ip address */
        char ipaddr[INET6_ADDRSTRLEN];
        /** A netmask */
        char netmask[INET6_ADDRSTRLEN];
        /** A MAC address */
        u_char macaddr[ETHER_ADDR_LEN];
        /** A network interface type */
        u_int32_t iftype;
        /** A port number of network interface */
        u_int16_t port_no;
};

#define IFINFO_LEN sizeof(struct ifinfo)
#define IFINFO_LIST_MAX_SIZE 20
#define IFINFO_LIST_INVALID -1

/**
 * @brief Get a pointer to a head of ifinfo list.
 * @return a pointer to a head of ifinfo list.
 */
struct ifinfo *get_ifinfo_list(void);

/**
 * @brief Set a pointer to a head of ifinfo list.
 * @param p A pointer of ifinfo list
 */
void set_ifinfo_list(void *p);

/**
 * @brief Get a number of stored ifinfo.
 * @return A number of stored ifinfo.
 */
int get_ifinfo_list_num(void);

/**
 * @brief Set a number of stored ifinfo.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_list_num(int num);

/**
 * @brief Increment a number of stored ifinfo.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int increment_ifinfo_list_num(void);

/**
 * @brief Get a size how many ifinfo can be stored in the ifinfo list.
 * @return A size of ifinfo list.
 */
int get_ifinfo_list_size(void);

/**
 * @brief Set a size how many ifinfo can be stored in the ifinfo list.
 * @param size A size of ifinfo list.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_list_size(int size);

/**
 * @brief Get a pointer to ifinfo specified type by ifname.
 * @return If succeeded, it returns a pointer to a ifinfo specified type by ifname. If failed, it returns NULL.
 */
struct ifinfo *search_ifinfo_by_ifname(const char *ifname);

/**
 * @brief Get a pointer to an empty ifinfo .
 * @return If succeeded, it returns a pointer to an empty ifinfo. If failed, it returns NULL.
 */
struct ifinfo *get_empty_ifinfo(void);

/**
 * @brief Add a new ifinfo with a network interface name.
 * @param ifname A network interface name.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_list_ifname(char *ifname);

/**
 * @brief Set an IP address and netmask to ifinfo with ifname.
 * @param ifname A network interface name.
 * @param ipaddr An IP address.
 * @param netmask A netmaask.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_addr(char *ifname, char *ipaddr, char *netmask);

/**
 * @brief Set a MAC address to ifinfo with ifname.
 * @param ifname A network interface name.
 * @param macaddr A MAC address.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_hwaddr(char *ifname, u_char *macaddr);

/**
 * @brief Set a network interface type to ifinfo with ifname.
 * @param ifname A network interface name.
 * @param iftype A type of network interface.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 * @pre iftype is a value described in <net/if_arp.h>. For example, if a network interface is ethernet, iftype is 1 (ARPHRD_ETHER).
 */
int set_ifinfo_iftype(char *ifname, int iftype);

/**
 * @brief Set a port number of FDB to ifinfo with ifname.
 * @param ifname A network interface name.
 * @param port_no A port number of FDB.
 * @return If succeeded, it returns 0. If failed, it returns -1.
 */
int set_ifinfo_portno(char *ifname, u_int16_t port_no);

/**
 * @brief Allocate a memory of ifinfo list.
 * @param size A size of ifinfo list(number of struct ifinfo).
 * @return If succeeded, it returns an allocated pointer. If failed, it returns NULL.
 */
struct ifinfo *malloc_ifinfo_list(int size);

/**
 * @brief Free a memory of ifinfo list.
 */
void free_ifinfo_list(void);

/**
 * @brief Open a file descriptor with promiscuous mode for each ifinfo.
 * @retrun If succeeded, it returns 0. If failed, it returns -1.
 */
int open_netif(void);

/**
 * @brief Close all opened file descripters and free a memory of ifinfo list.
 */
void close_netif(void);

/**
 * @brief Print ifinfo list.
 */
void print_ifinfo(void);

#ifdef __linux__
#define IFNAME_LOOPBACK "lo"
#define IPv4_LOOPBACK     "127.0.0.1"
#define HWADDR_LOOPBACK "00:00:00:00:00:00"
/**
 * @brief Read network interface information and store it to ifinfo list.
 * @return If succeeded, it returns 0. If failed, it returns 0.
 */
int read_ifinfo(void);

void print_netif(struct ifreq *ifr, struct ifconf *ifc);

/**
 * @brief Check whether a specified ifinfo is loopback interface or not.
 * @param p A pointer of struct ifinfo.
 * @return If it's valid (not loopback interface) ifinfo, it returns 0. If invalid, it returns -1.
 */
int is_valid_netif(const struct ifinfo *p);
#endif /* __linux__ */

#ifdef __APPLE__
#define IFNAME_LOOPBACK "lo0"
#endif /* __APPLE__ */

/**
 * @brief Check whether a specified ifinfo is loopback interface or not.
 * @param p A pointer of a network interface name.
 * @return If it's valid (not loopback interface) ifinfo, it returns 0. If invalid, it returns -1.
 */
int is_valid_ifname(const char *p);

/**
 * @brief Check whether a specified ifinfo is IPv4 and unique or not.
 * @param ifname A pointer of a network interface name.
 * @param valid_ifnames An array of ifnames.
 * @param ifa A pointer to ifaddrs.
 * @return If it's valid, it returns 0. If invalid, it returns -1.
 */
int is_valid_ifaddr(struct ifaddrs *ifa);

/**
 * @brief Read network interface types and store them to ifinfo list.
 * @return If succeeded, it returns 0. If failed, it returns 0.
 */
int read_net_type(void);

#ifdef __APPLE__
/**
 * @brief Read network interface information using getifaddrs() and store them.
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int read_ifinfo(void);

/**
 * @brief Print network interface information using getifaddrs().
 */
void print_netif(void);

/**
 * @brief Count an available number of specified ifa_list.
 * @param ifa_list A pointer to ifa_list
 * @return A number of avaiable ifa_list
 */
int num_netif(struct ifaddrs *ifa_list);

/**
 * @brief Check whether a specified pointer is available or not.
 * @param ifa A pointer to struct ifaddrs
 * @return If it's available, it returns 0. If not, it returns -1.
 */
int is_available_ifaddr(struct ifaddrs *ifa);
#endif /* __APPLE__ */

#ifdef __cplusplus
}
#endif

#endif /* IFINFO_H */
