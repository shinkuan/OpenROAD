#ifndef __UTIL_H__
#define __UTIL_H__

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "logger.hpp"


#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)

constexpr double EPSILON = 1e-9;

std::string removeComment(const std::string& input, const char comment_char);
void trimString(std::string& s);

#endif // __UTIL_H__