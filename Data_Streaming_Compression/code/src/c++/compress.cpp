#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"
#include "piecewise-approximation/model-seletion.h"

using namespace std;

int Monitor::counter;
double Monitor::latency;
bool Monitor::flag = false;
high_resolution_clock::time_point Monitor::clock;


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
    if (argc < 6) { 
        throw std::invalid_argument("Missing required parameters.");
    }

    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string OUT_MONITOR = argv[3];
    const string ALGO = argv[4];
    const float ERROR = atof(argv[5]);

    TimeSeries timeseries = loadTimeseries(INPUT);
    std::thread monitor(&Monitor::monitor, OUT_MONITOR);
    Monitor::start();

    if (ALGO == "pmc") {
        if (argc < 7) {
            throw std::invalid_argument("PMC ALgo: Missing MODE parameter.");
        }
        
        PMC::compress(timeseries, argv[6], ERROR, OUTPUT);
    }
    else if (ALGO == "hybrid-pmc") {
        if (argc < 8) {
            throw std::invalid_argument("PMC ALgo: Missing W_SIZE or/and M_WINDOW parameters.");
        }

        HybridPMC::compress(timeseries, atoi(argv[6]), atoi(argv[7]), ERROR, OUTPUT);
    }
    else if (ALGO == "swing") {
        SwingFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "slide") {
        SlideFilter::compress(timeseries, ERROR, OUTPUT);
    }
    else if (ALGO == "normal-equation") {
        if (argc < 7) {
            throw std::invalid_argument("Normal-Equation ALgo: Missing DEGREE parameter.");
        }

        NormalEquation::compress(timeseries, atoi(argv[6]), ERROR, OUTPUT);
    }

    timeseries.finalize();
    Monitor::stop();
    monitor.join();
    std::cout << "Time taken for each data points: " << Monitor::getLatency() << " nanoseconds \n";

    return 0;
}