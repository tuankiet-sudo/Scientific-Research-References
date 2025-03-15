#include "algebraic/matrix.hpp"
#include "model-selection/polynomial.hpp"

namespace Unbounded {

    Clock clock;

    // Linear approximate    
    class LinearModel {
        private:
            Line* u_line;
            Line* l_line;
            Point2D* p1;

            float A_num = 0;
            float A_den = 0;

        public:
            bool isComplete;
            short length;
            
            LinearModel() {
                this->length = 0;
                this->u_line = nullptr;
                this->l_line = nullptr;
                this->isComplete = false;
            }

            ~LinearModel() {
                if (this->p1 != nullptr) delete this->p1;
                if (this->l_line != nullptr) delete this->u_line;
                if (this->u_line != nullptr) delete this->l_line;
            }

            void translation(Point2D& p) {
                Line* n_l_line = new Line(this->l_line->get_slope(), this->l_line->subs(p.x));
                Line* n_u_line = new Line(this->u_line->get_slope(), this->u_line->subs(p.x));

                if (this->l_line != nullptr) delete this->l_line;
                if (this->u_line != nullptr) delete this->u_line;

                this->l_line = n_l_line; 
                this->u_line = n_u_line;
                this->p1->x = 0; 
            }

            Line* getLine() {
                float A_ig = this->A_num / this->A_den;
                float temp = A_ig > this->u_line->get_slope() ? this->u_line->get_slope() : A_ig;
                double a_ig = temp > this->l_line->get_slope() ? temp : this->l_line->get_slope();
                double b_ig = this->p1->y - a_ig * this->p1->x;

                return new Line(a_ig, b_ig);
            }

            void fit(float bound, Point2D& p)  {
                if (this->length == 0) {
                    this->p1 = new Point2D(p.x, p.y);
                }
                else if (this->length == 1) {
                    Line l = Line::line(*p1, Point2D(p.x, p.y-bound));
                    Line u = Line::line(*p1, Point2D(p.x, p.y+bound));

                    this->u_line = new Line(u.get_slope(), u.get_intercept());
                    this->l_line = new Line(l.get_slope(), l.get_intercept());
                }
                else {
                    if (this->length > 32500 || this->l_line->subs(p.x) > p.y + bound || p.y - bound > this->u_line->subs(p.x)) {
                        this->isComplete = true;
                        return;
                    }
                    else {
                        if (p.y + bound < this->u_line->subs(p.x)) {
                            Line l = Line::line(*p1, Point2D(p.x, p.y+bound));

                            delete this->u_line;
                            this->u_line = new Line(l.get_slope(), l.get_intercept());
                        }
                        
                        if (p.y - bound > this->l_line->subs(p.x)) {
                            Line l = Line::line(*p1, Point2D(p.x, p.y-bound));

                            delete this->l_line;
                            this->l_line = new Line(l.get_slope(), l.get_intercept());
                        }        
                    }
                }
                
                this->length++;
                this->A_num += (p.y - this->p1->y) * (p.x - this->p1->x);
                this->A_den += (p.x - this->p1->x) * (p.x - this->p1->x);
            }
    };

    // Polynomial approximate
    class PolynomialModel {
        private:
            Polynomial* polynomial;

        public:
            int degree;
            short length;

            PolynomialModel(int degree) {
                this->degree = degree;
                this->length = 0;   
                this->polynomial = nullptr;
            }

            ~PolynomialModel() {
                this->length = 0;
                delete this->polynomial;
            }

            Polynomial* getPolynomial() {
                return this->polynomial;
            }

            float getCompressionRatio() {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4 * (this->degree + 1));
            }

