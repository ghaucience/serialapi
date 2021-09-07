/* © 2019 Silicon Laboratories Inc.  */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

/*------------------------------------------------------*/
/*      Taken from Resource Directory                   */
/*------------------------------------------------------*/

typedef union uip_ip6addr_t {
  uint8_t  u8[16];         /* Initializer, must come first!!! */
  uint16_t u16[8];
} uip_ip6addr_t ;

#define ZW_MAX_NODES          232
#define RD_SMALLOC_SIZE    0x5E00

#define PREALLOCATED_VIRTUAL_NODE_COUNT 4
#define MAX_IP_ASSOCIATIONS (200 + PREALLOCATED_VIRTUAL_NODE_COUNT)
#define ENDIAN(hex) (((hex & 0x00ff) << 8) | ((hex & 0xff00) >> 8))

/** Obsoleted eeprom.dat magic number.  This magic indicates that the
 * hdr struct is unversioned.  The gateway will assume v2.61 format
 * and try to convert the data. */
#define RD_MAGIC_V0 0x491F00E5

/** The eeprom.dat magic number.  This magic indicates that the hdr struct
 * includes a versioning field. */
#define RD_MAGIC_V1 0x94F100E5

typedef uint8_t nodeid_t;
enum ASSOC_TYPE {temporary_assoc, permanent_assoc, local_assoc, proxy_assoc};

typedef struct cc_version_pair {
  uint16_t command_class;
  uint8_t version;
} cc_version_pair_t;

typedef enum {
  /* Initial state for probing version. */
  PCV_IDLE,
  /* GW is sending VERSION_COMMAND_CLASS_GET and wait for cc_version_callback to store the version. */
  PCV_SEND_VERSION_CC_GET,
  /* Check if this is the last command class we have to ask. */
  PCV_LAST_REPORT,
  /* Basic CC version probing is done. Check if this is a Version V3 node. */
  PCV_CHECK_IF_V3,
  /* It's a V3 node. GW is sending VERSION_CAPABILITIES_GET and wait for
   * version_capabilities_callback. */
  PCV_SEND_VERSION_CAP_GET,
  /* The node supports ZWS. GW is sending VERSION_ZWAVE_SOFTWARE_GET and wait
   * for version_zwave_software_callback. */
  PCV_SEND_VERSION_ZWS_GET,
  /* Full version probing is done. Finalize/Clean up the probing. */
  PCV_VERSION_PROBE_DONE,
} pcv_state_t;

typedef void (*_pcvs_callback)(void *user, uint8_t status_code);

typedef struct ProbeCCVersionState {
  /* The index for controlled_cc, mainly for looping over the CC */
  uint8_t probe_cc_idx;
  /* Global probing state */
  pcv_state_t state;
  /* The callback will be lost after sending asynchronous ZW_SendRequest. Keep the callback here for going back */
  _pcvs_callback callback;
} ProbeCCVersionState_t;

struct IP_association {
  void *next;
  nodeid_t virtual_id;
  enum ASSOC_TYPE type; /* unsolicited or association */
  uip_ip6addr_t resource_ip; /*Association Destination IP */
  uint8_t resource_endpoint;  /* From the IP_Association command. Association destination endpoint */
  uint16_t resource_port;
  uint8_t virtual_endpoint;   /* From the ZIP_Command command */
  uint8_t grouping;
  uint8_t han_nodeid; /* Association Source node ID*/
  uint8_t han_endpoint; /* Association Source endpoint*/
  uint8_t was_dtls;
  uint8_t mark_removal;
}; // __attribute__((packed));   /* Packed because we copy byte-for-byte from mem to eeprom */

#define ASSOCIATION_TABLE_EEPROM_SIZE (sizeof(uint16_t) + MAX_IP_ASSOCIATIONS * sizeof(struct IP_association))

