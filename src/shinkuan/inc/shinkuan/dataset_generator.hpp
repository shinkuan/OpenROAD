#ifndef __DATASET_GENERATOR_H__
#define __DATASET_GENERATOR_H__

#include <string>
#include <random>
#include <fstream>

#include "odb/db.h"
#include "utl/Logger.h"

#include "shinkuan/config.hpp"
#include "shinkuan/logger.hpp"


namespace shinkuan {

class DatasetGenerator {
public:
    DatasetGenerator();
    ~DatasetGenerator();

    int run(const std::string& inputODBFile, const std::string& configFile, const std::string& outputDir);

private:
    std::mt19937 rng;

    std::string inputODBFile;
    std::string configFile;
    std::string outputDir;

    odb::dbDatabase* db;

    Config config;

    int loadDatabase();
    int loadConfig();
    int generateDatasets();

    int loadDatabaseFromFile_(const std::string& filename, odb::dbDatabase** db_ptr);
};

} // namespace shinkuan

#endif // __DATASET_GENERATOR_H__