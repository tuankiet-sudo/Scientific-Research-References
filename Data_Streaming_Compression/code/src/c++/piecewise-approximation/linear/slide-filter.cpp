#include "piecewise-approximation/linear.h"

namespace SlideFilter {

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            std::cout << data->get_time() << ": " << data->get_value() << std::endl;
        }
    }

    void decompress(std::string input, std::string output, int interval) {

    }
    
};

