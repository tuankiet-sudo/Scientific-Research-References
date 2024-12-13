#include "system/io.h"
#include "system/monitor.h"
#include "dependencies.h"
#include "algebraic/function.h"
#include "algebraic/convex.h"
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

namespace OptimalPLA {
    void compress(TimeSeries& timeseries, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};