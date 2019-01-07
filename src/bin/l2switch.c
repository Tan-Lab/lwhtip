/**
 * @file l2switch.c
 * @brief: An implementation of JJ-300.00v3 HTIP-NW.
 *
 * HTIP-NW is a deamon.
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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>

#include "fdb.h"
#include "htip.h"
#include "ifinfo.h"

void usage(char *argv0)
{
        printf("Usage: %s -i {bridge_network_interface_name}\n", argv0);
}

void signal_handler(int sig)
{
        printf("Catch signal: %d\n", sig);

        free_ifinfo_list();
        free_fdb_entry();

        return;
}

const char *select_one(const char* first, const char *second) {
	return first?first:second;
}

u_char *get_device_category() {
    static u_char *device_category = NULL;
    if (device_category == NULL) {
	    device_category = strndup(select_one(getenv("DEVICE_CATEGORY"), "COM_Switch"), 255);
    }
    return device_category;
}

u_char *get_manufacturer_code() {
    static u_char *manufacturer_code = NULL;
    if (manufacturer_code == NULL) {
	    manufacturer_code = strndup(select_one(getenv("MANUFACTURER_CODE"), "JAIST"), 6);
    }
    return manufacturer_code;
}

u_char *get_model_name() {
    static u_char *model_name = NULL;
    if (model_name == NULL) {
	    model_name = strndup(select_one(getenv("MODEL_NAME"), "JAIST_VSW_01"), 31);
    }
    return model_name;
}

u_char *get_model_number() {
    static u_char *model_number = NULL;
    if (model_number == NULL) {
	    model_number = strndup(select_one(getenv("MODEL_NUMBER"), "VSW01"), 31);
    }
    return model_number;
}

int main(int argc, char** argv) {
        char *argv0 = NULL, *brifname = NULL;
        int c;
        /* 0 ~ 255 bytes */
        u_char *device_category = get_device_category();
        /* 6 bytes */
        u_char *manufacturer_code = get_manufacturer_code();
        /* 0 ~ 31 bytes */
        u_char *model_name = get_model_name();
        /* 0 ~ 31 bytes */
        u_char *model_number = get_model_number();

        argv0 = argv[0];
        while ((c = getopt(argc, argv, "i:l:")) != -1) {
                switch (c) {
                        case 'i':
                                brifname = optarg;
                                break;
                        case '?':
                        default:
                                usage(argv0);
                                exit(EXIT_FAILURE);
                }
        }
        argc -= optind;
        argv += optind;

        if (argc != 0) {
                usage(argv0);
                err(EXIT_FAILURE, "main");
        }

        if (brifname == NULL) {
                fprintf(stderr, "bridge network interface were not set.\n");
                usage(argv0);
                exit(EXIT_FAILURE);
        }

        if (signal(SIGINT, signal_handler) == SIG_ERR) {
                fprintf(stderr, "signal got SIG_ERR\n");
                goto finalize;
        }

	printf("device_category: %s\n", device_category);
	printf("manufacturer_code: %s\n", manufacturer_code);
	printf("model_name: %s\n", model_name);
	printf("model_number: %s\n", model_number);

        u_char *srcaddr = alloc_brifaddr(brifname);

        for (;;) {
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
                if (load_fdb(brifname, MAX_FDB_ENTRY_SIZE) == -1) {
                        fprintf(stderr, "load_fdb() failed.\n");
                        goto finalize;
                }

                if (send_htip_device_link_info(device_category,
                        sizeof(device_category), manufacturer_code, model_name,
                        sizeof(model_name), model_number, sizeof(model_number), alloc_brifaddr(brifname))
                        < 0) {
                        fprintf(stderr, "send_htip_device_link_info() failed\n");
                        goto finalize;
                }

                close_netif();

                free_fdb_entry();

                sleep(30);
        }

finalize:
        free_ifinfo_list();

        free_fdb_entry();

        return (EXIT_SUCCESS);
}
