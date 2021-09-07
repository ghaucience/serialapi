/* © 2014 Silicon Laboratories Inc.
 */
#ifndef DATASTORE_H_
#define DATASTORE_H_

#include "Bridge.h"

/** Size of the dynamic allocation area in the RD data store.
 * 
 * \ingroup rd_data_store
*/
#define RD_SMALLOC_SIZE 0x5E00

/** Convenience macro for saving VAR to FIELD in #rd_eeprom_static_hdr_t 
 * \ingroup rd_data_store
 * 
 * \param VAR The data to save.
 * \param FIELD The name of the field in the header.
 */
#define DS_EEPROM_SAVE(VAR, FIELD)   rd_eeprom_write( \
    offsetof(rd_eeprom_static_hdr_t, FIELD), \
    sizeof(VAR), \
    (unsigned char*)&VAR)

/** Convenience macro for loading VAR from FIELD in rd_eeprom_static_hdr_t.
 * \ingroup rd_data_store
*/
#define DS_EEPROM_LOAD(VAR, FIELD) rd_eeprom_read( \
    offsetof(rd_eeprom_static_hdr_t, FIELD), \
    sizeof(VAR), \
    &VAR)

/** Layout of the RD persistent data store.
 * \ingroup rd_data_store
 *
 * The data store contains the actual static header, a list of node
 * pointers, information about the virtual nodes, and information
 * about associations.  It also contains a small area for dynamic
 * allocation of node-related information.
 *
 * */
typedef struct rd_eeprom_static_hdr {
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
} rd_eeprom_static_hdr_t;

/** Read the eeprom version fields and put them in the uint8_t fields pointed to by the arguments.
 * \param major Pointer to a uint8_t field.
 * \param minor Pointer to a uint8_t field.
 */
void rd_data_store_version_get(uint8_t *major, uint8_t *minor);

/** Read bytes from offset in the data store to data.
 * \ingroup rd_data_store
 *
 * \param offset Offset to read from, measured from the start of the static header.
 * \param len How many bytes to copy.
 * \param data Pointer to a buffer that can hold at least len bytes.
 * \return This function always returns len.
 */
uint16_t rd_eeprom_read(uint16_t offset,uint8_t len,void* data);

/**
 * Write len bytes from data to offset offset in the data store.
 * \ingroup rd_data_store
 *
 * \param offset Offset to write into, measured from the start of the static header.
 * \param len How many bytes to write.
 * \param data Pointer to the data to write.
 * \return This function always returns len.
 */
uint16_t rd_eeprom_write(uint16_t offset,uint8_t len,void* data);

void rd_data_store_invalidate();

void data_store_init();

#endif /* DATASTORE_H_ */
