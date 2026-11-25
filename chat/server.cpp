#include "chat.hpp"
#include "line_protocol.hpp"
#include "tcp_server.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace
{
std::mutex outputMutex;

struct Participant
{
    explicit Participant(TcpClient socket) : connection(std::move(socket)) {}

    TcpClient connection;
    std::mutex sendMutex;
};

class ChatRoom
{
public:
    void Add(const std::shared_ptr<Participant>& participant)
    {
        const std::lock_guard lock(mutex_);
        participants_.push_back(participant);
    }

    void Remove(const std::shared_ptr<Participant>& participant)
    {
        const std::lock_guard lock(mutex_);
        std::erase(participants_, participant);
    }

    void Broadcast(const std::shared_ptr<Participant>& sender, const std::string& message)
    {
        // Список можно менять, пока выполняется блокирующая отправка.
        std::vector<std::shared_ptr<Participant>> recipients;
        {
            const std::lock_guard lock(mutex_);
            recipients = participants_;
        }

        for (const auto& recipient : recipients)
        {
            if (recipient == sender)
                continue;

            // Запись сообщения в один сокет выполняется целиком под одним замком.
            const std::lock_guard sendLock(recipient->sendMutex);
            try
            {
                recipient->connection.Send(message);
            }
            catch (const std::runtime_error&)
            {
                Remove(recipient);
                recipient->connection.Shutdown();
            }
        }
    }

private:
    std::mutex mutex_;
    std::vector<std::shared_ptr<Participant>> participants_;
};

void HandleClient(const std::shared_ptr<ChatRoom>& room,
                  const std::shared_ptr<Participant>& participant)
{
    try
    {
        auto& client = participant->connection;
        std::string pending;
        const auto name = ReadLine(client, pending);
        if (!name || name->empty())
            return;

        room->Add(participant);
        while (const auto message = ReadLine(client, pending))
        {
            const auto line = *name + ": " + *message;
            {
                const std::lock_guard lock(outputMutex);
                std::cout << line << std::endl;
            }
            room->Broadcast(participant, line + '\n');
        }
    }
    catch (const std::exception& error)
    {
        const std::lock_guard lock(outputMutex);
        std::cerr << "Chat connection error: " << error.what() << std::endl;
    }

    room->Remove(participant);
    participant->connection.Shutdown();
}
}

int main()
{
    try
    {
        TcpServer server;
        server.Listen("127.0.0.1", ChatPort);
        std::cout << "Chat server listening on 127.0.0.1:" << ChatPort << std::endl;
        const auto room = std::make_shared<ChatRoom>();

        while (true)
        {
            auto participant = std::make_shared<Participant>(server.AcceptClient());
            try
            {
                std::thread(HandleClient, room, std::move(participant)).detach();
            }
            catch (const std::system_error& error)
            {
                const std::lock_guard lock(outputMutex);
                std::cerr << "Could not start chat thread: " << error.what() << std::endl;
            }
        }
    }
    catch (const std::exception& error)
    {
        const std::lock_guard lock(outputMutex);
        std::cerr << "Chat server error: " << error.what() << std::endl;
        return 1;
    }
}
