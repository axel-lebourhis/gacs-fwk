
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