# Always add gcov support in unit test
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-undef -ggdb")


add_definitions(-DMCAST_MAX_GROUPS=5 -DTEST_MCAST)

include_directories(${CMAKE_SOURCE_DIR}/contiki/core/sys  ${CMAKE_SOURCE_DIR}/Z-Wave/include
${CMAKE_SOURCE_DIR}/contiki/cpu/native ${CMAKE_SOURCE_DIR}/src/transport  ${CMAKE_SOURCE_DIR}/test)

add_executable(test_nodemask
  ${CMAKE_SOURCE_DIR}/test/multicast_group_manager/test_nodemask.c
  ${CMAKE_SOURCE_DIR}/src/utls/zgw_nodemask.c
  ${CMAKE_SOURCE_DIR}/test/test_helpers.c
# For clock_time:
  ${CMAKE_SOURCE_DIR}/test/test_gw_helpers.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/assert.c
)

add_test(zgw_nodemask test_nodemask)

add_executable(test_multicast_group_manager
  ${CMAKE_SOURCE_DIR}/test/multicast_group_manager/test_multicast_group_manager.c
  ${CMAKE_SOURCE_DIR}/src/multicast_group_manager.c
  ${CMAKE_SOURCE_DIR}/src/utls/zgw_nodemask.c
  ${CMAKE_SOURCE_DIR}/test/test_helpers.c
# For clock_time:
  ${CMAKE_SOURCE_DIR}/test/test_gw_helpers.c
  ${CMAKE_SOURCE_DIR}/contiki/core/lib/assert.c
)

add_test(multicast_group_manager test_multicast_group_manager)

FILE(GLOB MCAST_GROUP_GCOV_OBJECTS
  ABSOLUTE "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_multicast_group_manager.dir/__/__/src/*.o")

FILE(GLOB NM_GCOV_OBJECTS
  ABSOLUTE "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_nodemask.dir/__/__/src/utils/*.o")

add_custom_target(mcg_gcov
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND gcov -b ${MCAST_GROUP_GCOV_OBJECTS}
  DEPENDS test_multicast_group_manager
  )
add_custom_target(nm_gcov
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND gcov -b ${NM_GCOV_OBJECTS}
  DEPENDS test_nodemask
  )
