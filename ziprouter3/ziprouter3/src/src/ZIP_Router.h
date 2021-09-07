/* © 2014 Silicon Laboratories Inc.
 */

#ifndef ZIP_ROUTER_H_
#define ZIP_ROUTER_H_

/**
 * \ingroup processes
 *
 * \defgroup ZIP_Router Z/IP Router Main process for zipgateway
 * This process is main process which spawns of series of sub processes
 * It is responsible for
 * handling all incoming events and perform actions from sub processes.
 * @{
 */
#include "sys/process.h"
#include "net/uip.h"
#include "TYPES.H"
#include "ZW_SendDataAppl.h"
#include "pkgconfig.h"

#include "assert.h"

#include "router_events.h"

#include "ZIP_Router_logging.h"
#include "zip_router_config.h"

/**
 * Virtual mac address pf the tunnel interface
 */
extern const uip_lladdr_t tun_lladdr;

/**
 * Callback LAN output function.
 * \param a destination link layer (MAC) address. If zero the packet must be broadcasted
 *          packet to be transmitted is in uip_buf buffer and the length of the packet is
 *          uip_len
 *
 */
typedef  u8_t(* outputfunc_t)(uip_lladdr_t *a);
/**
 * The set output function for the LAN
 * Output function for the IP packets
 */
void set_landev_outputfunc(outputfunc_t f);

/* TODO: Remove? */
extern BYTE Transport_SendRequest(BYTE tnodeID, BYTE *pBufData, BYTE dataLength,
    BYTE txOptions, void (*completedFunc)(BYTE));

/*Forward declaration*/
union _ZW_APPLICATION_TX_BUFFER_;

/** Application command handler for Z-wave packets. The packets might be the decrypted packets
 * \param p description of the send,receive params
 * \param pCmd Pointer to the command received
 * \param cmdLength Length of the command received
 */

void ApplicationCommandHandlerZIP(ts_param_t *p,
		union _ZW_APPLICATION_TX_BUFFER_ *pCmd, WORD cmdLength) CC_REENTRANT_ARG;

/** Application command handler for raw Z-wave packets. 
 * \see ApplicationCommandHandler
 */
void ApplicationCommandHandlerSerial(BYTE rxStatus,BYTE destNode, BYTE sourceNode,
    union _ZW_APPLICATION_TX_BUFFER_ *pCmd, BYTE cmdLength)CC_REENTRANT_ARG;

/** ID of this node
 */
extern BYTE MyNodeID; 
/** Home ID of this node
 */
extern DWORD homeID;

typedef enum {
    SECONDARY,
    SUC,
    SLAVE
} controller_role_t;
    
extern controller_role_t controller_role;

/**
 * Check on the expression.
 */
#define ASSERT assert

PROCESS_NAME(zip_process);

/**
 * Get the ipaddress of a given node
 * \param dst output
 * \param nodeID ID of the node
 */
void ipOfNode(uip_ip6addr_t* dst, BYTE nodeID);

/**
 * Return the node is corresponding to an ip address
 * \param ip IP address
 * \return ID of the node 
 */
BYTE nodeOfIP(const uip_ip6addr_t *ip);

/**
 * Returns True is address is a classic zwave address.
 * \param ip IP address
 */
BYTE isClassicZWAddr(uip_ip6addr_t* ip);

/* TODO: Move ? */
#ifdef __ASIX_C51__
void Reset_Gateway(void) CC_REENTRANT_ARG;

/* TODO: Move ? */
uint8_t validate_defaultconfig(uint32_t  addr, uint8_t isInt, uint16_t *pOutGwSettingsLen) CC_REENTRANT_ARG;
#endif
/* TODO: Move ? */
uint8_t MD5_ValidatFwImg(uint32_t baseaddr, uint32_t len) REENTRANT;
/* TODO: Do it in a different way. Use another code from the Resource directory*/
BYTE IsCCInNodeInfoSetList(BYTE cmdclass,BOOL secure) CC_REENTRANT_ARG;

void macOfNode(uip_lladdr_t* dst, u8_t nodeID);


extern BYTE SecureClasses[];
extern BYTE nSecureClasses;

void refresh_ipv6_addresses();

/**
 * The highest security scheme supported by this node.
 */
extern security_scheme_t net_scheme;

/** @} */ // end of ZIP_Router
#endif /* ZIP_ROUTER_H_ */
