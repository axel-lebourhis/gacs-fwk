cmake_minimum_required(VERSION 3.0.0)
project(GACS-FWK VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

add_library(gacs_common INTERFACE)

target_include_directories(gacs_common INTERFACE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/asio-1.18.2/include
)

target_link_libraries(gacs_common INTERFACE
    ws2_32
    wsock32
)

if(GACS_FWK_BUILD_TESTS)
    add_subdirectory(tests)
endif()
