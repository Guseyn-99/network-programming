add_executable(server server.cpp)
target_link_libraries(server PRIVATE tcp_socket)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE tcp_socket)
