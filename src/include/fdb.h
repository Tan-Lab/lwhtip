/**
 * @file   fdb.h
 * @brief A binary code library
 *
 * A header file of a library that handle FDB (Forwarding DataBase) of layer 2 network switch.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef FDB_H
#define FDB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
#include <netinet/in.h>
#endif
#include <sys/time.h>
#include <net/ethernet.h>

#define SYSFS_CLASS_NET "/sys/class/net/"
#define SYSFS_PATH_MAX 256
#define MAX_FDB_ENTRY_SIZE 256
#define FDB_ENTRY_PORT_IS_LOCAL 1
#define FDB_ENTRY_PORT_NOT_LOCAL 0
#define FDB_ENTRY_LIST_INVALID -1
#define FDB_ENTRY_PORT_INVALID 0xFFFF

/**
 * @brief FDB (Forwarding DataBase) entry
 */
struct fdb_entry {
        /** MAC address */
        u_int8_t macaddr[ETHER_ADDR_LEN];
        /** A port number of network interface */
        u_int16_t port_no;
        /** A flag whether the entry is local port or not */
        unsigned char is_local;
        /** An expiring timer */
        struct timeval ageing_timer_value;
};
#define FDB_ENTRY_LEN sizeof(struct fdb_entry)

/**
 * @brief Get a pointer to a head of FDB entry list.
 * @return a pointer to a head of FDB entry list.
 */
struct fdb_entry *get_fdb_entry_list(void);

/**
 * @brief Set a pointer to a head of FDB entry list.
 * @param p A pointer to a head of FDB entry list.
 */
void set_fdb_entry_list(void *p);

/**
 * @brief Get a number of FDB entry.
 * @return a number of FDB entry.
 */
int get_fdb_entry_num(void);

/**
 * @brief Set a number of FDB entry.
 * @param num A number of FDB entry.
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int set_fdb_entry_num(int num);

/**
 * @brief Get a size of FDB entry.
 * @return a size of FDB entry.
 */
int get_fdb_entry_size(void);

/**
 * @brief Set a size of FDB entry.
 * @param num A size of FDB entry.
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int set_fdb_entry_size(int size);

/**
 * @brief Allocate a memory with specified size of FDB entry.
 * @param size A size of FDB entry.
 * @return a pointer to a head of FDB entry list.
 */
void *malloc_fdb_entry(const int size);

/**
 * @brief Free a memory of FDB entry.
 */
void free_fdb_entry(void);

/**
 * @brief Add a new FDB entry to FDB entry list.
 * @param fdbp A pointer to a FDB entry.
 * @return If a new FDB entry is added to FDB entry list, it returns 0. If failed, it returns -1;
 */
int add_fdb_entry(const struct fdb_entry *fdbp);

/**
 * @brief Check whether specified FDB entry exists in FDB entry list.
 * @param fdbp A pointer to a FDB entry.
 * @return If same FDB entry exists in FDB entry list, it returns 1. If not, it returns 0;
 */
int exist_fdb_entry(const struct fdb_entry *fdbp);

/**
 * @brief Get a port number matching specified MAC address in FDB entry list.
 * @param macaddr A pointer to a FDB entry.
 * @return If found matched MAC address entry, it returns a port number. If not, it returns FDB_ENTRY_PORT_INVALID;
 */
u_int16_t get_portno_by_macaddr(const u_int8_t macaddr[]);

/**
 * @brief Count FDB entries matching with specified MAC address, store a pointer of matched MAC address to specified list.
 * @param macaddr A MAC address.
 * @return A number of counted FDB entries.
 */
int get_remote_entry_num_by_macaddr(const u_int8_t macaddr[], u_int8_t *macaddrs[]);

/**
 * @brief Count FDB entries matching with specified port number, store a pointer of matched MAC address to specified list.
 * @param port_no A port number
 * @return A number of counted FDB entries.
 */
int get_remote_entry_num_by_portno(const u_int16_t port_no, u_int8_t *macaddrs[]);

/**
 * @brief Get current forwarding database entries and return the number of entries.
 * @param bridge_name a poiter to bridge name.
 * @return If succeed, it returns a number of read entries. If failed, it returns -1.
 */
int read_fdb(const char *bridge_name);

/**
 * @brief Load current forwarding database entries.
 * @param brname A poiter to bridge name.
 * @param size A size of fdb entry buffer (number of struct fdb_entry).
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int load_fdb(const char *brname, const int size);

/**
 * @brief Print forwarding database entries from specified point.
 * @param fdbs a pointer to fdb entries buffer.
 * @param n a number of printed fdb.
 * @pre If the specified n is too large, this cause segmentation fault.
 * Please set correct pointer(fdbs) and n.
 */
void print_fdb(struct fdb_entry *fdbs, int n);

/**
 * @brief Print all FDB list.
 */
void print_fdb_entry(void);

#ifdef __cplusplus
}
#endif

#endif /* FDB_H */
