#include <cmath>
#include <iostream>

#include "system/io.hpp"
#include "system/monitor.hpp"
#include "algebraic/function.hpp"
#include "algebraic/convex.hpp"
#include "algebraic/sdlp.hpp"
#include "timeseries.hpp"


namespace SmartGridCompression {
    // Source paper: A time-series compression technique and its application to the smart grid
    // Source path: src/model-selection/smart-grid-compression.cpp
    void compress(TimeSeries& timeseries, int degree, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};