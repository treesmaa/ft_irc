#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include <vector>

#define MAX_LENGTH 512

typedef struct  t_msg
{
    std::string prefix;
    std::string command;
    std::vector<std::string> parameters;
}               s_msg;

int parser(const std::string& buffer, s_msg& message);

#endif