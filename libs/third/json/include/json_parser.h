#ifndef __JSON_PARSER_H_
#define __JSON_PARSER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "jansson.h"

//#define ASSERT(x)	do { if (!(x)) { printf("Assert Error [%s->%d]!\n", __func__, __LINE__);exit(0); } } while (0);

//#define FREE			free
//#define MALLOC		malloc

#ifndef u8 
#define u8 unsigned char
#endif
#ifndef u16
#define u16 unsigned short
#endif
#ifndef _u64
#define _u64 unsigned long long
#endif
#ifndef bool
#define bool unsigned char
#endif
#ifndef _s64
#define _s64 long long
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define MIN_S32		INT32_MIN
#define MAX_S32		INT32_MAX
#define MAX_U32		UINT32_MAX
#define MAX_U8		UINT8_MAX
#define MAX_U16		UINT16_MAX




#ifdef __cplusplus 
extern "C" {
#endif

int json_get_bool(const json_t *obj, const char *name, bool *value);
int json_get_int(const json_t *obj, const char *name, int *value);
int json_get_int64(const json_t *obj, const char *name, _s64 *value);
int json_get_uint(const json_t *obj, const char *name, unsigned *value);
int json_get_uint8(const json_t *obj, const char *name, u8 *value);
int json_get_uint16(const json_t *obj, const char *name, u16 *value);
int json_get_uint64(const json_t *obj, const char *name, _u64 *value);
int json_get_double(const json_t *obj, const char *name, double *value);
const char *json_get_string(const json_t *obj, const char *name);
char *json_get_string_dup(const json_t *obj, const char *name);
ssize_t json_get_string_copy(const json_t *obj, const char *name,
	char *buf, size_t size);


#ifdef __cplusplus 
}
#endif



#endif
