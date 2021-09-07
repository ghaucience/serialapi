/* © 2017 Silicon Laboratories Inc.
 */
/*********************************  types.h  ********************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Z-Wave, the wireless language.
 *
 *              Copyright (c) 2015
 *              Sigma Designs, Inc.
 *              Denmark
 *
 *              All Rights Reserved
 *
 *    This source file is subject to the terms and conditions of the
 *    Zensys Software License Agreement which restricts the manner
 *    in which it may be used.
 *
 *---------------------------------------------------------------------------
 *
 * Description: Type definitions for transport service.
 *     Generic defines for the protocol types BYTE, WORD etc.
 *
 * Author:
 *
 ****************************************************************************/

/****************************************************************************/
/*                      PUBLIC TYPES and DEFINITIONS                        */
/****************************************************************************/

#ifndef _TRANSPORT_SERVICE_TYPES_H_
#define _TRANSPORT_SERVICE_TYPES_H_
#ifdef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <stdint.h>
#include <ZW_typedefs.h>
#if !defined(ZIPGW)
#include <s2_protocol.h>
#endif
#include <ZW_classcmd.h>
#ifdef __C51__
#define ZW_SLAVE_ENHANCED_232  // Defined to pull in ZW_SendDataEx from ZW_transport_api.h
#include <ZW_transport_api.h>
#endif /* __C51__ */

//typedef signed char CHAR;
//typedef unsigned char UCHAR;
//typedef int INT;
//typedef unsigned int UINT;
//typedef unsigned long ULONG;
//
//typedef bit BOOL;
//typedef unsigned char BYTE;
//typedef unsigned short WORD;
//typedef signed short INT16;
//typedef unsigned long DWORD;
//typedef signed long LONG;
//typedef float FLOAT;
//typedef double DOUBLE;
//
//typedef	 BYTE	* BYTE_P;

typedef unsigned long u32;
typedef unsigned short u16;


/* Transmit frame option flags */
#define TRANSMIT_OPTION_ACK           0x01    /* request acknowledge from destination node */
#define TRANSMIT_OPTION_LOW_POWER     0x02    /* transmit at low output power level (1/3 of normal RF range)*/
#define TRANSMIT_OPTION_AUTO_ROUTE    0x04    /* request retransmission via repeater nodes */
/* do not use response route - Even if available */
#define TRANSMIT_OPTION_NO_ROUTE      0x10
/* Use explore frame if needed */
#define TRANSMIT_OPTION_EXPLORE       0x20

/**
 * A response route is locked by the application
 */
#define RECEIVE_STATUS_ROUTED_BUSY    0x01
/**
 * Received at low output power level, this must
 * have the same value as TRANSMIT_OPTION_LOW_POWER
 */
#define RECEIVE_STATUS_LOW_POWER      0x02
/**
 * Mask for masking out the received frametype bits
 */
#define RECEIVE_STATUS_TYPE_MASK      0x0C
/**
 * Received frame is singlecast frame (rxOptions == xxxx00xx)
 */
#define RECEIVE_STATUS_TYPE_SINGLE    0x00
/**
 * Received frame is broadcast frame  (rxOptions == xxxx01xx)
 */
#define RECEIVE_STATUS_TYPE_BROAD     0x04
/**
 * Received frame is multicast frame (rxOptions == xxxx10xx)
 */
#define RECEIVE_STATUS_TYPE_MULTI     0x08
/**
 * Received frame is an explore frame (rxOptions == xxx1xxxx)
 * Only TYPE_BROAD can be active at the same time as TYPE_EXPLORE
 */
#define RECEIVE_STATUS_TYPE_EXPLORE   0x10
/**
 * Received frame is not send to me (rxOptions == x1xxxxxx)
 * - useful only in promiscuous mode
 */
#define RECEIVE_STATUS_FOREIGN_FRAME  0x40


#ifndef __C51__
/****************************************************************************/
/* The security key a frame was received with or should be sent with.
 *
 * Special values:
 *     SECURITY_KEY_HIGHEST: Send the frame with the highest (most secure?)
 *                           key this node supports and/or has been included with.
*/
typedef enum SECURITY_KEY
{
  SECURITY_KEY_NONE = 0x00,
  SECURITY_KEY_S2_UNAUTHENTICATED = 0x01,
  SECURITY_KEY_S2_AUTHENTICATED = 0x02,
  SECURITY_KEY_S2_ACCESS = 0x03,
  SECURITY_KEY_S0 = 0x08
} security_key_t;
#endif /* ifndef __C51__ */

#if !defined(ZIPGW)
/**
 * Structure describing how a package was received / should be transmitted
 */
