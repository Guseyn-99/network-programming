#include "tcp_server.hpp"

void TcpServer::Listen(const std::string& address, std::uint16_t port)
{
    listener_.BindAndListen(address, port);
}

void TcpServer::Accept()
{
    AcceptFrom(listener_);
}

TcpClient TcpServer::AcceptClient()
{
    TcpClient client;
    client.AcceptFrom(listener_);
    return client;
}
