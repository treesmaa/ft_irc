#include "CommandParser.hpp"

int getPrefix(const std::string& buffer, s_msg& message, int pos) {
    size_t space = buffer.find(' ');//might be other whitespace??
    if (space == std::string::npos)
        return -1;
    message.prefix = buffer.substr(1, space - 1);
    while (pos < buffer.size() && buffer[pos] == ' ')//might be other whitespace??
        pos++;
    return pos;
}

int getCommand(const std::string& buffer, s_msg& message, int pos) {
    size_t space = buffer.find(" \r", pos);//parameters are optional, so i need to look for CRLF as well
    if (space == std::string::npos)
        return -1;
    message.command = buffer.substr(pos, space - 1);
    while (pos < buffer.size() && pos == ' ')//repeating code
        pos++;
    return pos;
}

int parser(const std::string& buffer, s_msg& message) {
    size_t pos = 0;
    if (buffer[pos] == ':') {
        if ((pos = getPrefix(buffer, message, pos)) == -1)
            return -1;
    }
    if ((pos = getCommand(buffer, message, pos)) == -1)
        return -1;
    while (buffer[pos] != '\r') {
        //parse parameters
    }
    return 0;
}