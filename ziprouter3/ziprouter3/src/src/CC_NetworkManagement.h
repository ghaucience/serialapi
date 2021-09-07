/* © 2014 Silicon Laboratories Inc.
 */

#ifndef NETWORKMANAGEMENT_H_
#define NETWORKMANAGEMENT_H_

/**
 *  \ingroup CMD_handler
 *  \defgroup NW_CMD_handler Network Management Command Handler
 *
 * Control of the Z-Wave network is carried out through a number of Network
 * Management Command Classes.
 *
 * The Z/IP Gateway MUST support all four Network Management Command Classes:
 * - Network Management Inclusion: Support for Adding/Removing, Set Default,
 *   Failed Node Replacement and Removal and Requesting Node Neighbor Update
 * - Network Management Basic: Support for entering Learn Mode, Node Information Send
 *   and Request Network Update
 * - Network Management Proxy: Support for Node List Get and Node Info Cached Get
 * - Network Management Primary: Support for Controller Change, only available
 *   when operating as Primary Controller without SIS functionality.
 *
 * These four command classes enable network management and access to simple network topology information.
 *
 * @{
 */
#include "contiki-net.h"
#include "TYPES.H"
#include "ZW_udp_server.h"
#include "s2_inclusion.h"
/**
 * State variable tracking it the network managemnet module has alread been initalized
 */
extern int network_management_init_done;

/**
 * State of the Network Management module.
 *
 * If the Network Management State machine (NMS) is not idle, Z/IP
 * Gateway will reply #COMMAND_BUSY to all Network Management
 * commands.
 */
