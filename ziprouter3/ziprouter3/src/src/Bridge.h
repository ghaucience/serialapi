/* © 2014 Silicon Laboratories Inc.
 */
#ifndef BRIDGE_H_
#define BRIDGE_H_
#include "ZW_SendDataAppl.h"
#include "ZW_udp_server.h"
#include "ZW_zip_classcmd.h"

/**
 * \ingroup ip_emulation
 *
 * @{
 */

#define PREALLOCATED_VIRTUAL_NODE_COUNT 4
#define MAX_IP_ASSOCIATIONS (200 + PREALLOCATED_VIRTUAL_NODE_COUNT)

/* EEPROM layout:
 * 2 bytes: element count
 * X bytes: IP_association 1
 * X bytes: IP_association 2
 * ....
 * X bytes: IP_association N */
/* TODO: Remove sizeof(uint16_t) from ASSOCIATION_TABLE_EEPROM_SIZE. It has been factored out as a separate field. */
#define ASSOCIATION_TABLE_EEPROM_SIZE (sizeof(uint16_t) + MAX_IP_ASSOCIATIONS * sizeof(struct IP_association))

/* Bridge association types */
enum ASSOC_TYPE {temporary_assoc, permanent_assoc, local_assoc, proxy_assoc};
#define ASSOC_TYPE_COUNT 4 /* Number of values in enum ASSOC_TYPE */

typedef uint8_t nodeid_t;

typedef enum {BRIDGE_FAIL, BRIDGE_OK} return_status_t;

    /* Logical structure of IP associations are tree as follows with association source node id as roof of the tree
     * second level leaves are association source endpoints and the last level is groupings
     *           association source node-id
     *              /                 \
     *             /                   \
     *       association            association
     *      source endpoint        source endpoint
     *        /      \               /       \
     *       /        \             /         \
     *grouping      grouping    grouping    grouping
     *  | | |       | | |          | | |       | | |
     *associations associations associations associations
     *
     * But following data structure storing it in memory is like a flat table 
     */
 
struct IP_association {
  void *next;
  nodeid_t virtual_id;
  enum ASSOC_TYPE type; /* unsolicited or association */
  uip_ip6addr_t resource_ip; /*Association Destination IP */
  uint8_t resource_endpoint;  /* From the IP_Association command. Association destination endpoint */
  u16_t resource_port;
  uint8_t virtual_endpoint;   /* From the ZIP_Command command */
  uint8_t grouping;
  uint8_t han_nodeid; /* Association Source node ID*/
  uint8_t han_endpoint; /* Association Source endpoint*/
  uint8_t was_dtls;
  uint8_t mark_removal;
}; // __attribute__((packed));   /* Packed because we copy byte-for-byte from mem to eeprom */

enum bridge_state {
  booting,
  initialized,
  initfail,
};

extern enum bridge_state bridge_state;

/* Last error status for bridge */
extern return_status_t bridge_error;
extern uint8_t virtual_nodes_mask[MAX_NODEMASK_LENGTH];

/*
 * Create a bridge association and corresponding virtual node
 */
return_status_t /* RET: Status code BRIDGE_OK or BRIDGE_FAIL */
create_bridge_association(enum ASSOC_TYPE type,
    void (*cb_func)(struct IP_association *),
    ZW_COMMAND_IP_ASSOCIATION_SET *ip_assoc_set_cmd,uint8_t was_dtls, uint8_t fw);

/*
 * Create a local bridge association between two HAN nodes
 */
struct IP_association* create_local_association(ZW_COMMAND_IP_ASSOCIATION_SET *cmd);

/*
 *  List active bridge associations
 */
void list_bridge_associations(void);

/*
 *   Print bridge status to console
 */
void print_bridge_status(void);

/*
 *  Remove a bridge association
 */
void remove_bridge_association(struct IP_association* a);

void remove_bridge_association_and_persist(struct IP_association *a);

/*
 *  Initialize the bridge
 */
void bridge_init(void);

void ApplicationCommandHandler_Bridge(
    BYTE  rxStatus,
    BYTE destNode,
    BYTE  sourceNode,
    ZW_APPLICATION_TX_BUFFER *pCmd,
    BYTE cmdLength);


char is_virtual_node(nodeid_t nid);

/**
 * Update virtual nodes mask
 */
void register_virtual_nodes(void);

/**
 * Boolean, TRUE if this is a bridge library.
 */
extern uint8_t is_bridge_library;


/**
 * Returns true if a matching virtual node session was found
 */
BOOL bridge_virtual_node_commandhandler(
    ts_param_t* p,
    BYTE *pCmd,
    BYTE cmdLength);

/*
 * Clear and persist the association table.
 */
void bridge_clear_association_table(void);

struct IP_association *find_ip_associ_to_remove_by_node_id(nodeid_t assoc_source_id);
uint8_t find_ip_assoc_for_report(
    uint8_t nid,
    uint8_t han_endpoint,
    uint8_t grouping,
    uint8_t index,
    struct IP_association **result);

/**
 * Locate an IP association by its source and destination nodes + desitnation endpoint
 */
struct IP_association* get_ip_assoc_by_nodes(u8_t snode,u8_t dnode, u8_t endpoint);

void resume_bridge_init(void);

/**
 * Invalidate bridge.
 *
 * Clear association table and set state to #booting.
 */
void bridge_reset(void);

/**
 * Locate an IP association by its virtual nodeid.
 */
struct IP_association* get_ip_assoc_by_virtual_node(u8_t virtnode);

/**
 * mark the in-memory ipa for removal in response to one of the bulk remove ipa
 * command or in response to the removal of a node which has multiple associations
 */
void mark_bulk_removals(nodeid_t assoc_source_id, uint8_t assoc_source_endpoint, uint8_t grouping,uip_ip6addr_t *res_ip,uint8_t res_endpoint);

/**
 * Return the in-memory ipa marked for removal in response to one of the bulk remove ipa
 * command or in response to the removal of a node which has multiple associations
 */
struct IP_association *get_ipa_marked_for_removal();
void decrememnt_bulk_ipa_remove();

/**
 * Return the value of global variable bulk_ipa_remove. This variable is used to count the
 * ipa association which are marked to be removed in response to one of the bulk remove ipa
 * command or in response to the removal of a node which has multiple associations 
 */
int bulk_ipa_remove_val();

/* Decrement the bulk_ipa_remove counter. See bulk_ipa_remove_val() for description of bulk_ipa_remove variable */
void decrememnt_bulk_ipa_remove();


/* Return value of the flag which differentiates between bulk ipa removal command and node removal */
int node_removal_val();

struct IP_association_fw_lock_t {
    struct IP_association *locked_a;
    struct ctimer reset_fw_timer;
} IP_association_fw_lock;

/**
 * @}
 */
#endif /* BRIDGE_H_ */
