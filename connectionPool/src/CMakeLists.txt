aux_source_directory(. SRC_LIST)

add_library(connectionPool ${SRC_LIST})

target_include_directories(connectionPool PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/config
    /opt/homebrew/opt/mysql/include/mysql
)

target_link_directories(connectionPool PRIVATE
    /opt/homebrew/opt/mysql/lib
)

target_link_libraries(connectionPool PRIVATE
    mysqlclient
)