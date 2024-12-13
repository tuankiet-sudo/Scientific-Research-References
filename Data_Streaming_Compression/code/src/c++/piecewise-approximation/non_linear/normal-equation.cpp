#include "piecewise-approximation/polynomial.h"

namespace NormalEquation {

    Clock clock;
    std::map<int, Matrix<float>*> cache;

    void _yield(BinObj* obj, int length, Polynomial* model) {
        obj->put(length);
        for (int i = 0; i <= model->degree; i++) {
            obj->put(model->coefficients[i]);
        }
    }

    struct Model {
        Polynomial* function;
        float error;

        Model(Polynomial* function) {
            this->function = function;
            this->error = -1;
        }

        ~Model() {
            delete this->function;
            this->error = -1;
        }
    };

    Polynomial* _calPolynomial(std::vector<Point2D>& window, int degree) {
            Matrix<float>* theta = nullptr;
            if (cache.find(window.size()) == cache.end()) {
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

                cache.insert({window.size(), X_T_X_inv_X_T});

                delete X; delete y; delete X_T; 
                delete X_T_X; delete X_T_X_inv;
            }
            else {
                Matrix<float> *y = new Matrix<float>(window.size(), 1);
                for (int i=0; i<window.size(); i++) {
                    y->cell[i][0] = window[i].y;
                }
                Matrix<float>* X_T_X_inv_X_T = cache.find(window.size())->second;
                theta = Matrix<float>::matrix_outter_product(X_T_X_inv_X_T, y);
            }
            
            float* coeffs = theta->toVec();
            Polynomial* model = new Polynomial(degree, coeffs);
            delete coeffs; delete theta;
            return model;
    }

    bool _approxSuccess(std::vector<Point2D>& window, Model* model, std::string mode, float bound) {
        if (mode == "individual") {
            if (model->error == -1) {
                model->error = -INFINITY;
                for (int i = 0; i < window.size(); i++) {
                    float error = std::abs(model->function->substitute<int>(i) - window[i].y);
                    model->error = model->error < error ? error : model->error;
                }
            }
            else {
                float error = std::abs(model->function->substitute<int>(window.size()-1) - window[window.size()-1].y);
                model->error = model->error < error ? error : model->error;
            }
            
            return model->error < bound;
        }
        else if (mode == "residual") {
            if (model->error == -1) {
                model->error = 0;
                for (int i = 0; i < window.size(); i++) {
                    model->error += std::abs(model->function->substitute<int>(i) - window[i].y);
                }
            }
            else {
                model->error += std::abs(model->function->substitute<int>(window.size()-1) - window[window.size()-1].y);
            }
            
            return model->error < bound * window.size();
        }

        return false;
    }

    void _approximate(IterIO& file, int interval, time_t basetime, int prev_point, int length, Polynomial* model) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + prev_point));
            obj.pushData(std::to_string(model->substitute<int>(i)));

            file.writeStr(&obj);
            prev_point += interval;
        }
    }

    void compress(TimeSeries& timeseries, std::string mode, int degree, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* obj = new BinObj;
        obj->put(degree);

        time_t time = -1;
        std::vector<Point2D> window;
        Model* model = nullptr;
        Model* n_model = nullptr;

        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (time == -1) {
                time = data->get_time();
                obj->put(time);
            }

            window.push_back(Point2D(data->get_time() - time, data->get_value()));
            if (window.size() == degree + 1) {
                model = new Model(_calPolynomial(window, degree));
            }
            if (window.size() > degree + 1) {
                if (!_approxSuccess(window, model, mode, bound)) {
                    n_model = new Model(_calPolynomial(window, degree));
                    if (!_approxSuccess(window, n_model, mode, bound)) {
                        _yield(obj, window.size()-1, model->function);
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

            clock.stop();
        }
        if (model != nullptr) {
            _yield(obj, window.size(), model->function);
            delete model;
        }

        for (auto& entry : cache) delete entry.second;
        outputFile.writeBin(obj);
        outputFile.close();
        delete obj;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }


    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* r_obj = inputFile.readBin();

        int prev_point = 0;
        int degree = r_obj->getInt();
        time_t time = r_obj->getLong();

        while (r_obj->getSize() != 0) {
            clock.start();
            int length = r_obj->getInt();
            float* coefficients = new float[degree+1];

            for (int i = 0; i <= degree; i++) {
                coefficients[i] = r_obj->getFloat();
            }
            Polynomial* model = new Polynomial(degree, coefficients);
            _approximate(outputFile, interval, time, prev_point, length, model);
            prev_point += length * interval;
            
            delete model;
            delete[] coefficients;
            clock.stop();
        }

        delete r_obj;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }

};