cmake_minimum_required(VERSION 3.16)

project(threadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

add_executable(test_threadpool
    test/main.cc
)

target_include_directories(test_threadpool PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_directories(test_threadpool PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(test_threadpool PRIVATE
    threadPool
)

set_target_properties(test_threadpool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

target_compile_options(test_threadpool PRIVATE
    -fsanitize=address
    -g
    -O0
)

target_link_options(test_threadpool PRIVATE
    -fsanitize=address
)