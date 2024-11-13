#include "dependencies.h"
#include "monitor.h"
#include "function.h"
#include "timeseries.h"


class SwingFilter {
    // Source paper: Online Piece-wise Linear Approximation of Numerical Streams with Precision Guarantees
    // Source path: src/piecewise-approximation/linear/swing-filter.cpp
    private:

    public:
        static void compress(TimeSeries& timeseries, float bound, std::string output);
        static void decompress(std::string input, std::string output, int interval);
};

class SlideFilter {
    // Source paper: Online Piece-wise Linear Approximation of Numerical Streams with Precision Guarantees
    // Source path: src/piecewise-approximation/linear/slide-filter.cpp
    private:

    public:
        static void compress(TimeSeries& timeseries, float bound, std::string output);
        static void decompress(std::string input, std::string output, int interval);
};