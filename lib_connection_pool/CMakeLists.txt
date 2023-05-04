aux_source_directory(. DIR_LIB_SRC)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(Lib_connection_pool STATIC ${DIR_LIB_SRC})

message("[hint]: Generate static link library: Lib_connection_pool")