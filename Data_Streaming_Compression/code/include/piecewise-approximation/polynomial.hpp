#include <map>
#include <iostream>
#include <eigen/Eigen>

#include "system/io.hpp"
#include "system/monitor.hpp"
#include "algebraic/function.hpp"
#include "algebraic/matrix.hpp"
#include "timeseries.hpp"


namespace NormalEquation {
    // Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
    // Source path: src/piecewise-approximation/polynomial/normal-equation.cpp
    void compress(TimeSeries& timeseries, std::string mode, int degree, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};