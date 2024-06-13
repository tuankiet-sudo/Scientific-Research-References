#include "polynomial/non_linear.h"

using namespace std;

void print(HPCLab::Matrix<int>* matrix) {
    for (int i=0; i<matrix->get_height(); i++) {
        for (int j=0; j<matrix->get_width(); j++) {
            cout << matrix->cell[i][j] << " ";
        }
        cout << endl;
    }
}

void print(HPCLab::Matrix<float>* matrix) {
    for (int i=0; i<matrix->get_height(); i++) {
        for (int j=0; j<matrix->get_width(); j++) {
            cout << matrix->cell[i][j] << " ";
        }
        cout << endl;
    }
}

int main() {
    HPCLab::TimeSeries<int> timeseries(1);

    timeseries.push(0, 1);
    timeseries.push(1, 20);
    timeseries.push(2, 30);
    timeseries.push(3, 40);
    timeseries.push(4, 50);
    timeseries.push(5, 60);

    FastNormalEquation(timeseries, 3, 4.5);
    
}