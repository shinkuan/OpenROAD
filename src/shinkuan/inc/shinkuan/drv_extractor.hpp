#ifndef __DRV_EXTRACTOR_H__
#define __DRV_EXTRACTOR_H__

#include <string>
#include <random>
#include <fstream>

#include "odb/db.h"
#include "utl/Logger.h"

#include "shinkuan/logger.hpp"


namespace shinkuan {

class DRVExtractor {
public:
    DRVExtractor();
    ~DRVExtractor();

    int run(const std::string& inputODBFile, const std::string& outputDir);

private:
    std::mt19937 rng;

    std::string inputODBFile;
    std::string inputPlacementTxt;
    std::string outputDir;

    odb::dbDatabase* db;

    int loadDatabaseFromFile_(const std::string& filename, odb::dbDatabase** db_ptr);
};

} // namespace shinkuan

#endif // __DRV_EXTRACTOR_H__