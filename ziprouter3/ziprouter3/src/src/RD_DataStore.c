/* © 2019 Silicon Laboratories Inc.  */

#include "smalloc.h"
#include "ResourceDirectory.h"
#include "RD_internal.h"
#include "RD_probe_cc_version.h"
//#include "ZIP_Router.h"
#include "ZIP_Router_logging.h"
#include "Serialapi.h"
#include "dev/eeprom.h"
#include "Bridge.h"
#include "DataStore.h"
#include "RD_DataStore.h"

#include "assert.h"

#undef DBG_PRINTF
#define DBG_PRINTF(a,...)

/* TODO-km: fix this when including ZIP_Router.h does not drag in the entire gateway. */
extern uint8_t MyNodeID;
extern uint32_t homeID;

/** Obsoleted eeprom.dat magic number.  This magic indicates that the
 * hdr struct is unversioned.  The gateway will assume v2.61 format
 * and try to convert the data. */
#define RD_MAGIC_V0 0x491F00E5

/** The eeprom.dat magic number.  This magic indicates that the hdr struct
 * includes a versioning field. */
#define RD_MAGIC_V1 0x94F100E5 

/** Convert from eeprom format, version 0.0, to version 2.0
 * (#RD_MAGIC_V1).
 *
 * Invalidate the eeprom file if it is not recognizable as version
 * 0.0.
 */
static void data_store_convert_none_to_2_0(uint8_t old_MyNodeID);
static void data_store_convert_2_0_to_2_1(uint8_t old_MyNodeID);

#include "mDNSService.h"
uint16_t rd_eeprom_read(uint16_t offset,uint8_t len,void* data) {
  //DBG_PRINTF("Reading at %x\n", 0x100 + offset);
  eeprom_read(0x40 + offset,data,len);
  return len;
}

uint16_t rd_eeprom_write(uint16_t offset,uint8_t len,void* data) {
  //DBG_PRINTF("Writing at %x\n", 0x100 + offset);
  if(len) {
    eeprom_write(0x40 + offset,data,len);
  }
  return len;
}

/** Memory device for the Resource directory data store.
 * \ingroup rd_data_store
 */
static const small_memory_device_t nvm_dev  = {
  .offset = offsetof(rd_eeprom_static_hdr_t, smalloc_space),
  .size = sizeof(((rd_eeprom_static_hdr_t*)0)->smalloc_space),
  .psize = 8,
  .read = rd_eeprom_read,
  .write = rd_eeprom_write,
};

#ifdef SMALL_MEMORY_TARGET
static char rd_membuf[0x1000];

static uint16_t rd_mem_read(uint16_t offset,uint8_t len,void* data) {
  memcpy(data,rd_membuf + offset, len);
  return len;
}


static uint16_t rd_mem_write(uint16_t offset,uint8_t len,void* data) {
  memcpy(rd_membuf + offset,data, len);
  return len;
}


const small_memory_device_t rd_mem_dev  = {
  .offset = 1,
  .size = sizeof(rd_membuf),
  .psize = 8,
  .read = rd_mem_read,
  .write = rd_mem_write,
};

void* rd_data_mem_alloc(uint8_t size) {
  uint16_t p;
  if(size ==0) {
    return 0;
  }
  p = smalloc(&rd_mem_dev,size);
  if(p) {
    return rd_membuf + p;
  } else {
    ERR_PRINTF("Out of memory\n");
    return 0;
  }
}

void rd_data_mem_free(void* p) {
  smfree(&rd_mem_dev,(char*)p - rd_membuf);
}
#else
#include <stdlib.h>
void* rd_data_mem_alloc(uint8_t size) {
  return malloc(size);
}
void rd_data_mem_free(void* p) {
  free(p);
}

#endif

void rd_store_mem_free_ep(rd_ep_database_entry_t* ep) {
  if(ep->endpoint_info) rd_data_mem_free(ep->endpoint_info);
  if(ep->endpoint_name) rd_data_mem_free(ep->endpoint_name);
  if(ep->endpoint_location) rd_data_mem_free(ep->endpoint_location);
  rd_data_mem_free(ep);
}

