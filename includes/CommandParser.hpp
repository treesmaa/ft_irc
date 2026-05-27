#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <iostream>
#include <string>
#include <vector>

typedef struct  t_msg
{
    std::string                 command;
    std::vector<std::string>    parameters;
}               s_msg;

int parse(const std::string& data, s_msg& message);

#endif