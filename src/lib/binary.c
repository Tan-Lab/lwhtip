/**
 * @file   binary.c
 * @brief A binary code library
 *
 * A source file of a library that treat binary code.
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
#include <ctype.h>


void hexdump(const char *h, const size_t len)
{
        char str[17];
        int r16, padlen;
        size_t i;

        for (i = 0, r16 = 0; i < len; i++, r16 = i % 16) {
                if (r16 == 0)
                        printf("%08x ", (unsigned int) i);
                if (i % 4 == 0)
                        printf(" ");

                printf("%02x", h[i]);
                str[r16] = isprint(h[i]) ? h[i] : '.';

                if (r16 == 15) {
                        str[16] = '\0';
                        printf("  %s\n", str);
                }
        }

        if (r16 != 0) {
                str[r16] = '\0';
                padlen = (16 - r16) * 2 + (16 - r16) / 4 + 1;
                printf("%*c %s\n", padlen, ' ', str);
        }
}


void print_hexdump(const char *p, const int len)
{
        int i, j;

        printf("--- hex dump --- size: %d\n", len);
        for (i = 0; i < len; i += 16) {
                printf("%08x", i);
                for (j = i; j < i + 16; j++) {
                        if (j < len)
                                printf(" %02hhx", p[j]);
                        else
                                printf(" --");
                }
                printf(" |");
                for (j = i; j < i + 16; j++) {
                        if (j < len) {
                                if ((p[j] > 0x1f) && (p[j] < 0x7f))
                                        printf("%1c", p[j]);
                                else
                                        printf(".");
                        } else {
                                printf(" ");
                        }
                }
                printf("|\n");
        }
        printf("----------------\n");
}


void print_hexlstr(const char *p, const int len)
{
        int i;

        for (i = 0; i < len; i++)
                printf(" %02hhx", p[i]);
}
