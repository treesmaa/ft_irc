#include "Debug.hpp"

void debugPrint(const std::string& tag, int fd, const std::string& s)
{
    std::cout << tag << "fd=" << fd << " ";

    for (size_t i = 0; i < s.size(); ++i)
    {
        unsigned char c = s[i];

        if (c == '\r')
            std::cout << "\\r";
        else if (c == '\n')
            std::cout << "\\n\n" << tag << "fd=" << fd << " ";
        else if (c < 32 || c == 127)
            std::cout << "<" << (int)c << ">";
        else
            std::cout << c;
    }

    std::cout << std::endl;
}