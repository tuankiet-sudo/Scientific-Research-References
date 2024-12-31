#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"
#include "piecewise-approximation/model-seletion.h"

using namespace std;

long Monitor::page_size;
bool Monitor::flag = false;


int main(int argc, char** argv) {
    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string OUT_MONITOR = argv[3];
    const int INTERVAL = atoi(argv[4]);
    const string ALGO = argv[5];
    const float BOUND = atof(argv[6]);

    std::thread monitor(&Monitor::monitor, OUT_MONITOR);
    Monitor::start();   

    if (ALGO == "pmc") {
        PMC::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "hybrid-pca") {
        HybridPCA::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "swing-filter") {
        SwingFilter::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "slide-filter") {
        SlideFilter::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "optimal-pla") {
        OptimalPLA::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "normal-equation") {
        NormalEquation::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "mix-piece") {
        MixPiece::decompress(INPUT, OUTPUT, INTERVAL);
    }

    Monitor::stop();
    monitor.join();

    return 0;
}