static rd_ep_database_entry_t* rd_data_store_read_ep(uint16_t ptr, rd_node_database_entry_t* n) {
  rd_ep_data_store_entry_t e;
  rd_ep_database_entry_t* ep;

  rd_eeprom_read(ptr,sizeof(rd_ep_data_store_entry_t),&e);

  ep = rd_data_mem_alloc(sizeof(rd_ep_database_entry_t));
  if(!ep) {
    return 0;
  }
  memcpy(&ep->endpoint_info_len, &e, sizeof(rd_ep_data_store_entry_t));

  /*Setup pointers*/
  ep->endpoint_info = rd_data_mem_alloc(e.endpoint_info_len);
  ep->endpoint_name = rd_data_mem_alloc(e.endpoint_name_len);
  ep->endpoint_location = rd_data_mem_alloc(e.endpoint_loc_len);
  ep->endpoint_agg =  rd_data_mem_alloc(e.endpoint_aggr_len);

  if(
      (e.endpoint_info_len && ep->endpoint_info==0) ||
      (e.endpoint_name_len && ep->endpoint_name==0) ||
      (e.endpoint_loc_len && ep->endpoint_location==0) ||
      (e.endpoint_aggr_len && ep->endpoint_agg==0)
    ) {
    rd_store_mem_free_ep(ep);
    return 0;
  }

  ptr += sizeof(rd_ep_data_store_entry_t);
  rd_eeprom_read(ptr,ep->endpoint_info_len,ep->endpoint_info);
  ptr += ep->endpoint_info_len;
  rd_eeprom_read(ptr,ep->endpoint_name_len,ep->endpoint_name);
  ptr += ep->endpoint_name_len;
  rd_eeprom_read(ptr,ep->endpoint_loc_len,ep->endpoint_location);

  ptr += ep->endpoint_loc_len;
  rd_eeprom_read(ptr,ep->endpoint_aggr_len,ep->endpoint_agg);

  ep->node = n;
  return ep;
}

void rd_data_store_cc_version_read(uint16_t ptr_offset, rd_node_database_entry_t *n) {
  cc_version_pair_t cv_pair;
  int cnt = n->node_cc_versions_len / sizeof(cc_version_pair_t);
  int i;
  for(i = 0; i < cnt; i++) {
    rd_eeprom_read(ptr_offset, sizeof(cc_version_pair_t), &cv_pair);
    rd_node_cc_version_set(n, cv_pair.command_class, cv_pair.version);
    ptr_offset += sizeof(cc_version_pair_t);
  }
}

/*
 * Data store layout
 *
 * global_hdr | node_ptr | node_ptr | node_ptr | .... | node_ptr
 *
 * node_hdr | node_name | ep_ptr | ep_ptr | .... | ep_ptr
 *
 * ep_hdr | info | name | location
 *
 * */

