#include "dependencies.h"
#include "matrix.h"
#include "function.h"
#include "timeseries.h"

// Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
// Journal: IEEE Access
// Source path: src/polynomial/non_linear/fne.cpp
void FastNormalEquation(HPCLab::TimeSeries<int>& series, int degree, float bound, std::string output);