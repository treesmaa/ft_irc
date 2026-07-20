#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <string>
#include <iostream>

#ifdef DEBUG

void debugPrint(const std::string& tag, int fd, const std::string& s);

#define DEBUG_PRINT(tag, fd, s)   debugPrint(tag, fd, s)

#else

#define DEBUG_PRINT(tag, fd, s)   ((void)0)

#endif
#endif