rd_node_database_entry_t* rd_data_store_read(uint8_t nodeID) {
  rd_node_database_entry_t *r;
  rd_ep_database_entry_t *ep;
  uint16_t n_ptr[2], e_ptr;
  uint8_t i;

  /*Size of static content of */
  const uint16_t static_size = offsetof(rd_node_database_entry_t,nodename);

  n_ptr[0] = 0;
  rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[nodeID]),sizeof(uint16_t),&n_ptr[0]);

  if(n_ptr[0]==0) {
    DBG_PRINTF("Node %i node not eeprom\n",nodeID);
    return 0;
  }


  r = rd_data_mem_alloc(sizeof(rd_node_database_entry_t));
  if(r==0) {
    ERR_PRINTF("Out of memory\n");
    return 0;
  }
  memset(r,0,sizeof(rd_node_database_entry_t));

  /*Read the first part of the node entry*/
  rd_eeprom_read(n_ptr[0], static_size,r );

  /* Init node_cc_versions and its length */
  r->node_cc_versions_len = controlled_cc_v_size();
  r->node_cc_versions = rd_data_mem_alloc(r->node_cc_versions_len);
  rd_node_cc_versions_set_default(r);
  rd_data_store_cc_version_read(n_ptr[0] + static_size, r);

  /* Due to the limitation of one chunk smalloc space size 0x80, we have to use
   * another pointer to keep the rest of node entry */
  rd_eeprom_read(n_ptr[0] + static_size + r->node_cc_versions_len, sizeof(uint16_t), &n_ptr[1]);

  /*Init the endpoint list */
  LIST_STRUCT_INIT(r,endpoints);

  if(r->nodeNameLen == 0) {
    r->nodename = NULL;
  } else {
    /*Read the node name*/
    r->nodename =  rd_data_mem_alloc(r->nodeNameLen);
    if(r->nodename != NULL) {
      rd_eeprom_read(n_ptr[1], r->nodeNameLen ,r->nodename );
    }
  }

  if(r->dskLen == 0) {
    r->dsk = NULL;
  } else {
    r->dsk =  rd_data_mem_alloc(r->dskLen);
    if(r->dsk != NULL) {
      rd_eeprom_read(n_ptr[1] + r->nodeNameLen, r->dskLen, r->dsk);
    }
  }

  /* PCVS: ProbeCCVersionState is not persisted in eeprom */
  r->pcvs = NULL;



  n_ptr[1] += r->nodeNameLen + r->dskLen;
  for(i=0; i < r->nEndpoints; i++) {
    rd_eeprom_read(n_ptr[1],sizeof(uint16_t),&e_ptr);

    ep =rd_data_store_read_ep(e_ptr,r);
    DBG_PRINTF("EP alloc %p\n",ep);
    assert(ep);
    list_add(r->endpoints,ep);
    n_ptr[1]+=sizeof(uint16_t);
  }

  return r;
}

void rd_data_store_version_get(uint8_t *major, uint8_t *minor) {
   rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t, version_major), sizeof(uint8_t), major);
   rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t, version_minor), sizeof(uint8_t), minor);
}

/**
 * Write a rd_node_database_entry_t and all its endpoints to storage.
 * This is called when a new node is added.
 */
void rd_data_store_nvm_write(rd_node_database_entry_t *n) {
  uint16_t n_ptr[2], e_ptr;
  uint8_t i;
  rd_ep_database_entry_t* ep;

  /*Size of static content of */
  const uint16_t static_size = offsetof(rd_node_database_entry_t,nodename);

  if (n->nEndpoints != list_length(n->endpoints)) {
    ERR_PRINTF("n->nEndpoints:%d\n", n->nEndpoints);
    ERR_PRINTF("list_length(n->endpoints):%d\n", list_length(n->endpoints));
    assert(0);
    DBG_PRINTF("Correcting number of endpoints\n");
    n->nEndpoints = list_length(n->endpoints);
  }

  rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr[0]);
  assert(n_ptr[0]==0);

  n_ptr[0] = smalloc(&nvm_dev, static_size + n->node_cc_versions_len + sizeof(uint16_t));
  if(n_ptr[0]==0) {
    ERR_PRINTF("EEPROM is FULL\n");
    return;
  }

  n_ptr[1] = smalloc(&nvm_dev, n->nodeNameLen + n->dskLen + n->nEndpoints*sizeof(uint16_t));
  if(n_ptr[1]==0) {
    ERR_PRINTF("EEPROM is FULL\n");
    return;
  }

  rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr[0]);
  n_ptr[0]+= rd_eeprom_write(n_ptr[0], static_size,n);
  n_ptr[0]+= rd_eeprom_write(n_ptr[0], n->node_cc_versions_len, n->node_cc_versions);
  n_ptr[0]+= rd_eeprom_write(n_ptr[0], sizeof(uint16_t), &n_ptr[1]);
  n_ptr[1]+= rd_eeprom_write(n_ptr[1], n->nodeNameLen,n->nodename);
  n_ptr[1]+= rd_eeprom_write(n_ptr[1], n->dskLen, n->dsk);

  ep = list_head(n->endpoints);

  /*Now for the endpoints*/
  for(i=0; i < n->nEndpoints; i++) {
    e_ptr = smalloc(&nvm_dev, sizeof(rd_ep_data_store_entry_t) + ep->endpoint_info_len + ep->endpoint_loc_len + ep->endpoint_name_len
        + ep->endpoint_aggr_len);
    assert(e_ptr > 0);
    /*Write the pointer to the endpoint */
    n_ptr[1]+= rd_eeprom_write(n_ptr[1], sizeof(uint16_t),&e_ptr);

    DBG_PRINTF("++++++ ep %i info  len %i name %s \n",n->nodeid,  ep->endpoint_info_len, ep->endpoint_name);
    /*Write the endpoint structure */
    e_ptr+= rd_eeprom_write(e_ptr, sizeof(rd_ep_data_store_entry_t),&(ep->endpoint_info_len));
    e_ptr+= rd_eeprom_write(e_ptr, ep->endpoint_info_len,ep->endpoint_info);
    e_ptr+= rd_eeprom_write(e_ptr, ep->endpoint_name_len,ep->endpoint_name);
    e_ptr+= rd_eeprom_write(e_ptr, ep->endpoint_loc_len,ep->endpoint_location);
    e_ptr+= rd_eeprom_write(e_ptr, ep->endpoint_aggr_len,ep->endpoint_agg);

    ep = list_item_next(ep);
  }
}

