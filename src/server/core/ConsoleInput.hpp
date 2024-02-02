#ifndef CONSOLEINPUT_HPP_
#define CONSOLEINPUT_HPP_

#include <functional>
#include <string>
#include <thread>

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <unordered_map>

class ConsoleInput {
public:
    using CommandHandler = std::function<void(const std::string&)>;

    ConsoleInput(asio::io_context& io_context);
    ~ConsoleInput();

    void register_command(const std::string& command, CommandHandler handler);

private:
    void read_thread();

    asio::io_context& m_ioContext;
    std::thread m_thread;

    std::unordered_map<std::string, CommandHandler> command_handlers_;
};

#endif // CONSOLEINPUT_HPP_