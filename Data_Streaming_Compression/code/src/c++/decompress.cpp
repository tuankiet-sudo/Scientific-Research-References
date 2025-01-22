#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"

using namespace std;

Monitor Monitor::instance;

int main(int argc, char** argv) {
    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const int INTERVAL = atoi(argv[3]);
    const string ALGO = argv[4];

    Monitor::instance.start(OUTPUT+".mon"); 

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

    Monitor::instance.stop();

    return 0;
}