typedef enum {
  MODE_PROBING,
  MODE_NONLISTENING,
  MODE_ALWAYSLISTENING,
  MODE_FREQUENTLYLISTENING,
  MODE_MAILBOX,
} rd_node_mode_t;

typedef enum {
  //STATUS_ADDING,
  STATUS_CREATED,
  //STATUS_PROBE_PROTOCOL_INFO,
  STATUS_PROBE_NODE_INFO,
  STATUS_PROBE_PRODUCT_ID,
  STATUS_ENUMERATE_ENDPOINTS,
  STATUS_SET_WAKE_UP_INTERVAL,
  STATUS_ASSIGN_RETURN_ROUTE,
  STATUS_PROBE_WAKE_UP_INTERVAL,
  STATUS_PROBE_ENDPOINTS,
  STATUS_MDNS_PROBE,
  STATUS_MDNS_EP_PROBE,
  STATUS_DONE,
  STATUS_PROBE_FAIL,
  STATUS_FAILING,
} rd_node_state_t;

typedef enum {
    EP_STATE_PROBE_INFO,
    EP_STATE_PROBE_SEC_INFO,
    EP_STATE_PROBE_ZWAVE_PLUS,
    EP_STATE_MDNS_PROBE,
    EP_STATE_MDNS_PROBE_IN_PROGRESS,
    EP_STATE_PROBE_DONE,
    EP_STATE_PROBE_FAIL
} rd_ep_state_t;

typedef void ** list_t;

#define LIST_CONCAT2(s1, s2) s1##s2
#define LIST_CONCAT(s1, s2) LIST_CONCAT2(s1, s2)
#define LIST_STRUCT(name) \
         void *LIST_CONCAT(name,_list); \
         list_t name

/**********************************/
/*          Ancient format        */
/**********************************/
typedef struct rd_ep_data_store_entry_ancient {
  uint8_t endpoint_info_len;
  uint8_t endpoint_name_len;
  uint8_t endpoint_loc_len;
  uint8_t endpoint_id;
  rd_ep_state_t state;
  uint16_t iconID;
} rd_ep_data_store_entry_ancient_t;

typedef struct rd_eeprom_static_hdr_ancient {
  uint32_t magic;
  uint32_t homeID;
  uint8_t  nodeID;
  uint32_t flags;
  uint16_t node_ptrs[ZW_MAX_NODES];
  uint8_t  smalloc_space[RD_SMALLOC_SIZE];
  uint8_t temp_assoc_virtual_nodeid_count;
  nodeid_t temp_assoc_virtual_nodeids[PREALLOCATED_VIRTUAL_NODE_COUNT];
  uint16_t association_table_length;
  uint8_t association_table[ASSOCIATION_TABLE_EEPROM_SIZE];
} rd_eeprom_static_hdr_ancient_t;

typedef struct rd_node_database_entry_ancient {

  uint32_t wakeUp_interval;
  uint32_t lastAwake;
  uint32_t lastUpdate;

  uip_ip6addr_t ipv6_address;

  uint8_t nodeid;
  uint8_t security_flags;
  /*uint32_t homeID;*/

  rd_node_mode_t mode;
  rd_node_state_t state;

  uint16_t manufacturerID;
  uint16_t productType;
  uint16_t productID;

  uint8_t nodeType; // Is this a controller, routing slave ... etc

  uint8_t refCnt;
  uint8_t nEndpoints;

  LIST_STRUCT(endpoints);

  uint8_t nodeNameLen;
  char* nodename;
} rd_node_database_entry_ancient_t;

/**********************************/
/*           v0 format          */
/**********************************/
typedef struct rd_ep_data_store_entry_v0 {
  uint8_t endpoint_info_len;
  uint8_t endpoint_name_len;
  uint8_t endpoint_loc_len;
  uint8_t endpoint_aggr_len;
  uint8_t endpoint_id;
  rd_ep_state_t state;
  uint16_t iconID;
} rd_ep_data_store_entry_v0_t;
typedef rd_eeprom_static_hdr_ancient_t rd_eeprom_static_hdr_v0_t;

