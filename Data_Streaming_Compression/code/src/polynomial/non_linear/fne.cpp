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

std::map<int, HPCLab::Matrix<float>*> dict;
HPCLab::Matrix<float>* cache_X_T_X = NULL;
HPCLab::Matrix<float>* cache_X_T_y = NULL;

float calError(const HPCLab::Polynomial* function, const std::vector<double>& data) {
    float error = 0;
    for (int i=0; i<data.size(); i++) {
        error += pow((function->substitute<int>(i) - data[i]), 2);
    }

    return error/data.size();
}

HPCLab::Polynomial* _periodic(const std::vector<double>& data, int degree) {
    HPCLab::Matrix<float>* theta = NULL;
    if (dict.find(data.size()) == dict.end()) {
        HPCLab::Matrix<float> *X = new HPCLab::Matrix<float>(data.size(), degree+1);
        HPCLab::Matrix<float> *y = new HPCLab::Matrix<float>(data.size(), 1);
        
        for (int i=0; i<data.size(); i++) {
            for (int k=0; k<degree+1; k++) {
                X->cell[i][k] = pow(i, k);
            } 
            y->cell[i][0] = data[i];
        }

        HPCLab::Matrix<float>* X_T = X->transpose();
        HPCLab::Matrix<float>* X_T_X = HPCLab::matrix_outter_product(X_T, X);
        HPCLab::Matrix<float>* X_T_X_inv = X_T_X->inverse();
        HPCLab::Matrix<float>* X_T_X_inv_X_T = HPCLab::matrix_outter_product(X_T_X_inv, X_T);
        theta = HPCLab::matrix_outter_product(X_T_X_inv_X_T, y);
        dict.insert({data.size(), X_T_X_inv_X_T});

        delete X; delete y; delete X_T; 
        delete X_T_X; delete X_T_X_inv;
    }
    else {
        HPCLab::Matrix<float> *y = new HPCLab::Matrix<float>(data.size(), 1);
        for (int i=0; i<data.size(); i++) {
            y->cell[i][0] = data[i];
        }
        HPCLab::Matrix<float>* X_T_X_inv_X_T = dict.find(data.size())->second;
        theta = HPCLab::matrix_outter_product(X_T_X_inv_X_T, y);
    }
    

    float* coeffs = theta->toVec();
    HPCLab::Polynomial* model = new HPCLab::Polynomial(degree, coeffs);

    delete coeffs; delete theta;

    return model;
}

HPCLab::Polynomial* _aperiodic(const std::vector<double>& data, int degree) {
    if (data.size() <= degree+1) {
        delete cache_X_T_X; delete cache_X_T_y;
        cache_X_T_X = NULL; cache_X_T_y = NULL;
    }
    if (cache_X_T_X == NULL || cache_X_T_y == NULL) {
        HPCLab::Matrix<float> *X = new HPCLab::Matrix<float>(data.size(), degree+1);
        HPCLab::Matrix<float> *y = new HPCLab::Matrix<float>(data.size(), 1);
        
        for (int i=0; i<data.size(); i++) {
            for (int k=0; k<degree+1; k++) {
                X->cell[i][k] = pow(i, k);
            } 
            y->cell[i][0] = data[i];
        }

        HPCLab::Matrix<float>* X_T = X->transpose();
        cache_X_T_X = HPCLab::matrix_outter_product(X_T, X);
        cache_X_T_y = HPCLab::matrix_outter_product(X_T, y);

        delete X; delete y; delete X_T;
    }
    else {
        int t = data.size()-1;
        for (int i=0; i<degree+1; i++) {
            for (int j=0; j<degree+1; j++) {
                cache_X_T_X->cell[i][j] += pow(t, i) * pow(t, j);
            }
            cache_X_T_y->cell[i][0] += pow(t, i)*data.back();
        }
    }

    HPCLab::Matrix<float>* X_T_X_inv = cache_X_T_X->inverse();
    HPCLab::Matrix<float>* theta = HPCLab::matrix_outter_product(X_T_X_inv, cache_X_T_y);

    float* coeffs = theta->toVec();
    HPCLab::Polynomial* model = new HPCLab::Polynomial(degree, coeffs);

    delete coeffs; delete theta; delete X_T_X_inv;

    return model;
}

HPCLab::Polynomial* _normal(const std::vector<double>& data, int degree) {
    HPCLab::Matrix<float> *X = new HPCLab::Matrix<float>(data.size(), degree+1);
    HPCLab::Matrix<float> *y = new HPCLab::Matrix<float>(data.size(), 1);
    
    for (int i=0; i<data.size(); i++) {
        for (int k=0; k<degree+1; k++) {
            X->cell[i][k] = pow(i, k);
        } 
        y->cell[i][0] = data[i];
    }

    HPCLab::Matrix<float>* X_T = X->transpose();
    HPCLab::Matrix<float>* X_T_X = HPCLab::matrix_outter_product(X_T, X);
    HPCLab::Matrix<float>* X_T_X_inv = X_T_X->inverse();
    HPCLab::Matrix<float>* X_T_X_inv_X_T = HPCLab::matrix_outter_product(X_T_X_inv, X_T);
    HPCLab::Matrix<float>* theta = HPCLab::matrix_outter_product(X_T_X_inv_X_T, y);

    float* coeffs = theta->toVec();
    HPCLab::Polynomial* model = new HPCLab::Polynomial(degree, coeffs);

    delete X; delete y; delete X_T, delete X_T_X; delete coeffs; 
    delete X_T_X_inv; delete X_T_X_inv_X_T; delete theta;

    return model;
}

HPCLab::Polynomial* calPolynomial(const std::vector<double>& data, int degree, std::string mode) {
    if (mode == "normal") return _normal(data, degree);
    else if (mode == "periodic") return _periodic(data, degree);
    else if (mode == "aperiodic") return _aperiodic(data, degree);
    else return NULL;
}

void finalize(std::vector<FNE>& result, std::string path) {
    std::fstream file;
    file.open(path, std::ofstream::out | std::ofstream::trunc);

    for (FNE& ele : result) {
        for (int i=0; i<ele.segment_length; i++) {
            file << i << "," << ele.model->substitute<int>(i) << std::endl;
        }
        delete ele.model;
    }
    
    file.close();

    for (auto it=dict.begin(); it!=dict.end(); it++) {
        delete it->second;
    }
    delete cache_X_T_X; delete cache_X_T_y;
}


void normal_equation(HPCLab::TimeSeries& series, int degree, float bound, std::string mode, std::string output) {
    std::vector<double> segment;
    HPCLab::Polynomial* model = NULL;
    std::vector<FNE> result;

    for (auto& data : series.get()) {
        segment.push_back(data->get_data());

        if (segment.size() == degree+1) {
            model = calPolynomial(segment, degree, mode);
        }
        else if (segment.size() > degree+1) {
            if (calError(model, segment) > bound) {
                HPCLab::Polynomial* temp_model = calPolynomial(segment, degree, mode);
                if (calError(temp_model, segment) > bound) {
                    result.push_back(FNE(segment.size()-1, model));
                    model = NULL;
                    segment = {data->get_data()};
                }
                else {
                    HPCLab::Polynomial* temp = model;
                    model = temp_model;
                    delete temp;
                }
            }
        }
    }

    if (segment.size() > degree) {
        if (model == NULL) {
            model = calPolynomial(segment, degree, mode);
        }
        result.push_back(FNE(segment.size(), model));
    }

    finalize(result, output);

}

