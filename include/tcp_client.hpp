#pragma once

#include "tcp_socket.hpp"

class TcpClient final : private TcpSocket
{
    friend class TcpServer;

public:
    using TcpSocket::Connect;
    using TcpSocket::Receive;
    using TcpSocket::Send;
    using TcpSocket::Shutdown;
};
