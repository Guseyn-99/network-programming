#pragma once

#include "tcp_client.hpp"

class TcpServer final : private TcpSocket
{
public:
    using TcpSocket::Receive;
    using TcpSocket::Send;

    void Listen(const std::string& address, std::uint16_t port);
    void Accept();
    TcpClient AcceptClient();

private:
    TcpSocket listener_;
};
