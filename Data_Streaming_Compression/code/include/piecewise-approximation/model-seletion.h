#include "system/io.h"
#include "system/monitor.h"
#include "dependencies.h"
#include "algebraic/function.h"
#include "timeseries.h"


class SmartGridCompression {
    // Source paper: A time-series compression technique and its application to the smart grid
    // Source path: src/piecewise-approximation/polynomial/smart-grid-compression.cpp
    private:

    public:
        static void compress(TimeSeries& timeseries, int degree, float bound, std::string output);
        static void decompress(std::string input, std::string output);
};