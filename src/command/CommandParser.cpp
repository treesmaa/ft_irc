#include "CommandParser.hpp"

int skipWS(const std::string& data, int pos) {
    while (pos < (int)data.size() && data[pos] == ' ')
        pos++;
    return pos;
}

int getPrefix(const std::string& data, s_msg& message, int pos) {
    size_t space = data.find(' ');
    if (space == std::string::npos)
        return -1;
    message.prefix = data.substr(1, space - 1);
    pos = space;
    return skipWS(data, pos);
}

int getCommand(const std::string& data, s_msg& message, int pos) {
    size_t space = data.find(' ', pos);
    if (space == std::string::npos) {
        space = data.find('\r', pos);
        if (space == std::string::npos) {
            return -1;
        }
    }
    message.command = data.substr(pos, space - pos);
    pos = space;
    return skipWS(data, pos);
}

int getParameters(const std::string& data, s_msg& message, int pos){
    while (pos < static_cast<int>(data.size()) && data[pos] != '\r') {
        size_t end = data.find(' ', pos);
        if (end == std::string::npos) {
            end = data.find('\r', pos);
            if (end == std::string::npos)
                return -1;
        }
        message.parameters.push_back(data.substr(pos, end - pos));
        pos = end;
        pos = skipWS(data, pos);
    }
    return 0;
}

int parser(const std::string& data, s_msg& message) {
    int pos = 0;
    if (data[pos] == ':')
        if ((pos = getPrefix(data, message, pos)) == -1)
            return -1;
    if ((pos = getCommand(data, message, pos)) == -1)
        return -1;
    if (getParameters(data, message, pos) == -1)
        return -1;
    return 0;
}