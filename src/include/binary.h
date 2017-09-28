/**
 * @file   binary.h
 * @brief A binary code library
 *
 * A header file of a library that treat binary code.
 *
 * @author Takashi OKADA
 * @date 2017.09.30
 * @version 0.1
 * @copyright 2017 Takashi OKADA. All rights reserved.
 *
 * @par ChangeLog:
 * - 2017.09.30: Takashi OKADA: Created.
 */

#ifndef BINARY_H
#define BINARY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Print hex dump strings.
 * @param h A pointer to head to print.
 * @param len A length of strings.
 */
void hexdump(const char *h, const size_t len);

/**
 * @brief Print hex dump strings.
 * @param h A pointer to head to print.
 * @param len A length of strings.
 */
void print_hexdump(const char *p, const int len);

/**
 * @brief Print hex strings.
 * @param h A pointer to head to print.
 * @param len A length of strings.
 */
void print_hexlstr(const char *p, const int len);
#ifdef __cplusplus
}
#endif

#endif /* BINARY_H */
