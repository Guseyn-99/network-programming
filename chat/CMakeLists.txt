find_package(Threads REQUIRED)

add_executable(chat_server server.cpp)
target_link_libraries(chat_server PRIVATE tcp_socket Threads::Threads)

add_executable(chat_client client.cpp)
target_link_libraries(chat_client PRIVATE tcp_socket Threads::Threads)
