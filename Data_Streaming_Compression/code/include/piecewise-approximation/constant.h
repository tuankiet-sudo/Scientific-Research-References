#include "dependencies.h"
#include "function.h"
#include "timeseries.h"


class PMC {
    // Source paper: Capturing Sensor-Generated Time Series with Quality Guarantees
    // Source path: src/piecewise-approximation/constant/pmc.cpp
    private:

    public:
        static void compress(TimeSeries& timeseries, int mode, float bound, std::string output);
        static void decompress(std::string input, std::string output);
};
