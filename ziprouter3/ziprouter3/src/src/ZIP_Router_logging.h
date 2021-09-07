/* © 2017 Silicon Laboratories Inc.
 */
/*
 * ZIP_Router_logging.h
 *
 *  Created on: Aug 11, 2015
 *      Author: aes
 */

#ifndef SRC_ZIP_ROUTER_LOGGING_H_
#define SRC_ZIP_ROUTER_LOGGING_H_



#if defined(WIN32)
#define LOG_PRINTF(f, ...) printf(f,  __VA_ARGS__ )
#define ERR_PRINTF(f, ...) printf(f,  __VA_ARGS__ )
#define WRN_PRINTF(f, ...) printf(f,  __VA_ARGS__ )
#define DBG_PRINTF(f, ...) printf(f,  __VA_ARGS__ )
#elif defined(__ASIX_C51__)
#define LOG_PRINTF
#define ERR_PRINTF printf
#define WRN_PRINTF printf
#define DBG_PRINTF
#elif defined(__BIONIC__)
#include <android/log.h>
#define LOG_PRINTF(f, ...) __android_log_print(ANDROID_LOG_INFO , "zgw", f , ## __VA_ARGS__ )
#define ERR_PRINTF(f, ...) __android_log_print(ANDROID_LOG_ERROR, "zgw", f , ## __VA_ARGS__ )
#define WRN_PRINTF(f, ...) __android_log_print(ANDROID_LOG_WARN , "zgw", f , ## __VA_ARGS__ )
#define DBG_PRINTF(f, ...) __android_log_print(ANDROID_LOG_DEBUG, "zgw", f , ## __VA_ARGS__ )
#elif defined(SYSLOG)
#include <syslog.h>

#define LOG_PRINTF(f, ...) syslog(LOG_INFO , f , ## __VA_ARGS__ )
#define ERR_PRINTF(f, ...) syslog(LOG_ERR,  f , ## __VA_ARGS__ )
#define WRN_PRINTF(f, ...) syslog(LOG_WARNING , f , ## __VA_ARGS__ )
#define DBG_PRINTF(f, ...) syslog(LOG_DEBUG,  f , ## __VA_ARGS__ )


#else
#include <stdio.h>
#include "sys/clock.h"
/**
 * Information level logging.
 * \param f argument similar to the one passed to printf
 */
#define LOG_PRINTF(f, ...) printf("\033[32;1m%lu " f "\033[0m",  clock_time(), ## __VA_ARGS__ )
/**
 * Error level logging.
 * \param f argument similar to the one passed to printf
 */
#define ERR_PRINTF(f, ...) printf("\033[31;1m%lu " f "\033[0m",  clock_time(), ## __VA_ARGS__ )
/**
 * Warning level logging.
 * \param f argument similar to the one passed to printf
 */
#define WRN_PRINTF(f, ...) printf("\033[33;1m%lu " f "\033[0m",  clock_time(), ## __VA_ARGS__ )
/**
 * Debug level logging.
 * \param f argument similar to the one passed to printf
 */
#define DBG_PRINTF(f, ...) printf("\033[34;1m%lu " f "\033[0m", clock_time(), ## __VA_ARGS__ )
#endif


#endif /* SRC_ZIP_ROUTER_LOGGING_H_ */
