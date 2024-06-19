#include "dependencies.h"
#include "matrix.h"
#include "function.h"
#include "timeseries.h"

// Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
// Journal: IEEE Access
// Source path: src/polynomial/non_linear/fne.cpp
void normal_equation(int degree, float bound, std::string mode, std::string out_appro, std::string out_coeff);