#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <functional>
#include <unordered_map>

#include "shinkuan/util.hpp"
#include "shinkuan/logger.hpp"

namespace shinkuan {

constexpr char CONFIG_COMMENT_CHAR = '#';

template<typename T>
struct Rect {
    T xl;
    T yl;
    T xh;
    T yh;
};
typedef Rect<int64_t> HotspotBBox;

struct Hotspot {
    double disturbance;
    int swap_count;
    HotspotBBox bbox;

    Hotspot() : disturbance(0.0), swap_count(0), bbox{0, 0, 0, 0} {}
};
typedef std::vector<Hotspot> HotspotList;
typedef std::unordered_map<std::string, HotspotList> GenerateConfigMap;

class Config {
public:
    Config();
    ~Config();

    GenerateConfigMap generate_configs;

    int loadFromFile(const std::string& filename);

private:
    int parseGenerateBlock(std::ifstream& file);

};

} // namespace shinkuan

#endif // __CONFIG_H__