/* © 2017 Silicon Laboratories Inc.
 */
/*
 * CC_ZIPNaming.c
 *
 *  Created on: Nov 9, 2016
 *      Author: aes
 */

#include "Serialapi.h"
#include "command_handler.h"
#include "ZIP_Router.h"
#include "ResourceDirectory.h"
extern ZW_APPLICATION_TX_BUFFER txBuf;

static zwave_connection_t naming_reply;

static void
send_ep_name_reply(rd_ep_database_entry_t* ep, void* user)
{
  u8_t *f = (u8_t*) &txBuf;
  u8_t l;

  f[0] = COMMAND_CLASS_ZIP_NAMING;
  f[1] = (uintptr_t) user & 0xFF;

  if ((uintptr_t) user == ZIP_NAMING_NAME_REPORT)
  {
    l = rd_get_ep_name(ep, (char*) &f[2], 64);
  }
  else
  {
    l = rd_get_ep_location(ep, (char*) &f[2], 64);
  }
  ZW_SendDataZIP(&naming_reply, f, l + 2, 0);
  memset(&naming_reply, 0, sizeof(naming_reply));
}

command_handler_codes_t
ZIPNamingHandler(zwave_connection_t *c, uint8_t* pData, uint16_t bDatalen)
{
  ZW_APPLICATION_TX_BUFFER* pCmd = (ZW_APPLICATION_TX_BUFFER*) pData;
  uint8_t node = nodeOfIP(&c->lipaddr);
  switch (pCmd->ZW_Common.cmd)
  {
  case ZIP_NAMING_NAME_SET:
    {
      ZW_ZIP_NAMING_NAME_SET_1BYTE_FRAME* f = (ZW_ZIP_NAMING_NAME_SET_1BYTE_FRAME*) pData;
      rd_update_ep_name(rd_get_ep(node, 0), (char*) &f->name1, bDatalen - 2);
      break;
    }
  case ZIP_NAMING_NAME_GET:

    naming_reply = *c;
    rd_register_ep_probe_notifier(node, 0, (void*) ZIP_NAMING_NAME_REPORT, send_ep_name_reply);
    rd_node_probe_update(rd_get_ep(node,0)->node);
    break;
  case ZIP_NAMING_LOCATION_SET:
    {
      /*TODO Handle simultaneous sessions */
      naming_reply = *c;
      ZW_ZIP_NAMING_LOCATION_SET_1BYTE_FRAME* f = (ZW_ZIP_NAMING_LOCATION_SET_1BYTE_FRAME*) pData;
      rd_update_ep_location(rd_get_ep(node, 0), (char*) &f->location1, bDatalen - 2);
      break;
    }
  case ZIP_NAMING_LOCATION_GET:
    naming_reply = *c;
    rd_register_ep_probe_notifier(node, 0, (void*) ZIP_NAMING_LOCATION_REPORT, send_ep_name_reply);
    rd_node_probe_update(rd_get_ep(node,0)->node);
    break;
  default:
    return COMMAND_NOT_SUPPORTED;
  }
  return COMMAND_HANDLED;
}

REGISTER_HANDLER(ZIPNamingHandler, 0, COMMAND_CLASS_ZIP_NAMING, ZIP_NAMING_VERSION, SECURITY_SCHEME_UDP);
