/* © 2014 Silicon Laboratories Inc. */

#ifndef RESOURCEDIRECTORY_H
#define RESOURCEDIRECTORY_H

/**
 * \ingroup ZIP_MDNS
 * \defgroup ZIP_Resource Z/IP Resource Directory
 *
 * The Resource Directory is a database containing information about
 * all nodes in the PAN network.
 *
 * This module probes/interviews nodes in the PAN network for their
 * supported command classes, security, multi-endpoint capabilities,
 * etc., and stores the results.
 *
 * #### Dead Node Detection ####
 *
 * The Resource Directory also tries to keep track of the health state
 * of the nodes in the PAN network.  It does so by periodically
 * sending a NOP to all #MODE_ALWAYSLISTENING nodes in the network.
 * If a node fails to respond to the NOP it is set to failing
 * (#STATUS_FAILING). As soon as the Z/IP Gateway receives a message
 * from a failing node, it will mark it as not failing (#STATUS_DONE).
 * Wakeup nodes (#MODE_MAILBOX) will become failing if they miss 2
 * times their wakeup interval, again they will revert their failing
 * state as soon as the gateway receives a command or an ACK from the
 * failing node.  A FLIRS device (#MODE_FREQUENTLYLISTENING) will not
 * be checked, which means it will never become failing.
 *
 * \see #rd_set_failing, #rd_check_for_dead_nodes_worker().
 *
 *
 * #### mDNS Support ####
 *
 * The Resource Directory works closely with the \ref ZIP_MDNS. \ref
 * ZIP_MDNS uses the information collected in the resource directory
 * to send replies on the LAN side.  When nodes are added or removed
 * on the PAN side, the Resource Directory may trigger gratuitous mDNS
 * packets on the LAN side (e.g., mDNS goodbye).
 *
 * #### Network Management Interaction ####
 * The Resource Directory also works closely with \ref NW_CMD_handler.
 * The NetworkManagement module can create and destroy entries in the
 * database and triggers/blocks the probe machine.
 *
 * @{
 */
#include "RD_internal.h"
#include "ZW_SendDataAppl.h"
//#include "contiki-net.h"
#include "sys/ctimer.h"
#include "sys/clock.h"

/**
 * Flag to indicate that a node has been deleted from the network.
 *
 * This flag is set on the #rd_node_database_entry_t::mode field.
 *
 * When a node has been removed from the PAN network, the gateway
 * keeps the data-structures associated with the node for a brief time
 * longer, so that it can announce the removal on mDNS.
 */
#define MODE_FLAGS_DELETED 0x0100
#define MODE_FLAGS_FAILED  0x0200
#define MODE_FLAGS_LOWBAT  0x0400


#define RD_ALL_NODES 0

/**
 * 
 */
typedef struct rd_group_entry {
  char name[64];
  list_t* endpoints;
} rd_group_entry_t;

/**
 * Register a new node into database.  This will cause the Resource
 * Directory probe engine to interview nodes for information.
 *
 * Set #RD_NODE_FLAG_JUST_ADDED on node_properties_flag if the controller is still adding node.
 * Set #RD_NODE_FLAG_ADDED_BY_ME on node_properties_flag if the controller is SIS.
 *
 * @param node node is to probe
 * @param node_properties_flag Additional properties flags with respect to the node
 */
void rd_register_new_node(uint8_t node,uint8_t node_properties_flag);

/**
 * Do a complete re-discovery and "hijack" wakeup nodes
 */
void rd_full_network_discovery();

/**
 * Re-discover nodes which are not in the database
 * @return the number of new nodes to probe
 */
u8_t rd_probe_new_nodes();

/**
 * Remove a node from resource database
 */
void rd_remove_node(uint8_t node);


/**
 * Initialize the resource directory and the nat table (and DHCP).
 *
 * @param lock boolean, set true if node probe machine should be locked after init
 * @return TRUE if the homeID has changed since the last rd_init.
 */
int rd_init(uint8_t lock);

