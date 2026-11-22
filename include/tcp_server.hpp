#pragma once

#include "tcp_socket.hpp"

class TcpServer final : private TcpSocket
{
public:
    using TcpSocket::Receive;
    using TcpSocket::Send;

    void Listen(const std::string& address, std::uint16_t port);
    void Accept();

private:
    TcpSocket listener_;
};
