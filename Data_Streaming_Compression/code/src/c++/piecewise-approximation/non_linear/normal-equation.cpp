#include "piecewise-approximation/polynomial.hpp"

namespace NormalEquation {

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

    Clock clock;
    std::map<int, Matrix<float>*> cache;

    void __yield(BinObj* obj, short length, Polynomial* model) {
        obj->put(length);
        for (int i = 0; i <= model->degree; i++) {
            obj->put(model->coefficients[i]);
        }
    }

    Polynomial* __calPolynomial(std::vector<Point2D>& window, int degree) {
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

    bool __approxSuccess(std::vector<Point2D>& window, Model* model, std::string mode, float bound) {
        if (mode == "individual") {
            if (model->error == -1) {
                model->error = -INFINITY;
                for (int i = 0; i < window.size(); i++) {
                    float error = std::abs(model->function->subs(i) - window[i].y);
                    model->error = model->error < error ? error : model->error;
                }
            }
            else {
                float error = std::abs(model->function->subs(window.size()-1) - window[window.size()-1].y);
                model->error = model->error < error ? error : model->error;
            }
            
            return model->error < bound;
        }
        else if (mode == "accumulate") {
            if (model->error == -1) {
                model->error = 0;
                for (int i = 0; i < window.size(); i++) {
                    model->error += std::abs(model->function->subs(i) - window[i].y);
                }
            }
            else {
                model->error += std::abs(model->function->subs(window.size()-1) - window[window.size()-1].y);
            }
            
            return model->error < bound * window.size();
        }

        return false;
    }

    void compress(TimeSeries& timeseries, std::string mode, int degree, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;
        
        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(degree);
        compress_data->put(d->get_time());
        std::vector<Point2D> window = {Point2D(0, d->get_value())};

        Model* model = nullptr;
        Model* n_model = nullptr;

        unsigned short length = 1;
        clock.start();
        while (timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();
            window.push_back(Point2D(window.size(), data->get_value()));

            if (window.size() == degree + 1) {
                model = new Model(__calPolynomial(window, degree));
            }
            if (length > 65000 || window.size() > degree + 1) {
                if (!__approxSuccess(window, model, mode, bound)) {
                    n_model = new Model(__calPolynomial(window, degree));
                    if (!__approxSuccess(window, n_model, mode, bound)) {
                        __yield(compress_data, length, model->function);
                        window = {Point2D(0, data->get_value())};
                        length = 0;

                        delete model;
                        delete n_model;
                    }
                    else {
                        delete model;
                        model = n_model;
                    }
                }
            }

            length++;
            clock.tick();
        }

        for (auto& entry : cache) delete entry.second;
        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;

        // Profile average latency
        std::cout << std::fixed << "Time taken for each data point (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, Polynomial& model) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(model.subs(i)));
            file.write(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        int degree = compress_data->getInt();
        time_t basetime = compress_data->getLong();
        clock.start();
        while (compress_data->getSize() != 0) {
            unsigned short length = compress_data->getShort();
            float* coefficients = new float[degree+1];

            for (int i = 0; i <= degree; i++) {
                coefficients[i] = compress_data->getFloat();
            }
            Polynomial model(degree, coefficients);
            __decompress_segment(outputFile, interval, basetime, length, model);
            
            basetime += length * interval;
            delete[] coefficients;
            clock.tick();
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();

        // Profile average latency
        std::cout << std::fixed << "Time taken for each segment (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each segment (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }

};