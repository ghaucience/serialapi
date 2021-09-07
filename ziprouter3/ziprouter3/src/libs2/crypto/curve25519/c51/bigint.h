/* © 2017 Silicon Laboratories Inc.
 */
#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>

/* Arithmetic on big integers represented as arrays of unsigned char in radix 2^256 */

/*
#define bigint_add avrnacl_bigint_add
#define bigint_sub avrnacl_bigint_sub
#define bigint_mul avrnacl_bigint_mul
#define bigint_mul32 avrnacl_bigint_mul32
#define bigint_cmov avrnacl_bigint_cmov
*/

uint8_t bigint_add(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b, uint8_t len);

uint8_t bigint_sub(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b, uint8_t len);

void bigint_mul(unsigned char xdata *r, const unsigned char xdata *a, const unsigned char xdata *b, uint8_t len);

void bigint_mul32(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b);

void bigint_cmov(uint8_t xdata *r, const uint8_t xdata *x, uint8_t b, uint8_t len);

#endif
