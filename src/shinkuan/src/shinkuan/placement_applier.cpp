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

    std::ifstream placement_file(inputPlacementTxt);
    if (!placement_file.is_open()) {
        logger.error("Failed to open placement file: %s", inputPlacementTxt.c_str());
        return 1;
    }

    std::string test_master_name = "DFF_X1";
    odb::dbMaster* master = db->findMaster(test_master_name.c_str());
    logger.info("Tested finding master: %s", test_master_name.c_str());
    logger.info("    isSequential: %d", master->isSequential());

    odb::dbBlock* block = db->getChips().begin()->getBlock();

    // List out all instance
    logger.info("Total instances in block: %zu", block->getInsts().size());
    for (auto inst : block->getInsts()) {  
        logger.debug("Instance Name: %s, Master: %s, Location: (%d, %d), Status: %s",
                    inst->getName().c_str(),
                    inst->getMaster()->getName().c_str(),
                    inst->getLocation().x(),
                    inst->getLocation().y(),
                    inst->getPlacementStatus().getString());
    }

    std::string line;
    while (std::getline(placement_file, line)) {
        logger.info("Read placement line: %s", line.c_str());
        // <NAME> <X> <Y>
        std::istringstream iss(line);
        std::string inst_name;
        int x, y;
        if (!(iss >> inst_name >> x >> y)) {
            logger.warning("Invalid line format: %s", line.c_str());
            continue;
        }

        // if inst_name contains `[` or `]`, replace them with `\[` and `\]` for odb lookup
        std::string escaped_inst_name;
        escaped_inst_name.reserve(inst_name.size() * 2);
        for (char ch : inst_name) {
            if (ch == '[' || ch == ']') {
                escaped_inst_name.push_back('\\');
            }
            escaped_inst_name.push_back(ch);
        }
        inst_name.swap(escaped_inst_name);

        odb::dbInst* inst = block->findInst(inst_name.c_str());
        if (inst == nullptr) {
            logger.warning("Instance not found: %s", inst_name.c_str());
            continue;
        }
        inst->setLocation(x, y);
    }
    placement_file.close();

    std::string outputODBFile = outputDir + "/placed_design.odb";
    std::ofstream output_file(outputODBFile);
    if (!output_file.is_open()) {
        logger.error("Failed to open output ODB file: %s", outputODBFile.c_str());
        return 1;
    }
    db->write(output_file);
    output_file.close();
    logger.info("Placed design written to: %s", outputODBFile.c_str());
    odb::dbDatabase::destroy(db);
    db = nullptr;
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
    logger.info("Creating utl::Logger instance for database operations.");
    utl::Logger* utl_logger = new utl::Logger();
    logger.info("utl::Logger instance created successfully.");
    (*db_ptr)->setLogger(utl_logger);
    (*db_ptr)->read(input_file);
    if ((*db_ptr)->getChips().empty()) {
        logger.error("No chips found in the database.");
        return 1;
    }
    logger.info("Chips in database: %zu", (*db_ptr)->getChips().size());
    logger.info("Database loaded successfully from file: %s", filename.c_str());
    input_file.close();
    return 0;
}

} // namespace shinkuan