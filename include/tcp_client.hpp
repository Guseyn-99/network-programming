#pragma once

#include "tcp_socket.hpp"

class TcpClient final : private TcpSocket
{
public:
    using TcpSocket::Connect;
    using TcpSocket::Receive;
    using TcpSocket::Send;
};
