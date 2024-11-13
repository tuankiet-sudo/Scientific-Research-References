#include "piecewise-approximation/constant.h"
#include "piecewise-approximation/linear.h"
#include "piecewise-approximation/polynomial.h"
#include "piecewise-approximation/model-seletion.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 5) { 
        throw std::invalid_argument("Missing required parameters.");
    }

    const string INPUT = argv[1];
    const string OUTPUT = argv[2];
    const string ALGO = argv[3];
    const int INTERVAL = atoi(argv[4]);

    if (ALGO == "pmc") {
        PMC::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "hybrid-pmc") {
        HybridPMC::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "swing") {
        SwingFilter::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "slide") {
        SlideFilter::decompress(INPUT, OUTPUT, INTERVAL);
    }
    else if (ALGO == "normal-equation") {
        NormalEquation::decompress(INPUT, OUTPUT, INTERVAL);
    }

    return 0;
}