typedef enum {
   /** Idle state. */
  NM_IDLE,

  /** Add mode state.
   * Classic, S2, or Smart Start.
   *
   * Waiting for protocol.  On \ref NM_EV_ADD_NODE_FOUND, go to \ref NM_NODE_FOUND.
   */
  NM_WAITING_FOR_ADD,
  /** Add mode state.
   * Classic, S2, Smart Start, Proxy Inclusion, or Proxy Replace.
   *
   * Waiting for protocol to receive node info from a node.  On \ref
   * NM_EV_ADD_CONTROLLER or \ref NM_EV_ADD_SLAVE, cache the node
   * id and node info and go to \ref NM_WAIT_FOR_PROTOCOL.
   */
  NM_NODE_FOUND,
  /** Add mode state.
   * Classic, S2, Smart Start, Proxy Inclusion, or Proxy Replace.
   *
   * Waiting for protocol to assign nodeid and homeid to the new node.
   *
   * On \ref NM_EV_ADD_PROTOCOL_DONE, send an ADD_NODE_STOP to
   * protocol.  On \ref NM_EV_ADD_NODE_STATUS_DONE, register the new
   * node in \ref node_db and go to \ref NM_WAIT_FOR_SECURE_ADD.  If
   * GW is handling inclusion, start security processing the new node
   * (S2 or S0 inclusion).  Otherwise, start a 2s timer and go to \ref
   * NM_PREPARE_SUC_INCLISION.
   */
  NM_WAIT_FOR_PROTOCOL,

  /** GW is processing \a NETWORK_UPDATE_REQUEST.
   *
   * GW is not SUC/SIS and is processing a \ref NETWORK_UPDATE_REQUEST.
   *
   * ResetState() from nm_send_reply() will bring NMS back to \ref NM_IDLE.
   */
  NM_NETWORK_UPDATE,

  /** Learn mode state.  Also used if GW is processing \ref
   * NODE_INFO_CACHED_GET, \ref NETWORK_UPDATE_REQUEST, or \ref
   * DEFAULT_SET.
   *
   * Waiting on networkUpdateStatusFlags.  When all flags are in,
   * ResetState() from nm_send_reply() will bring NMS back to \ref NM_IDLE.
   *
   * Cannot be interrupted.
   */
  NM_WAITING_FOR_PROBE,
  /** Learn mode state (exclusion).  Also used if GW is processing DEFAULT_SET.
   *
   * Waiting for NetworkManagement_mdns_exited() to be called from
   * \ref ZIP_Router.  Post \ref ZIP_EVENT_RESET to \ref ZIP_Router
   * and call SendReplyWhenNetworkIsUpdated() to go to \ref
   * NM_WAITING_FOR_PROBE.  SendReplyWhenNetworkIsUpdated() sets up a
   * trigger to bring NMS back to \ref NM_IDLE when the gateway reset
   * is completed.
   *
   * \note When changing to this state, we MUST call rd_exit() to advance
   * the state machine.
   */
  NM_SET_DEFAULT,
  /** Learn mode state.
   *
   * GW is processing LEARN_SET, waiting for protocol.
   *
   * On callback (LearnModeStatus()) with LEARN_MODE_STARTED, go to \ref
   * NM_LEARN_MODE_STARTED, Lock RD probe machine, invalidate
   * MyNodeID, and start S2 learn-mode state machine.
   *
   * Can be cancelled or time out.  This will trigger a partial
   * re-initialization of the gateway (ApplicationInitSW()), unlock RD
   * probe.  ResetState() from nm_send_reply() with the
   * LEARN_MODE_FAILED will bring NMS back to \ref NM_IDLE.
   */
  NM_LEARN_MODE,
  /** Learn mode state.
   *
   * The gateway is processing LEARN_SET and has received
   * LEARN_MODE_STARTED from protocol.  Now the protocol is committed
   * and the gateway waits for protocol completion.
   *
   * On protocol callback (LearnModeStatus()) with LEARN_MODE_DONE,
   * the gateway has been assigned a nodeid and homeid.  Go to \ref
   * NM_WAIT_FOR_SECURE_LEARN, detect new environment and reset
   * gateway accordingly.
   *
   * Cannot be interrupted because the gateway properties have changed
   * and we are waiting on the protocol.
   */
  NM_LEARN_MODE_STARTED,

  /** Add mode state.
   * Classic, S2, Smart Start, Proxy Inclusion, Proxy Replace.
   *
   * Waiting for the S2 inclusion state machine to complete.  On \ref
   * NM_EV_SECURITY_DONE, go to \ref NM_WAIT_FOR_PROBE_AFTER_ADD and
   * start probe.
   *
   * If Smart Start security fails, start a
   * #SMART_START_SELF_DESTRUCT_TIMEOUT sec. timer and go to
   * #NM_WAIT_FOR_SELF_DESTRUCT.
   */
  NM_WAIT_FOR_SECURE_ADD,

  /** GW is processing NODE_INFORMATION_SEND.
   *
   * __ResetState() callback from protocol will bring NMS back to \ref NM_IDLE.
   */
  NM_SENDING_NODE_INFO,

  /** Remove node state.
   *
   * Waiting for protocol.
   * In RemoveNodeStatusUpdate() callback on REMOVE_NODE_STATUS_DONE
   * status, remove the node from Resource Directory, start removing
   * the node's ip associations, and go to \ref
   * NM_REMOVING_ASSOCIATIONS.  On REMOVE_NODE_STATUS_FAILED,
   * ResetState() from nm_send_reply() will bring NMS back to \ref
   * NM_IDLE.
   */
  NM_WAITING_FOR_NODE_REMOVAL,
  /** Remove failed node state.
   *
   * Waiting for protocol.  In callback RemoveFailedNodeStatus(),
   * ResetState() from nm_send_reply() will bring NMS back to \ref
   * NM_IDLE.
   */
  NM_WAITING_FOR_FAIL_NODE_REMOVAL,

  /** GW is processing NODE_NEIGHBOR_UPDATE_REQUEST.
   *
   * When receiving REQUEST_NEIGHBOR_UPDATE_DONE or
   * REQUEST_NEIGHBOR_UPDATE_DONE, ResetState() from nm_send_reply()
   * will bring NMS back to \ref NM_IDLE.
   */
  NM_WAITING_FOR_NODE_NEIGH_UPDATE,

  /** GW is processing RETURN_ROUTE_ASSIGN.
   *
   * Waiting for protocol.  In callback AssignReturnRouteStatus(),
   * ResetState() from nm_send_reply() will bring NMS back to \ref
   * NM_IDLE.
   */
  NM_WAITING_FOR_RETURN_ROUTE_ASSIGN,

  /** GW is processing RETURN_ROUTE_DELETE.
   *
   * Waiting for protocol.  In callback DeleteReturnRouteStatus(),
   * ResetState() from nm_send_reply() will bring NMS back to \ref
   * NM_IDLE.
   */
  NM_WAITING_FOR_RETURN_ROUTE_DELETE,

  /** Add mode state.
   * Classic, S2, Smart Start, Replace Failed, Proxy Inclusion, Proxy
   * Replace.
   *
   * Depends on NodeProbeDone() callback from Resource Directory to
   * advance NMS to \ref NM_WAIT_DHCP.
   */
  NM_WAIT_FOR_PROBE_AFTER_ADD,

  /** Learn mode state.
   *
   * The gateway has received #LEARN_MODE_DONE from protocol.
   *
   * In case of inclusion, Network Management is waiting for S2/S0
   * learn mode to complete.  On \ref NM_EV_SECURITY_DONE, go to \ref
   * NM_WAIT_FOR_MDNS and call rd_exit() to reset the gateway for a
   * new network.
   *
   * In case of #NM_EV_ADD_SECURITY_KEY_CHALLENGE, accept.
   * In case of NM_EV_LEARN_SET/DISABLE, abort S2.
   *
   * In case of exclusion, controller replication, and controller
   * shift, #NM_EV_SECURITY_DONE is triggered synchronously in NMS and
   * state is changed synchronously when #LEARN_MODE_DONE is
   * received to either #NM_SET_DEFAULT or #NM_WAIT_FOR_MDNS.
   */
  NM_WAIT_FOR_SECURE_LEARN,
  /** Learn mode state.
   *
   * After #NM_WAIT_FOR_SECURE_LEARN, NMS is waiting for
   * #NetworkManagement_mdns_exited() to be called from \ref
   * ZIP_Router.  For inclusion or exclusion, NMS then calls \ref
   * bridge_reset().
   *
   * If \ref NMS_FLAG_LEARNMODE_NEW is set, send the
   * LEARN_MODE_SET_STATUS reply, go to \ref
   * NM_WAIT_FOR_PROBE_BY_SIS and set up a 6s timer.  Otherwise post
   * \ref ZIP_EVENT_RESET to \ref ZIP_Router and call
   * SendReplyWhenNetworkIsUpdated() to go to \ref
   * NM_WAITING_FOR_PROBE.  SendReplyWhenNetworkIsUpdated() sets up a
   * trigger to bring NMS back to \ref NM_IDLE when the gateway reset
   * is completed.
   *
   * \note When changing to this state, we MUST call rd_exit() to advance
   * the state machine.
   */
  NM_WAIT_FOR_MDNS,
  /** Learn mode state.
   * Only used with #NMS_FLAG_LEARNMODE_NEW.
   *
   * After learn mode, the gateway must wait for the SIS to probe
   * before resetting.  So we stay in #NM_WAIT_FOR_PROBE_BY_SIS, and
   * restart the 6s timer every time a frame is received.
   *
   * On timeout, ie, when no frames have been receeived for at least
   * 6s, we assume the SIS is done, post \ref ZIP_EVENT_RESET to \ref
   * ZIP_Router and go to \ref NM_WAIT_FOR_OUR_PROBE.
   */
  NM_WAIT_FOR_PROBE_BY_SIS,
  /** Learn mode state.
   *
   * Only used with \ref NMS_FLAG_LEARNMODE_NEW.
   *
   * Wait for \ref NM_EV_ALL_PROBED, then call
   * SendReplyWhenNetworkIsUpdated() to go to \ref
   * NM_WAITING_FOR_PROBE.  SendReplyWhenNetworkIsUpdated() sets up a
   * trigger to bring NMS back to \ref NM_IDLE when the gateway reset
   * is completed.
   */
  NM_WAIT_FOR_OUR_PROBE,

  /** Add mode state.
   *
   * Classic, S2, Smart Start, Proxy Inclusion, Proxy Replace.
   *
   * Wait for \ref NM_EV_DHCP_DONE, when the new node has an IPv4
   * address, or timeout (5s timer).  Then call nm_send_reply(), where
   * the callback ResetState() will bring NMS back to \ref NM_IDLE.
   * For smart start, call ResetState() directly to bring NMS back to
   * \ref NM_IDLE.
   */
  NM_WAIT_DHCP,

  /** Remove node state.
   *
   * Waiting for bridge.  Callback
   * NetworkManagement_VirtualNodes_removed() will bring NMS back to
   * \ref NM_IDLE. */
  NM_REMOVING_ASSOCIATIONS,

  /** Replace failed state.
   *
   * Replace and replace S2.
   *
   * On \ref NM_EV_REPLACE_FAILED_DONE, if GW is handling inclusion, go to
   * \ref NM_WAIT_FOR_SECURE_ADD.  Otherwise go to
   * \ref NM_PREPARE_SUC_INCLISION.
   */
  NM_REPLACE_FAILED_REQ,

  /** Add mode state.
   *
   * Classic, S2, Smart Start, Replace Failed, Proxy Inclusion, or
   * Proxy Replace.
   *
   * Wait for timeout to let protocol finish up its add by sending a
   * transfer end to SUC. Then go to \ref NM_WIAT_FOR_SUC_INCLUSION
   * and request handover.
   */
  NM_PREPARE_SUC_INCLISION,
  /** Add mode state.
   *
   * Classic, S2, Smart Start, Replace Failed, Proxy Inclusion, or
   * Proxy Replace.
   *
   * Waiting for the SUC to complete the secure part of inclusion.
   *
   * On \ref NM_EV_PROXY_COMPLETE, go to \ref NM_WAIT_FOR_SECURE_ADD
   * and post \ref NM_EV_SECURITY_DONE.
   */
  NM_WIAT_FOR_SUC_INCLUSION,
  /** Add mode state.
   * Proxy inclusion and proxy replace.
   *
   * Safe-guarded by a timer, on timeout, go to \ref NM_IDLE.
   *
   * On \ref NM_EV_NODE_INFO, inclusion goes to \ref NM_NODE_FOUND and fakes
   * \ref NM_EV_ADD_CONTROLLER and \ref NM_EV_ADD_NODE_STATUS_DONE.  Replace
   * goes to \ref NM_REPLACE_FAILED_REQ and fakes
   * \ref NM_EV_REPLACE_FAILED_DONE.
   */
  NM_PROXY_INCLUSION_WAIT_NIF,
  /** Add mode state.
   * Smart Start.
   *
   * Smart Start inclusion has failed and gateway is waiting for the
   * node to self destruct.
   *
   * Wait for timeout, then send a NOP and go to
   * \ref NM_WAIT_FOR_TX_TO_SELF_DESTRUCT.
   */
  NM_WAIT_FOR_SELF_DESTRUCT,
  /** Add mode state.
   * Smart Start.
   *
   * The gateway must send NOPs to a failing node and wait for the
   * result, before doing ZW_RemoveFailed().
   *
   * The NOP is sent before entering this state.  Now the gateway
   * waits for protocol callback nop_send_done() to trigger \ref
   * NM_EV_TX_DONE_SELF_DESTRUCT.  Then we start a timer, call
   * ZW_RemoveFailedNode() and go to \ref
   * NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL.
   */
  NM_WAIT_FOR_TX_TO_SELF_DESTRUCT,
  /** Add mode state.
   * Smart Start.
   *
   * Waiting for protocol to complete the ZW_RemoveFailedNode() (or
   * for timeout).  Protocol callback RemoveSelfDestructStatus()
   * triggers sending status to unsolicited destinations with
   * #ResetState() callback.  Timeout calls #ResetState() directly.
   */
  NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL,
} nm_state_t;


