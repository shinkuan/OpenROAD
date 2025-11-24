#include "shinkuan/dataset_generator.hpp"

namespace shinkuan {

DatasetGenerator::DatasetGenerator() {
    db = nullptr;
    std::random_device rd;
    rng = std::mt19937(rd());
}

DatasetGenerator::~DatasetGenerator() {
    if (db != nullptr) {
        odb::dbDatabase::destroy(db);
        db = nullptr;
    }
}

int DatasetGenerator::run(const std::string& inputODBFile, const std::string& configFile, const std::string& outputDir) {
    logger.info("Running DatasetGenerator with input ODB file: %s, config file: %s, output directory: %s",
                inputODBFile.c_str(), configFile.c_str(), outputDir.c_str());
    this->inputODBFile = inputODBFile;
    this->configFile = configFile;
    this->outputDir = outputDir;

    // if (loadDatabase() != 0) {
    //     logger.error("Failed to load database.");
    //     return 1;
    // }
    if (loadConfig() != 0) {
        logger.error("Failed to load configuration.");
        return 1;
    }
    if (generateDatasets() != 0) {
        logger.error("Failed to generate datasets.");
        return 1;
    }
    return 0;
}

int DatasetGenerator::loadDatabase() {
    logger.info("Loading database from ODB file: %s", inputODBFile.c_str());
    if (db != nullptr) {
        logger.warning("Database is already loaded. Destroying the existing database.");
        odb::dbDatabase::destroy(db);
        db = nullptr;
    }
    db = odb::dbDatabase::create();
    logger.debug("Created new dbDatabase instance.");
    std::ifstream input_file(inputODBFile);
    if (!input_file.is_open()) {
        logger.error("Failed to open ODB file: %s", inputODBFile.c_str());
        return 1;
    }
    db->read(input_file);

    if (db->getChips().empty()) {
        logger.error("No chips found in the database.");
        return 1;
    }
    logger.info("Chips in database: %zu", db->getChips().size());
    logger.info("Database loaded successfully.");
    return 0;
}

int DatasetGenerator::loadConfig() {
    logger.info("Loading configuration from file: %s", configFile.c_str());
    config = Config();
    if (config.loadFromFile(configFile) != 0) {
        logger.error("Failed to load configuration from file: %s", configFile.c_str());
        return 1;
    }
    return 0;
}

int DatasetGenerator::generateDatasets() {
    logger.info("Generating datasets in output directory: %s", outputDir.c_str());
    
    for (const auto& [config_name, hotspot_list] : config.generate_configs) {
        logger.info("Generating dataset for configuration: %s with %zu hotspots", config_name.c_str(), hotspot_list.size());
        
        odb::dbDatabase* db_output = nullptr;
        if (loadDatabaseFromFile_(inputODBFile, &db_output) != 0) {
            logger.error("Failed to load database for dataset generation.");
            return 1;
        }
        odb::dbBlock* block = db_output->getChips().begin()->getBlock();
        std::vector<std::vector<odb::dbInst*>> hotspot_instances(hotspot_list.size());
        std::vector<std::vector<odb::dbInst*>> hotspot_instances_top_left(hotspot_list.size());
        std::vector<std::vector<odb::dbInst*>> hotspot_instances_top_right(hotspot_list.size());
        std::vector<std::vector<odb::dbInst*>> hotspot_instances_bottom_left(hotspot_list.size());
        std::vector<std::vector<odb::dbInst*>> hotspot_instances_bottom_right(hotspot_list.size());
        for (odb::dbInst* inst : block->getInsts()) {
            odb::dbPlacementStatus status = inst->getPlacementStatus();
            if (
                status != odb::dbPlacementStatus::PLACED
            ) {
                continue;
            }

            odb::dbBox* bbox = inst->getBBox();
            for (size_t i = 0; i < hotspot_list.size(); ++i) {
                const Hotspot& hotspot = hotspot_list[i];
                if (bbox->xMin() >= hotspot.bbox.xl && bbox->xMax() <= hotspot.bbox.xh &&
                    bbox->yMin() >= hotspot.bbox.yl && bbox->yMax() <= hotspot.bbox.yh) {
                    hotspot_instances[i].push_back(inst);
                    const int center_x = hotspot.bbox.xl + (hotspot.bbox.xh - hotspot.bbox.xl) / 2;
                    const int center_y = hotspot.bbox.yl + (hotspot.bbox.yh - hotspot.bbox.yl) / 2;
                    const int inst_center_x = (bbox->xMin() + bbox->xMax()) / 2;
                    const int inst_center_y = (bbox->yMin() + bbox->yMax()) / 2;
                    if (inst_center_x <= center_x) {
                        if (inst_center_y >= center_y) {
                            hotspot_instances_top_left[i].push_back(inst);
                        } else {
                            hotspot_instances_bottom_left[i].push_back(inst);
                        }
                    } else {
                        if (inst_center_y >= center_y) {
                            hotspot_instances_top_right[i].push_back(inst);
                        } else {
                            hotspot_instances_bottom_right[i].push_back(inst);
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < hotspot_list.size(); ++i) {
            const Hotspot& hotspot = hotspot_list[i];
            logger.info("Hotspot %zu: BBox[(%d, %d), (%d, %d)] has %zu instances",
                        i,
                        hotspot.bbox.xl, hotspot.bbox.yl,
                        hotspot.bbox.xh, hotspot.bbox.yh,
                        hotspot_instances[i].size());
            int64_t total_distance = 0;
            int64_t target_distance = static_cast<int64_t>(hotspot.disturbance *
                (hotspot.bbox.xh - hotspot.bbox.xl + hotspot.bbox.yh - hotspot.bbox.yl) * 2 * hotspot_instances[i].size());
            int64_t total_swaps = 0;
            int64_t target_swaps = hotspot.swap_count;

            auto pick_random_instance = [&](std::vector<odb::dbInst*>& insts) -> odb::dbInst* {
                std::uniform_int_distribution<size_t> dist(0, insts.size() - 1);
                const size_t idx = dist(rng);
                odb::dbInst* inst = insts[idx];
                if (idx != insts.size() - 1) {
                    std::swap(insts[idx], insts.back());
                }
                insts.pop_back();
                return inst;
            };
            const auto abs64 = [](int64_t value) -> int64_t {
                return value < 0 ? -value : value;
            };

            auto while_condition = [&]() -> bool {
                if (target_swaps > 0) {
                    return total_swaps < target_swaps;
                } else {
                    return total_distance < target_distance;
                }
            };

            while (while_condition()) {
                const bool has_tl_br = !hotspot_instances_top_left[i].empty() && !hotspot_instances_bottom_right[i].empty();
                const bool has_tr_bl = !hotspot_instances_top_right[i].empty() && !hotspot_instances_bottom_left[i].empty();
                if (!has_tl_br && !has_tr_bl) {
                    logger.warning("Hotspot %zu cannot reach target disturbance due to insufficient quadrant pairs.", i);
                    break;
                }

                bool use_tl_br = has_tl_br;
                if (has_tl_br && has_tr_bl) {
                    use_tl_br = std::uniform_int_distribution<int>(0, 1)(rng) == 0;
                } else if (!has_tl_br && has_tr_bl) {
                    use_tl_br = false;
                }

                std::vector<odb::dbInst*>& first_group = use_tl_br ? hotspot_instances_top_left[i] : hotspot_instances_top_right[i];
                std::vector<odb::dbInst*>& second_group = use_tl_br ? hotspot_instances_bottom_right[i] : hotspot_instances_bottom_left[i];

                odb::dbInst* inst_a = pick_random_instance(first_group);
                odb::dbInst* inst_b = pick_random_instance(second_group);

                int ax, ay, bx, by;
                inst_a->getLocation(ax, ay);
                inst_b->getLocation(bx, by);

                inst_a->setLocation(bx, by);
                inst_b->setLocation(ax, ay);

                int64_t swap_distance = abs64(static_cast<int64_t>(ax) - bx) +
                                        abs64(static_cast<int64_t>(ay) - by);
                if (swap_distance == 0) {
                    logger.warning("Swapped instances are at the same location, skipping.");
                    continue;
                }
                total_distance += swap_distance;
                total_swaps += 1;
            }
        }

        std::ofstream output_file(outputDir + "/" + config_name + "_output.odb");
        db_output->write(output_file);
        odb::dbDatabase::destroy(db_output);
        logger.info("Dataset for configuration %s generated successfully.", config_name.c_str());
        output_file.close();
    }

    return 0;
}

int DatasetGenerator::loadDatabaseFromFile_(const std::string& filename, odb::dbDatabase** db_ptr) {
    logger.info("Loading database from file: %s", filename.c_str());
    std::ifstream input_file(filename);
    if (!input_file.is_open()) {
        logger.error("Failed to open ODB file: %s", filename.c_str());
        return 1;
    }
    *db_ptr = odb::dbDatabase::create();
    (*db_ptr)->read(input_file);
    if ((*db_ptr)->getChips().empty()) {
        logger.error("No chips found in the database.");
        return 1;
    }
    logger.info("Chips in database: %zu", (*db_ptr)->getChips().size());
    logger.info("Database loaded successfully from file: %s", filename.c_str());
    input_file.close();

    logger.info("Creating utl::Logger instance for database operations.");
    utl::Logger* utl_logger = new utl::Logger();
    logger.info("utl::Logger instance created successfully.");
    (*db_ptr)->setLogger(utl_logger);
    return 0;
}

} // namespace shinkuan