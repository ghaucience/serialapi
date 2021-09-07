#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef uint8 
#define uint8 unsigned char
#define u8 unsigned char
#endif
#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef int8 
#define int8 char
#endif
#ifndef int8_t
#define int8_t char
#define s8 char
#endif
#ifndef uint16
#define uint16 unsigned short
#endif
#ifndef uint16_t
#define uint16_t unsigned short
#define u16 unsigned short
#endif
#ifndef int16 
#define int16 short
#endif
#ifndef int16_t
#define int16_t short
#endif
#ifndef uint32 
#define uint32 unsigned int
//#define u32 unsigned int
#endif
#ifndef uint32_t
#define int32_t unsigned int
#endif
#ifndef int32 
#define int32 int
#endif
#ifndef int32_t
#define int32_t int
#endif
#ifndef uint64
#define uint64 unsigned long long
#endif
#ifndef uint64_t
#define uint64_t unsigned long long
#define u64 unsigned long long
#endif
#ifndef int64 
#define int64 long long
#define s64 long long
#endif
#ifndef int64_t
#define int64_t long long
#endif
#ifndef bool
#define bool unsigned int 
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define HI_UINT16(a) ((uint8)(((a) >> 8) & 0xFF))
#define LO_UINT16(a) ((uint8)(((a) & 0xFF)))
#define BUILD_UINT16(loByte, hiByte)  ((uint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define ASSERT(x) do { if (!(x)) exit(0); } while (0)

#define MAJOR    1
#define MINOR    0
#define PATCH    30
#define RELEASE  1
#define DATE		 __DATE__
#define TIME		 __TIME__
#define VERSION_INT() (RELEASE | (PATCH << 8) | (MINOR << 16) | (MAJOR << 24))
#define VERSION_STR() ("V"##MAJOR##"."##MINOR##"."##PATCH##"."##RELEASE)

#define MALLOC(size) malloc(size)
#define FREE(p) free(p)

#ifdef __cplusplus
}
#endif

#endif