typedef struct ts_param
{
  /**
   * Source node
   */
  uint8_t snode;
  /**
   * Destination node
   */
  uint8_t dnode;

  /**
   * Source endpoint
   */
  uint8_t sendpoint;

  /**
   * Destination endpoint
   */
  uint8_t dendpoint;

  /**
   * Transmit flags
   * see txOptions in \ref ZW_SendData
   */
  uint8_t tx_flags;

  /**
   * Receive flags
   * see rxOptions in \ref ApplicationCommandHandler
   */
  uint8_t rx_flags;

  /**
   * Security scheme used for this package
   */
  uint8_t scheme; // Security scheme
} ts_param_t;

#include <transport_service2_external.h>
#endif

#ifdef __C51__

#define VOID_CALLBACKFUNC_PVOID(func, puser) void func(void* puser);\
code const void (code * func##_p)(void* puser) = &##func;\
void func(void *puser)

typedef void (code *ZW_CommandHandler_Callback_t)(ts_CommandHandler_t *pCmdStruct);
typedef void (code *ZW_TransportService_SendData_Callback_t)(uint8_t txStatus, void *t);

extern XBYTE nodeID;                /* This nodes Node-ID defined in ZW_non_zero_vars.c*/

#define MyNodeID nodeID

#define TS_SEND_RAW(src, dst, buf, buflen, txopt, cb) TS_SendRaw(dst, buf, buflen, txopt, cb)

/*===============================   ZW_Random   =============================
**  Pseudo-random number function
**
**  Side effects: seed updated
**
**--------------------------------------------------------------------------*/
extern BYTE            /*RET Random number */
ZW_Random( void );  /*IN Nothing */

#define rand()  ZW_Random()


#ifndef NO_MEM_FUNCTIONS
#define memcpy(dst, src, len)  __ZW_memcpy(len, dst, src)
#define memcmp(dst, src, len)  __ZW_memcmp(len, dst, src)
#define memset(dst, val, len)  __ZW_memset(val, len, dst)
#define ZW_memcpy(dst, src, len)  __ZW_memcpy(len, dst, src)
#define ZW_memcmp(dst, src, len)  __ZW_memcmp(len, dst, src)
#define ZW_memset(dst, val, len)  __ZW_memset(val, len, dst)
#endif

/*=============================   ZW_memcpy   ===============================
**    Copies length bytes from src to dst
**
**--------------------------------------------------------------------------*/
extern void       /* RET  Nothing */
__ZW_memcpy(
  BYTE length,    /* IN   Number of bytes to copy */
  BYTE *dst,      /* IN   Pointer to destination */
  BYTE *src);     /* IN   Pointer to source */


/*=============================   ZW_memcmp   ===============================
**    Compares length bytes of src and dest
**
**--------------------------------------------------------------------------*/
extern BYTE       /* RET  0 if *src and *dst are equal, else 1 */
__ZW_memcmp(
  BYTE length,    /* IN   Number of bytes to compare */
  BYTE *dst,      /* IN   Pointer to buffer 1 */
  BYTE *src);     /* IN   Pointer to buffer 2 */


/*===============================   ZW_memset   ==============================
**    Fill length bytes in dst with val
**
**--------------------------------------------------------------------------*/
void           /*RET Nothing */
__ZW_memset(
  BYTE val,    /* IN Value to fill buffer with */
  BYTE length, /* IN Number of bytes to set */
  BYTE *dst);  /* IN Pointer to buffer to set/fill */

extern WORD
ZW_CheckCrc16(
  WORD crc,
  BYTE *pDataAddr,
  WORD bDataLen);

extern BYTE MyNodeID;

#else
#define CALLBACKFUNC(func) func

typedef void (*ZW_CommandHandler_Callback_t)(ts_param_t* p, ZW_APPLICATION_TX_BUFFER *pCmd, WORD cmdLength);
typedef void (*ZW_TransportService_SendData_Callback_t)(uint8_t txStatus, void *t);

#define ZIPCommandHandler(srcNode, count) \
    if(TSApplicationCommandHandler) {\
      ts_param_t p;     \
      p.dendpoint = 0; \
      p.sendpoint = 0; \
      p.snode = srcNode; \
      p.dnode = rcb.cmn.p.dnode; \
      p.rx_flags =0; \
      p.tx_flags = TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_EXPLORE;\
      p.scheme = 0xff; \
      TSApplicationCommandHandler(&p,(ZW_APPLICATION_TX_BUFFER*) rcb.datagramData, count); \
    }


extern uint8_t MyNodeID;
#if !defined(NEW_TEST_T2)
#define TS_SEND_RAW(src, dst, buf, buflen, txopt, cb) ZW_SendData_Bridge(src, dst, buf, buflen, txopt, cb)
#endif
#endif




#endif /* _TRANSPORT_SERVICE_TYPES_H_ */


