 # © 2018 Silicon Laboratories Inc.
 #include( ExternalProject )

include_directories(
  ${OPENSSL_INCLUDE_DIRS}
)
add_definitions(-Wno-undef)

add_executable(test_gecko_fwu
  test_gecko_fwu.c
  ${TEST_HELPERS}
  ${CMAKE_SOURCE_DIR}/test/test_CC_helpers.c
  ${CMAKE_SOURCE_DIR}/test/eeprom-stub.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/assert.c
  ${CMAKE_SOURCE_DIR}/contiki/platform/linux/zgw_log_int.c
  ${CMAKE_SOURCE_DIR}/src/CC_FirmwareUpdate.c
)

target_compile_definitions(test_gecko_fwu PUBLIC
  ZGW_LOG ZGW_LOG_LOG_TO_FILE ZGW_LOG_LVL_INIT=5
  )

# clock time
if( NOT APPLE )
  target_link_libraries(test_gecko_fwu rt)
endif()


add_test(gecko_fwu test_gecko_fwu )
