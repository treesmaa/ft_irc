#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <string>
#include <iostream>

#ifdef DEBUG

void debugPrint(const std::string& s);

#define DEBUG_PRINT(s)   debugPrint(s)

#else

#define DEBUG_PRINT(s)   ((void)0)

#endif
#endif