#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"
#include "piecewise-approximation/model-seletion.h"

using namespace std;


TimeSeries loadTimeseries(string input) {
    TimeSeries timeseries;

    fstream inputFile(input, ios_base::in);
    string line; float val;

    while(getline(inputFile, line)) {
        stringstream ss(line);
        vector<float> data;
        while (ss >> val) {
            data.push_back(val);
            if (ss.peek() == ',') ss.ignore();
        }

        timeseries.push(new Univariate<float>((time_t) data[0], data[1]));
    }

    inputFile.close();
    return timeseries;
}


int main(int argc, char** argv) {
    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string ALGO = argv[3];
    const float ERROR = atof(argv[4]);

    TimeSeries timeseries = loadTimeseries("data/input/synthesis/quadratic.csv");

    if (ALGO == "pmc") {
        const string MODE = argv[5];
    }
    else if (ALGO == "swing") {

    }
    else if (ALGO == "slide") {

    }

    timeseries.finalize();
    return 0;
}