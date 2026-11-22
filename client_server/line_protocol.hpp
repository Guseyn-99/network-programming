#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

// TCP передаёт поток байтов. В консольном примере конец сообщения отмечаем '\n'.
template <typename Socket>
std::optional<std::string> ReadLine(const Socket& socket, std::string& pending)
{
    std::size_t searchFrom = 0;
    while (true)
    {
        const auto end = pending.find('\n', searchFrom);
        if (end != std::string::npos)
        {
            auto line = pending.substr(0, end);
            pending.erase(0, end + 1);
            return line;
        }

        const auto chunk = socket.Receive();
        if (chunk.empty())
        {
            if (!pending.empty())
                throw std::runtime_error("Connection closed in the middle of a message");
            return std::nullopt;
        }
        searchFrom = pending.size();
        pending += chunk;
    }
}
