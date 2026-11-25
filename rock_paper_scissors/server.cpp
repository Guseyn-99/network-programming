#include "game.hpp"
#include "line_protocol.hpp"
#include "tcp_server.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace
{
std::optional<Move> ReadMove(const TcpClient& player)
{
    std::string pending;
    const auto choice = ReadLine(player, pending);
    if (!choice)
        return std::nullopt;
    return ParseMove(*choice);
}

void PlayRound(const TcpClient& first, const TcpClient& second)
{
    const auto firstMove = ReadMove(first);
    if (!firstMove)
    {
        second.Send("Round cancelled: player 1 disconnected or sent an invalid move.\n");
        return;
    }

    const auto secondMove = ReadMove(second);
    if (!secondMove)
    {
        first.Send("Round cancelled: player 2 disconnected or sent an invalid move.\n");
        return;
    }

    if (*firstMove == *secondMove)
    {
        first.Send("Draw.\n");
        second.Send("Draw.\n");
        return;
    }

    const bool firstWon = Beats(*firstMove, *secondMove);
    first.Send(firstWon ? "You win.\n" : "You lose.\n");
    second.Send(firstWon ? "You lose.\n" : "You win.\n");
}
}

int main()
{
    try
    {
        TcpServer server;
        server.Listen("127.0.0.1", GamePort);
        std::cout << "Game server listening on 127.0.0.1:" << GamePort << std::endl;

        while (true)
        {
            auto first = server.AcceptClient();
            std::cout << "Player 1 connected." << std::endl;
            auto second = server.AcceptClient();
            std::cout << "Player 2 connected." << std::endl;

            try
            {
                PlayRound(first, second);
                std::cout << "Round finished." << std::endl;
            }
            catch (const std::exception& error)
            {
                std::cerr << "Round error: " << error.what() << std::endl;
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Game server error: " << error.what() << std::endl;
        return 1;
    }
}
