/* © 2014 Silicon Laboratories Inc.
 */

#ifndef CC_PORTAL_H_
#define CC_PORTAL_H_

/** \ingroup CMD_handler
 * \defgroup PO_CMD_handler Portal command handler
 *
 * The Z/IP Portal Command Class is used for configuration and management
 * communication between a Z/IP portal server and a Z/IP gateway through
 * a secure connection.
 *
 * The Z/IP Portal command class is intended for use together with the Z/IP Gateway
 * command class to provide a streamlined workflow for preparing and performing
 * installation of Z/IP Gateways in consumer premises.
 *
 * The command class MUST NOT be used outside trusted environments,
 * unless via a secure connection. The command class SHOULD be further
 * limited for use only via a secure connection to an authenticated portal server.
 *
 * See also \ref Portal_handler
 * @{
 */
#include "contiki-net.h"
#include "TYPES.H"
#include "ZW_udp_server.h"



/** @} */
#endif /* CC_PORTAL_H_ */
