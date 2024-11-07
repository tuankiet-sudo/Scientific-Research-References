#include "piecewise-approximation/linear.h"

void SwingFilter::compress(TimeSeries& timeseries, float bound, std::string output) {
    while (timeseries.hasNext()) {
        Univariate<float>* data = (Univariate<float>*) timeseries.next();
        std::cout << data->get_time() << ": " << data->get_value() << std::endl;
    }
}

void SwingFilter::decompress(std::string input, std::string output) {

}