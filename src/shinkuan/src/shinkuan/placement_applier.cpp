#include "shinkuan/placement_applier.hpp"

namespace shinkuan {

PlacementApplier::PlacementApplier() {
    db = nullptr;
    std::random_device rd;
    rng = std::mt19937(rd());
}

PlacementApplier::~PlacementApplier() {
    if (db != nullptr) {
        odb::dbDatabase::destroy(db);
        db = nullptr;
    }
}

int PlacementApplier::run(const std::string& inputODBFile, const std::string& inputPlacementTxt, const std::string& outputDir) {
    logger.info("Running PlacementApplier with ODB file: %s, placement file: %s, output dir: %s",
                inputODBFile.c_str(), inputPlacementTxt.c_str(), outputDir.c_str());
    // Load the database
    if (loadDatabaseFromFile_(inputODBFile, &db) != 0) {
        logger.error("Failed to load database from file: %s", inputODBFile.c_str());
        return 1;
    }

    
    return 0;
}

int PlacementApplier::loadDatabaseFromFile_(const std::string& filename, odb::dbDatabase** db_ptr) {
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