typedef struct rd_node_database_entry_v0 {

  uint32_t wakeUp_interval;
  uint32_t lastAwake;
  uint32_t lastUpdate;

  uip_ip6addr_t ipv6_address;

  uint8_t nodeid;
  uint8_t security_flags;
  /*uint32_t homeID;*/

  rd_node_mode_t mode;
  rd_node_state_t state;

  uint16_t manufacturerID;
  uint16_t productType;
  uint16_t productID;

  uint8_t nodeType; // Is this a controller, routing slave ... etc

  uint8_t refCnt;
  uint8_t nEndpoints;
  uint8_t nAggEndpoints;

  LIST_STRUCT(endpoints);

  uint8_t nodeNameLen;
  char* nodename;
} rd_node_database_entry_v0_t;

/**********************************/
/*           v2.0 format          */
/**********************************/
typedef rd_ep_data_store_entry_v0_t rd_ep_data_store_entry_v20_t;

typedef struct rd_eeprom_static_hdr_v20 {
  uint32_t magic;
   /** Home ID of the stored gateway. */
  uint32_t homeID;
   /** Node ID of the stored gateway. */
  uint8_t  nodeID;
  uint8_t version_major;
  uint8_t version_minor;
  uint32_t flags;
  uint16_t node_ptrs[ZW_MAX_NODES];
  /** The area used for dynamic allocations with the \ref small_mem. */
  uint8_t  smalloc_space[RD_SMALLOC_SIZE];
  uint8_t temp_assoc_virtual_nodeid_count;
  nodeid_t temp_assoc_virtual_nodeids[PREALLOCATED_VIRTUAL_NODE_COUNT];
  uint16_t association_table_length;
  uint8_t association_table[ASSOCIATION_TABLE_EEPROM_SIZE];
} rd_eeprom_static_hdr_v20_t;

typedef struct rd_node_database_entry_v20 {

  uint32_t wakeUp_interval;
  uint32_t lastAwake;
  uint32_t lastUpdate;

  uip_ip6addr_t ipv6_address;

  uint8_t nodeid;
  uint8_t security_flags;
  /*uint32_t homeID;*/

  rd_node_mode_t mode;
  rd_node_state_t state;

  uint16_t manufacturerID;
  uint16_t productType;
  uint16_t productID;

  uint8_t nodeType; // Is this a controller, routing slave ... etc

  uint8_t refCnt;
  uint8_t nEndpoints;
  uint8_t nAggEndpoints;

  LIST_STRUCT(endpoints);

  uint8_t nodeNameLen;
  uint8_t dskLen;
  char* nodename;
  uint8_t *dsk;

} rd_node_database_entry_v20_t;

/**********************************/
/*           v2.1 format          */
/**********************************/
typedef rd_ep_data_store_entry_v0_t rd_ep_data_store_entry_v21_t;

typedef rd_eeprom_static_hdr_v20_t rd_eeprom_static_hdr_v21_t;

typedef struct rd_node_database_entry_v21 {

  uint32_t wakeUp_interval;
  uint32_t lastAwake;
  uint32_t lastUpdate;

  uip_ip6addr_t ipv6_address;

  uint8_t nodeid;
  uint8_t security_flags;
  /*uint32_t homeID;*/

  rd_node_mode_t mode;
  rd_node_state_t state;

  uint16_t manufacturerID;
  uint16_t productType;
  uint16_t productID;

  uint8_t nodeType; // Is this a controller, routing slave ... etc

  uint8_t refCnt;
  uint8_t nEndpoints;
  uint8_t nAggEndpoints;

  LIST_STRUCT(endpoints);

  uint8_t nodeNameLen;
  uint8_t dskLen;
  uint8_t node_version_cap_and_zwave_sw;
  uint16_t probe_flags;
  uint16_t node_properties_flags;
  uint8_t node_cc_versions_len;
  uint8_t node_is_zws_probed;
  char* nodename;
  uint8_t *dsk;
  cc_version_pair_t *node_cc_versions;
  ProbeCCVersionState_t *pcvs;

} rd_node_database_entry_v21_t;

