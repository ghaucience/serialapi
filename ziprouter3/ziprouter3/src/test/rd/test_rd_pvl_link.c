/* © 2018 Silicon Laboratories Inc.
 */

#include "RD_internal.h"
#include <string.h>
#include <lib/zgw_log.h>
#include "test_helpers.h"

zgw_log_id_define(rd_pvl_test);
zgw_log_id_default_set(rd_pvl_test);

/**
\defgroup rd_test Ressource Directory unit test.

Test Plan

Test RD link helpers

Basic
- Lookup non-included device
- Lookup included device
- Look up failing device
 - Lookup non-existing device
- Lookup removed device

Store to file of rd/pvl link
- Lookup included device after pvl .dat re-read
- Lookup included device after pvl .dat and eeprom re-read
- Lookup removed device after pvl .dat re-read
- Lookup failing device after pvl .dat re-read

*/


FILE *log_strm = NULL;

static uint8_t dsk5[] =       {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

static void test_dsk_lookup(void);

int main()
{
    verbosity = test_case_start_stop;

    zgw_log_setup(NULL);
    log_strm = test_create_log(TEST_CREATE_LOG_NAME);

    start_case("Hello world", log_strm);
    check_true(1, "Framework setup completed");
    close_case("Hello world");

    test_dsk_lookup();

    close_run();
    fclose(log_strm);

    return numErrs;
}

static void test_dsk_lookup(void) {
    rd_node_database_entry_t *rd_dbe;
    rd_node_database_entry_t *tmp;
    uint8_t nodeid5 = 6;

   start_case("Lookup non-included device", log_strm);
   rd_dbe = rd_lookup_by_dsk(sizeof(dsk5), dsk5);
   check_null(rd_dbe, "dsk5 is not in RD, so we should get NULL");
   close_case("Lookup non-included device");

   start_case("Lookup included device", log_strm);
   
   rd_dbe = rd_node_entry_alloc(nodeid5);
   rd_node_add_dsk(nodeid5, sizeof(dsk5), dsk5);
   tmp = rd_lookup_by_dsk(sizeof(dsk5), dsk5);
   check_true(rd_dbe == tmp, "After adding, dsk5 should be found in RD");
   check_true(tmp->nodeid == 6, "Nodeid 6 was assigned to dsk5 in RD");
   close_case("Lookup included device");


   /* Node id tlv */
   start_case("Node id of included device", log_strm);
   close_case("Node id of included device");
   start_case("Node id of non-included device", log_strm);
   close_case("Node id of non-included device");

   /* inclusion tlv */

   start_case("Look up failing device", log_strm);
   close_case("Look up failing device");

   start_case("Lookup removed device", log_strm);
   close_case("Lookup removed device");

   start_case("Lookup non-existing device", log_strm);
   close_case("Lookup non-existing device");

}
