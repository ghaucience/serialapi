/* © 2014 Silicon Laboratories Inc.
 */

#ifndef DHCPC_H_
#define DHCPC_H_

/** \ingroup processes
 * \defgroup ZIP_DHCP Z/IP DHCP handler
 * @{
 */
#include "process.h"

/**
 * The DHCP client process is responsible for managing all DCHP sessions for all Z-Wave nodes,
 * including the ZIP gateway itself.
 * All dhcp discover and requests are sent with a pr node unique client id.
 * The clientid is used by the DHCP server to distinguish the sessions.
 *
 * This process emits two contiki events, a \ref ZIP_EVENT_NODE_IPV4_ASSIGNED and a \ref ZIP_EVENT_ALL_IPV4_ASSIGNED.
 *
 */
PROCESS_NAME(dhcp_client_process);

/**
 * Abort all running DHCP session.
 */
void dhcpc_session_abort();

/*
 * Returns true if we are waiting for an answer.
 */
u8_t dhcpc_answer_pending();
/** @} */
#endif /* DHCPC_H_ */
