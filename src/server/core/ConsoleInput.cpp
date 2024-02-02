#include "server/core/ConsoleInput.hpp"
#include "common/utils/Debug.hpp"

#include <iostream>

ConsoleInput::ConsoleInput(asio::io_context& io_context)
    : m_ioContext(io_context)
    , m_thread([this]() { this->read_thread(); })
{
    register_command("stop", [this](const std::string&) {
        this->m_ioContext.stop();
    });
}

ConsoleInput::~ConsoleInput()
{
    m_thread.join();
}

void ConsoleInput::register_command(const std::string& command, CommandHandler handler)
{
    command_handlers_.emplace(command, handler);
}

void ConsoleInput::read_thread()
{
    std::string line;

    while (std::getline(std::cin, line) && !m_ioContext.stopped()) {
        asio::post(m_ioContext, [this, line]() {
            logDebug() << "Console Input: " << line;
            auto it = command_handlers_.find(line);
            if (it != command_handlers_.end()) {
                it->second(line);
            } else {
                logError() << "Unknown command: " << line;
            }
        });
    }
}