/* © 2014 Silicon Laboratories Inc.
 */

#ifndef DTLS_SERVER_H_
#define DTLS_SERVER_H_

#include "contiki.h"
#include "contiki-net.h"

/**
 * \ingroup processes
 * \defgroup DTLSService DTLS Service
 *
 * Z/IP LAN security provides a secure connection for clients, and other Z/IP Gateways,
 * connecting to the Z/IP Gateway. The Z/IP LAN Security framework provides a means
 * of securing the communication paths between:
 *
 *   - Z/IP Clients
 *   - Z/IP Clients and Z/IP Gateways
 *   - Z/IP Gateways.
 *
 * Starting with Z/IP Gateway 2.x it is mandatory to send secure Z/IP UDP frames to and
 * from the gateway. The same mechanism may be used to send secure Z/IP UDP frames between
 * Z/IP Clients, and between Z/IP Gateways.
 *
 * Secure Z/IP UDP frames are ordinary Z/IP frames wrapped in a DTLS 1.0 wrapper. DTLS is
 * the datagram version of TLS. Z/IP LAN Security default UDP port number is 41230.
 *
 * This current version of Z/IP LAN Security only supports the Pre-Shared-Key
 * (PSK) Key Exchange Algorithm.
 * @{
 */

#define DTLS_PORT 41230
PROCESS_NAME(dtls_server_process);

/**
 * Send encrypted data to a client. If a session exists this will send
 * data synchronously. Otherwise it will create a client session and send async.
 * @param conn udp connection
 * @param data pointer to data
 * @param len length of data
 * @param create_new create new client connection if needed
 * @return the length of the transmitted data
 */

int dtls_send(struct uip_udp_conn* conn, const void* data, u16_t len,u8_t create_new);

/**
 * Return true if the session between the two parties present in the UDP buf
 * has been established.
 */
int session_done_for_uipbuf();

/* Indicator to see if the last SSL read has failed */
extern int dtls_ssl_read_failed;

enum {DTLS_SERVER_INPUT_EVENT,
  DTLS_CONNECTION_CLOSE_EVENT,
  DLTS_CONNECTION_INIT_DONE_EVENT,
  DLTS_SESSION_UPDATE_EVENT,
};

void dtls_exited();

/**
 * @}
 */
#endif /* DTLS_SERVER_H_ */
