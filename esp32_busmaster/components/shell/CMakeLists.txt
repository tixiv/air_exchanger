idf_component_register(SRCS command.c readline.c commands_util.c
                       INCLUDE_DIRS .
                       LDFRAGMENTS linker.lf)
                       
# This is equivalent to adding WHOLE_ARCHIVE option to the idf_component_register call above:
idf_component_set_property(${COMPONENT_NAME} WHOLE_ARCHIVE TRUE)
