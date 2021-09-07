/* © 2017 Silicon Laboratories Inc.
 */
/*
 * s2_keystore.c

 *
 *  Created on: Nov 2, 2015
 *      Author: aes
 */
#include "ZIP_Router.h"
#include "s2_keystore.h"
#include "Security_Scheme0.h"

#define DllExport
#include "curve25519.h"
#include "ctr_drbg.h"
#include "CC_NetworkManagement.h"
#include "ZIP_Router_logging.h"

extern uint8_t ecdh_dynamic_key[32];

void
print_hex(uint8_t* buf, int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    printf("%02X", buf[i]);
  }
  printf("\n");
}


bool keystore_network_key_clear(uint8_t keyclass) {
  uint8_t random_bytes[64];
  uint8_t assigned_keys;
  uint8_t i;

  AES_CTR_DRBG_Generate(&s2_ctr_drbg, random_bytes);

  if(keyclass == KEY_CLASS_ALL) {
    keystore_network_key_clear(KEY_CLASS_S0);
    keystore_network_key_clear(KEY_CLASS_S2_UNAUTHENTICATED);
    keystore_network_key_clear(KEY_CLASS_S2_AUTHENTICATED);
    keystore_network_key_clear(KEY_CLASS_S2_ACCESS);
    return 1;
  }

  if( keystore_network_key_write(keyclass,random_bytes) ) {
    nvm_config_get(assigned_keys,&assigned_keys);
    assigned_keys &= ~keyclass;
    nvm_config_set(assigned_keys,&assigned_keys);
    memset(random_bytes,0,sizeof(random_bytes));
    return 1;
  }
  return 0;
}

void keystore_private_key_read(uint8_t *buf) {
  int i;

  if((NetworkManagement_getState() == NM_WAIT_FOR_SECURE_LEARN)
      || (NetworkManagement_getState() == NM_LEARN_MODE_STARTED)
      || (NetworkManagement_getState() == NM_LEARN_MODE)) {
    nvm_config_get(ecdh_priv_key,buf);
    printf(" static \n");
  } else {
    memcpy(buf,ecdh_dynamic_key,32);
    printf(" dynamic \n");
  }


#if 0
  printf("ECDH Private key is \n");
  print_hex(buf,32);
#endif
}


void
keystore_public_key_read(uint8_t *buf)
{
  int i;
  uint8_t priv_key[32];

  keystore_private_key_read(priv_key);
  crypto_scalarmult_curve25519_base(buf,priv_key);

  printf("ECDH Public key is \n");
  for(i=0; i < 16; i++) {
    uint16_t d = (buf[2*i]<<8) | buf[2*i +1];
    printf("%05hu-", d);
    if( (i&3)==3 )printf("\n");
  }
  memset(priv_key,0,sizeof(priv_key));
}


bool keystore_network_key_read(uint8_t keyclass, uint8_t *buf)
{
  uint8_t assigned_keys;

  nvm_config_get(assigned_keys,&assigned_keys);
  if(0==(keyclass & assigned_keys)) {
    return 0;
  }

  switch(keyclass)
  {
  case KEY_CLASS_S0:
    nvm_config_get(security_netkey,buf);
  break;
  case KEY_CLASS_S2_UNAUTHENTICATED:
    nvm_config_get(security2_key[0],buf);
    break;
  case KEY_CLASS_S2_AUTHENTICATED:
    nvm_config_get(security2_key[1],buf);
    break;
  case KEY_CLASS_S2_ACCESS:
    nvm_config_get(security2_key[2],buf);
    break;
  default:
    assert(0);
    return 0;
  }

  DBG_PRINTF("Key  class  %x \n",keyclass);
  print_hex(buf,16);

  return 1;
}


bool keystore_network_key_write(uint8_t keyclass, const uint8_t *buf)
{
  uint8_t assigned_keys;

  switch(keyclass)
  {
  case KEY_CLASS_S0:
    nvm_config_set(security_netkey,buf);
    sec0_set_key(buf);
  break;
  case KEY_CLASS_S2_UNAUTHENTICATED:
    nvm_config_set(security2_key[0],buf);
    break;
  case KEY_CLASS_S2_AUTHENTICATED:
    nvm_config_set(security2_key[1],buf);
    break;
  case KEY_CLASS_S2_ACCESS:
    nvm_config_set(security2_key[2],buf);
    break;
  default:
    assert(0);
    return 0;
  }

  nvm_config_get(assigned_keys,&assigned_keys);
  assigned_keys |= keyclass;
  nvm_config_set(assigned_keys,&assigned_keys);
  return 1;
}