void rd_data_store_update(rd_node_database_entry_t *n) {
  uint16_t n_ptr[2], e_ptr;
  uint8_t i;
  rd_ep_database_entry_t* ep;

  /*Size of static content of */
  const uint16_t static_size = offsetof(rd_node_database_entry_t,nodename);

  if (n->nEndpoints != list_length(n->endpoints)) {
    ERR_PRINTF("n->nEndpoints:%d\n", n->nEndpoints);
    ERR_PRINTF("list_length(n->endpoints):%d\n", list_length(n->endpoints));
    assert(0);
    DBG_PRINTF("Correcting number of endpoints\n");
    n->nEndpoints = list_length(n->endpoints);
  }

  rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr[0]);
  if(n_ptr[0]==0) return;
  rd_eeprom_read(n_ptr[0] + static_size + n->node_cc_versions_len, sizeof(uint16_t), &n_ptr[1]);
  if(n_ptr[1]==0) return;

  n_ptr[0]+= rd_eeprom_write(n_ptr[0], static_size,n);
  n_ptr[0]+= rd_eeprom_write(n_ptr[0], n->node_cc_versions_len, n->node_cc_versions);
  n_ptr[1]+= n->nodeNameLen;
  n_ptr[1]+= n->dskLen;

  ep = list_head(n->endpoints);


  /*Now for the endpoints*/
  for(i=0; i < n->nEndpoints; i++) {
    n_ptr[1]+= rd_eeprom_read(n_ptr[1], sizeof(uint16_t),&e_ptr);
    rd_eeprom_write(e_ptr, sizeof(rd_ep_data_store_entry_t),&(ep->endpoint_info_len));
    ep = list_item_next(ep);
  }

}

void rd_data_store_mem_free(rd_node_database_entry_t *n) {
  rd_ep_database_entry_t *ep;

  while( (ep = list_pop(n->endpoints)) ) {
    DBG_PRINTF("EP free %p\n",ep);

    mdns_remove_from_answers(ep);
    rd_store_mem_free_ep(ep);
  }

  if(n->nodename) rd_data_mem_free(n->nodename);
  if(n->dsk) rd_data_mem_free(n->dsk);
  if(n->node_cc_versions) rd_data_mem_free(n->node_cc_versions);
  /* pcvs is not persisted in eeprom */
  if(n->pcvs) free(n->pcvs);

  rd_data_mem_free(n);
}

