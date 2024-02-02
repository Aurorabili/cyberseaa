#ifndef LOGSTREAM_HPP_
#define LOGSTREAM_HPP_

#include <fstream>
#include <iostream>
#include <string>

class LogStream
{
  public:
    void openFile(const std::string &filename)
    {
        m_file.open(filename, std::ofstream::out | std::ofstream::trunc);
        if (!m_file.is_open())
            std::cerr << "Can't open log file: '" << filename << "'" << std::endl;
    }

    LogStream &operator<<(const std::string &str)
    {
        std::cout << str;
        if (m_file.is_open())
            m_file << str;
        return *this;
    }

    LogStream &operator<<(int i)
    {
        std::cout << i;
        if (m_file.is_open())
            m_file << i;
        return *this;
    }

    LogStream &operator<<(char c)
    {
        std::cout << c;
        if (m_file.is_open())
            m_file << c;
        return *this;
    }

    LogStream &operator<<(std::ostream &(*f)(std::ostream &))
    {
        f(std::cout);
        if (m_file.is_open())
            f(m_file);
        return *this;
    }

  private:
    std::ofstream m_file;
};

#endif /* LOGSTREAM_HPP_ */
