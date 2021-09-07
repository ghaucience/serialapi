/* © 2014 Silicon Laboratories Inc.
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

/****************************  Mailbox.c  ****************************
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
 *              Copyright (c) 2012
 *              Zensys A/S
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
 * Description: ...
 *
 * Author:   aes
 *
 * Last Changed By:  $Author: $
 * Revision:         $Revision: $
 * Last Changed:     $Date: $
 *
 ****************************************************************************/
#include "contiki.h"
#include "contiki-net.h"
#include "Serialapi.h"
#include "command_handler.h"

#define MAX_MAILBOX_ENTRIES 500

/**
 * \defgroup mailbox Z/IP Mailbox service
 *
 * The Mail Box provides support for any IP application to communicate with
 * Wake Up Nodes without them having to implement or understand the
 * Wake Up Command class. The principle of the mail box functionality means
 * that sending application does not need any knowledge about the receiving
 * node sleeping state. The sending application will simply receive a “Delayed”
 * packet each minute, indicating that ACK is expected at a later point, and
 *  that the application should not attempt retransmission.
 *
 * In addition, the Mailbox supports the Mailbox Command Class,
 * which allows a lightweight Z/IP Gateway to offload the mailbox functionality
 * to a more powerful mailbox service such as a portal.
 *
 * @{
 */

/**
 * Send post the package in uip_buf
 */
uint8_t mb_post_uipbuf(uip_ip6addr_t* proxy,uint8_t handle);

/**
 * Returns true if node is awake
 */
uint8_t
mb_is_node_awake(uint8_t n);

/**
 * Called when a node has found to be failing.
 */
void
mb_failing_notify(uint8_t node);


/**
 * Send a node to sleep after there has been no transmissions to it for a while.
 */
void
mb_put_node_to_sleep_later(int node);

/**
 * Purge all messages in the mailbox queue destined for a particular node.
 * Called instead of mb_failing_notify() for nodes without fixed wakeup interval.
 */
void mb_purge_messages(uint8_t node);

/**
 * Initialize the mailbox module
 */
void
mb_init();

/**
 * Called when a Wake Up Notification (WUN) is received from the Z-Wave interface.
 * \param node NodeID sending the WUN
 * \param is_broadcast Flag set to true if WUN was received as broadcast.
 *
 */
void
mb_wakeup_event(
    u8_t node,
    u8_t is_broadcast);


uint8_t mb_enabled();

/**
 * Returns true if mailbox is busy
 */
uint8_t
mb_is_busy(void);

#include "ZW_udp_server.h"
command_handler_codes_t
mb_command_handler(zwave_connection_t *c, uint8_t* pData, uint16_t bDatalen);

/* Event telling the mailbox that we have sucsessfully sent at frame to the node */
void mb_node_transmission_ok_event(u8_t noce);


/**
 * Abort mailbox transmission, this function has no effect if the mailbox is not active;
 */
void mb_abort_sending();

uint8_t is_zip_pkt(uint8_t *);
/**
 * @}
 */
#endif /* MAILBOX_H_ */