/**
 * The DHCP stage is completed.
 *
 * \see #ZIP_EVENT_ALL_IPV4_ASSIGNED
 */
#define NETWORK_UPDATE_FLAG_DHCPv4    0x1
/**
 * Node probe is completed.
 * \see #ZIP_EVENT_ALL_NODES_PROBED
 */
#define NETWORK_UPDATE_FLAG_PROBE     0x2
/**
 * Bridge has told ZIP_Router that it has completed booting.
 * \see #ZIP_EVENT_BRIDGE_INITIALIZED
 */
#define NETWORK_UPDATE_FLAG_VIRTUAL   0x4
#define NETWORK_UPDATE_FLAG_DISABLED  0x80
/**
 * Notify the network management module that some event has occurred.
 *
 * Should be called when \ref ZIP_Router is told that all nodes have IPV4
 * addresses, that the bridge is up, that all nodes have been probed,
 * or when NMS times out while waiting for network update to complete.
 *
 * Determines the gateway status and calls nm_send_reply() when all
 * requirements are fulfilled.
 *
 * The function will test for the gateway status, but it will always
 * assume that the event indicated by \p flag has happened, to ensure
 * that network management does not get stuck.
 *
 * \param flag Mask indicating which events have completed.  Will be or'd to reality.
 */
