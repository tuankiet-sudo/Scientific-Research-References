#include "dependencies.h"
#include "matrix.h"
#include "function.h"
#include "timeseries.h"
#include "io.h"


class NormalEquation {
    // Source paper: Fast Piecewise Polynomial Fitting of Time-Series Data for Streaming Computing
    // Source path: src/piecewise-approximation/polynomial/normal-equation.cpp
    private:
        static std::map<int, Matrix<float>*> cache;

        static void _yield(BinObj* obj, int length, Polynomial* model);
        static Polynomial* _calPolynomial(std::vector<Point2D>& window, int degree);
        static float _calError(std::vector<Point2D>& window, Polynomial* model);
        static void _approximate(IterIO& file, int interval, time_t basetime, int prev_point, int length, Polynomial* model);

    public:
        static void compress(TimeSeries& timeseries, int degree, float bound, std::string output);
        static void decompress(std::string input, std::string output, int interval);
};