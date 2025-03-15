#include <iomanip>
#include "eigen/Dense"
#include "algebraic/matrix.hpp"
#include "model-selection/polynomial.hpp"

namespace Bounded {

    Clock clock;

    void __yield(BinObj* obj, Polynomial* model, int degree, short length) {
        unsigned char d = degree;
        obj->put(d);
        obj->put(length);
        for (int i = 0; i <= degree; i++) {
            obj->put(model->coefficients[i]);
        }
    }

    // Polynomial* fit(float bound, std::vector<Point2D>& segment, int degree)  {
    //     Eigen::VectorXd x(degree + 2);                    // decision variables
    //     Eigen::VectorXd c(degree + 2);                    // objective coefficients
    //     Eigen::MatrixXd A(2*(segment.size()), degree + 2);  // constraint matrix
    //     Eigen::VectorXd b(2*(segment.size()));                    // constraint bound

    //     for (int i=0; i<degree+2; i++) {
    //         if (i != degree + 1) c(i) = 0.0;
    //         else c(i) = 1.0;
    //     }
        
    //     for (int i=0; i<segment.size(); i++) {                                                              
    //         for (int j=degree; j>=0; j--) {
    //             A(2*i, degree-j) = -pow(segment[i].x, j);
    //             A(2*i+1, degree-j) = pow(segment[i].x, j);
    //         }
    //         A(2*i, degree+1) = -1.0;
    //         A(2*i+1, degree+1) = -1.0;

    //         b(2*i) = -segment[i].y;
    //         b(2*i+1) = segment[i].y;
    //     }

    //     double minobj = sdlp::linprog(c, A, b, x); 
    //     std::cout << "minobj: " << minobj << "\n"; 
    //     std::cout << "model: " << x.transpose() << "\n";      

    //     float* coefficients = new float[degree+1];
    //     for (int i = 0; i <= degree; i++) {
    //         coefficients[degree-i] = x(i);
    //     }

    //     std::cout << "-----\n";
    //     for (int i=0; i<c.rows(); i++) std::cout << std::fixed << std::setprecision(4) << c(i) << "\n";
    //     std::cout << "-----\n";
    //     for (int i=0; i<b.rows(); i++) std::cout << std::fixed << std::setprecision(4) << b(i) << "\n";
    //     std::cout << "-----\n";
    //     for (int i=0; i<A.rows(); i++) {
    //         for (int j=0; j<A.cols(); j++) {
    //             std::cout << std::fixed << std::setprecision(4) << A(i, j) << " ";
    //         }
    //         std::cout << "\n";
    //     }
    //     std::cout << "-----\n";    

    //     Polynomial* model = new Polynomial(degree, coefficients);
    //     delete coefficients;

    //     return model;
    // }

    Polynomial* fit(float bound, std::vector<Point2D>& segment, int degree)  {                
        // int n = segment.size();
        // Eigen::MatrixXd A(n, degree + 1);
        // Eigen::VectorXd b(n);

        // // Construct the Vandermonde matrix A and vector b
        // for (int i = 0; i < n; ++i) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
        //     b(i) = segment[i].y;
        //     for (int j = 0; j <= degree; ++j) {
        //         A(i, j) = pow(i, j);
        //     }
        // }

        // // Solve the normal equations using Eigen
        // Eigen::MatrixXd E = A.transpose() * A;
        // Eigen::MatrixXd T = E.inverse();
        // Eigen::VectorXd coefficients = (A.transpose() * A).ldlt().solve(A.transpose() * b);
        // // return model;
        // Polynomial* model = new Polynomial(degree, (float*) coefficients.data());

        Matrix<double> *X = new Matrix<double>(segment.size(), degree+1);
        Matrix<double> *y = new Matrix<double>(segment.size(), 1);

        for (int i = 0; i < segment.size(); i++) {
            for (int k=0; k<degree+1; k++) {
                X->cell[i][k] = pow(i, k);
            } 
            y->cell[i][0] = segment[i].y;
        }

        Matrix<double>* X_T = X->transpose();
        Matrix<double>* X_T_X = Matrix<double>::matrix_outter_product(X_T, X);
        Matrix<double>* X_T_X_inv = X_T_X->inverse();
        Matrix<double>* X_T_X_inv_X_T = Matrix<double>::matrix_outter_product(X_T_X_inv, X_T);
        Matrix<double>* theta = Matrix<double>::matrix_outter_product(X_T_X_inv_X_T, y);
        
        double* coeffs = theta->toVec();
        Polynomial* model = new Polynomial(degree, coeffs);

        delete coeffs; delete theta;
        delete X; delete y; delete X_T; 
        delete X_T_X; delete X_T_X_inv;
        return model;
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        std::vector<Point2D> segment;
        time_t basetime = -1;
        while(timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();
            Point2D p(segment.size(), data->get_value());
            segment.push_back(p);

            if (basetime == -1) {
                basetime = data->get_time();
                compress_data->put(basetime);
            }
        }

        int degree = 6;
        Polynomial* polynomial = fit(bound, segment, degree);
        __yield(compress_data, polynomial, degree, segment.size());

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;
        delete polynomial;
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, Polynomial& polynomial) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(polynomial.subs(i)));
            file.write(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        time_t basetime = compress_data->getLong();
        while (compress_data->getSize() != 0) {
            int degree = (int) compress_data->getByte();
            unsigned short length = compress_data->getShort();

            
            float* coefficients = new float[degree+1];
            for (int i = 0; i <= degree; i++) {
                coefficients[i] = compress_data->getFloat();
            }

            Polynomial polynomial(degree, coefficients);
            __decompress_segment(outputFile, interval, basetime, length, polynomial);
            delete[] coefficients;
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();
    }
    
};