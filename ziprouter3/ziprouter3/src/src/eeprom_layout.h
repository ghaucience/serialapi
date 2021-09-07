/* © 2014 Silicon Laboratories Inc.
 */

#ifndef EEPROM_LAYOUT_H_
#define EEPROM_LAYOUT_H_


#include "NodeCache.h"
#include "zip_router_config.h"

#define ZIPMAGIC 1645985

/* ZW firmware related Error Messages */
#define FW_DOWNLOAD_SUCCESSFUL		0x0
#define FW_DOWNLOAD_READY			0x1
#define ERROR_CAN_NOT_SYNCH			0x2
#define ERROR_CHECKSUM_FAILED		0x3
#define ERROR_CHECKSUM_NOT_DONE		0x4
#define ERROR_FLASH_WRITE_FAILED	0x5
#define ERROR_NVR_LOAD_FAILED		0x6


/*ZW firmware download status messages */
#define ZW_FW_DOWNLOAD_IDLE			0x00
#define ZW_FW_UPDATE_IN_PROGRESS	0x01
#define ZW_FW_UPDATE_BACKUP_DONE	0x02
#define ZW_FW_UPDATE_AP_SET_DONE	0x03
#define ZW_FW_UPDATE_FAILED			0x04

/* Default config download status message */
#define DEFAULT_CONFIG_DOWNLOAD_IDLE			0x00
#define DEFAULT_CONFIG_UPDATE_IN_PROGRESS		0x01


#define ZW_FW_HEADER_LEN		0x08UL


#define EEOFFSET_MAGIC_START 0x0000
#define EEOFFSET_MAGIC_SIZE sizeof(zip_nvm_config_t)

#define EXT_FLASH_START_ADDR	0x000000UL

#define EXT_ZW_FW_UPGRADE_STATUS_ADDR				EXT_FLASH_START_ADDR
#define ZW_FW_UPGRADE_STATUS_LEN					0x1

#define EXT_ZW_FW_DOWNLOAD_DATA_LEN_ADDR			(EXT_ZW_FW_UPGRADE_STATUS_ADDR+ZW_FW_UPGRADE_STATUS_LEN)
#define ZW_FW_DATA_LEN					 			0x4

#define EXT_DEFAULT_CONFIG_UPGRADE_STATUS_ADDR		(EXT_ZW_FW_DOWNLOAD_DATA_LEN_ADDR+ZW_FW_DATA_LEN)
#define DEFAULT_CONFIG_UPGRADE_STATUS_LEN			0x1

#define EXT_DEFAULT_CONFIG_DATA_LEN_ADDR			(EXT_DEFAULT_CONFIG_UPGRADE_STATUS_ADDR+DEFAULT_CONFIG_UPGRADE_STATUS_LEN)
#define DEFAULT_CONFIG_DATA_LEN						0x2

#define EXT_FLASH_RESERVED_LEN		0x200

#define EEOFFSET_NODE_CACHE_START (EXT_FLASH_RESERVED_LEN)
#define EEOFFSET_NODE_CACHE_SIZE  NODECACHE_EEPROM_SIZE

#define MD5_CHECKSUM_LEN						0x10
#define MAX_ZW_FWLEN							(0x20000UL + ZW_FW_HEADER_LEN)
#define MAX_ZW_WITH_MD5_FWLEN					(0x20000UL + ZW_FW_HEADER_LEN + MD5_CHECKSUM_LEN)

/** Max firmware image size for the 700 series */
#define MAX_ZW_GECKO_FWLEN (256 * 1024)

//External flash partitions
#define TOTAL_FLASH_SIZE						0x80000UL

/* Last 4KB of first sector reserved for portal config and extra classes */
/* Portal config start address is 60KB */
#define EXT_PORTAL_CONFIG_PARTITION_START_ADDR		0xF000UL
#define PORTAL_CONFIG_MAGIC_LEN						4
#define EXT_PORTAL_CONFIG_START_ADDR			    (EXT_PORTAL_CONFIG_PARTITION_START_ADDR + PORTAL_CONFIG_MAGIC_LEN)
#define PORTAL_CONFIG_PARTITION_LEN					256

#define EXT_EXTRA_CLASS_CONFIG_PARTITION_START		(EXT_PORTAL_CONFIG_PARTITION_START_ADDR + PORTAL_CONFIG_PARTITION_LEN)
#define EXTRA_CLASS_CONFIG_MAGIC_LEN				4
#define EXT_EXTRA_CLASS_CONFIG_START				(EXT_EXTRA_CLASS_CONFIG_PARTITION_START + EXTRA_CLASS_CONFIG_MAGIC_LEN)


#define NIF_CACHE_LEN							0x10000UL
#define EXT_GW_SETTINGS_START_ADDR				(EXT_FLASH_START_ADDR+NIF_CACHE_LEN)
#define GW_SETTINGS_PORFILE_CONFIG_LEN			0x800  //2KB Bytes

/* Index to GW profile and certs --start */
#define EXT_GW_PROFILE_START_ADDR				(EXT_GW_SETTINGS_START_ADDR+sizeof(Gw_Config_St_t))
/* Index to GW profile and certs --end */

#define EXT_CERT_SCRATCH_PAD_ADDR		    	(EXT_GW_SETTINGS_START_ADDR+GW_SETTINGS_PORFILE_CONFIG_LEN)
#define CERT_SCRATCH_PAD_LEN					0x2000  //8KB
#define DEVICE_CONFIG_LEN						(GW_SETTINGS_PORFILE_CONFIG_LEN+CERT_SCRATCH_PAD_LEN)
#define EXT_FW_START_ADDR				    	(EXT_CERT_SCRATCH_PAD_ADDR+CERT_SCRATCH_PAD_LEN)
#define MAX_FW_LEN 								(TOTAL_FLASH_SIZE-EXT_FW_START_ADDR-DEVICE_CONFIG_LEN)
#define EXT_DEFAULT_CONFIG_START_ADDR			(EXT_FW_START_ADDR+MAX_FW_LEN)
#define DEFAULT_CONFIG_LEN						(DEVICE_CONFIG_LEN) //10KB(Profile+ Default Certs)

#define EXT_DEFAULT_CONFIG_SCRATCH_PAD_SART		(EXT_FW_START_ADDR)

void Ext_Eeprom_Config_Read(u32_t baddr, u32_t start,void* dst, u16_t size);
void Ext_Eeprom_Config_Write(u32_t baddr, u32_t start,void* dst, u16_t size);

#define ext_eeprom_ipconfig_get(par_name,dst) Ext_Eeprom_Config_Read(EXT_PORTAL_CONFIG_PARTITION_START_ADDR, offsetof(portal_ip_configuration_with_magic_st_t,par_name),dst,sizeof(((portal_ip_configuration_with_magic_st_t*)0)->par_name))
#define ext_eeprom_ipconfig_set(par_name,src) Ext_Eeprom_Config_Write(EXT_PORTAL_CONFIG_PARTITION_START_ADDR, offsetof(portal_ip_configuration_with_magic_st_t,par_name),src,sizeof(((portal_ip_configuration_with_magic_st_t*)0)->par_name))


#endif /* EEPROM_LAYOUT_H_ */
