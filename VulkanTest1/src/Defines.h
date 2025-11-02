#pragma once

#define _BREAK() __debugbreak();

#include "Logging.h"

#define DEBUG_ASSERT(x,f,...) if(!(x)) {LOG_ERROR(f,__VA_ARGS__); _BREAK();std::exit(-1);}
#define ASSERT(x,f,...)		if(!(x)) {LOG_ERROR(f,__VA_ARGS__); _BREAK();std::exit(-1);}