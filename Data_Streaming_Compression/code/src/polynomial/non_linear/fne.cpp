#include "dependencies.h"
#include "polynomial/non_linear.h"


struct FNE {
    int segment_length;
    HPCLab::Polynomial* model;

    FNE(int segment_length, HPCLab::Polynomial* model) {
        this->segment_length = segment_length;
        this->model = model;
    }
};

float calError(const HPCLab::Polynomial* function, const std::vector<int>& data) {
    float error = 0;
    for (int i=0; i<data.size(); i++) {
        error += pow((function->substitute<int>(i) - data[i]), 2);
    }

    return error/data.size();
}

HPCLab::Polynomial* calPolynomial(const std::vector<int>& data) {
    return NULL;
}

void finalize(std::vector<FNE>& result, std::string path) {
    std::ofstream file;
    file.open(path, std::ofstream::out | std::ofstream::trunc);

    for (FNE& ele : result) {
        for (int i=0; i<ele.segment_length; i++) {
            file << i << "," << ele.model->substitute<int>(i) << std::endl;
        }
        delete ele.model;
    }
    
    file.close();
}


void FastNormalEquation(HPCLab::TimeSeries<int>& series, int degree, float bound, std::string output) {
    
    std::vector<int> segment;
    HPCLab::Polynomial* model = NULL;
    std::vector<FNE> result;

    for (auto& data : series.get()) {
        segment.push_back(data->get_data()[0]);

        if (segment.size() == degree+1) {
            model = calPolynomial(segment);
        }
        else if (segment.size() > degree+1) {
            if (calError(model, segment) > bound) {
                HPCLab::Polynomial* temp_model = calPolynomial(segment);
                if (calError(temp_model, segment) > bound) {
                    result.push_back(FNE(segment.size()-1, model));
                    model = NULL;
                    segment = {data->get_data()[0]};
                }
                else {
                    HPCLab::Polynomial* temp = model;
                    model = temp_model;
                    delete temp;
                }
            }
        }
    }

    finalize(result, output);
}

