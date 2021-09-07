/* © 2014 Silicon Laboratories Inc.
 */

#ifndef ZW_SENDDATAAPPL_H_
#define ZW_SENDDATAAPPL_H_
#include <ZW_typedefs.h>
#include <ZW_transport_api.h>
#include "zgw_nodemask.h"

#define TRANSMIT_OPTION_SECURE 0x0100

/** \ingroup transport
 * \defgroup Send_Data_Appl Send Data Appl
 * SendDataAppl is the top level module for sending encapsulated packages on
 * the Z-Wave network.
 * @{
 */

/**
 * Enumeration of security schemes, this must map to the bit numbers in NODE_FLAGS
 */
typedef enum SECURITY_SCHEME {
  /**
   * Do not encrypt message
   */
  NO_SCHEME = 0xFF,

  /**
   * Let the SendDataAppl layer decide how to send the message
   */
  AUTO_SCHEME = 0xFE,

  NET_SCHEME = 0xFC, //Highest network shceme
  USE_CRC16 = 0xFD, //Force use of CRC16, not really a security scheme..

  /**
   * Send the message with security Scheme 0
   */
  SECURITY_SCHEME_0 = 7,

  /**
   * Send the message with security Scheme 2
   */
  SECURITY_SCHEME_2_UNAUTHENTICATED = 1,
  SECURITY_SCHEME_2_AUTHENTICATED = 2,
  SECURITY_SCHEME_2_ACCESS = 3,
  SECURITY_SCHEME_UDP = 4,

} security_scheme_t ;


//typedef uint8_t node_list_t[ZW_MAX_NODES/8];


/**
 * Structure describing how a package was received / should be transmitted
 */
typedef struct tx_param {
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
  security_scheme_t scheme; // Security scheme

  /**
   * Nodemask used when sending multicast frames.
   */
  nodemask_t node_list;
  /**
   * True if this is a multicast with followup enabled.
   */
  BOOL is_mcast_with_folloup;
} ts_param_t;


/**
 * Get standard transmit parameters
 * @param p
 * @param dnode
 */
void ts_set_std(ts_param_t* p, uint8_t dnode);

/**
 * Copy transport parameter structure
 * @param dst
 * @param src
 */
void ts_param_copy(ts_param_t* dst, const ts_param_t* src);

/**
 * Convert transport parameters to a reply
 * @param dst
 * @param src
 */
void ts_param_make_reply(ts_param_t* dst, const ts_param_t* src);

/**
 * \return true if source and destinations are identical
 */
uint8_t ts_param_cmp(ts_param_t* a1, const ts_param_t* a2);

/**
 * \param txStatus, see \ref ZW_SendData
 * \param user User defined pointer
 */
typedef void( *ZW_SendDataAppl_Callback_t)(BYTE txStatus,void* user, TX_STATUS_TYPE *txStatEx);

/**
 * Old style send data callback
 * @param txStatus  see \ref ZW_SendData
 */
typedef void( *ZW_SendData_Callback_t)(BYTE txStatus);



/**
 * This function will automatically
 * send a Z-Wave frame with the right encapsulation, like Multichannel, Security, CRC16, transport
 * service or no encapsulation at all.
 *
 * The decision is done based in the parameter given in \a p, \a dataLength and the
 * know capabilities of the destination node.
 *
 * \param p Parameters used for the transmission
 * \param pData Data to send
 * \param dataLength Length of frame
 * \param callback function to be called in completion
 * \param user user defined pointer which will provided a the second argument of the callback
 * see \ref ZW_SendDataAppl_Callback_t
 *
 */
uint8_t ZW_SendDataAppl(ts_param_t* p,
  const void *pData,
  uint16_t  dataLength,
  ZW_SendDataAppl_Callback_t callback,
  void* user) __attribute__((warn_unused_result));
/**
 * Initialize senddataAppl module
 */
void ZW_SendDataAppl_init();

/**
 * Abort the transmission
 */
void
ZW_SendDataApplAbort(uint8_t handle ) ;


/**
 * return true if a has a larger or equal scheme to b
 */
int scheme_compare(security_scheme_t a, security_scheme_t b);

/* Returns the highest security scheme of */
security_scheme_t highest_scheme(uint8_t scheme_mask);

/** @} */

#endif /* ZW_SENDDATAAPPL_H_ */