void NetworkManagement_NetworkUpdateStatusUpdate(u8_t flag);


/**
 * Initialize this module, and if possible send a message from the previous life of
 * the gateway.
 *
 * Some network management commands will cause the Z/IP gateway to reset, Ie it will restart
 * its IPv6 stack, cancel all pending processes, and reprobe the network. This does all take time. The
 * IP reply to the command should only be sent when all the probing is done etc.
 *
 * Allong the boot process NetworkManagement_Init will be called several times. Each time it will
 * check if it is able to send the pending ip package. If the package is sent this function will return 1
 * otherwise it will return 0.
 *
 * if network_management_init_done == 1 calling this function will have no effect.
 * if init is successful \ref network_management_init_done will be set to 1.
 * \return TRUE on success.
 */
int NetworkManagement_Init();


/**
 * Called when a node gets an Ipv4 address assigned.
 */
void NetworkManagement_IPv4_assigned(BYTE node);

/**
 * Called when mdns process has exited
 */
void NetworkManagement_mdns_exited();

/**
 * Get the state of the Network management module
 */
nm_state_t NetworkManagement_getState();

/**
 * Return TRUE if the current network management peer(if any) is the unsolicited destination
 */
BOOL NetworkManagement_is_Unsolicited_peer();

/**
 * Return TRUE if the current network management peer(if any) is the unsolicited destination
 */