unsigned long size = 0;
/* Buf for reading from old eeprom */
unsigned char *buf;
/* Buf for the whole eeprom file with size 65548 */
unsigned char *eeprom_buf;
static int f;
const char *linux_conf_eeprom_file;

void eeprom_read(unsigned long addr, unsigned char *buf, int size)
{
  lseek(f, addr, SEEK_SET);
  if(read(f, buf, size)!=size) {
    perror("Read error. Eeprom conversion failed. Eeprom file should not be used");
  }
}

uint16_t rd_eeprom_read(uint16_t offset, int len,void* data)
{
  //DBG_PRINTF("Reading at %x\n", 0x100 + offset);
  eeprom_read(0x40 + offset,data,len);
  return len;
}

void eeprom_write(unsigned long addr, unsigned char *buf, int size)
{
  lseek(f, addr, SEEK_SET);
  if(write(f, buf, size) != size) {
    perror("Write error. Eeprom conversion failed. Eeprom file should not be used");
  }

  sync();
}

uint16_t rd_eeprom_write(uint16_t offset,int len,void* data)
{
  //DBG_PRINTF("Writing at %x\n", 0x100 + offset);
  if(len) {
    eeprom_write(0x40 + offset,data,len);
  }
  return len;
}

int check_structure_alignemnts()
{
    int *ptr;
    if (sizeof(ptr) != 4) {
        printf("Error: This program is developed for 32bit systems\n");
        return 1;
    }
    if ((offsetof(rd_ep_data_store_entry_ancient_t, state) != 4) || offsetof(rd_ep_data_store_entry_ancient_t, iconID) != 8) {
        printf("-------------error. Eeprom conversion failed. Eeprom file should not be used\n");
        return 1;
    }
    if(sizeof(rd_ep_data_store_entry_v0_t) != 16) {
        printf("-------------error. Eeprom conversion failed. Eeprom file should not be used\n");
        return 1;
    }
    if ((offsetof(rd_ep_data_store_entry_v0_t, endpoint_aggr_len) != 3)) {
        printf("-------------error. Eeprom conversion failed. Eeprom file should not be used\n");
        return 1;
    }
    if ((offsetof(rd_ep_data_store_entry_v0_t, state) != 8) || (offsetof(rd_ep_data_store_entry_v0_t, iconID) != 12)) {
        printf("-------------error. Eeprom conversion failed. Eeprom file should not be used2\n");
        return 1;
    }
    if (sizeof(rd_node_database_entry_v0_t) != sizeof(rd_node_database_entry_ancient_t)) {
        printf("-------------error. Eeprom conversion failed. Eeprom file should not be used1\n");
        return 1;
    }
    return 0;
}

