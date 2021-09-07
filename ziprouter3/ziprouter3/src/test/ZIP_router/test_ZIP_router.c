/* © 2017 Silicon Laboratories Inc.
 */

/*
 * Test of a tiny part of ZIP_Router.c. In particular, test the detection of
 * Wake Up Notifications inside multi cmd encapsulations.
 */
/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include "test_helpers.h"

#include "../src/ZIP_Router.c"

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/

/****************************************************************************/
/*                               MOCKS FUNCTIONS                            */
/****************************************************************************/
static struct {
  uint8_t node;
  uint8_t is_broadcast;
} mb_wakeup_event_args = {0};

void
mb_wakeup_event(uint8_t node, uint8_t is_broadcast)
{
  mb_wakeup_event_args.node = node;
  mb_wakeup_event_args.is_broadcast = is_broadcast;
}

security_scheme_t
highest_scheme(uint8_t scheme_mask)
{
  return 0;
}

uint8_t GetCacheEntryFlag(uint8_t nodeid)
{
  return 0;
}
/****************************************************************************/
/*                              TEST FUNCTIONS                              */
/****************************************************************************/

static void init_test()
{
  memset(&mb_wakeup_event_args, 0, sizeof mb_wakeup_event_args);
}

void test_find_WUN_in_multi()
{
  uint8_t cmd1[] = {COMMAND_CLASS_MULTI_CMD,
    MULTI_CMD_ENCAP,
    255, /* no of commands */
    3,
    1,2,3,
    5,
    11,12,13,14,15,
    2,
    0x84, /* COMMAND_CLASS_WAKE_UP */
    0x07, /* WAKE_UP_NOTIFICATION */
  };

  ts_param_t tsp = {42, 0, 0, 0, 0, 0, 0};
  cfg.mb_conf_mode = ENABLE_MAILBOX_SERVICE;
  find_WUN_in_multi(&tsp, (ZW_APPLICATION_TX_BUFFER *)cmd1, sizeof cmd1);
  check_true(mb_wakeup_event_args.node == 42, "WUN not detected #1");

  init_test();

  uint8_t cmd2[] = {COMMAND_CLASS_MULTI_CMD,
    MULTI_CMD_ENCAP,
    3, /* no of commands */
    2,
    0x84,0,
    2,
    0x84,1,
    2,
    0x84, /* COMMAND_CLASS_WAKE_UP */
    0x06,
  };

  find_WUN_in_multi(&tsp, (ZW_APPLICATION_TX_BUFFER *)cmd2, sizeof cmd2);
  check_true(mb_wakeup_event_args.node == 0, "WUN not detected #2");

  init_test();

  uint8_t cmd3[] = {COMMAND_CLASS_MULTI_CMD,
    MULTI_CMD_ENCAP,
    3, /* no of commands */
    2,
    0x84,7, /* WAKE_UP_NOTIFICATION */
    2,
    0x84,7, /* WAKE_UP_NOTIFICATION */
    2,
    0x84, /* COMMAND_CLASS_WAKE_UP */
    0x07,
  };
  find_WUN_in_multi(&tsp, (ZW_APPLICATION_TX_BUFFER *)cmd3, sizeof cmd3);
  check_true(mb_wakeup_event_args.node == 42, "WUN not detected #2");

  init_test();

  /* Check for invalid no of commands */
  cmd1[2] = 255;
  find_WUN_in_multi(&tsp, (ZW_APPLICATION_TX_BUFFER *)cmd1, sizeof cmd1);
  check_true(mb_wakeup_event_args.node == 42, "WUN not detected #4");
}

int main()
{
   test_find_WUN_in_multi();

   if (numErrs) {
       test_print(0, "\nFAILED, %d errors\n", numErrs);
   } else {
       test_print(0, "\nPASSED\n");
   }
   return numErrs;
}
