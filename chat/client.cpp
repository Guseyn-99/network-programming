#include "chat.hpp"
#include "line_protocol.hpp"
#include "tcp_client.hpp"

#include <atomic>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

namespace
{
void ReceiveMessages(const TcpClient& client, std::atomic_bool& connected)
{
    try
    {
        std::string pending;
        while (const auto message = ReadLine(client, pending))
        {
            std::cout << *message << std::endl;
        }

        if (connected.exchange(false))
            std::cout << "Server disconnected. Press Enter to quit." << std::endl;
    }
    catch (const std::exception& error)
    {
        if (connected.exchange(false))
            std::cerr << "Chat receive error: " << error.what()
                      << ". Press Enter to quit." << std::endl;
    }
}
}

int main()
{
    try
    {
        std::string name;
        do
        {
            std::cout << "Enter your nickname:" << std::endl;
            if (!std::getline(std::cin, name))
                return 0;
        } while (name.empty());

        TcpClient client;
        client.Connect("127.0.0.1", ChatPort);
        client.Send(name + '\n');
        std::cout << "Connected. Type a message or .exit to quit." << std::endl;

        std::atomic_bool connected{true};
        std::jthread receiver([&] { ReceiveMessages(client, connected); });
        try
        {
            std::string message;
            while (std::getline(std::cin, message) && connected && message != ".exit")
            {
                client.Send(message + '\n');
            }
        }
        catch (...)
        {
            connected = false;
            client.Shutdown();
            throw;
        }

        // Завершает блокирующий Receive перед присоединением потока.
        connected = false;
        client.Shutdown();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Chat client error: " << error.what() << std::endl;
        return 1;
    }
}
