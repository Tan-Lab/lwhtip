/**
 * @file   fdb.c
 * @brief A binary code library
 *
 * A source file of a library that handle FDB (Forwarding DataBase) of layer 2 network switch.
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
#include <sys/socket.h>

#ifdef __linux__
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#endif

#ifdef __APPLE__
#include <netinet/if_ether.h>
#endif /* __APPLE__ */

#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>

#ifdef __linux__
#include <linux/sockios.h>
#endif

#include <errno.h>
#include <inttypes.h>
#include <dirent.h>
#include <unistd.h>

#include "datalink.h"
#include "fdb.h"

/* global variables */
/** A list of FDB entry, currently the size is fixed. */
struct fdb_entry fdb_entry_list[MAX_FDB_ENTRY_SIZE];
/** A number of FDB entry in current FDB entry list */
int fdb_entry_num = FDB_ENTRY_LIST_INVALID;
/** A size of FDB entry list */
int fdb_entry_size = FDB_ENTRY_LIST_INVALID;

struct fdb_entry *get_fdb_entry_list(void)
{
        return fdb_entry_list;
}

void set_fdb_entry_list(void *p)
{
}

int get_fdb_entry_num(void)
{
        return fdb_entry_num;
}

int set_fdb_entry_num(int num)
{
        if (num <= FDB_ENTRY_LIST_INVALID || num > MAX_FDB_ENTRY_SIZE) {
                fprintf(stderr, "Invalid FDB entry number: %d\n", num);
                return -1;
        }

        fdb_entry_num = num;

        return 0;
}

int get_fdb_entry_size(void)
{
        return fdb_entry_size;
}

int set_fdb_entry_size(int size)
{
        if (size <= FDB_ENTRY_LIST_INVALID || size > MAX_FDB_ENTRY_SIZE) {
                fprintf(stderr, "Invalid FDB entry size: %d\n", size);
                return -1;
        }

        fdb_entry_size = size;

        return 0;
}

void *malloc_fdb_entry(const int size)
{
        void *p = (void *) fdb_entry_list;

        if ((size < 1) || (size > MAX_FDB_ENTRY_SIZE)) {
                fprintf(stderr, "invalid FDB entry size: %d\n", size);
                return NULL;
        }

        memset(fdb_entry_list, 0, MAX_FDB_ENTRY_SIZE * FDB_ENTRY_LEN);

        if (set_fdb_entry_num(0) == -1) {
                fprintf(stderr, "set_fdb_entry_num() failed.\n");
                return NULL;
        }

        if (set_fdb_entry_size(size) == -1) {
                fprintf(stderr, "set_fdb_entry_size() failed.\n");
                return NULL;
        }

        return p;
}

void free_fdb_entry(void)
{
        memset(fdb_entry_list, 0, MAX_FDB_ENTRY_SIZE * FDB_ENTRY_LEN);
        fdb_entry_num = FDB_ENTRY_LIST_INVALID;
        fdb_entry_size = FDB_ENTRY_LIST_INVALID;
}

int add_fdb_entry(const struct fdb_entry *fdbp)
{
        struct fdb_entry *p;
        int i = 0, n = get_fdb_entry_num();

        if (n >= MAX_FDB_ENTRY_SIZE) {
                fprintf(stderr, "FDB entry already full.\n");
                return -1;
        }

        if (exist_fdb_entry(fdbp) == 1) {
                fprintf(stderr, "Specified FDB entry already exist. n: %d\n", n);
                return -1;
        }

        p = &fdb_entry_list[n];
        memcpy(p, fdbp, FDB_ENTRY_LEN);

        if (set_fdb_entry_num(n + 1) == -1) {
                fprintf(stderr, "set_fdb_entry_num() failed.\n");
                return -1;
        }

        return 0;
}

int exist_fdb_entry(const struct fdb_entry *fdbp)
{
        struct fdb_entry *p;
        int i, n = get_fdb_entry_num();

        for (i = 0; i < n; i++) {
                p = &fdb_entry_list[i];
                /* Skip same MAC address and port number */
                if (ether_addr_cmp(p->macaddr, fdbp->macaddr) &&
                                p->port_no == fdbp->port_no)
                        return 1;
        }

        return 0;
}

u_int16_t get_portno_by_macaddr(const u_int8_t macaddr[])
{
        struct fdb_entry *p;
        int i, n = get_fdb_entry_num();

        for (i = 0; i < n; i++) {
                p = &fdb_entry_list[i];
                if (ether_addr_cmp(p->macaddr, macaddr) && (p->is_local == FDB_ENTRY_PORT_IS_LOCAL))
                        return p->port_no;
        }

        return FDB_ENTRY_PORT_INVALID;
}

