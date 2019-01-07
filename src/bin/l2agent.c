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

const char *select_one(const char* first, const char *second) {
	return first?first:second;
}

u_char *get_device_category() {
    static u_char *device_category = NULL;
    if (device_category == NULL) {
	    device_category = strndup(select_one(getenv("DEVICE_CATEGORY"), "AV_TV"), 255);
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
	    model_name = strndup(select_one(getenv("MODEL_NAME"), "JAIST_VTV_01"), 31);
    }
    return model_name;
}

u_char *get_model_number() {
    static u_char *model_number = NULL;
    if (model_number == NULL) {
	    model_number = strndup(select_one(getenv("MODEL_NUMBER"), "VTV01"), 31);
    }
    return model_number;
}

int main(int argc, char **argv) {
        char *argv0;
        int c;
        /** HTIP device category, 0 ~ 255 bytes */
        u_char *device_category = get_device_category();
        /** HTIP manufacturer code, 6 bytes */
        u_char *manufacturer_code = get_manufacturer_code();
        /** HTIP model name, 0 ~ 31 bytes */
        u_char *model_name = get_model_name();
        /** HTIP model number, 0 ~ 31 bytes */
        u_char *model_number = get_model_number();

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

	printf("device_category: %s\n", device_category);
	printf("manufacturer_code: %s\n", manufacturer_code);
	printf("model_name: %s\n", model_name);
	printf("model_number: %s\n", model_number);

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
