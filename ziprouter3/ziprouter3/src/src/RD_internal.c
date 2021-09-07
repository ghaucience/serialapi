/* © 2019 Silicon Laboratories Inc.  */

#include "ResourceDirectory.h"
#include "ZW_classcmd_ex.h"
#include "RD_internal.h"
#include "RD_DataStore.h"
#include "TYPES.H"
#include "ZW_transport_api.h"
#include "ZIP_Router_logging.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* TODO-km: fix this when including ZIP_Router.h does not drag in the entire gateway. */
extern uint8_t MyNodeID;
extern uint32_t homeID;
/* TODO-ath: Consider to move this to a GW component, ref: ZGW-1243 */
const cc_version_pair_t controlled_cc_v[] = {{COMMAND_CLASS_VERSION, 0x0},
                                             {COMMAND_CLASS_ZWAVEPLUS_INFO, 0x0},
                                             {COMMAND_CLASS_MANUFACTURER_SPECIFIC, 0x0},
                                             {COMMAND_CLASS_WAKE_UP, 0x0},
                                             {COMMAND_CLASS_MULTI_CHANNEL_V4, 0x0},
                                             {COMMAND_CLASS_ASSOCIATION, 0x0},
                                             {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3, 0x0},
                                             {0xffff, 0xff}};
/* This eats memory ! */
/**
 * The node database.
 */
static rd_node_database_entry_t* ndb[ZW_MAX_NODES];

uint8_t controlled_cc_v_size()
{
  return sizeof(controlled_cc_v);
}

void rd_node_cc_versions_set_default(rd_node_database_entry_t *n)
{
  if(!n)
    return;
  if(n->node_cc_versions) {
    memcpy(n->node_cc_versions, controlled_cc_v, n->node_cc_versions_len);
  } else {
    WRN_PRINTF("Node CC version set default failed.\n");
  }
}

uint8_t rd_node_cc_version_get(rd_node_database_entry_t *n, uint16_t command_class)
{
  int i, cnt;
  uint8_t version;

  if(!n) {
    return 0;
  }

  if(!n->node_cc_versions) {
    return 0;
  }

  if(n->mode & MODE_FLAGS_DELETED) {
    return 0;
  }

  version = 0;
  cnt = n->node_cc_versions_len / sizeof(cc_version_pair_t);

  for(i = 0; i < cnt ; i++) {
    if(n->node_cc_versions[i].command_class == command_class) {
      version = n->node_cc_versions[i].version;
      break;
    }
  }
  return version;
}

void rd_node_cc_version_set(rd_node_database_entry_t *n, uint16_t command_class, uint8_t version)
{
  int i, cnt;

  if(!n) {
    return;
  }

  if(!n->node_cc_versions) {
    return;
  }

  if(n->mode & MODE_FLAGS_DELETED) {
    return;
  }
  cnt = n->node_cc_versions_len / sizeof(cc_version_pair_t);

  for(i = 0; i < cnt; i++) {
    if(n->node_cc_versions[i].command_class == command_class) {
      n->node_cc_versions[i].version = version;
      break;
    }
  }
}

rd_node_database_entry_t* rd_node_entry_alloc(uint8_t nodeid)
{
   rd_node_database_entry_t* nd = rd_data_mem_alloc(sizeof(rd_node_database_entry_t));
   ndb[nodeid - 1] = nd;
   if (nd != NULL) {
      memset(nd, 0, sizeof(rd_node_database_entry_t));
      nd->nodeid = nodeid;

      /*When node name is 0, then we use the default names*/
      nd->nodeNameLen = 0;
      nd->nodename = NULL;
      nd->dskLen = 0;
      nd->dsk = NULL;
      nd->pcvs = NULL;
      nd->state = STATUS_CREATED;
      nd->mode = MODE_PROBING;
      nd->security_flags = 0;
      nd->wakeUp_interval = DEFAULT_WAKE_UP_INTERVAL ; //Default wakeup interval is 70 minutes
      nd->node_cc_versions_len = controlled_cc_v_size();
      nd->node_cc_versions = rd_data_mem_alloc(nd->node_cc_versions_len);
      rd_node_cc_versions_set_default(nd);
      nd->node_version_cap_and_zwave_sw = 0x00;
      nd->probe_flags = 0x0000;
      nd->node_is_zws_probed = 0x00;
      nd->node_properties_flags = 0x0000;

      LIST_STRUCT_INIT(nd, endpoints);
   }

   return nd;
}

rd_node_database_entry_t* rd_node_entry_import(uint8_t nodeid)
{
   ndb[nodeid-1] = rd_data_store_read(nodeid);
   return ndb[nodeid-1];
}

void rd_node_entry_free(uint8_t nodeid)
{
   rd_node_database_entry_t* nd = ndb[nodeid - 1];
   rd_data_store_nvm_free(nd);
   rd_data_store_mem_free(nd);
   ndb[nodeid - 1] = NULL;
}

