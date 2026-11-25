#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "tcp_socket.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace
{
#ifdef _WIN32
using NativeSocket = SOCKET;
using AddressSize = int;
constexpr NativeSocket InvalidSocket = INVALID_SOCKET;
constexpr int SendFlags = 0;

class WinsockSession
{
public:
    WinsockSession()
    {
        WSADATA data{};
        const int error = WSAStartup(MAKEWORD(2, 2), &data);
        if (error != 0)
            throw std::system_error(error, std::system_category(), "WSAStartup");
    }

    ~WinsockSession()
    {
        WSACleanup();
    }
};

int LastSocketError()
{
    return WSAGetLastError();
}

bool IsInterrupted(int error)
{
    return error == WSAEINTR;
}
#else
using NativeSocket = int;
using AddressSize = socklen_t;
constexpr NativeSocket InvalidSocket = -1;
constexpr int SendFlags = MSG_NOSIGNAL;

int LastSocketError()
{
    return errno;
}

bool IsInterrupted(int error)
{
    return error == EINTR;
}
#endif

constexpr int BufferSize = 4096;

sockaddr_in MakeAddress(const std::string& address, std::uint16_t port)
{
    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);
    if (inet_pton(AF_INET, address.c_str(), &endpoint.sin_addr) != 1)
        throw std::invalid_argument("Invalid IPv4 address: " + address);
    return endpoint;
}
}

struct TcpSocket::Handle
{
#ifdef _WIN32
    WinsockSession winsock;
#endif
    NativeSocket value = InvalidSocket;

    void Open()
    {
        value = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (value == InvalidSocket)
            throw std::system_error(LastSocketError(), std::system_category(), "socket");
    }

    ~Handle()
    {
        if (value == InvalidSocket)
            return;
#ifdef _WIN32
        ::closesocket(value);
#else
        ::close(value);
#endif
    }
};

TcpSocket::TcpSocket() = default;
TcpSocket::TcpSocket(TcpSocket&&) noexcept = default;
TcpSocket::~TcpSocket() = default;

void TcpSocket::Connect(const std::string& address, std::uint16_t port)
{
    auto socket = std::make_unique<Handle>();
    const auto endpoint = MakeAddress(address, port);
    socket->Open();

    if (::connect(socket->value, reinterpret_cast<const sockaddr*>(&endpoint),
                  static_cast<AddressSize>(sizeof(endpoint))) != 0)
        throw std::system_error(LastSocketError(), std::system_category(), "connect");

    handle_ = std::move(socket);
}

void TcpSocket::BindAndListen(const std::string& address, std::uint16_t port)
{
    auto socket = std::make_unique<Handle>();
    const auto endpoint = MakeAddress(address, port);
    socket->Open();

#ifndef _WIN32
    const int reuseAddress = 1;
    if (::setsockopt(socket->value, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                     sizeof(reuseAddress)) != 0)
        throw std::system_error(LastSocketError(), std::system_category(), "setsockopt");
#endif

    if (::bind(socket->value, reinterpret_cast<const sockaddr*>(&endpoint),
               static_cast<AddressSize>(sizeof(endpoint))) != 0)
        throw std::system_error(LastSocketError(), std::system_category(), "bind");
    if (::listen(socket->value, SOMAXCONN) != 0)
        throw std::system_error(LastSocketError(), std::system_category(), "listen");

    handle_ = std::move(socket);
}

void TcpSocket::AcceptFrom(const TcpSocket& listener)
{
    if (!listener.handle_)
        throw std::logic_error("Call Listen before Accept");

    auto peer = std::make_unique<Handle>();
    while (true)
    {
        peer->value = ::accept(listener.handle_->value, nullptr, nullptr);
        if (peer->value != InvalidSocket)
            break;

        const int error = LastSocketError();
        if (!IsInterrupted(error))
            throw std::system_error(error, std::system_category(), "accept");
    }

    handle_ = std::move(peer);
}

void TcpSocket::Send(const std::string& data) const
{
    if (!handle_)
        throw std::logic_error("No connected socket");

    // send может отправить только часть переданных байтов.
    std::size_t offset = 0;
    while (offset < data.size())
    {
        const auto remaining = data.size() - offset;
        const auto size = static_cast<int>(std::min(remaining, std::size_t{BufferSize}));
        const auto sent = ::send(handle_->value, data.data() + offset, size, SendFlags);
        if (sent < 0)
        {
            const int error = LastSocketError();
            if (IsInterrupted(error))
                continue;
            throw std::system_error(error, std::system_category(), "send");
        }
        if (sent == 0)
            throw std::runtime_error("send returned zero bytes");

        offset += static_cast<std::size_t>(sent);
    }
}

std::string TcpSocket::Receive() const
{
    if (!handle_)
        throw std::logic_error("No connected socket");

    std::string data(BufferSize, '\0');
    while (true)
    {
        const auto received = ::recv(handle_->value, data.data(), BufferSize, 0);
        if (received >= 0)
        {
            data.resize(static_cast<std::size_t>(received));
            return data;
        }

        const int error = LastSocketError();
        if (!IsInterrupted(error))
            throw std::system_error(error, std::system_category(), "recv");
    }
}

void TcpSocket::Shutdown() const noexcept
{
    if (!handle_)
        return;

#ifdef _WIN32
    ::shutdown(handle_->value, SD_BOTH);
#else
    ::shutdown(handle_->value, SHUT_RDWR);
#endif
}