void print_v0_eeprom(int f)
{
    uint16_t n_ptr, e_ptr;
    rd_node_database_entry_v0_t r;
    rd_ep_data_store_entry_v0_t e;
    int i, j;
    size = lseek(f, 0, SEEK_END);
    const uint16_t static_size = offsetof(rd_node_database_entry_v0_t, nodename);
    for (i = 1; i < ZW_MAX_NODES; i++) {
        rd_eeprom_read(offsetof(rd_eeprom_static_hdr_v0_t, node_ptrs[i]), sizeof(uint16_t), &n_ptr);
        if(n_ptr == 0) {
            continue;
        }
        printf("------------------------------------\n");
        rd_eeprom_read(n_ptr, static_size , &r);
        printf("n_ptr %x node ID %d\n", n_ptr, r.nodeid);
        printf("\twake up interval %d\n\tlast awake %d\n\tlast update %d\n", r.wakeUp_interval, r.lastAwake, r.lastUpdate);
        printf("\tipv6 address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", ENDIAN(r.ipv6_address.u16[0]), ENDIAN(r.ipv6_address.u16[1]), ENDIAN(r.ipv6_address.u16[2]), ENDIAN(r.ipv6_address.u16[3]), ENDIAN(r.ipv6_address.u16[4]), ENDIAN(r.ipv6_address.u16[5]), ENDIAN(r.ipv6_address.u16[6]), ENDIAN(r.ipv6_address.u16[7]));
        printf("\tsecurity flags %02x\n\tmode %02x\n\tstate %02x\n\tmanufacturerID %04x\n\tproduct type %04x\n\tproduct ID %04x\n\tnode type %02x\n\tref count %02x\n\tnEndpoints %02x\n\tnAggEndpoints %02x\n\tnode name length %02x\n", r.security_flags, r.mode, r.state, r.manufacturerID, r.productType, r.productID, r.nodeType, r.refCnt, r.nEndpoints, r.nAggEndpoints, r.nodeNameLen);

        n_ptr = n_ptr + static_size + r.nodeNameLen;

        for(j = 0; j < r.nEndpoints; j++) {
            rd_eeprom_read(n_ptr, sizeof(uint16_t), &e_ptr);
            printf("\t\te_ptr %x\n", e_ptr);

            rd_eeprom_read(e_ptr, sizeof(rd_ep_data_store_entry_v0_t), &e);
            printf("\t\tendpoint ID %d\n\t\tendpoint info len %d\n\t\tendpoint name len %d\n\t\tendpoint location len %d\n\t\tendpoint aggr len %d\n\t\tstate %d\n\t\ticon ID %d\n", e.endpoint_id, e.endpoint_info_len, e.endpoint_name_len, e.endpoint_loc_len, e.endpoint_aggr_len, e.state, e.iconID);

            n_ptr+=sizeof(uint16_t);
        }
    }
}

void print_v20_eeprom(int f)
{
  uint16_t n_ptr, e_ptr;
  rd_node_database_entry_v20_t r;
  rd_ep_data_store_entry_v20_t e;
  int i, j;
  size = lseek(f, 0, SEEK_END);
  const uint16_t static_size = offsetof(rd_node_database_entry_v20_t, nodename);
  for (i = 1; i < ZW_MAX_NODES; i++) {
    rd_eeprom_read(offsetof(rd_eeprom_static_hdr_v20_t, node_ptrs[i]), sizeof(uint16_t), &n_ptr);
    if(n_ptr == 0) {
      continue;
    }
    printf("------------------------------------\n");
    rd_eeprom_read(n_ptr, static_size , &r);
    printf("n_ptr %04x node ID %d\n", n_ptr, r.nodeid);
    printf("\twake up interval %d\n\tlast awake %d\n\tlast update %d\n", r.wakeUp_interval, r.lastAwake, r.lastUpdate);
    printf("\tipv6 address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", ENDIAN(r.ipv6_address.u16[0]), ENDIAN(r.ipv6_address.u16[1]), ENDIAN(r.ipv6_address.u16[2]), ENDIAN(r.ipv6_address.u16[3]), ENDIAN(r.ipv6_address.u16[4]), ENDIAN(r.ipv6_address.u16[5]), ENDIAN(r.ipv6_address.u16[6]), ENDIAN(r.ipv6_address.u16[7]));
    printf("\tsecurity flags %02x\n\tmode %d\n\tstate %d\n\tmanufacturerID %d\n\tproduct type %d\n\tproduct ID %d\n\tnode type %d\n\tref count %d\n\tnEndpoints %d\n\tnAggEndpoints %d\n\tnode name length %d\n\tdsk length %d\n", r.security_flags, r.mode, r.state, r.manufacturerID, r.productType, r.productID, r.nodeType, r.refCnt, r.nEndpoints, r.nAggEndpoints, r.nodeNameLen, r.dskLen);

    n_ptr = n_ptr + static_size + r.nodeNameLen + r.dskLen;

    for(j = 0; j < r.nEndpoints; j++) {
      rd_eeprom_read(n_ptr, sizeof(uint16_t), &e_ptr);
      printf("\t\te_ptr %04x\n", e_ptr);

      rd_eeprom_read(e_ptr, sizeof(rd_ep_data_store_entry_v20_t), &e);
      printf("\t\tendpoint ID %d\n\t\tendpoint info len %d\n\t\tendpoint name len %d\n\t\tendpoint location len %d\n\t\tendpoint aggr len %d\n\t\tstate %d\n\t\ticon ID %d\n", e.endpoint_id, e.endpoint_info_len, e.endpoint_name_len, e.endpoint_loc_len, e.endpoint_aggr_len, e.state, e.iconID);

      n_ptr+=sizeof(uint16_t);
    }
  }
}

