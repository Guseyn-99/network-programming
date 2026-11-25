#include "game.hpp"
#include "line_protocol.hpp"
#include "tcp_client.hpp"

#include <exception>
#include <iostream>
#include <string>

int main()
{
    try
    {
        TcpClient player;
        player.Connect("127.0.0.1", GamePort);
        std::cout << "Choose 1 (rock), 2 (paper) or 3 (scissors):" << std::endl;

        std::string choice;
        while (std::getline(std::cin, choice))
        {
            if (ParseMove(choice))
            {
                player.Send(choice + '\n');
                std::string pending;
                const auto result = ReadLine(player, pending);
                if (!result)
                    throw std::runtime_error("Server disconnected before sending the result");

                std::cout << *result << std::endl;
                return 0;
            }

            std::cout << "Enter 1, 2 or 3." << std::endl;
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Game client error: " << error.what() << std::endl;
        return 1;
    }
}