void rd_data_store_nvm_free(rd_node_database_entry_t *n) {
  uint16_t n_ptr[2], e_ptr;
  uint8_t i;

  /*Size of static content of */
  const uint16_t static_size = offsetof(rd_node_database_entry_t,nodename);

  rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr[0]);
  if(!n_ptr[0]) {
 //   WRN_PRINTF("Tried to free a node from NVM which does not exist\n");
    return;
  }
  rd_eeprom_read(n_ptr[0] + static_size + n->node_cc_versions_len,sizeof(uint16_t),&n_ptr[1]);
  if(!n_ptr[1]) {
    return;
  }
  /*Read the first part of the node entry*/
  smfree(&nvm_dev,n_ptr[0]);
  smfree(&nvm_dev,n_ptr[1]);

  n_ptr[1] += n->nodeNameLen + n->dskLen;
  for(i=0; i < n->nEndpoints; i++) {
    n_ptr[1] += rd_eeprom_read(n_ptr[1],sizeof(uint16_t),&e_ptr);
    smfree(&nvm_dev,e_ptr);
  }

  n_ptr[1] = 0;
  rd_eeprom_write(n_ptr[0] + static_size + n->node_cc_versions_len ,sizeof(uint16_t),&n_ptr[1]);
  n_ptr[0] = 0;
  rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr[0]);
}

void rd_data_store_nvm_free_v20(rd_node_database_entry_t *n) {
  uint16_t n_ptr,e_ptr;
  uint8_t i;

  /*Size of static content of */
  const uint16_t static_size = offsetof(rd_node_database_entry_v20_t, nodename);

  rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr);
  if(!n_ptr) {
 //   WRN_PRINTF("Tried to free a node from NVM which does not exist\n");
    return;
  }
  smfree(&nvm_dev,n_ptr);

  n_ptr += static_size + n->nodeNameLen + n->dskLen;
  for(i=0; i < n->nEndpoints; i++) {
    n_ptr += rd_eeprom_read(n_ptr,sizeof(uint16_t),&e_ptr);
    smfree(&nvm_dev,e_ptr);
  }

  n_ptr = 0;
  rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t,node_ptrs[n->nodeid]),sizeof(uint16_t),&n_ptr);
}

static void data_store_convert_2_0_to_2_1(uint8_t old_MyNodeID) {
  rd_node_database_entry_t *n;
  rd_ep_database_entry_t *e;
  uint16_t n_ptr = 0, e_ptr = 0;
  uint8_t i = 0, j = 0;
  uint8_t maj = 2;
  uint8_t min = 1;
  const uint16_t static_size_v20 = offsetof(rd_node_database_entry_v20_t, nodename);
  const uint16_t static_size_v21 = offsetof(rd_node_database_entry_t, nodename);

  /* Write the v21 version */
  rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t, version_major),1,&maj); // Major Version 2
  rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t, version_minor),1,&min);

  /* Convert the eeprom node by node */
  for(i=1; i<ZW_MAX_NODES; i++) {
    n_ptr = 0;
    rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[i]), sizeof(uint16_t), &n_ptr);
    if(n_ptr != 0) {
      n = rd_data_mem_alloc(sizeof(rd_node_database_entry_t));
      memset(n, 0, sizeof(rd_node_database_entry_t));
      rd_eeprom_read(n_ptr, static_size_v20, n);

      /* JUST_ADDED and ADDED_BY_ME are flagged in node_properties_flags instead
       * of security_flags  */
      if(n->security_flags & OBSOLETED_NODE_FLAG_JUST_ADDED) {
        n->security_flags &= ~OBSOLETED_NODE_FLAG_JUST_ADDED;
        n->node_properties_flags |= RD_NODE_FLAG_JUST_ADDED;
      }
      if(n->security_flags & OBSOLETED_NODE_FLAG_ADDED_BY_ME) {
        n->security_flags &= ~OBSOLETED_NODE_FLAG_ADDED_BY_ME;
        n->node_properties_flags |= RD_NODE_FLAG_ADDED_BY_ME;
      }
      DBG_PRINTF("wake up %d, last awake %d, last update %d, nodeid %d, security flags %d, mode %d, state %d, manufacturer id %d, product type %d, product id %d, node type %d, nEndpoint %d, nAggEndpoint %d\n", n->wakeUp_interval, n->lastAwake, n->lastUpdate, n->nodeid, n->security_flags, n->mode, n->state, n->manufacturerID, n->productType, n->productID, n->nodeType, n->nEndpoints, n->nAggEndpoints);
      LIST_STRUCT_INIT(n, endpoints);
      n->nodename = rd_data_mem_alloc(n->nodeNameLen);
      rd_eeprom_read(n_ptr + static_size_v20, n->nodeNameLen, n->nodename);

      n->dsk = rd_data_mem_alloc(n->dskLen);
      rd_eeprom_read(n_ptr + static_size_v20 + n->nodeNameLen, n->dskLen, n->dsk);

      n->node_cc_versions_len = controlled_cc_v_size();
      n->node_cc_versions = rd_data_mem_alloc(n->node_cc_versions_len);
      rd_node_cc_versions_set_default(n);
      /* v20 eeprom contains no node_cc_version */
      n_ptr+= static_size_v20 + n->nodeNameLen + n->dskLen;

      for(j=0; j < n->nEndpoints; j++) {
        n_ptr+= rd_eeprom_read(n_ptr,sizeof(uint16_t),&e_ptr);

        e =rd_data_store_read_ep(e_ptr, n);
        DBG_PRINTF("EP alloc %p\n",e);
        assert(e);
        list_add(n->endpoints, e);
      }
      /* Free the v20 node entry in v20 smalloc space */
      rd_data_store_nvm_free_v20(n);
      /* Write v21 node entry from v21 malloc space into v21 smalloc space */
      rd_data_store_nvm_write(n);
      /* Free the v21 malloc space */
      rd_data_store_mem_free(n);
    }
  }
}

