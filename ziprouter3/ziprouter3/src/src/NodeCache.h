/* © 2014 Silicon Laboratories Inc.
 */

#ifndef NODECACHE_H_
#define NODECACHE_H_

#include"TYPES.H"
#include"sys/clock.h"

#ifdef __ASIX_C51__
#undef data
#define data _data
#endif
typedef enum UpdateStatus {
	UPDATE_STATUS_OK=0,
	UPDATE_STATUS_NOT_RESPONDING=1,
	UPDATE_STATUS_UNKNOWN=2,
	UPDATE_STATUS_RESOURCE_UNAVAIL=3
} UpdateStatus_t;

/**
 * Place holder for a node Info.
 * After this structure comes a list of classes.
 */

struct NodeInfo {
    BYTE      basicDeviceClass;             /**/
    BYTE      genericDeviceClass;           /**/
    BYTE      specificDeviceClass;          /**/
//    BYTE      classes[0];
};

typedef struct NodeCacheHdr
{
  DWORD homeid; //Home id of which this cache belongs.
  BYTE n_nodes; //Number of nodes in cache
} NodeCacheHdr_t;

#define NIF_MAX_SIZE 64
struct NodeCacheEntry {
  BYTE node;
  BYTE info_length;
  BYTE sec_info_length;
  clock_time_t last_update;
  BYTE flags;
  BYTE node_info[NIF_MAX_SIZE];
  BYTE sec_info[NIF_MAX_SIZE];
};


#define CACHE_DATA_OFFSET (EEOFFSET_NODE_CACHE_START + sizeof(NodeCacheHdr_t))
#define NCE_OFFSET(node) (CACHE_DATA_OFFSET + (node * sizeof(struct NodeCacheEntry)))


#define NODECACHE_EEPROM_SIZE (sizeof(NodeCacheHdr_t) + 233*sizeof(struct NodeCacheEntry))


#define NODE_FLAG_SECURITY0      0x01
#define NODE_FLAG_KNOWN_BAD     0x02

#define NODE_FLAG_INFO_ONLY     0x08 /* Only probe the node info */
#define NODE_FLAG_SECURITY2_UNAUTHENTICATED 0x10
#define NODE_FLAG_SECURITY2_AUTHENTICATED   0x20
#define NODE_FLAG_SECURITY2_ACCESS          0x40

#define NODE_FLAGS_SECURITY2 (\
    NODE_FLAG_SECURITY2_UNAUTHENTICATED| \
    NODE_FLAG_SECURITY2_AUTHENTICATED| \
    NODE_FLAG_SECURITY2_ACCESS)

#define NODE_FLAGS_SECURITY (\
    NODE_FLAG_SECURITY0 | \
    NODE_FLAG_KNOWN_BAD| \
    NODE_FLAGS_SECURITY2)

#define CACHE_INFO_REQ 1
#define CACHE_SEC_INFO_REQ 2


#define INFINITE_AGE 0xf

#define  isNodeBad(n) ((GetCacheEntryFlag(n) & NODE_FLAG_KNOWN_BAD) !=0)
#define  isNodeSecure(n) ( (GetCacheEntryFlag(n) & (NODE_FLAG_SECURITY0 | NODE_FLAG_KNOWN_BAD)) == NODE_FLAG_SECURITY0)
int isNodeController(BYTE nodeid);

/**
 * Initializeses internal data structures.
 */
BOOL CacheInit();

/**
 * Add/Update node cache entry
 *
 * field should be of either CACHE_FIELD_INFO or CACHE_FIELD_SECURITY
 */
void UpdateCacheEntry(BYTE nodeid, BYTE* data, BYTE data_len,BYTE field) CC_REENTRANT_ARG;

/**
 * Remove cache entry from node cache.
 */
void ClearCacheEntry(BYTE nodeid) CC_REENTRANT_ARG;

/**
 * Retrieve Cache entry
 * If info is NULL only the length of the node cache entry is returned.
 *
 * @sa GetCacheSecureClasses
 * @param nodeid Node in question
 * @param maxage Maximum age of the node cache entry.
 * @param callback This function is called when the node cache entry has been retrieved.
 */
void GetCacheNodeInfo(BYTE nodeid,BYTE maxage, void (*callback)(struct NodeCacheEntry* e,UpdateStatus_t status));


int get_entry_age(struct NodeCacheEntry* e);
struct NodeCacheEntry* get_cache_entry(BYTE nodeid);
/**
 * Retrieve a list of classes which are handled securely.
 * @param[in]  nodeid Node to be queried.
 * @param[in]  maxage Maximum age of the requested entry in a log time scale.
 * The maxage is calculated as 2^age minutes, age==INFINITE_AGE is infinite.
 * If the cache entry is older than the requested age the entry an attempt is
 * made to update the cache entry.
 * @param[out] classes Destination buffer to hold the node id.
 * @param[out] age  The actual age of the returned entry, calculated as in maxage
 *
 * @return The length of the class list in bytes
 */
int GetCacheSecureClasses(BYTE nodeid,BYTE maxage, BYTE* classes,  BYTE* age);

/**
 * Retrieve Cache entry flag
 */
BYTE GetCacheEntryFlag(BYTE nodeid) CC_REENTRANT_ARG;

/**
 * Checks if a given node supports a command class non secure.
 * @param nodeid ID of the node to check
 * @param class Class code to check for. This may be an extended class.
 * @retval TRUE The node supports this class
 * @retval FALSE The node does not support this class.
 */
int SupportsCmdClass(BYTE nodeid, WORD class);

/**
 * Checks if a given node supports a command class securely.
 * \param nodeid ID of the node to check
 * \param class Class code to check for. This may be an extended class.
 * \retval TRUE The node supports this class
 * \retval FALSE The node does not support this class.
 */
int SupportsCmdClassSecure(BYTE nodeid, WORD class)CC_REENTRANT_ARG;


/**
 * Set node attribute flags
 */
BYTE SetCacheEntryFlagMasked(BYTE nodeid,BYTE value, BYTE mask)CC_REENTRANT_ARG;


/**
 * Used to indicate that node info was received or timed out from protocol side.
 */
void NodeInfoRequestDone(BYTE status);


/**
 * Used to indicate that a secure node info was received or timed out from protocol side.
 */
void NodeInfoSecureRequestDone();

/**
 * Attempt to do a live update of a node
 * @param[in] nodeID The node to update.
 */
void CacheRefreshNode(BYTE nodeID);


/**
 * Refresh all nodes, by forcing them to send a node info.
 */
void CacheRefreshAllNodes();


void request_nodeupdate(BYTE node,BOOL skip_info, void(*callback)(struct NodeCacheEntry* e,UpdateStatus_t status))CC_REENTRANT_ARG;

BOOL nodeExsists(BYTE nodeid);

/**
 * Check for updates in the node lists and do probe of new nodes.
 */
u8_t CacheProbeNewNodes();

/**
 * Get the number of pending node probes
 * @return
 */
u8_t getPendingUpdatesCount();
#endif /* NODECACHE_H_ */
