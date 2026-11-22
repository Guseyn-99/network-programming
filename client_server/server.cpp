#include "tcp_server.hpp"
#include "line_protocol.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main()
{
    try
    {
        const std::string address = "127.0.0.1";
        constexpr std::uint16_t port = 12345;

        TcpServer server;
        server.Listen(address, port);
        std::cout << "Listening on " << address << ':' << port << std::endl;

        server.Accept();
        std::cout << "Client connected." << std::endl;

        std::string pending;
        while (const auto message = ReadLine(server, pending))
        {
            std::cout << "Client: " << *message << std::endl;
            server.Send("Echo: " + *message + '\n');
        }

        std::cout << "Client disconnected." << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Server error: " << error.what() << std::endl;
        return 1;
    }
}