static void data_store_convert_none_to_2_0(uint8_t old_MyNodeID) {
   rd_node_database_entry_t r;
   uint8_t maj = 2;
   uint8_t min = 0;
   uint32_t magic = RD_MAGIC_V1; 
   uint8_t i;
   uint16_t n_ptr = 0;
   uint8_t zero = 0;
   uint8_t state_version_offset = 0;
   rd_node_state_t node_state;

    WRN_PRINTF("Found old eeprom, attempting conversion.\n");

    /* No need to insert bytes in eeprom file for version_major, version_minor as by
       structure alignment on 32 bit systems, these fields take offset where there was garbage
       so we just neeed to set those bytes at those offsets. Same for dskLen */
    rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t, version_major),1,&maj); // Major Version 2
    rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t, version_minor),1,&min); // Minor Version 0
    rd_eeprom_write(offsetof(rd_eeprom_static_hdr_t, magic),sizeof(magic), &magic);

    /* Read in gw info from eeprom to determine version. */
    rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t, node_ptrs[old_MyNodeID]),
                   sizeof(uint16_t), &n_ptr);
    if (n_ptr < offsetof(rd_eeprom_static_hdr_t, smalloc_space)) {
       ERR_PRINTF("Invalid gateway node ptr 0x%x for gateway node id %d\n",
                  n_ptr, old_MyNodeID);
       rd_data_store_invalidate();
       return;
    }
    rd_eeprom_read(n_ptr + offsetof(rd_node_database_entry_t, state),
                   sizeof(node_state), &node_state);
    if (node_state < 0xA || node_state > 0xB) {
       /* Gateway probe state must always be DONE. */
       ERR_PRINTF("Invalid eeprom file\n");
       rd_data_store_invalidate();
       return;
    }
    state_version_offset = STATUS_DONE - node_state;
    for (i = 1; i < ZW_MAX_NODES; i++) {
        n_ptr = 0;
        rd_eeprom_read(offsetof(rd_eeprom_static_hdr_t,node_ptrs[i]),sizeof(uint16_t),&n_ptr);
        if (n_ptr < offsetof(rd_eeprom_static_hdr_t, smalloc_space)) {
           if (n_ptr != 0) {
              WRN_PRINTF("Skipping invalid node ptr 0x%x (0x%x) for position %d\n",
                         n_ptr, offsetof(rd_eeprom_static_hdr_t, smalloc_space), i);
           }
           continue;
        }
        WRN_PRINTF("Converting node: %d\n", i);

        /* Update node state */
        /* Default to a sane state in case read fails. */
        node_state = STATUS_DONE-state_version_offset;
        rd_eeprom_read(n_ptr + offsetof(rd_node_database_entry_t, state),
                       sizeof(node_state), &node_state);
        node_state += state_version_offset;
        rd_eeprom_write(n_ptr + offsetof(rd_node_database_entry_t, state),
                        sizeof(node_state), &node_state);
        /* Write 0 (of size 1 byte) at dskLen */
        rd_eeprom_write(n_ptr + offsetof(rd_node_database_entry_t, dskLen),
                        sizeof(r.dskLen), &zero);
        /* Not writing DSK as we set dsk len to zero */
    }
}

