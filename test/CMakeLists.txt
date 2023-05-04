aux_source_directory(. TEST_SRC_LIST)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/test)

# link_directories(${PROJECT_BINARY_DIR}/lib)
find_library(MYSQL_LIBRARY mysqlclient HINTS /usr/lib/x86_64-linux-gnu)
find_library(JSONCPP_LIBRARY jsoncpp HINTS /usr/local/lib)

# add_executable(test_util ${TEST_SRC_LIST})
foreach(TEST_SRC ${TEST_SRC_LIST})
    STRING(REGEX REPLACE "^\\./|\\.c[a-zA-Z0-9_]*$" "" TEST_EXE_NAME ${TEST_SRC})
    message("[hint]: 添加测试程序:${TEST_EXE_NAME}")
    add_executable(${TEST_EXE_NAME} ${TEST_SRC})

    target_link_libraries(${TEST_EXE_NAME} Lib_mysql_connect)
    message("[hint]: Connect to static libraries: Lib_mysql_connect")

    target_link_libraries(${TEST_EXE_NAME} Lib_connection_pool)
    message("[hint]: Connect to static libraries: Lib_connection_pool")

    # 第三方库：mysql
    target_link_libraries(${TEST_EXE_NAME} ${MYSQL_LIBRARY})
    message("[hint]: Connect to static libraries: MYSQL_LIB")

    # 第三方库：jsoncpp
    target_link_libraries(${TEST_EXE_NAME} ${JSONCPP_LIBRARY})
    message("[hint]: Connect to static libraries: JSON_LIBRARY")
endforeach(TEST_SRC ${TEST_SRC_LIST})