int get_remote_entry_num_by_macaddr(const u_int8_t macaddr[], u_int8_t *macaddrs[])
{
        const u_int16_t port_no = get_portno_by_macaddr(macaddr);

        if (port_no == FDB_ENTRY_PORT_INVALID)
                return 0;

        return get_remote_entry_num_by_portno(port_no, macaddrs);
}

int get_remote_entry_num_by_portno(const u_int16_t port_no, u_int8_t *macaddrs[])
{
        struct fdb_entry *p;
        int i, n = get_fdb_entry_num(), c = 0;

        for (i = 0; i < n; i++) {
                p = &fdb_entry_list[i];
                if ((p->port_no == port_no) && (p->is_local == FDB_ENTRY_PORT_NOT_LOCAL)) {
                        macaddrs[c] = p->macaddr;
                        c += 1;
                }
        }

        return c;
}

#ifdef __linux__
static inline void jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
        unsigned long long tvusec;
        tvusec = 10000ULL * jiffies;
        tv->tv_sec = tvusec / 1000000;
        tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}
#endif /* __linux__ */

#ifdef __linux__
static inline void copy_fdb(struct fdb_entry *ent,
                              const struct __fdb_entry *f)
{
        memcpy(ent->macaddr, f->mac_addr, ETHER_ADDR_LEN);
        ent->port_no = f->port_no;
        ent->is_local = f->is_local;
        jiffies_to_tv(&ent->ageing_timer_value, f->ageing_timer_value);
}
#endif /* __linux__ */

int read_fdb(const char *bridge_name)
{
#ifdef __linux__
        FILE *f;
        int sock;
        int i, n;
        int retries = 0;
        char path[SYSFS_PATH_MAX];
        struct ifreq ifr;
        struct fdb_entry *fdbs, fdb;
        unsigned long offset = 0;
        int num = MAX_FDB_ENTRY_SIZE;
        struct __fdb_entry fe[num];
        unsigned long args[4] = {BRCTL_GET_FDB_ENTRIES,
                                 (unsigned long) fe, num, offset};

        if (snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brforward", bridge_name) < 0) {
                fprintf(stderr, "snprintf() failed.\n");
                return -1;
        }

        if ((f = fopen(path, "r")) == NULL) {
                perror("fopen");
                return -1;
        }

        if (f) {
                if (fseek(f, offset * sizeof(struct __fdb_entry), SEEK_SET) == -1) {
                        perror("fseek");
                        return -1;
                }

                n = fread(fe, sizeof(struct __fdb_entry), num, f);

                if (fclose(f) == EOF) {
                        perror("fclose");
                        return -1;
                }
        } else {
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("socket");
                        return errno;
                }

                strncpy(ifr.ifr_name, bridge_name, IFNAMSIZ);
                ifr.ifr_data = (char *) args;

                retry:
                        n = ioctl(sock, SIOCDEVPRIVATE, &ifr);

                        if (n < 0 && errno == EAGAIN && ++retries < 10) {
                                sleep(0);
                                goto retry;
                        }

                close(sock);
        }

        for (i = 0; i < n; i++) {
                copy_fdb(&fdb, &fe[i]);

                if (add_fdb_entry(&fdb) < 0) {
                        fprintf(stderr, "add_fdb_entry() failed.\n");
                }
        }

        return n;
#endif /* __linux__ */
        return 0;
}

int load_fdb(const char *brname, const int size)
{
        struct fdb_entry *p;

        if (get_fdb_entry_list() != NULL) {
                free_fdb_entry();
        }

        if ((p = malloc_fdb_entry(size)) == NULL) {
                fprintf(stderr, "malloc_fdb_entry() failed.\n");
                return -1;
        }

        if (read_fdb(brname) == -1) {
                fprintf(stderr, "read_fdb() failed.\n");
                return -1;
        }

        return 0;
}

void print_fdb(struct fdb_entry *fdbs, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        printf("port: %" PRIu16 "", fdbs->port_no);
        printf(", MAC: %02x:%02x:%02x:%02x:%02x:%02x",
               (unsigned char) fdbs->macaddr[0],
               (unsigned char) fdbs->macaddr[1],
               (unsigned char) fdbs->macaddr[2],
               (unsigned char) fdbs->macaddr[3],
               (unsigned char) fdbs->macaddr[4],
               (unsigned char) fdbs->macaddr[5]
        );
        printf(", local: %x", fdbs->is_local);
        printf(", age: %ld.%06ld\n",
               fdbs->ageing_timer_value.tv_sec,
               (long) fdbs->ageing_timer_value.tv_usec);
        fdbs++;
    }
}

void print_fdb_entry(void)
{
        int i;
        struct fdb_entry *p;

        for (i = 0; i < fdb_entry_num; i++) {
                p = &fdb_entry_list[i];
                print_fdb(p, 1);
        }
}
