/**
 * @file   htip.h
 * @brief A library handling HTIP frame.
 *
 * A header file of a library that handle HTIP frame.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef HTIP_H
#define HTIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define HTIP_L2AGENT_DST_MACADDR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

/**
 * @brief Send a HTIP device information with specified parameters.
 * @param device_category A pointer to device category
 * @param device_category_len A length of a device category, the max length is 255
 * @param manufacturer_code A pointer to a manufacturer code, the length is 6
 * @param model_name A pointer to a model name
 * @param model_name_len A length of a model name, the max length is 31
 * @param mdoel_number A pointer to a model number
 * @param model_number_len A length of a model number, the max length is 31
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int send_htip_device_info(u_char *device_category, int device_category_len,
        u_char *manufacturer_code, u_char *model_name, int model_name_len,
        u_char *model_number, int model_number_len);

/**
 * @brief Send a HTIP link information.
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int send_htip_link_info(void);

/**
 * @brief Send a HTIP device information and HTIP link information at once.
 * @param device_category A pointer to device category
 * @param device_category_len A length of a device category, the max length is 255
 * @param manufacturer_code A pointer to a manufacturer code, the length is 6
 * @param model_name A pointer to a model name
 * @param model_name_len A length of a model name, the max length is 31
 * @param mdoel_number A pointer to a model number
 * @param model_number_len A length of a model number, the max length is 31
 * @param srcaddr Source address of the HTIP frame
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int send_htip_device_link_info(u_char *device_category,
        int device_category_len, u_char *manufacturer_code, u_char *model_name,
        int model_name_len, u_char *model_number, int model_number_len, u_char *srcaddr);
#ifdef __cplusplus
}
#endif

#endif /* HTIP_H */