            void fit(float bound, std::vector<Point2D>& segment, int pivot = 0)  {                
                if (segment.size() < this->degree + 1) {
                    return;
                }

                Eigen::VectorXd x(this->degree + 2);                    // decision variables
                Eigen::VectorXd c(this->degree + 2);                    // objective coefficients
                Eigen::MatrixXd A(2*(segment.size() - pivot), this->degree + 2);  // constraint matrix
                Eigen::VectorXd b(2*(segment.size() - pivot));                    // constraint bound

                for (int i=0; i<this->degree+2; i++) {
                    if (i != this->degree + 1) c(i) = 0.0;
                    else c(i) = 1.0;
                }

                for (int i=0; i<segment.size() - pivot; i++) {
                    for (int j=this->degree; j>=0; j--) {
                        A(2*i, this->degree-j) = -pow(segment[i].x, j);
                        A(2*i+1, this->degree-j) = pow(segment[i].x, j);
                    }
                    A(2*i, this->degree+1) = -1.0;
                    A(2*i+1, this->degree+1) = -1.0;

                    b(2*i) = -segment[i].y;
                    b(2*i+1) = segment[i].y;
                }

                double minobj = sdlp::linprog(c, A, b, x);                
                float* coefficients = new float[this->degree+1];
                for (int i = 0; i <= this->degree; i++) {
                    coefficients[this->degree-i] = x(i);
                }
                
                std::cout << "*****************\n";
                // std::cout << "minobj: " << minobj << "\n"; 
                // std::cout << "model: " << x.transpose() << "\n";  
                for (int i=0; i<segment.size() - pivot; i++) {
                    std::cout << segment[i].x << "," << segment[i].y << "\n"; 
                }
                std::cout << "*****************\n";

                if (this->polynomial != nullptr) delete this->polynomial;
                this->polynomial = new Polynomial(this->degree, coefficients);
                this->length = segment.size() - pivot;

                delete coefficients;
            }

            // void fit(float bound, std::vector<Point2D>& segment, int pivot = 0)  {                
            //     if (segment.size() < this->degree + 1) {
            //         return;
            //     }

            //     Matrix<double> *X = new Matrix<double>(segment.size(), degree+1);
            //     Matrix<double> *y = new Matrix<double>(segment.size(), 1);

            //     for (int i = 0; i < segment.size(); i++) {
            //         for (int k=0; k<degree+1; k++) {
            //             X->cell[i][k] = pow(i, k);
            //         } 
            //         y->cell[i][0] = segment[i].y;
            //     }

            //     Matrix<double>* X_T = X->transpose();
            //     Matrix<double>* X_T_X = Matrix<double>::matrix_outter_product(X_T, X);
            //     Matrix<double>* X_T_X_inv = X_T_X->inverse();
            //     Matrix<double>* X_T_X_inv_X_T = Matrix<double>::matrix_outter_product(X_T_X_inv, X_T);
            //     Matrix<double>* theta = Matrix<double>::matrix_outter_product(X_T_X_inv_X_T, y);
                
            //     double* coeffs = theta->toVec();
            //     if (this->polynomial != nullptr) delete this->polynomial;
            //     this->polynomial = new Polynomial(degree, coeffs);

            //     for (int i=0; i<=degree; i++) {
            //         std::cout << coeffs[i] << "---\n";
            //     }

