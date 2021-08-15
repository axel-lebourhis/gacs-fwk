
add_executable(test_gacs_server
    test_gacs_server.cpp
)

target_include_directories(test_gacs_server
    PRIVATE
        $<TARGET_PROPERTY:gacs_common,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(test_gacs_server
    gacs_common
)

add_executable(test_gacs_client
    test_gacs_client.cpp
)

target_include_directories(test_gacs_client
    PRIVATE
        $<TARGET_PROPERTY:gacs_common,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(test_gacs_client
    gacs_common
)