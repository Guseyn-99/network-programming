add_executable(rps_server server.cpp)
target_link_libraries(rps_server PRIVATE tcp_socket)

add_executable(rps_client client.cpp)
target_link_libraries(rps_client PRIVATE tcp_socket)