rd_node_database_entry_t* rd_node_get_raw(uint8_t nodeid)
{
   return ndb[nodeid - 1];
}


void
rd_destroy()
{
  u8_t i;
  rd_node_database_entry_t *n;
  for (i = 0; i < ZW_MAX_NODES; i++)
  {
    n = ndb[i];
    if (n)
    {
      rd_data_store_mem_free(n);
      ndb[i] = 0;
    }
  }
}

u8_t rd_node_exists(u8_t node)
{
  if (node > 0 && node <= ZW_MAX_NODES)
  {
    return (ndb[node - 1] != 0);
  }
  return FALSE;
}

rd_ep_database_entry_t* rd_ep_first(uint8_t node)
{
  int i;

  if (node == 0)
  {
    for (i = 0; i < ZW_MAX_NODES; i++)
    {
      if (ndb[i])
      {
        return list_head(ndb[i]->endpoints);
      }
    }
  }
  else if (ndb[node - 1])
  {
    return list_head(ndb[node - 1]->endpoints);
  }
  return 0;
}

rd_ep_database_entry_t* rd_ep_next(uint8_t node, rd_ep_database_entry_t* ep)
{
  uint8_t i;
  rd_ep_database_entry_t* next = list_item_next(ep);

  if (next == 0 && node == 0)
  {
    for (i = ep->node->nodeid; i < ZW_MAX_NODES; i++)
    {
      if (ndb[i])
      {
        return list_head(ndb[i]->endpoints);
      }
    }
  }
  return next;
}

u8_t
rd_get_node_name(rd_node_database_entry_t* n, char* buf, u8_t size)
{
  if (n->nodeNameLen)
  {
    if (size > n->nodeNameLen)
    {
      size = n->nodeNameLen;
    }
    memcpy(buf, n->nodename, size);
    return size;
  }
  else
  {
    return snprintf(buf, size, "zw%08X%02X", UIP_HTONL(homeID), n->nodeid);
  }
}

rd_node_database_entry_t* rd_lookup_by_node_name(const char* name)
{
  uint8_t i, j;
  char buf[64];
  for (i = 0; i < ZW_MAX_NODES; i++)
  {
    if (ndb[i])
    {
      j = rd_get_node_name(ndb[i], buf, sizeof(buf));
      if (strncasecmp(buf,name, j) == 0)
      {
        return ndb[i];
      }
    }
  }
  return 0;
}

void rd_node_add_dsk(u8_t node, uint8_t dsklen, const uint8_t *dsk)
{
   rd_node_database_entry_t *nd;

   if ((dsklen == 0) || (dsk == NULL)) {
      return;
   }

   nd = rd_node_get_raw(node);
   if (nd) {
      rd_node_database_entry_t* old_nd = rd_lookup_by_dsk(dsklen, dsk);
      if (old_nd) {
         /* Unlikely, but possible: the same device gets added again. */
         DBG_PRINTF("New node id %d replaces existing node id %d for device with this dsk.\n",
                    node, old_nd->nodeid);
         rd_data_mem_free(old_nd->dsk);
         old_nd->dsk = NULL;
         old_nd->dskLen = 0;
         /* TODO: Should the node id also be set failing here? */
      }
      if (nd->dskLen != 0) {
         /* TODO: this is not supposed to happen - replace DSK is not supported */
         assert(nd->dskLen == 0);
         if (nd->dsk != NULL) {
            WRN_PRINTF("Replacing old dsk\n");
            /* Silently replace, for now */
            rd_data_mem_free(nd->dsk);
            nd->dskLen = 0;
         }
      }

      nd->dsk = rd_data_mem_alloc(dsklen);
      if (nd->dsk) {
         memcpy(nd->dsk, dsk, dsklen);
         nd->dskLen = dsklen;
         DBG_PRINTF("Setting dsk 0x%02x%02x%02x%02x... on node %u.\n",
                    dsk[0],dsk[1],dsk[2],dsk[3],node);
      } else {
         /* TODO: should we return an error here. */
         nd->dskLen = 0;
      }
   }
   /* TODO: should we return an error if no nd?. */
}

rd_node_database_entry_t* rd_lookup_by_dsk(uint8_t dsklen, const uint8_t* dsk)
{
   uint8_t ii;

   if (dsklen == 0) {
      return NULL;
   }

   for (ii = 0; ii < ZW_MAX_NODES; ii++)
   {
      if (ndb[ii] && (ndb[ii]->dskLen >= dsklen))
      {
         if (memcmp(ndb[ii]->dsk, dsk, dsklen) == 0)
         {
            return ndb[ii];
         }
      }
   }
   return NULL;
}

rd_node_database_entry_t* rd_get_node_dbe(uint8_t nodeid)
{
  //ASSERT(nodeid>0);
  if (nodeid == 0)
    return 0;
  if (ndb[nodeid - 1])
    ndb[nodeid - 1]->refCnt++;
  return ndb[nodeid - 1];
}
