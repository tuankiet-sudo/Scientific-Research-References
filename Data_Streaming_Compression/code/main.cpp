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
    
}