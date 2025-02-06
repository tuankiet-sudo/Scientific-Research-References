#include <iostream>

#include "system/io.hpp"
#include "system/monitor.hpp"
#include "algebraic/function.hpp"
#include "timeseries.hpp"


namespace PMC {
    // Source paper: Capturing Sensor-Generated Time Series with Quality Guarantees
    // Source path: src/piecewise-approximation/constant/pmc.cpp
    void compress(TimeSeries& timeseries, std::string mode, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};


namespace HybridPCA {
    // Source paper: Improved Piecewise Constant Approximation Method for Compressing Data Streams
    // Source path: src/piecewise-approximation/constant/hybrid-pmc
    void compress(TimeSeries& timeseries, int w_size, int n_window, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};