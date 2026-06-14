#include "Debug.hpp"

void debugPrint(const std::string& s)
{
    std::cout << "[RECV] ";

    for (size_t i = 0; i < s.size(); ++i)
    {
        unsigned char c = s[i];

        if (c == '\r')
            std::cout << "\\r";
        else if (c == '\n')
            std::cout << "\\n\n[RECV] "; // split lines visually
        else if (c < 32 || c == 127)
            std::cout << "<" << (int)c << ">";
        else
            std::cout << c;
    }

    std::cout << std::endl;
}