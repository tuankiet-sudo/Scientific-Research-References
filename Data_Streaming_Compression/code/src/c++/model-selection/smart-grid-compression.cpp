#include "model-selection.hpp"

namespace SmartGridCompression {

    Clock clock;

    class Model {
        public:
            int degree;
            unsigned short length;

            virtual void clear() = 0;
            virtual float getCompressionRatio() = 0;
            virtual bool approximate(float bound, std::vector<Point2D>& segment) = 0;
    };

    // Constant approximate
    class ConstantModel : public Model {
        private:
            float value;
            float min;
            float max;

        public:
            ConstantModel() {
                this->degree = 0;
                this->length = 0;
                
                this->value = 0;
                this->min = INFINITY;
                this->max = -INFINITY;
            }

            ~ConstantModel() {
                this->degree = 0;
                this->length = 0;
                
                this->min = INFINITY;
                this->max = -INFINITY;
            }

            float getValue() {
                return this->value;
            }

            float getCompressionRatio() override {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4);
            }

            void clear() override {
                this->degree = 0;
                this->length = 0;
                
                this->min = INFINITY;
                this->max = -INFINITY;
            }

            bool approximate(float bound, std::vector<Point2D>& segment) override {
                if (this->length == 0) {
                    for (Point2D& p : segment) {
                        this->min = this->min < p.y ? this->min : p.y;
                        this->max = this->max > p.y ? this->max : p.y;
                        
                        if (this->max - this->min <= 2 * bound) {
                            this->value = (this->max + this->min) / 2;
                            this->length++;
                        }
                        else {
                            return false;
                        }
                    }
                    
                    return true;
                }
                else {
                    Point2D& p = segment.back();
                    this->min = this->min < p.y ? min : p.y;
                    this->max = this->max > p.y ? max : p.y;
                    
                    if (max - min <= 2 * bound) {
                        this->value = (max + min) / 2;
                        this->length++;
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }
    };

    // Linear approximate    
    class LinearModel : public Model {
        private:
            Line* line;
            ConvexHull cvx;

            float __cal_error(Line& line, std::vector<Point2D>& segment) {
                float max_error = 0;
                for (Point2D& p : segment) {
                    float error = std::abs(line.subs(p.x) - p.y);
                    max_error = error > max_error ? error : max_error;
                }
                return max_error;
            }

            float __distance(Line line, Point2D p) {
                return std::abs(p.y - line.subs(p.x)) / sqrt(line.get_slope()*line.get_slope()+1);
            }

            int __x_external(int x, int x1, int x2) {
                // 0:  x_external
                // 1:  x_external to the right
                // -1: x_external to the left

                if (x >= x1 && x >= x2) return 1;
                else if (x <= x1 && x <= x2) return -1;
                else return 0;
            }

            int __search(int m, int p, int j) {
                Line line = Line::line(this->cvx.at_ccw(p), this->cvx.at_ccw(j));

                // search in ccw order
                // if pj(i+1) < pj(i) return i
                int v = -1; float prev_dis = -INFINITY;
                for (int i=0; i<this->cvx.size(); i++) {
                    float dis = this->__distance(line, this->cvx.at_ccw((m+i)%this->cvx.size()));
                    if (prev_dis < dis) {
                        prev_dis = dis;
                        v = (m+i)%this->cvx.size();
                    }
                    else break;

                    
                }

                return v;
            }

            Line __approx(const std::vector<Point2D>& segment) {
                if (this->cvx.size() <= 2) return Line::line(segment[0], segment[1]);
                else if (this->cvx.size() > 2) {
                    // find the A, B and C
                    Point2D A(0, 0), B(0,0), C(0,0);
                    int prev_v_l = this->cvx.rightmost_index_ccw();

                    // find the first v(l1)
                    for (int i=0; i<this->cvx.size(); i++) {
                        int v_l = this->__search(prev_v_l, i, (i+1)%this->cvx.size());                        
                        int external = this->__x_external(
                            this->cvx.at_ccw(v_l).x, this->cvx.at_ccw(i).x, this->cvx.at_ccw((i+1)%this->cvx.size()).x
                        );

                        if (external == 1) {
                            prev_v_l = v_l;
                        }
                        else if (external == 0) {
                            A = this->cvx.at_ccw(i);
                            B = this->cvx.at_ccw((i+1)%this->cvx.size());
                            C = this->cvx.at_ccw(v_l);
                            break;
                        }
                        else {
                            int step = v_l > prev_v_l ? v_l - prev_v_l : v_l - prev_v_l + this->cvx.size();
                            for (int j=0; j<step; j++) {
                                int external = this->__x_external(
                                    this->cvx.at_ccw(i).x,
                                    this->cvx.at_ccw((prev_v_l+j)%this->cvx.size()).x,
                                    this->cvx.at_ccw((prev_v_l+j+1)%this->cvx.size()).x
                                );

                                if (external == 0) {
                                    A = this->cvx.at_ccw((prev_v_l+j)%this->cvx.size());
                                    B = this->cvx.at_ccw((prev_v_l+j+1)%this->cvx.size());
                                    C = this->cvx.at_ccw(i);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    
                    // find the optimal approximation line
                    Line line = Line::line(A, B);
                    float a = line.get_slope();
                    float b = line.get_intercept();
                    float c = C.y > A.x ? b + this->__distance(line, C)/2*sqrt(a*a+1) 
                                        : b - this->__distance(line, C)/2*sqrt(a*a+1);
                    
                    return Line(a, c);
                }

                return Line(this->line->get_slope(), this->line->get_intercept());
            }

        public:
            LinearModel() {
                this->degree = 1;
                this->length = 0;
                this->line = nullptr;
            }

            ~LinearModel() {
                this->length = 0;
                this->cvx.clear();

                if (this->line != nullptr) {
                    delete this->line;
                    this->line = nullptr;
                }
            }

            Line* getLine() {
                return this->line;
            }

            float getCompressionRatio() override {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4 + 4);
            }

            void clear() override {
                this->length = 0;
                this->cvx.clear();

                if (this->line != nullptr) {
                    delete this->line;
                    this->line = nullptr;
                }
            }

            bool approximate(float bound, std::vector<Point2D>& segment) override {
                if (this->length == 0) {
                    for (Point2D& p : segment) {
                        this->cvx.append(p);
                        
                        if (this->cvx.size() > 1) {
                            Line line = this->__approx(segment);
                            if (this->__cal_error(line, segment) > bound) return false;
                            else {
                                if (this->line != nullptr) delete this->line;
                                this->line = new Line(line.get_slope(), line.get_intercept());
                            }
                        }
                        this->length++;
                    }
                    
                    return true;
                }
                else {
                    Point2D& p = segment.back();
                    this->cvx.append(p);

                    if (this->cvx.size() > 1) {
                        Line line = this->__approx(segment);
                        if (this->__cal_error(line, segment) > bound) return false;
                        else {
                            if (this->line != nullptr) delete this->line;
                            this->line = new Line(line.get_slope(), line.get_intercept());
                        } 
                    }

                    this->length++;
                    return true;
                }
            }
    };

    // Polynomial approximate
    class PolynomialModel : public Model {
        private:
            Polynomial* polynomial;

            float __cal_error(std::vector<Point2D>& segment) {
                float max_error = 0;
                for (Point2D& p : segment) {
                    float error = std::abs(this->polynomial->subs(p.x) - p.y);
                    max_error = error > max_error ? error : max_error;
                }
                return max_error;
            }

        public:
            PolynomialModel(int degree) {
                this->degree = degree;
                this->length = 0;   
                this->polynomial = nullptr;
            }

            ~PolynomialModel() {
                this->length = 0;
                if (this->polynomial != nullptr) {
                    delete this->polynomial;
                    this->polynomial = nullptr;
                }
            }

            Polynomial* getPolynomial() {
                return this->polynomial;
            }

            float getCompressionRatio() override {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4 * (degree + 1));
            }

            void clear() override {
                this->length = 0;
                if (this->polynomial != nullptr) {
                    delete this->polynomial;
                    this->polynomial = nullptr;
                }
            }

            bool approximate(float bound, std::vector<Point2D>& segment) override {
                
                Eigen::VectorXd x(this->degree + 2);                    // decision variables
                Eigen::VectorXd c(this->degree + 2);                    // objective coefficients
                Eigen::MatrixXd A(2*segment.size(), this->degree + 2);  // constraint matrix
                Eigen::VectorXd b(2*segment.size());                    // constraint bound

                for (int i=0; i<this->degree+2; i++) {
                    if (i != this->degree + 1) c(i) = 0.0;
                    else c(i) = 1.0;
                }

                for (int i=0; i<segment.size(); i++) {
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
                if (minobj == INFINITY || minobj == -INFINITY || minobj > bound) {
                    return false;
                }
                else {
                    float* coefficients = new float[this->degree+1];
                    for (int i = 0; i <= this->degree; i++) {
                        coefficients[this->degree-i] = x(i);
                    }

                    if (this->polynomial != nullptr) delete this->polynomial;
                    this->polynomial = new Polynomial(this->degree, coefficients);

                    this->length = segment.size();
                    return true;
                }
            }
    };

    void __yield(BinObj* obj, Model* model) {
        if (model->degree == 0) {
            unsigned char degree = 0;
            short length = model->length;

            obj->put(degree);
            obj->put(length);
            obj->put(((ConstantModel*) model)->getValue());
        }
        else if (model->degree == 1) {
            Line* line = ((LinearModel*) model)->getLine();
            unsigned char degree = 1;
            short length = model->length;

            obj->put(degree);
            obj->put(length);
            obj->put(line->get_slope());
            obj->put(line->get_intercept());
        }
        else {
            Polynomial* polynomial = ((PolynomialModel*) model)->getPolynomial();
            unsigned char degree = polynomial->degree;
            short length = model->length;

            obj->put(degree);
            obj->put(length);
            for (int i = 0; i <= polynomial->degree; i++) {
                obj->put(polynomial->coefficients[i]);
            }
        }
    }

    bool __approxSuccess(int degree, float bound, Model* model, std::vector<Point2D>& segment) {
        bool success = true;

        if (degree == 0) {
            ConstantModel* constantModel = (ConstantModel*) model;
            success = constantModel->approximate(bound, segment);
        }
        else if (degree == 1) {
            LinearModel* linearModel = (LinearModel*) model;
            success = linearModel->approximate(bound, segment);
        }
        else {
            if (segment.size() > degree) {
                PolynomialModel* polynomialModel = (PolynomialModel*) model;
                success = polynomialModel->approximate(bound, segment);
            }
        }

        clock.tick();
        return success;
    }

    int __choose_best_model(std::vector<Model*>& models) {
        int index = 0;
        float max_gain = -INFINITY;

        for (int i=0; i<models.size(); i++) {
            float gain = models[i]->getCompressionRatio();
            if (gain > max_gain) {
                index = i;
                max_gain = gain;
            }
        }

        return index;
    }

    void compress(TimeSeries& timeseries, int degree, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        std::vector<Point2D> segment;
        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        segment.push_back(Point2D(0, d->get_value()));

        std::vector<Model*> models;
        for (int i=0; i<=degree; i++) {
            if (i == 0) models.push_back(new ConstantModel());
            else if (i == 1) models.push_back(new LinearModel());
            else models.push_back(new PolynomialModel(i));
        }

        clock.start();
        while (!segment.empty() || timeseries.hasNext()) {
            if (timeseries.hasNext()) {
                Point2D p(segment.size(), ((Univariate*) timeseries.next())->get_value());
                segment.push_back(p);
            }
            else {
                if (segment.size() == 1) break;
            }

            if (segment.size() > 1) {
                for (int k=0; k<=degree; k++) {
                    while (__approxSuccess(k, bound, models[k], segment) && timeseries.hasNext() && segment.size() < 65000) {
                        segment.push_back(Point2D(segment.size(), ((Univariate*) timeseries.next())->get_value()));
                    }

                    if (!timeseries.hasNext() || segment.size() >= 65000) {
                        break;
                    }
                }

                int index = __choose_best_model(models);
                __yield(compress_data, models[index]);

                segment.erase(segment.begin(), segment.begin() + models[index]->length);
                for (int i=0; i<segment.size(); i++) {
                    segment[i].x = i;
                }
                for (int i=0; i<=degree; i++) {
                    models[i]->clear();
                }                
            }
        }

        for (int i=0; i<=degree; i++) {
            delete models[i];
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;
        
        // Profile average latency
        std::cout << std::fixed << "Time taken for each data point (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float value) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(value));
            file.write(&obj);
        }
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
            int degree = (int) compress_data->getByte();
            unsigned short length = compress_data->getShort();

            if (degree == 0) {
                float value = compress_data->getFloat();
                __decompress_segment(outputFile, interval, basetime, length, value);
            }
            else if (degree == 1) {
                float slope = compress_data->getFloat();
                float intercept = compress_data->getFloat();
                
                Line line(slope, intercept);
                __decompress_segment(outputFile, interval, basetime, length, line);
            }
            else {
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
}