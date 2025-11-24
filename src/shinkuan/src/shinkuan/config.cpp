#include "shinkuan/config.hpp"


namespace shinkuan {

Config::Config() {}
Config::~Config() {}


// #######################
// # generate
// #   name <name>
// #   hotspot
// #     disturbance <float>
// #     swap_count <int>
// #     bbox <l> <b> <r> <t>
// #   end_hotspot
// #   ...
// # end_generate
// #######################
// generate
//     name generate_example
//     hotspot
//         disturbance 0.5
//         bbox 380 2800 380 2800
//     end_hotspot
//     hotspot
//         disturbance 0.3
//         bbox 760 5600 760 5600
//     end_hotspot
// end_generate

int Config::loadFromFile(const std::string& filename) {
    logger.info("[Config] Loading configuration from file: %s", filename.c_str());
    std::ifstream file(filename);
    if (!file.is_open()) {
        logger.error("[Config] Failed to open config file: %s", filename.c_str());
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        line = removeComment(line, CONFIG_COMMENT_CHAR);
        trimString(line);
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string first_word;
        iss >> first_word;
        if (first_word == "generate") {
            if (parseGenerateBlock(file) != 0) {
                logger.error("[Config] Failed to parse generate block.");
                return 1;
            }
        } else {
            logger.warning("[Config] Unknown configuration directive: %s", first_word.c_str());
        }
    }

    logger.info("[Config] Configuration loaded successfully.");
    return 0;
}

int Config::parseGenerateBlock(std::ifstream& file) {
    std::string line;
    std::string gen_config_name = "";
    HotspotList gen_config_hotspots;
    while (std::getline(file, line)) {
        line = removeComment(line, CONFIG_COMMENT_CHAR);
        trimString(line);
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string first_word;
        iss >> first_word;
        if (first_word == "name") {
            if (!(iss >> gen_config_name)) {
                logger.error("[Config] Missing name in generate block.");
                return 1;
            }
        } else if (first_word == "hotspot") {
            Hotspot hotspot;
            while (std::getline(file, line)) {
                line = removeComment(line, CONFIG_COMMENT_CHAR);
                trimString(line);
                if (line.empty()) {
                    continue;
                }
                std::istringstream hotspot_iss(line);
                std::string hotspot_word;
                hotspot_iss >> hotspot_word;
                if (hotspot_word == "disturbance") {
                    double disturbance;
                    if (!(hotspot_iss >> disturbance)) {
                        logger.error("[Config] Missing disturbance value in hotspot.");
                        return 1;
                    }
                    hotspot.disturbance = disturbance;
                } else if (hotspot_word == "swap_count") {
                    int swap_count;
                    if (!(hotspot_iss >> swap_count)) {
                        logger.error("[Config] Missing swap_count value in hotspot.");
                        return 1;
                    }
                    hotspot.swap_count = swap_count;
                } else if (hotspot_word == "bbox") {
                    int64_t xl, yl, xh, yh;
                    if (!(hotspot_iss >> xl >> yl >> xh >> yh)) {
                        logger.error("[Config] Invalid bbox values in hotspot.");
                        return 1;
                    }
                    hotspot.bbox = {xl, yl, xh, yh};
                } else if (hotspot_word == "end_hotspot") {
                    logger.debug("[Config] Parsed hotspot: disturbance=%.3f, bbox=(%d, %d, %d, %d)",
                                 hotspot.disturbance,
                                 hotspot.bbox.xl, hotspot.bbox.yl,
                                 hotspot.bbox.xh, hotspot.bbox.yh);
                    gen_config_hotspots.push_back(hotspot);
                    break;
                } else {
                    logger.warning("[Config] Unknown hotspot directive: %s", hotspot_word.c_str());
                }
            }
        } else if (first_word == "end_generate") {
            if (gen_config_name.empty()) {
                logger.error("[Config] Missing name for generate block.");
                return 1;
            }
            generate_configs[gen_config_name] = gen_config_hotspots;
            logger.info("[Config] Parsed generate block: name=%s, hotspots=%zu",
                        gen_config_name.c_str(), gen_config_hotspots.size());
            return 0;
        } else {
            logger.warning("[Config] Unknown generate directive: %s", first_word.c_str());
        }
    }
    logger.error("[Config] Missing end_generate directive.");
    return 1;
}

} // namespace shinkuan