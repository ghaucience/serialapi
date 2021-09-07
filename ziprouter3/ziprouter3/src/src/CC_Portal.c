/* © 2014 Silicon Laboratories Inc.
 */
#include "CC_Portal.h"
#include "ZW_classcmd.h"
#include "ZW_classcmd_ex.h"
#include "ZIP_Router.h"
#include "eeprom_layout.h"
#include "eeprom.h"
#include "ZW_tcp_client.h"
#include "ZW_udp_server.h"
#include "Serialapi.h"
#include "ZW_ZIPApplication.h"
#include "command_handler.h"

uint8_t gGwLockEnable = TRUE;


command_handler_codes_t ZIP_Portal_CommandHandler(zwave_connection_t *c, uint8_t* pData, uint16_t bDatalen)
{
  ZW_APPLICATION_TX_BUFFER* pCmd = (ZW_APPLICATION_TX_BUFFER*) pData;

  switch (pCmd->ZW_Common.cmd)
  {
  case GATEWAY_CONFIGURATION_SET:
  {
    ZW_GATEWAY_CONFIGURATION_STATUS GwConfigStatus;
    ZW_GATEWAY_CONFIGURATION_SET_FRAME *f = (ZW_GATEWAY_CONFIGURATION_SET_FRAME*) pData;

    GwConfigStatus.cmdClass = COMMAND_CLASS_ZIP_PORTAL;
    GwConfigStatus.cmd = GATEWAY_CONFIGURATION_STATUS;

    if (parse_portal_config(&f->lanIpv6Address1, (bDatalen - sizeof(ZW_GATEWAY_CONFIGURATION_SET) + 1)))
    {
      GwConfigStatus.status = ZIPR_READY_OK;
      process_post(&zip_process, ZIP_EVENT_TUNNEL_READY, 0);
    } else
    {
      GwConfigStatus.status = INVALID_CONFIG;
    }

    ZW_SendDataZIP(c, (BYTE*) &GwConfigStatus, sizeof(ZW_GATEWAY_CONFIGURATION_STATUS), NULL);
  }
    break;

  case GATEWAY_CONFIGURATION_GET:
  {
    uint8_t buf[128] =
    { 0 }; //buffer should be more than size of portal config + 2
    ZW_GATEWAY_CONFIGURATION_REPORT *pGwConfigReport = (ZW_GATEWAY_CONFIGURATION_REPORT *) buf;
    portal_ip_configuration_st_t *portal_config = (portal_ip_configuration_st_t *) (&pGwConfigReport->payload[0]);

    pGwConfigReport->cmdClass = COMMAND_CLASS_ZIP_PORTAL;
    pGwConfigReport->cmd = GATEWAY_CONFIGURATION_REPORT;

    memcpy(&portal_config->lan_address, &cfg.lan_addr, IPV6_ADDR_LEN);
    portal_config->lan_prefix_length = cfg.lan_prefix_length;

    memcpy(&portal_config->tun_prefix, &cfg.tun_prefix, IPV6_ADDR_LEN);
    portal_config->tun_prefix_length = cfg.tun_prefix_length;

    memcpy(&portal_config->gw_address, &cfg.gw_addr, IPV6_ADDR_LEN);

    memcpy(&portal_config->pan_prefix, &cfg.pan_prefix, IPV6_ADDR_LEN);
#if 0
    memcpy(&portal_config->unsolicited_dest, &cfg.unsolicited_dest, IPV6_ADDR_LEN);
    portal_config->unsolicited_destination_port = UIP_HTONS(cfg.unsolicited_port);
#endif

    ZW_SendDataZIP(c, buf,
        (sizeof(ZW_GATEWAY_CONFIGURATION_REPORT) - 1 + sizeof(portal_ip_configuration_st_t)), NULL);

  }
    break;

  default:
    return COMMAND_NOT_SUPPORTED;
    break;
  }
  return COMMAND_HANDLED;
}

#ifdef __ASIX_C51__
void Reset_Gateway(void) CC_REENTRANT_ARG
{
  Gw_Config_St_t GwConfig;
  uint16_t gwSettingsLen = 0;

  printf("Resetting Gateway start..\n");

  //Validate default configuration
  if (validate_defaultconfig(EXT_DEFAULT_CONFIG_START_ADDR, FALSE, &gwSettingsLen) != TRUE)
  {
    printf("Reset_Gateway FAILED: Invalid GW Config.\r\n");
#ifndef ZIP_GW_SP_MODE
    //Unlock the Gateway
    eeprom_read(EXT_GW_SETTINGS_START_ADDR, &GwConfig, sizeof(Gw_Config_St_t));
    //clear the lock bit
    GwConfig.showlock &= ~(1 << LOCK_BIT);
    eeprom_write(EXT_GW_SETTINGS_START_ADDR, &GwConfig, sizeof(Gw_Config_St_t));

    gGwLockEnable = FALSE;
#endif
    return;
  }

  //Write the default GW settings to active settings partition...
  gaconfig_WriteConfigToDefault(EXT_DEFAULT_CONFIG_START_ADDR, EXT_GW_SETTINGS_START_ADDR, gwSettingsLen);

#ifndef ZIP_GW_SP_MODE
  eeprom_read(EXT_GW_SETTINGS_START_ADDR, &GwConfig, sizeof(Gw_Config_St_t));
  //clear the lock bit
  GwConfig.showlock &= ~(1 << LOCK_BIT);
  eeprom_write(EXT_GW_SETTINGS_START_ADDR, &GwConfig, sizeof(Gw_Config_St_t));
  gGwLockEnable = FALSE;
#endif

  //copy the certs from the default partition
  gaconfig_WriteCertToFlash(EXT_DEFAULT_CONFIG_START_ADDR+gwSettingsLen);

  if(gconfig_ValidateCertPartition() != TRUE)
  {
    printf("Reset_Gateway FAILED: Cert default set failed.\r\n");
  }

  if (validate_gw_profile(EXT_GW_SETTINGS_START_ADDR, FALSE) != TRUE)
  {
    printf("Reset_Gateway FAILED: Gw settings default set failed.\r\n");
  }

  ResetGW_Callback();

  return;
}
#endif



REGISTER_HANDLER(
    ZIP_Portal_CommandHandler,
    0,
    COMMAND_CLASS_ZIP_PORTAL, ZIP_PORTAL_VERSION, SECURITY_SCHEME_UDP);
