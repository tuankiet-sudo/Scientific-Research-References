#include "piecewise-approximation/polynomial.h"

std::map<int, Matrix<float>*> NormalEquation::cache;

void NormalEquation::_yield(BinObj* obj, int length, Polynomial* model) {
    obj->put(length);
    for (int i = 0; i <= model->degree; i++) {
        obj->put(model->coefficients[i]);
    }
}

Polynomial* NormalEquation::_calPolynomial(std::vector<Point2D>& window, int degree) {
        Matrix<float>* theta = nullptr;
        if (NormalEquation::cache.find(window.size()) == NormalEquation::cache.end()) {
            Matrix<float> *X = new Matrix<float>(window.size(), degree+1);
            Matrix<float> *y = new Matrix<float>(window.size(), 1);

            for (int i = 0; i < window.size(); i++) {
                for (int k=0; k<degree+1; k++) {
                    X->cell[i][k] = pow(i, k);
                } 
                y->cell[i][0] = window[i].y;
            }

            Matrix<float>* X_T = X->transpose();
            Matrix<float>* X_T_X = Matrix<float>::matrix_outter_product(X_T, X);
            Matrix<float>* X_T_X_inv = X_T_X->inverse();
            Matrix<float>* X_T_X_inv_X_T = Matrix<float>::matrix_outter_product(X_T_X_inv, X_T);
            theta = Matrix<float>::matrix_outter_product(X_T_X_inv_X_T, y);
            NormalEquation::cache.insert({window.size(), X_T_X_inv_X_T});

            float* coeffs = theta->toVec();
            Polynomial* model = new Polynomial(degree, coeffs);

            delete X; delete y; delete X_T, delete X_T_X; 
            delete coeffs; delete X_T_X_inv;
        }
        else {
            Matrix<float> *y = new Matrix<float>(window.size(), 1);
            for (int i=0; i<window.size(); i++) {
                y->cell[i][0] = window[i].y;
            }
            Matrix<float>* X_T_X_inv_X_T = NormalEquation::cache.find(window.size())->second;
            theta = Matrix<float>::matrix_outter_product(X_T_X_inv_X_T, y);
        }
        
        float* coeffs = theta->toVec();
        Polynomial* model = new Polynomial(degree, coeffs);
        delete coeffs; delete theta;

        return model;
}

float NormalEquation::_calError(std::vector<Point2D>& window, Polynomial* model) {
    float max = -INFINITY;
    for (int i = 0; i < window.size(); i++) {
        float error = abs(model->substitute<int>(i) - window[i].y);
        max = max < error ? error : max;
    }

    return max;
}

void NormalEquation::_approximate(IterIO& file, int interval, time_t basetime, int prev_point, int length, Polynomial* model) {
    for (int i = 0; i < length; i++) {
        CSVObj obj;
        obj.pushData(std::to_string(basetime + prev_point));
        obj.pushData(std::to_string(model->substitute<int>(i)));

        file.writeStr(&obj);
        prev_point += interval;
    }
}

void NormalEquation::compress(TimeSeries& timeseries, int degree, float bound, std::string output) {
    IterIO outputFile(output, false);
    BinObj* obj = new BinObj;
    obj->put(degree);

    time_t time = -1;
    std::vector<Point2D> window;
    Polynomial* model = nullptr;

    Monitor::clockReset();
    while (timeseries.hasNext()) {
        Univariate<float>* data = (Univariate<float>*) timeseries.next();
        Monitor::startClock();

        if (time == -1) {
            time = data->get_time();
            obj->put(time);
        }

        window.push_back(Point2D(data->get_time() - time, data->get_value()));
        if (window.size() == degree + 1) {
            model = NormalEquation::_calPolynomial(window, degree);
        }
        if (window.size() > degree + 1) {
            if (NormalEquation::_calError(window, model) > bound) {
                Polynomial* n_model = NormalEquation::_calPolynomial(window, degree);
                if (NormalEquation::_calError(window, model) > bound) {
                    NormalEquation::_yield(obj, window.size()-1, model);
                    delete model;
                    delete n_model;
                    model = nullptr;
                    window = {Point2D(data->get_time() - time, data->get_value())};
                }
                else {
                    delete model;
                    model = n_model;
                }
            }
        }

        Monitor::endClock();
    }
    if (model != nullptr) {
        NormalEquation::_yield(obj, window.size(), model);
        delete model;
    }

    for (auto& entry : NormalEquation::cache) delete entry.second;
    outputFile.writeBin(obj);
    outputFile.close();
    delete obj;
}


void NormalEquation::decompress(std::string input, std::string output, int interval) {
    IterIO inputFile(input, true, true);
    IterIO outputFile(output, false);
    BinObj* r_obj = inputFile.readBin();

    int prev_point = 0;
    int degree = r_obj->getInt();
    time_t time = r_obj->getLong();

    while (r_obj->getSize() != 0) {
        int length = r_obj->getInt();
        float* coefficients = new float[degree+1];

        for (int i = 0; i <= degree; i++) {
            coefficients[i] = r_obj->getFloat();
        }
        Polynomial* model = new Polynomial(degree, coefficients);
        NormalEquation::_approximate(outputFile, interval, time, prev_point, length, model);
        prev_point += length * interval;
        
        delete model;
        delete[] coefficients;
    }

    delete r_obj;
    inputFile.close();
    outputFile.close();
}
