/* © 2014 Silicon Laboratories Inc.
 */

#ifndef MDNSSERVICE_H_
#define MDNSSERVICE_H_

/**
 * \ingroup processes
 * \defgroup ZIP_MDNS mDNS Service
 *
 * The mDNS service allows an IP application automatically discover all Z-Wave nodes
 * available on any Full Z/IP Gateway on the backbone. The application will receive
 * information about all nodes added to the network as well as any changes that may be made.
 * The mDNS discovery service must announce all nodes and node endpoints as individual
 * mDNS resources (see draft-cheshire-dnsext-dns-sd.). As new nodes are added and removed
 * from the Z-Wave network the mDNS resource changes must be dynamically
 * multicast on the LAN backbone. The mDNS resource announcements must contain detailed
 * information about the underlying node/endpoint such as its device type and supported
 * command classes and IPv6 address(es).
 *
 * The names of the mDNS resources are statically generated.
 *
 * @{
 */
#include "ResourceDirectory.h"

PROCESS_NAME(mDNS_server_process);


/**
 * Send out a mDNS notification if mDNS records has been added/removed or changed.
 *
 * If express==TRUE then the notification is sent right away.
 */
void mdns_endpoint_nofify(rd_ep_database_entry_t* ep,u8_t bTxt);

/**
 * Remove an endpoint from the list of answers
 */
void mdns_remove_from_answers(rd_ep_database_entry_t*);

/**
 * Returns true if name probe has started.
 * Begin 3 mDNS name probes for the endpoint name. The name probe makes sure that there are no
 * other services on the network with the same name.
 *
 * \param ep Endpoint to probe
 * \param callback Function which will be called on completion. if bSucsess is true there
 * was no reply on the 3 attempts.
 * \param ctx user pointer which will be returned as the second argument to callback.
 *
 */
int mdns_endpoint_name_probe(rd_ep_database_entry_t* ep, void(*callback)(int bSucsess,void*), void*ctx );

/**
 * Like \ref mdns_endpoint_name_probe, but for node host names.
 */
int mdns_node_name_probe(rd_node_database_entry_t* nd, void(*callback)(int bSucsess,void*), void*ctx);

/**
 * Gracefully close mDNS process, which means that pending answers should be sent before the
 * process exits.
 *
 * The mDNS process MUST be stopped with this function, not by calling process_exit() directly.
 */
void mdns_exit();


/**
 * Block until all mDNS answers has been sent
 */
void mdns_sync();

/**
 * Case insensitive string compare
 * \return true if the two string a identical
 *
 */
int mdns_str_cmp(const char* s1, const char* s2, u8_t len);
/** @} */
#endif /* MDNSSERVICE_H_ */
