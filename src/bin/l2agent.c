/**
 * @file l2agent.c
 * @brief: An implementation of JJ-300.00v3 L2Agent.
 * 
 * L2Agent is a deamon.
 * 
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 * 
 * @par ChangeLog:
 * - 2017/09/30: Takashi OKADA: Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/ethernet.h>

#ifdef __APPLE__
#include <net/bpf.h>
#endif /* __APPLE__ */

#ifdef __linux__
#include <signal.h>
#endif /* __linux__ */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <err.h>

#include "binary.h"
#include "datalink.h"
#include "fdb.h"
#include "ifinfo.h"
#include "tlv.h"
#include "htip.h"

void usage(char *argv0)
{
        printf("Usage: %s -i {network_interface_name}\n", argv0);
}

void signal_handler(int sig)
{
        printf("Catch signal: %d\n", sig);

        close_netif();

        free_ifinfo_list();

        exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
        char *argv0;
        int c;
        /** HTIP device category, 0 ~ 255 bytes */
        u_char device_category[] = "AV_TV";
        /** HTIP manufacturer code, 6 bytes */
        u_char manufacturer_code[] = "JAIST";
        /** HTIP model name, 0 ~ 31 bytes */
        u_char model_name[] = "JAIST_VTV_01";
        /** HTIP model number, 0 ~ 31 bytes */
        u_char model_number[] = "VTV01";

        argv0 = argv[0];

        while ((c = getopt(argc, argv, "i:l:")) != -1) {
                switch (c) {
                case 'i':
                        break;
                case '?':
                default:
                        usage(argv0);
                        exit(EXIT_SUCCESS);
                }
        }

        argc -= optind;
        argv += optind;

        if (argc != 0) {
                usage(argv0);
                err(EXIT_FAILURE, "main");
        }

        if (signal(SIGINT, signal_handler) == SIG_ERR) {
                goto finalize;
        }

        /* store network interface information */
        if (read_ifinfo() < 0) {
                fprintf(stderr, "read_ifinfo() failed\n");
                return (EXIT_FAILURE);
        }

        /* store network interface type */
        if (read_net_type() == -1) {
                fprintf(stderr, "read_net_type() failed.\n");
                goto finalize;
        }

        /* get network interfaces */
        if (open_netif() < 0) {
                fprintf(stderr, "get_netif_osx() failed\n");
                goto finalize;
        }

        /* check stored network interface list */
        print_ifinfo();

        /* main loop: send HTIP frame every 30 seconds */
        for (;;) {
                if (send_htip_device_info(device_category,
                        sizeof(device_category), manufacturer_code, model_name,
                        sizeof(model_name), model_number, sizeof(model_number))
                        < 0) {
                        fprintf(stderr, "send_htip_device_info() failed\n");
                        goto finalize;
                }
                printf("sent htip device info\n");
                sleep(30);
        }

        goto finalize;

finalize:
        close_netif();

        free_ifinfo_list();

        return (EXIT_SUCCESS);
}