BOOL NetworkManagement_is_Unsolicited2_peer();

/**
 * Send an Unsolicited NodeList report to the unsolicited destination
 * if NMS is idle.
 *
 * @return False is NMS is not idle, true otherwise.
 */
BOOL NetworkManagement_SendNodeList_To_Unsolicited();


void NetworkManagement_VirtualNodes_removed();

/*
 * Event from security2 layer to the networkmanagement fsm to notify dsk of the node to be included.
 */
void NetworkManagement_dsk_challenge(s2_node_inclusion_challenge_t *challenge_evt);

/*
 * Event from security2 layer to the networkmanagement fsm to notify about requested keys
 */
void NetworkManagement_key_request(s2_node_inclusion_request_t* inclusion_request);


/**
 * Initiate proxy inclusion, requesting information about the node and ask unsolicited destination
 * for permission to proceed inclusion process.
 */
void  NetworkManagement_start_proxy_inclusion(uint8_t node_id);


/**
 * Initiate proxy replace, requesting information about the node and ask unsolicited destination
 * for permission to proceed inclusion process.
 */
void NetworkManagement_start_proxy_replace(uint8_t node_id) ;

/**
 * Notify the NM state machine about a incoming nif
 */
void NetworkManagement_nif_notify(uint8_t bNodeID,uint8_t* pCmd,uint8_t bLen);

/**
 * Notify the NM state machine about a incoming frame
 */
void NetworkManagement_frame_notify();

/**
 * Notify the NM state machine about all nodes probed
 */
void NetworkManagement_all_nodes_probed();

void
NetworkManagement_smart_start_inclusion(uint8_t inclusion_options, uint8_t *prekit_homeID);

/**
 * Notify that S0 inclusion has started
 */
void NetworkManagement_s0_started();

/**
 * Enable Smart Start mode if pending DSKs exist in provisioning list.
 */
void NetworkManagement_smart_start_init_if_pending();

/**
 *  Called when an Included NIF has been received from SerialAPI
 */
void NetworkManagement_INIF_Received(uint8_t bNodeID, uint8_t INIF_rxStatus,
    uint8_t *INIF_NWI_homeid);

/** Return newly included smart start nodeid only when we are waiting for
 * middleware to finish probing it. Ot
 *
 * \returns NodeID of newly included smart start node if we are waiting for middleware probe to finish.
 * \returns 0 otherwise.
 * */
uint8_t NM_get_newly_included_nodeid(void);

/** Called when node queue forwards a frame to a newly included smart start node.
 * We detect middleware has finished probing when 10 seconds pass without another
 * frame to the node.
 * */
void extend_middleware_probe_timeout(void);

/** Check if Network Management is idle
 *
 * \return TRUE if Network Management state is \ref NM_IDLE
 * \return
 */
int nm_is_idle();
/** @} */
#endif /* NETWORKMANAGEMENT_H_ */
