

find_program(XSLTPROC xsltproc)

add_custom_command( OUTPUT command_class_validator.c 
    COMMAND
        ${XSLTPROC} ${CMAKE_CURRENT_SOURCE_DIR}/cvalidator.xsl ${CMAKE_CURRENT_SOURCE_DIR}/ZWave_custom_cmd_classes.xml > command_class_validator.c
    DEPENDS cvalidator.xsl ZWave_custom_cmd_classes.xml 
)


add_custom_command( OUTPUT get_list.c 
    COMMAND
    cat ${CMAKE_CURRENT_SOURCE_DIR}/ZWave_custom_cmd_classes.xml |
     ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/extract_get.py -
    > get_list.c )



add_library( ZWaveAnalyzer command_class_validator.c get_list.c CommandAnalyzer.c )
target_include_directories( ZWaveAnalyzer PUBLIC . )

