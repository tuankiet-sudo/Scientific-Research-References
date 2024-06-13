#include "dependencies.h"
#include "polynomial/non_linear.h"


void FastNormalEquation(HPCLab::TimeSeries<int>& series, int degree, float bound) {
    
    std::vector<int> segment;
    HPCLab::Polynomial* model = NULL;
    
    while (!series.empty()) {
        HPCLab::Data<int>* data = series.pop();
        segment.push_back(data->get_data()[0]);
        delete data;

        if (segment.size() == degree+1) {

        }
        else if (segment.size() > degree+1) {

        }
    }
}

bool calError(const HPCLab::Polynomial& function, float bound, std::vector<int> data) {
    float error = 0;
    for (int i=0; i<data.size(); i++) {
        error += pow((function.substitute(i) - data[i]), 2);
    }

    // check whether error > n*e^2
    if (error > data.size()*bound*bound) return true;
    else return false;
}

void calPolynomial() {

}