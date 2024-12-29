#include "system/io.h"
#include "system/monitor.h"
#include "dependencies.h"
#include "algebraic/function.h"
#include "algebraic/convex.h"
#include "timeseries.h"


namespace SwingFilter {
    // Source paper: Online Piece-wise Linear Approximation of Numerical Streams with Precision Guarantees
    // Source path: src/piecewise-approximation/linear/swing-filter.cpp
    void compress(TimeSeries& timeseries, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};

namespace SlideFilter {
    // Source paper: Online Piece-wise Linear Approximation of Numerical Streams with Precision Guarantees
    // Source path: src/piecewise-approximation/linear/slide-filter.cpp
    void compress(TimeSeries& timeseries, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};

namespace OptimalPLA {
    // Source paper: Maximum error-bounded Piecewise Linear Representation for online stream approximation
    // Source path: src/piecewise-approximation/linear/optimal-pla.cpp
    void compress(TimeSeries& timeseries, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};

namespace MixPiece {
    // Source paper: Flexible grouping of linear segments for highly accurate lossy compression of time series data
    // Source path: src/piecewise-approximation/linear/mix-piece.cpp
    void compress(TimeSeries& timeseries, int n_segment, float bound, std::string output);
    void decompress(std::string input, std::string output, int interval);
};