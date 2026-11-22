#pragma once

#include <cstdint>
#include <memory>
#include <string>

class TcpSocket
{
protected:
    TcpSocket();
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    void Connect(const std::string& address, std::uint16_t port);
    void Send(const std::string& data) const;

    // Возвращает часть потока. Пустая строка означает, что собеседник закрыл соединение на запись.
    std::string Receive() const;

private:
    friend class TcpServer;

    void BindAndListen(const std::string& address, std::uint16_t port);
    void AcceptFrom(const TcpSocket& listener);

    struct Handle;
    std::unique_ptr<Handle> handle_;
};
