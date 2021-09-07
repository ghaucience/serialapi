add_definitions( -D_GNU_SOURCE )
set(ZIPGW_SRC
  ${CMAKE_SOURCE_DIR}/src
  )
set(SMALLOC_SRC
  ${ZIPGW_SRC}/smalloc.c
  )
include_directories(
  ${ZIPGW_SRC}
  )
add_executable( zgw_convert_eeprom convert_eeprom.c ${SMALLOC_SRC})
