#include "tcp_client.hpp"
#include "line_protocol.hpp"

#include <charconv>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>

int main()
{
    try
    {
        std::cout << "Enter IP address and port (for example, 127.0.0.1:12345):" << std::endl;
        std::string line;
        if (!std::getline(std::cin, line))
            return 0;

        const auto colon = line.find(':');
        if (colon == std::string::npos)
            throw std::invalid_argument("Expected IPv4 address and port separated by ':'");

        const auto address = line.substr(0, colon);
        const auto portText = line.substr(colon + 1);
        std::uint16_t port = 0;
        const auto end = portText.data() + portText.size();
        const auto result = std::from_chars(portText.data(), end, port);
        if (result.ec != std::errc{} || result.ptr != end || port == 0)
            throw std::invalid_argument("The port must be a number from 1 to 65535");

        TcpClient client;
        client.Connect(address, port);
        std::cout << "Connected. Type a message or .exit to quit." << std::endl;

        std::string pending;
        while (std::getline(std::cin, line) && line != ".exit")
        {
            client.Send(line + '\n');
            const auto reply = ReadLine(client, pending);
            if (!reply)
                throw std::runtime_error("Server closed the connection before replying");

            std::cout << "Server: " << *reply << std::endl;
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Client error: " << error.what() << std::endl;
        return 1;
    }
}
