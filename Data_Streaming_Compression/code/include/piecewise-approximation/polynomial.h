#include "dependencies.h"
#include "matrix.h"
#include "function.h"
#include "timeseries.h"


class NormalEquation {
    // Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
    // Source path: src/piecewise-approximation/polynomial/pne.cpp
    private:

    public:
        static void compress(TimeSeries& timeseries, int mode, int degree, float bound, std::string output);
        static void decompress(std::string input, std::string output);
};