void print_node_cc_version(rd_node_database_entry_v21_t n)
{
  int i = 0;
  if(!n.node_cc_versions)
    return;
  printf("\tNode CC version list\n");
  int cnt = n.node_cc_versions_len / sizeof(cc_version_pair_t);
  for(i = 0; i < cnt; i++) {
    printf("\t\tCC: %04x, Version: %04x\n", n.node_cc_versions[i].command_class, n.node_cc_versions[i].version);
  }
}

void print_v21_eeprom(int f)
{
    uint16_t n_ptr[2], e_ptr;
    rd_node_database_entry_v21_t r;
    rd_ep_data_store_entry_v21_t e;
    int i, j;
    size = lseek(f, 0, SEEK_END);
    const uint16_t static_size = offsetof(rd_node_database_entry_v21_t, nodename);
    for (i = 1; i < ZW_MAX_NODES; i++) {
        rd_eeprom_read(offsetof(rd_eeprom_static_hdr_v21_t, node_ptrs[i]), sizeof(uint16_t), &n_ptr[0]);
        if(n_ptr[0] == 0) {
            continue;
        }
        printf("------------------------------------\n");
        rd_eeprom_read(n_ptr[0], static_size , &r);
        printf("n_ptr %04x node ID %d\n", n_ptr[0], r.nodeid);
        printf("\twake up interval %d\n\tlast awake %d\n\tlast update %d\n", r.wakeUp_interval, r.lastAwake, r.lastUpdate);
        printf("\tipv6 address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", ENDIAN(r.ipv6_address.u16[0]), ENDIAN(r.ipv6_address.u16[1]), ENDIAN(r.ipv6_address.u16[2]), ENDIAN(r.ipv6_address.u16[3]), ENDIAN(r.ipv6_address.u16[4]), ENDIAN(r.ipv6_address.u16[5]), ENDIAN(r.ipv6_address.u16[6]), ENDIAN(r.ipv6_address.u16[7]));
        printf("\tsecurity flags %02x\n\tmode %d\n\tstate %d\n\tmanufacturerID %d\n\tproduct type %d\n\tproduct ID %d\n\tnode type %d\n\tref count %d\n\tnEndpoints %d\n\tnAggEndpoints %d\n\tnode name length %d\n\tdsk length %d\n", r.security_flags, r.mode, r.state, r.manufacturerID, r.productType, r.productID, r.nodeType, r.refCnt, r.nEndpoints, r.nAggEndpoints, r.nodeNameLen, r.dskLen);
        printf("\tcap_report %02x\n\tprobe_flags %04x\n\tnode_properties_flags %04x\n\tnode_cc_versions_len %d\n\tnode_is_zws_probed %02x\n", r.node_version_cap_and_zwave_sw, r.probe_flags, r.node_properties_flags, r.node_cc_versions_len, r.node_is_zws_probed);

        rd_eeprom_read(n_ptr[0] + static_size, r.node_cc_versions_len, r.node_cc_versions);
        print_node_cc_version(r);
        rd_eeprom_read(n_ptr[0] + static_size + r.node_cc_versions_len, sizeof(uint16_t), &n_ptr[1]);
        n_ptr[1] = n_ptr[1] + r.nodeNameLen + r.dskLen;

        printf("\tEndpoints:\n");
        for(j = 0; j < r.nEndpoints; j++) {
            rd_eeprom_read(n_ptr[1], sizeof(uint16_t), &e_ptr);
            printf("\t\te_ptr %04x\n", e_ptr);

            rd_eeprom_read(e_ptr, sizeof(rd_ep_data_store_entry_v21_t), &e);
            printf("\t\tendpoint ID %d\n\t\tendpoint info len %d\n\t\tendpoint name len %d\n\t\tendpoint location len %d\n\t\tendpoint aggr len %d\n\t\tstate %d\n\t\ticon ID %d\n", e.endpoint_id, e.endpoint_info_len, e.endpoint_name_len, e.endpoint_loc_len, e.endpoint_aggr_len, e.state, e.iconID);

            n_ptr[1]+=sizeof(uint16_t);
        }
    }
}