            //     delete coeffs; delete theta;
            //     delete X; delete y; delete X_T; 
            //     delete X_T_X; delete X_T_X_inv;
            // }
    };

    void __yield(BinObj* obj, LinearModel* model) {
        unsigned char degree = 1;
        short length = -model->length;
        Line* line = model->getLine();

        obj->put(length);
        obj->put(line->get_slope());
        obj->put(line->get_intercept());

        // std::cout << "Linear: " << length << "\n";
    }

    void __yield(BinObj* obj, PolynomialModel* model) {
        unsigned char degree = model->degree;
        short length = model->length;
        Polynomial* polynomial = model->getPolynomial();

        obj->put(length);
        obj->put(degree);
        for (int i = 0; i <= model->degree; i++) {
            obj->put(polynomial->coefficients[i]);
        }

        // std::cout << "Polynomial: " << model->degree << " --- " << length << "\n";
    }

    Point2D __check(Point2D& p1, Point2D& p2, Point2D& p3) {
        Eigen::MatrixXd A(3, 3);
        A << p1.x*p1.x, p1.x, 1,
             p2.x*p2.x, p2.x, 1,
             p3.x*p3.x, p3.x, 1;
    
        Eigen::VectorXd b(3);
        b << p1.y, p2.y, p3.y;
    
        Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

        float extreme_x = (-x(1)) / (2*x(0));
        float extreme_y = x(0)*extreme_x*extreme_x + x(1)*extreme_x + x(2);

        // std::cout << "In: " << p1.x << "," << p1.y << " --- ";
        // std::cout << p2.x << "," << p2.y << " --- ";
        // std::cout << p3.x << "," << p3.y << " -> ";
        // std::cout << "extreme: " << extreme_x << "," << extreme_y << "\n";

        return Point2D(extreme_x, extreme_y);
    }
    
    void compress(TimeSeries& timeseries, float bound, std::string output) {
        clock.start();
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        std::vector<Point2D> segment = {Point2D(0, d->get_value())};
        Point2D p1(0, d->get_value());
        Point2D p2(-1, -1); Point2D p3(-1, -1);
        
        int phase = 1;
        LinearModel* l_1 = new LinearModel(); l_1->fit(bound, p1);
        LinearModel* l_2 = new LinearModel();

        int index = 1;
        bool flag = false;
        int degree = 1;     // current degree of polynomial
        int direction = 0;  // -1 decrease, 1 increase
        while (timeseries.hasNext()) {
            Point2D p(segment.size(), ((Univariate*) timeseries.next())->get_value());

            if (phase == 1) {
                if (!l_1->isComplete) l_1->fit(bound, p);

                if (l_1->isComplete) {
                    phase = 2;
                    p2 = Point2D(p.x, p.y);
                    l_2->fit(bound, p);
                }
            }
            else if (phase == 2) {
                if (!l_2->isComplete) l_2->fit(bound, p);
                if (l_2->isComplete) {
                    p3 = Point2D(
                        p2.x + l_2->length - 1,
                        l_2->getLine()->subs(p2.x + l_2->length - 1)
                    );
                }
            }

            if (l_1->isComplete && l_2->isComplete) {
                Line* line_1 = l_1->getLine();
                Line* line_2 = l_2->getLine();

                Point2D intersection = Line::intersection(*line_1, *line_2);
                if (intersection.x < p1.x || intersection.x > p3.x) flag = true;
                else {
                    Point2D extreme = __check(p1, intersection, p3);
                    if (line_1->get_slope() * line_2->get_slope() > 0) {
                        if (extreme.x > p1.x && extreme.x < p3.x) flag = true;
                    }
                    else {
                        if (std::abs(extreme.y - intersection.y) > bound) flag = true;
                    }
                }
                // flag = true;
                if (!flag) {
                    int n_direction = line_1->get_slope() > line_2->get_slope() ? -1 : 1;
                    if (direction != n_direction) {
                        direction = n_direction;
                        degree += 1;
                    }
                    if (degree > 4) {
                        flag = true;
                        degree = 4;
                    }
                    else {
                        delete l_1; l_1 = l_2;
                        l_2 = new LinearModel();
                        l_2->fit(bound, p);

                        p1 = Point2D(p2.x, p2.y);
                        p2 = Point2D(p.x, p.y);
                    }
                }
                 
                if (flag) {
                    // std::cout << "Index: " << index << " ";
                    if (degree == 1) {
                        __yield(compress_data, l_1);
                    }
                    else {
                        PolynomialModel* polynomialModel = new PolynomialModel(degree);
                        polynomialModel->fit(bound, segment, l_2->length);

                        __yield(compress_data, polynomialModel);
                    }

                    segment = { segment.end() - l_2->length, segment.end() };
                    for (int i=0; i<segment.size(); i++) segment[i].x = i; 
                    delete l_1; l_1 = l_2; l_1->translation(p3);

                    phase = 2;
                    p1 = Point2D(0, p2.y);
                    p2 = Point2D(segment.size(), p.y);
                    p = Point2D(segment.size(), p.y);

                    l_2 = new LinearModel(); l_2->fit(bound, p);

                    degree = 1; 
                    direction = 0;
                    flag = false;
                }
            }

            index++;
            segment.push_back(p);
        }

        delete l_1; delete l_2;

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;
        
        clock.tick();
        double avg_time = clock.getAvgDuration() / timeseries.size();

        // Profile average latency
        std::cout << std::fixed << "Time taken for each data point (ns): " << avg_time << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(avg_time));
        timeFile.close();
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, Line& line) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(line.subs(i)));
            file.write(&obj);
        }
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
        clock.start();
        while (compress_data->getSize() != 0) {
            short length = compress_data->getShort();

            if (length < 0) {
                // Linear segment
                float slope = compress_data->getFloat();
                float intercept = compress_data->getFloat();

                length = -length;
                Line line(slope, intercept);
                __decompress_segment(outputFile, interval, basetime, length, line);
            }
            else {
                int degree = (int) compress_data->getByte();
                float* coefficients = new float[degree+1];
                for (int i = 0; i <= degree; i++) {
                    coefficients[i] = compress_data->getFloat();
                }

                Polynomial polynomial(degree, coefficients);
                __decompress_segment(outputFile, interval, basetime, length, polynomial);
                delete[] coefficients;
            }
            
            basetime += length * interval;
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