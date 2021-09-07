
set(CMAKE_BUILD_TYPE "Debug")
# Detect Debug build, below match is case-insensitive
if(CMAKE_BUILD_TYPE MATCHES [dD][eE][bB][uU][gG])
  message("Adding debug flags")
  # Add gcov support
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# flex does not like unused and sign-compare
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wno-sign-compare")

set(PVL_SRC
  ${CMAKE_SOURCE_DIR}/src/pvs_parse_help.c
  ${CMAKE_SOURCE_DIR}/src/provisioning_list.c
  ${CMAKE_SOURCE_DIR}/src/RD_internal.c
  ${CMAKE_SOURCE_DIR}/src/RD_DataStore.c
  ${CMAKE_SOURCE_DIR}/src/smalloc.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/assert.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/list.c
  ${CMAKE_SOURCE_DIR}/test/eeprom-stub.c
  )

set(TEST_HELPERS
  ${CMAKE_SOURCE_DIR}/test/test_helpers.c
  ${CMAKE_SOURCE_DIR}/test/test_utils.c
  ${CMAKE_SOURCE_DIR}/test/test_gw_helpers.c
)

#add_definitions(-DUIP_CONF_IPV6=1)
add_definitions(-DUIP_CONF_IPV6=1 -DAUTOSTART_ENABLE -DUIP_CONF_ROUTER=1 -DZIPGW_UNITTEST)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/
  ${CMAKE_SOURCE_DIR}/src/serialapi
  ${CMAKE_SOURCE_DIR}/src/transport
  ${CMAKE_SOURCE_DIR}/src/utls
  ${CMAKE_SOURCE_DIR}/contiki/platform/linux/
  ${CMAKE_SOURCE_DIR}/contiki/core
  ${CMAKE_SOURCE_DIR}/contiki/core/sys
  ${CMAKE_SOURCE_DIR}/contiki/core/lib # list.h
  ${CMAKE_SOURCE_DIR}/contiki/core/net # uip.h
  ${CMAKE_SOURCE_DIR}/contiki/cpu/native/
  ${CMAKE_SOURCE_DIR}/Z-Wave/include
  ${CMAKE_SOURCE_DIR}/test
)

add_subdirectory(pvs)

add_subdirectory(CC_provisioning_list)

add_subdirectory(rd)

add_subdirectory(ZIP_router)

add_subdirectory( zwlog )

add_subdirectory(CC_Indicator)

add_subdirectory(CC_Time)

add_subdirectory(multicast_group_manager)

add_subdirectory(multicast_tlv)

add_subdirectory(test_multicast_auto)

add_subdirectory(CC_fw_upd)

add_executable(eeprom-printer ${CMAKE_SOURCE_DIR}/test/eeprom-printer.c)
