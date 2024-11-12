#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"
#include "piecewise-approximation/model-seletion.h"

using namespace std;


TimeSeries loadTimeseries(string input) {
    TimeSeries timeseries;
    CSVObj* obj = BatchIO::readCSV(input);
    CSVObj* d_obj = nullptr;

    while (obj != nullptr) {
        time_t time = (time_t) stol(obj->getData(0));
        float value = stof(obj->getData(1));        
        timeseries.push(new Univariate<float>(time, value));

        d_obj = obj;
        obj = (CSVObj*) obj->getNext();
        delete d_obj;
    }

    return timeseries;
}


int main(int argc, char** argv) {
    if (argc < 5) { 
        throw std::invalid_argument("Missing required parameters.");
    }

    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string ALGO = argv[3];
    const float ERROR = atof(argv[4]);

    TimeSeries timeseries = loadTimeseries("data/input/synthesis/quadratic.csv");
    if (ALGO == "pmc") {
        if (argc < 6) {
            throw std::invalid_argument("PMC ALgo: Missing MODE parameter.");
        }

        PMC::compress(timeseries, argv[5], ERROR, OUTPUT);
    }
    else if (ALGO == "swing") {
        SwingFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "slide") {
        SlideFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "normal-equation") {
        if (argc < 6) {
            throw std::invalid_argument("Normal-Equation ALgo: Missing DEGREE parameter.");
        }

        NormalEquation::compress(timeseries, atoi(argv[5]), ERROR, OUTPUT);
    }

    timeseries.finalize();
    return 0;
}