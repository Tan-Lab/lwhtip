/**
 * @file   upnp.h
 * @brief An UPNP communication library.
 *
 * A header file of an UPNP communication library.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef UPNP_H
#define UPNP_H

#ifdef __cplusplus
extern "C" {
#endif

#define UPNP_MULTICAST_ADDR "239.255.255.250"
#define UPNP_PORT 1900

/**
 * @brief Open a socket following UPNP specification.
 * @return A socket file descriptor.
 */
int upnp_sock(void);

/**
 * @brief Dump received UPNP messages.
 */
void print_upnp(int sock);

/**
 * @brief Close an UPNP socket.
 * @return If succeed, it returns 0. If failed, it returns -1.
 */
int close_upnp(int sock);

/**
 * @brief Send an UPNP message.
 * @param send_buf Sending message buffer.
 * @param len A length of sending message buffer.
 * @src_address A source ip address.
 * @dst_address A destination ip address.
 * @port A port number.
 */
int send_upnp_message(char *send_buf, int len, char *src_address, char *dst_address, int port);
#ifdef __cplusplus
}
#endif

#endif /* UPNP_H */
