#pragma once

#include <stdio.h>
#include <string>

#define PRINT_COLOR_WHITE	std::string("\x1b[0m" )
#define PRINT_COLOR_GREEN	std::string("\x1b[32m")
#define PRINT_COLOR_YELLOW	std::string("\x1b[33m")
#define PRINT_COLOR_RED		std::string("\x1b[31m")
#define PRINT_COLOR_MAGENTA	std::string("\x1b[35m")

#define LOG_ERROR(f,...) printf((PRINT_COLOR_RED + f + PRINT_COLOR_WHITE + '\n').c_str(),__VA_ARGS__)
#define LOG_WARN(f,...) printf((PRINT_COLOR_YELLOW + f + PRINT_COLOR_WHITE + '\n').c_str(),__VA_ARGS__)
#define LOG_INFO(f,...) printf((PRINT_COLOR_GREEN + f + PRINT_COLOR_WHITE + '\n').c_str(),__VA_ARGS__)
#define LOG_TRACE(f,...) printf((PRINT_COLOR_WHITE + f + '\n').c_str(),__VA_ARGS__)