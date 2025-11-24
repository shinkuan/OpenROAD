#include <iostream>
#include <fstream>
#include <string>

// #include "ord/OpenRoad.hh"
#include "odb/db.h"

#include "shinkuan/dataset_generator.hpp"
#include "shinkuan/logger.hpp"


int main(int argc, char* argv[])
{   
    if (argc != 4) {
        logger.error("Usage: %s <input_odb_file> <config_file> <output_dir>", argv[0]);
        return 1;
    }

    // logger.info("Starting...");
    // odb::dbDatabase* db = odb::dbDatabase::create();
    // std::string input_filename = "design.odb";
    // std::ifstream input_file(input_filename);
    // if (!input_file.is_open()) {
    //     logger.error("Failed to open file: %s", input_filename.c_str()) ;
    //     return 1;
    // }
    // db->read(input_file);
    // logger.info("Database loaded.");
    // logger.info("Chips in database: %zu", db->getChips().size());
    // odb::dbBlock* block = db->getChips().begin()->getBlock();  
        
    // // 列出所有 instances  
    // logger.info("Total instances: %zu", block->getInsts().size());
    // for (odb::dbInst* inst : block->getInsts()) {  
    //     logger.info("Name: %s, Master: %s, Location: (%d, %d), Status: %s",
    //                 inst->getName().c_str(),
    //                 inst->getMaster()->getName().c_str(),
    //                 inst->getLocation().x(),
    //                 inst->getLocation().y(),
    //                 inst->getPlacementStatus().getString());
    // }

    const std::string inputODBFile = argv[1];
    const std::string configFile = argv[2];
    const std::string outputDir = argv[3];

    shinkuan::DatasetGenerator dataset_generator;
    if (dataset_generator.run(inputODBFile, configFile, outputDir) != 0) {
        logger.error("Dataset generation failed.");
        return 1;
    }

    logger.info("Dataset generation completed successfully.");
    return 0;
}