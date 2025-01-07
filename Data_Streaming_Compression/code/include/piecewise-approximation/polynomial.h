#include "system/io.h"
#include "system/monitor.h"
#include "dependencies.h"
#include "algebraic/function.h"
#include "algebraic/matrix.h"
#include "algebraic/convex.h"
#include "timeseries.h"


namespace NormalEquation {
    // Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
    // Source path: src/piecewise-approximation/polynomial/normal-equation.cpp
    void compress(TimeSeries& timeseries, std::string mode, int degree, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};

namespace Algo {
    void compress(TimeSeries& timeseries, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};