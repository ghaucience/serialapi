add_definitions(-DUIP_CONF_IPV6=1 -DPROVISIONING_LIST_SIZE=232 -DTEST_PROBE_CC_VERSION)

if(NOT APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wparentheses -Wno-array-bounds -Wall -Wno-unused -Wextra -Wno-sign-compare -Wcomment -Wno-format-extra-args -Wmaybe-uninitialized -Wno-shadow -Wundef -ggdb")
endif()

set(RD_BASIC_SRC
  ${CMAKE_SOURCE_DIR}/src/RD_internal.c
  ${CMAKE_SOURCE_DIR}/src/RD_DataStore.c
  ${CMAKE_SOURCE_DIR}/src/smalloc.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/assert.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/list.c
  ${CMAKE_SOURCE_DIR}/test/eeprom-stub.c
  )

add_executable(test_rd_pvl_link test_rd_pvl_link.c
  ${RD_BASIC_SRC}
  ${CMAKE_SOURCE_DIR}/test/test_helpers.c
  ${CMAKE_SOURCE_DIR}/test/test_gw_helpers.c
  ${CMAKE_SOURCE_DIR}/contiki/platform/linux/zgw_log_int.c
  )
target_compile_definitions(test_rd_pvl_link PRIVATE
  ZGW_LOG ZGW_LOG_LOG_TO_FILE ZGW_LOG_LVL_INIT=5)

add_executable(test_rd_probe_cc_version test_rd_probe_cc_version.c
  ${RD_BASIC_SRC}
  ${CMAKE_SOURCE_DIR}/src/RD_probe_cc_version.c
  ${CMAKE_SOURCE_DIR}/test/test_helpers.c
  ${CMAKE_SOURCE_DIR}/test/test_gw_helpers.c
  ${CMAKE_SOURCE_DIR}/contiki/platform/linux/zgw_log_int.c
)

add_test(rd_pvl_link test_rd_pvl_link)
add_test(rd_probe_cc_version test_rd_probe_cc_version)
