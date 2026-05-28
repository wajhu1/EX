aux_source_directory(. SRC_LIST)

# 如果 src 目录里有 main 文件，生成库时需要排除掉 main 文件
list(FILTER SRC_LIST EXCLUDE REGEX ".*main\\.(cc|cpp|cxx)$")

add_library(threadPool STATIC
    ${SRC_LIST}
)

# 把生成的静态库 libthreadPool.a 放到项目根目录的 lib 文件夹下
set_target_properties(threadPool PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

target_include_directories(threadPool PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(threadPool PUBLIC Threads::Threads)