/*
 * EEPROM layout
 *
 * magic | homeID | nodeID | flags | node_ptrs[ZW_MAX_NODES]
 *
 * SMALLOC_SPACE[RD_SMALLOC_SIZE] //5E00
 *     rd_node_database_entry_1 | rd_ep_data_store_entry_1 + ep_info_1 + ep_name_1 + ep_loc_1 | rd_ep_data_store_entry_2 ...
 *     rd_node_database_entry_2 | rd_ep_data_store_entry_1 + ep_info_1 + ep_name_1 + ep_loc_1 | rd_ep_data_store_entry_2 ...
 *
 * virtual node info
 */

/*
 * Create a eeprom buffer(eeprom_buf) with the same size and fill it with
 * converted data in the offset indicated by smalloc. When done, copy it back to
 * original eeprom.
 */
int main(int argc, char **argv)
{
    int i, j;

    if (argc < 2) {
        printf("Usage: eeprom_printer <eeprom_file_path>\n");
        printf("Format will be decided based on the version in eeprom file. Note that eeprom without version is not supported by this printer.\n");
        exit(1);
    }
    linux_conf_eeprom_file = argv[1];
    printf("Opening eeprom file %s\n", linux_conf_eeprom_file);
    f = open(linux_conf_eeprom_file, O_RDWR | O_CREAT, 0644);

    if(f < 0) {
        fprintf(stderr, "Error opening eeprom file %s\n.", linux_conf_eeprom_file);
        perror("");
        exit(1);
    }

    size = lseek(f, 0, SEEK_END);
    buf = malloc(size + 64000);
    eeprom_buf = malloc(size);

    if(check_structure_alignemnts()) {
        exit(1);
    }

    rd_eeprom_static_hdr_v20_t static_hdr;
    rd_eeprom_read(0, sizeof(static_hdr), &static_hdr);

    printf("MAGIC %08x version %d.%d\n", static_hdr.magic, static_hdr.version_major, static_hdr.version_minor);
    if(static_hdr.magic == RD_MAGIC_V0) {
      print_v0_eeprom(f);
    } else if(static_hdr.magic == RD_MAGIC_V1
        && ((static_hdr.version_major == 0 && static_hdr.version_minor == 0)
          || (static_hdr.version_major == 2 && static_hdr.version_minor == 0))) {
      print_v20_eeprom(f);
    } else if(static_hdr.magic == RD_MAGIC_V1
        && static_hdr.version_major == 2
        && static_hdr.version_minor == 1) {
      print_v21_eeprom(f);
    } else {
      printf("Unsupported eeprom MAGIC %08x version %d.%d\n", static_hdr.magic, static_hdr.version_major, static_hdr.version_minor);
    }

    close(f);
    free(buf);
    free(eeprom_buf);
}
