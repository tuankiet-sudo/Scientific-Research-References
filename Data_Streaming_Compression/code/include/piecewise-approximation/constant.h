#include "io.h"
#include "dependencies.h"
#include "function.h"
#include "timeseries.h"


class PMC {
    // Source paper: Capturing Sensor-Generated Time Series with Quality Guarantees
    // Source path: src/piecewise-approximation/constant/pmc.cpp
    private:
        static void _yield(BinObj* obj, int offset, float value);
        static void _approximate(IterIO& file, int interval, time_t basetime, int prev_point, int end_point, float value);

    public:
        static void compress(TimeSeries& timeseries, std::string mode, float bound, std::string output);
        static void decompress(std::string input, std::string output, int interval);
};