/**
 * Gracefully shut down Resource Directory and mDNS.
 *
 * Mark all nodes and endpoints as \ref MODE_FLAGS_DELETED.  Queue up
 * sending of goodbye packets for all endpoints.  Call mdns_exit() to
 * tell the mDNS process to shut down when the queue is empty.
 *
 * \note It is mandatory to call rd_destroy() before calling rd_init()
 * again.  No actual freeing of memory is done in rd_exit(), since
 * mDNS needs access to the nodes until the goodbyes are sent.
 */
void rd_exit();

/**
 * Finally free memory allocated by RD
 */
void rd_destroy();

/**
 *  Lock/Unlock the node probe machine. When the node probe lock is enabled, all probing will stop.
 *  Probing is resumed when the lock is disabled. The probe lock is used during a add node process or during learn mode.
 *  \param enable Boolean - whether to enable the lock.
 */
void rd_probe_lock(uint8_t enable);

/**
 * Unlock the probe machine but do not resume probe engine.
 *
 * If probe was locked during NM add node, but the node should not be
 * probed because it is a self-destructing smart start node, this
 * function resets the probe lock.
 *
 * When removal of the node succeeds, \ref current_probe_entry will be
 * reset when the node is deleted.  We also clear \ref
 * current_probe_entry here so that this function can be used in the
 * "removal failed" scenarios.
 */
void rd_probe_cancel(void);

/**
 * Retrieve a
 */
rd_ep_database_entry_t* rd_lookup_by_ep_name(const char* name,const char* location);
rd_node_database_entry_t* rd_lookup_by_node_name(const char* name);

rd_group_entry_t* rd_lookup_group_by_name(const char* name);

/**
 * \param node A valid node id or #RD_ALL_NODES.
 */
rd_ep_database_entry_t* rd_ep_first(uint8_t node);
rd_ep_database_entry_t* rd_ep_iterator_group_begin(rd_group_entry_t*);
rd_ep_database_entry_t* rd_ep_next(uint8_t node,rd_ep_database_entry_t* ep);
rd_ep_database_entry_t* rd_group_ep_iterator_next(rd_group_entry_t* ge,rd_ep_database_entry_t* ep);

/**
 * Used with \ref rd_ep_class_support()
 */
#define SUPPORTED_NON_SEC   0x1
/**
 * Used with \ref rd_ep_class_support()
 */
#define CONTROLLED_NON_SEC  0x2
/**
 * Used with \ref rd_ep_class_support()
 */
#define SUPPORTED_SEC       0x4
/**
 * Used with \ref rd_ep_class_support()
 */
#define CONTROLLED_SEC      0x8
/**
 * Used with \ref rd_ep_class_support()
 */
#define SUPPORTED  0x5
/**
 * Used with \ref rd_ep_class_support()
 */
#define CONTROLLED 0xA

/**
 * Look up a command class in an end point and return capability mask.
 *
 * \param ep Pointer to the #rd_ep_database_entry for the endpoint.
 * \param cls The command class to look for.
 */
int rd_ep_class_support(rd_ep_database_entry_t* ep, uint16_t cls);

/** Called when a nif is received or when a nif request has timed out */
void rd_nif_request_notify(uint8_t bStatus,uint8_t bNodeID,uint8_t* pCmd,uint8_t bLen);

/** Register an endpoint notifier. The endpoint notifier will be called when the probe of an endpoint has completed.
 *
 * If the endpoints is already in state PROBE_DONE or PROBE_FAIL, the callback is called synchronously. */
int rd_register_ep_probe_notifier(
    uint16_t node_id,
    uint16_t epid,
    void* user,
    void (*callback)(rd_ep_database_entry_t* ep, void* user));

/**
 * Get nodedatabase entry from data store. free_node_dbe() MUST
 * be called when the node entry is no longer needed.
 */
rd_node_database_entry_t* rd_get_node_dbe(uint8_t nodeid);

/**
 * MUST be called when a node entry is no longer needed.
 */
