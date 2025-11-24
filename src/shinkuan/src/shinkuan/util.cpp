#include "shinkuan/util.hpp"

std::string removeComment(const std::string& input, const char comment_char) {
    size_t pos = input.find(comment_char);
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input;
}

void trimString(std::string& s) {
    // Trim leading spaces
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    // Trim trailing spaces
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}