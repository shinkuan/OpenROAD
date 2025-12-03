#include <iostream>
#include <fstream>
#include <string>

// #include "ord/OpenRoad.hh"
#include "odb/db.h"

#include "shinkuan/drv_extractor.hpp"
#include "shinkuan/logger.hpp"


int main(int argc, char* argv[])
{   
    if (argc != 3) {
        logger.error("Usage: %s <input_odb_file> <output_dir>", argv[0]);
        return 1;
    }

    const std::string inputODBFile = argv[1];
    const std::string outputDir = argv[2];

    shinkuan::DRVExtractor dataset_generator;
    if (dataset_generator.run(inputODBFile, outputDir) != 0) {
        logger.error("DRV Json generation failed.");
        return 1;
    }

    logger.info("DRV Json generation completed successfully.");
    return 0;
}