void data_store_init() {
  /* init: Precondition: SerialAPI up */
  char buf[offsetof(rd_eeprom_static_hdr_t,node_ptrs)];
  uint8_t i;
  uint16_t ptr,q;
  rd_eeprom_static_hdr_t* hdr = (rd_eeprom_static_hdr_t*)buf;

  /* Following checks are done to make sure the destination system is 32bit
     as till now we had only 32 bit gateways. This works on i386 and arm. If
     somehow we try to convert eeprom file written in 32bit machine to 64 bit machine
     anyways it will be Reformatted */
  if((offsetof(rd_node_database_entry_t, dskLen) != 61) || 
      (offsetof(rd_eeprom_static_hdr_t, version_major) != 9) ||
      (offsetof(rd_eeprom_static_hdr_t, version_minor) != 10)) {

      ERR_PRINTF("Structure misalignment between gateway and eeprom.dat file?\n");
      assert(0);
  }

  rd_eeprom_read(0,sizeof(buf),buf);
  if (hdr->magic == RD_MAGIC_V0) {
     WRN_PRINTF("Converting eeprom v0 to v2.0\n");
     data_store_convert_none_to_2_0(hdr->nodeID);
  }
  rd_eeprom_read(0,sizeof(buf),buf);
  if ((hdr->magic == RD_MAGIC_V1)
      && (((hdr->version_major == 2) && (hdr->version_minor == 0))
          || ((hdr->version_major == 0) && (hdr->version_minor ==0)))) {
    WRN_PRINTF("Converting eeprom v2.0 to v2.1\n");
    data_store_convert_2_0_to_2_1(hdr->nodeID);
  }
  rd_eeprom_read(0,sizeof(buf),buf);
  /* Note: Controllers dont necessarily change homeid after SetDefault. */
  if(hdr->magic != RD_MAGIC_V1 || hdr->homeID !=homeID || hdr->nodeID != MyNodeID)
  {
    WRN_PRINTF("Reformatting eeprom\n");
    memset(buf,0,sizeof(buf));
    hdr->magic = RD_MAGIC_V1;
    hdr->version_major = RD_EEPROM_VERSION_MAJOR;
    hdr->version_minor = RD_EEPROM_VERSION_MINOR;
    hdr->homeID = homeID;
    hdr->flags = 0;
    hdr->nodeID = MyNodeID;

    ptr = rd_eeprom_write(0,sizeof(buf),buf);
    for(i=0; i < ZW_MAX_NODES; i++) {
      q=0;
      ptr += rd_eeprom_write(ptr,sizeof(uint16_t),&q);
    }
    smformat(&nvm_dev);

    bridge_reset();
  }
#ifdef SMALL_MEMORY_TARGET
  /*Reset the mem device*/
  smformat(&rd_mem_dev);
#endif
}

/** Invalidate the data store, so that it is reformatted on SetDefault. 
 * \ingroup rd_data_store
*/
void rd_data_store_invalidate()
{
  const uint32_t data = 0;

  uint16_t o = offsetof(rd_eeprom_static_hdr_t, magic);
  rd_eeprom_write(o, sizeof(uint32_t), (void*)&data);
}