void rd_free_node_dbe(rd_node_database_entry_t* n);


/**
 * Lookup node id in \ref node_db.
 *
 * \ingroup node_db
 *
 * @param node The node id
 * @return Returns true if node is registered in database
 */
u8_t rd_node_exists(u8_t node);

/**
 * Get an endpoint entry in the \ref node_db from nodeid and epid.
 *
 * @return A pointer to an endpoint entry or NULL.
 */
rd_ep_database_entry_t* rd_get_ep(uint8_t nodeid, uint8_t epid);

rd_node_mode_t rd_get_node_mode(uint8_t nodeid);
rd_node_state_t rd_get_node_state(uint8_t nodeid);

u8_t rd_get_node_schemes(uint8_t nodeid); // reutrn the security scheme suported by a node

/** Set a node in \ref node_db to be alive.
 *
 * Should be called whenever a node has been found to be alive,
 * i.e., when a command was successfully sent to the node or when we
 * received a command from the node.
 */
void rd_node_is_alive(uint8_t node);

/** Set failing status of a node in \ref node_db.
 */
void rd_set_failing(rd_node_database_entry_t* n,uint8_t failing);

u8_t rd_get_ep_name(rd_ep_database_entry_t* ep,char* buf,u8_t size);
u8_t rd_get_ep_location(rd_ep_database_entry_t* ep,char* buf,u8_t size);
/**
 * Sets name/location of an endpoint. This will trigger a mdns probing. Note that
 * the name might be changed if the probe fails.
 */

void rd_update_ep_name(rd_ep_database_entry_t* ep,char* name,u8_t size);

void rd_update_ep_location(rd_ep_database_entry_t* ep,char* name,u8_t size);

/**
 * Update both the name and location of an endpoint.
 *
 * For creating a name/location during inclusion use \ref rd_add_name_and_location().
 */
void
rd_update_ep_name_and_location(rd_ep_database_entry_t* ep, char* name,
    u8_t name_size, char* location, u8_t location_size);

/**
 * return true if a probe is in progress
 */
u8_t rd_probe_in_progress();

/**
 * Mark the mode of node MODE_FLAGS_DELETED so that
 */
void rd_mark_node_deleted(uint8_t nodeid);

/**
 * Check node database to see if there are any more nodes to probe.
 *
 * If a probe is already ongoing (#current_probe_entry is not NULL), resume that probe.
 *
 * Then probe all nodes not in one of the final states: #STATUS_DONE,
 * #STATUS_PROBE_FAIL, or #STATUS_FAILING.
 *
 * Post #ZIP_EVENT_ALL_NODES_PROBED when all nodes are in a final
 * state and the probe machine is unlocked.
 */
void rd_probe_resume();

/** Start probe of \a n or resume probe of #current_probe_entry.
 *
 * Do nothing if probe machine is locked, bridge is \ref booting, \ref
 * ZIP_MDNS is not running or another node probe is on/going.
 *
 * Set #current_probe_entry to \a n if it is not set.
 *
 * When probe is complete, store node data in eeprom file, send out
 * mDNS notification for all endpoints, and trigger ep probe callback
 * if it exists.
 *
 * Finally call \ref rd_probe_resume(), to trigger probe of the next
 * node.
 *
 * \param n A node to probe.
 */
void rd_node_probe_update(rd_node_database_entry_t* n);


u8_t validate_name(char* name, u8_t len);
u8_t validate_location(char* name, u8_t len);



/**
 * Check whether a frame should be forwarded to the unsolicited destination or not, based on
 *    - its command type supporting/controlling,
 *    - the security level which the frame was received on
 *    - if its present in the secure/non-secure nif.
 *
 *    @param rnode    Remote node
 *    @param scheme   Scheme which was used to send the frame
 *    @param p        Data of frame
 *    @param bDatalen length of data.
 */
int rd_check_security_for_unsolicited_dest( uint8_t rnode,  security_scheme_t scheme, void *pData, u16_t bDatalen );

/**  @} */
#endif

