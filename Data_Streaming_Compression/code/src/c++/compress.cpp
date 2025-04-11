#include "piecewise-approximation/constant.hpp"
#include "piecewise-approximation/linear.hpp"
#include "piecewise-approximation/polynomial.hpp"
#include "model-selection/polynomial.hpp"

using namespace std;

Monitor Monitor::instance;

TimeSeries loadTimeseries(string input) {
    TimeSeries timeseries;
    CSVObj* obj = BatchIO::readCSV(input);
    CSVObj* d_obj = nullptr;

    while (obj != nullptr) {
        time_t time = (time_t) stol(obj->getData(0));
        float value = stof(obj->getData(1));        
        timeseries.push(new Univariate(time, value));

        d_obj = obj;
        obj = (CSVObj*) obj->getNext();
        delete d_obj;
    }

    return timeseries;
}


int main(int argc, char** argv) {
    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string ALGO = argv[3];
    const float ERROR = atof(argv[4]);

    TimeSeries timeseries = loadTimeseries(INPUT);
    Monitor::instance.start(OUTPUT+".mon");

    if (ALGO == "pmc") {
        PMC::compress(timeseries, argv[5], ERROR, OUTPUT);
    }
    else if (ALGO == "hybrid-pca") {
        HybridPCA::compress(timeseries, atoi(argv[5]), atoi(argv[6]), ERROR, OUTPUT);
    }
    else if (ALGO == "swing-filter") {
        SwingFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "slide-filter") {
        SlideFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "optimal-pla") {
        OptimalPLA::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "normal-equation") {
        NormalEquation::compress(timeseries, argv[5], atoi(argv[6]), ERROR, OUTPUT);
    }
    else if (ALGO == "mix-piece") {
        MixPiece::compress(timeseries, atoi(argv[5]), ERROR, OUTPUT);
    }
    else if (ALGO == "smart-grid-compression") {
        SmartGridCompression::compress(timeseries, atoi(argv[5]), ERROR, OUTPUT);
    }
    else if (ALGO == "unbounded") {
        Unbounded::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "bounded") {
        Bounded::compress(timeseries, atoi(argv[5]), ERROR, OUTPUT);
    }

    timeseries.finalize();
    Monitor::instance.stop();

    return 0;
}