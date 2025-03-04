#include "model-selection/polynomial.hpp"

namespace Unbounded {

    Clock clock;

    // Constant approximate
    class ConstantModel {
        private:
            float min;
            float max;

        public:
            bool isComplete;
            unsigned short length;

            ConstantModel() {
                this->length = 0;
                this->min = INFINITY;
                this->max = -INFINITY;
                this->isComplete = false;
            }

            float getConstant() {
                return (this->max + this->min) / 2;
            }

            float getCompressionRatio() {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4);
            }

            void fit(float bound, Point2D& p)  {
                float n_min = this->min < p.y ? min : p.y;
                float n_max = this->max > p.y ? max : p.y;
                
                if (n_max - n_min <= 2 * bound) {
                    this->length++;
                    this->max = n_max;
                    this->min = n_min;
                }
                else {
                    this->isComplete = true;
                    return;
                }
            }
    };

    // Linear approximate    
    class LinearModel {
        private:
            Line* u_line;
            Line* l_line;
            UpperHull u_cvx;
            LowerHull l_cvx;
            Point2D* p1;

        public:
            bool isComplete;
            unsigned short length;
            
            LinearModel() {
                this->length = 0;
                this->u_line = nullptr;
                this->l_line = nullptr;
                this->isComplete = false;
            }

            ~LinearModel() {
                delete this->p1;
                delete this->u_line;
                delete this->l_line;
                this->u_cvx.clear();
                this->l_cvx.clear();
                this->length = 0;
                this->isComplete = false;
            }

            void translation(float index) {
                Line* l = new Line(this->l_line->get_slope(), this->l_line->subs(index));
                Line* u = new Line(this->u_line->get_slope(), this->u_line->subs(index));

                if (this->l_line != nullptr) delete this->l_line;
                if (this->u_line != nullptr) delete this->u_line;
                
                this->l_line = l; this->u_line = u;
            }

            Line* getLine() {
                return new Line(
                    (this->u_line->get_slope() + this->l_line->get_slope()) / 2,
                    (this->u_line->get_intercept() + this->l_line->get_intercept()) / 2
                );
            }

            float getCompressionRatio() {
                if (this->length == 0) return -1;       // Ignore this model
                else return (float) this->length / (1 + 2 + 4 + 4);
            }

            void fit(float bound, Point2D& p)  {
                // if (this->length == 0) {
                //     this->u_cvx.append(Point2D(p.x, p.y-bound));
                //     this->l_cvx.append(Point2D(p.x, p.y+bound));
                // }
                // else if (this->length == 1) {
                //     this->u_cvx.append(Point2D(p.x, p.y-bound));
                //     this->l_cvx.append(Point2D(p.x, p.y+bound));

                //     Line u = Line::line(this->u_cvx.at(0), Point2D(p.x, p.y+bound));
                //     Line l = Line::line(this->l_cvx.at(0), Point2D(p.x, p.y-bound));

                //     this->u_line = new Line(u.get_slope(), u.get_intercept());
                //     this->l_line = new Line(l.get_slope(), l.get_intercept());
                // }
                // else {
                //     if (this->length > 65000 || this->l_line->subs(p.x) > p.y + bound || p.y - bound > this->u_line->subs(p.x)) {
                //         this->isComplete = true;
                //         return;
                //     }
                //     else {
                //         bool update_u = p.y + bound < this->u_line->subs(p.x);
                //         bool update_l = p.y - bound > this->l_line->subs(p.x);

                //         if (update_u) {
                //             int index = 0;
                //             float min_slp = INFINITY;

                //             for (int i=0; i<this->u_cvx.size(); i++) {
                //                 Line l = Line::line(this->u_cvx.at(i), Point2D(p.x, p.y+bound));
                //                 if (l.get_slope() < min_slp) {
                //                     min_slp = l.get_slope();
                //                     index = i;

                //                     delete this->u_line;
                //                     this->u_line = new Line(l.get_slope(), l.get_intercept());
                //                 }
                //             }
                //             this->u_cvx.erase_from_begin(index);
                //         }
                //         if (update_l) {
                //             int index = 0;
                //             float max_slp = -INFINITY;

                //             for (int i=0; i<this->l_cvx.size(); i++) {
                //                 Line l = Line::line(this->l_cvx.at(i), Point2D(p.x, p.y-bound));
                //                 if (l.get_slope() > max_slp) {
                //                     max_slp = l.get_slope();
                //                     index = i;

                //                     delete this->l_line;
                //                     this->l_line = new Line(l.get_slope(), l.get_intercept());
                //                 }
                //             }
                //             l_cvx.erase_from_begin(index);
                //         }

                //         if (update_u) this->l_cvx.append(Point2D(p.x, p.y+bound));
                //         if (update_l) this->u_cvx.append(Point2D(p.x, p.y-bound));
                //     }
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
                    if (this->length > 65000 || this->l_line->subs(p.x) > p.y + bound || p.y - bound > this->u_line->subs(p.x)) {
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
            }
    };

    // Polynomial approximate
    class PolynomialModel {
        private:
            Polynomial* polynomial;

        public:
            int degree;
            unsigned short length;

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

                if (this->polynomial != nullptr) delete this->polynomial;
                this->polynomial = new Polynomial(this->degree, coefficients);
                this->length = segment.size() - pivot;
            }
    };

    void __yield(BinObj* obj, ConstantModel* model) {
        unsigned char degree = 0;
        short length = model->length;
        float value = model->getConstant();

        obj->put(degree);
        obj->put(length);
        obj->put(value);
    }

    void __yield(BinObj* obj, LinearModel* model) {
        unsigned char degree = 1;
        short length = model->length;
        Line* line = model->getLine();

        obj->put(degree);
        obj->put(length);
        obj->put(line->get_slope());
        obj->put(line->get_intercept());
    }

    void __yield(BinObj* obj, PolynomialModel* model) {
        unsigned char degree = model->degree;
        short length = model->length;
        Polynomial* polynomial = model->getPolynomial();

        obj->put(degree);
        obj->put(length);
        for (int i = 0; i <= model->degree; i++) {
            obj->put(polynomial->coefficients[i]);
        }
    }

    // void compress(TimeSeries& timeseries, float bound, std::string output) {
    //     IterIO outputFile(output, false);
    //     BinObj* compress_data = new BinObj;

    //     Univariate* d = (Univariate*) timeseries.next();
    //     compress_data->put(d->get_time());
    //     std::vector<float> buffer = {d->get_value()};
    //     std::vector<Point2D> segment;

    //     bool f_c_1 = true; ConstantModel* c_1 = new ConstantModel(); 
    //     bool f_c_2 = false; ConstantModel* c_2 = new ConstantModel();
    //     bool f_c_3 = false; ConstantModel* c_3 = new ConstantModel();
    //     bool f_l_1 = true; LinearModel* l_1 = new LinearModel();
    //     bool f_l_2 = false; LinearModel* l_2 = new LinearModel();
    //     bool f_l_3 = false; LinearModel* l_3 = new LinearModel(); 
    //     float s_index = -1; float s_val = -1; float e_val = -1;

    //     bool f_p = false; int degree = 2; int direction = 0; // -1 decrease, 1 increase
    //     bool flag = false; int length = 0; int scenario = -1;

    //     clock.start();
    //     while (!buffer.empty()) {
    //         if (timeseries.hasNext()) {
    //             buffer.push_back(((Univariate*) timeseries.next())->get_value());
    //         }
            
    //         Point2D p(segment.size(), buffer.front());
    //         buffer.erase(buffer.begin());

    //         // for (Point2D& point : segment) {
    //         //     std::cout << point.x << " " << point.y << " --- ";
    //         // }
    //         // std::cout << "\n";

    //         if (f_c_1) {
    //             c_1->fit(bound, p);
    //             if (c_1->isComplete) {
    //                 f_c_1 = false;
    //                 f_c_2 = true;
    //                 f_l_2 = true;
    //             }
    //         }
    //         if (f_c_2) {
    //             c_2->fit(bound, p);
    //             if (c_2->isComplete) {
    //                 f_c_2 = false;
    //                 f_c_3 = true;
    //             }
    //         }
    //         if (f_c_3) {
    //             c_3->fit(bound, p);
    //             if (c_3->isComplete) {
    //                 f_c_3 = false;
    //             }
    //         }
    //         if (f_l_1) {
    //             l_1->fit(bound, p);
    //             if (l_1->isComplete) {
    //                 f_l_1 = false;
    //                 f_l_3 = true;
    //                 s_val = p.y;
    //                 s_index = p.x;
    //                 e_val = segment.back().y;
    //             }
    //         }
    //         if (f_l_2) {
    //             l_2->fit(bound, p);
    //             if (l_2->isComplete) {
    //                 f_l_2 = false;
    //             }
    //         }
    //         if (f_l_3) {
    //             l_3->fit(bound, p);
    //             if (l_3->isComplete) {
    //                 f_l_3 = false;
    //             }
    //         }


    //         if (!f_l_1 && !f_l_3) {
    //             Line* line_1 = l_1->getLine();
    //             Line* line_3 = l_3->getLine();

    //             if (line_1->get_slope() > line_3->get_slope() && line_1->subs(s_index) > s_val+bound) {
    //                 scenario = 4; f_p = true;
    //                 if (direction == 1) {
    //                     direction = -1;
    //                     degree += 1;
    //                     if (degree >= 4) flag = true;
    //                 }
    //                 else if (direction == 0) {
    //                     direction = -1;
    //                 }

    //                 delete l_1; l_1 = l_3;
    //                 f_l_3 = true; l_3 = new LinearModel();
    //                 l_3->fit(bound, p);

    //                 s_val = p.y;
    //                 s_index = p.x;
    //                 e_val = segment.back().y;
    //             }
    //             else if (line_1->get_slope() < line_3->get_slope() && line_1->subs(s_index) < s_val-bound) {
    //                 scenario = 4; f_p = true;
    //                 if (direction == -1) {
    //                     direction = 1;
    //                     degree += 1;
    //                     if (degree >= 4) flag = true;
    //                 }
    //                 else if (direction == 0) {
    //                     direction = 1;
    //                 }

    //                 delete l_1; l_1 = l_3;
    //                 f_l_3 = true; l_3 = new LinearModel();
    //                 l_3->fit(bound, p);

    //                 s_val = p.y;
    //                 s_index = p.x;
    //                 e_val = segment.back().y;
    //             }
    //             else if (f_p) {
    //                 flag = true;
    //             }
    //             else {
    //                 flag = true;
    //                 float ratio_1 = (l_1->length + l_3->length) / ((float) 2 * (1 + 2 + 8)); // use 2 linear
    //                 float ratio_2 = (l_2->length + c_1->length) / ((float) 1 + 2 + 8 + 1 + 2 + 4); // use constant and linear
    //                 float ratio_3 = c_3->length == 0 ? (c_1->length + c_2->length) / ((float) 2 * (1 + 2 + 4))  // use all constants
    //                                 :  (c_1->length + c_2->length + c_3->length) / ((float) 3 * (1 + 2 + 4));

    //                 if (ratio_1 > ratio_2 && ratio_1 > ratio_3) {
    //                     length = l_1->length + l_3->length;
    //                     scenario = 0;
    //                 }
    //                 else if (ratio_2 > ratio_1 && ratio_2 > ratio_3) {
    //                     length = l_2->length + c_1->length;
    //                     scenario = 1;
    //                 }
    //                 else {
    //                     length = c_3->length == 0 ? c_1->length + c_2->length : c_1->length + c_2->length + c_3->length;
    //                     scenario = 3;
    //                 }
    //             }

    //         }

    //         if (flag) {
    //             if (scenario == 0) {
    //                 __yield(compress_data, l_1);
    //                 __yield(compress_data, l_3);
    //             }
    //             else if (scenario == 1) {
    //                 __yield(compress_data, c_1);
    //                 __yield(compress_data, l_2);
    //             }
    //             else if (scenario == 2) {
    //                 __yield(compress_data, c_1);
    //                 __yield(compress_data, c_2);
    //                 if (c_3->length != 0) __yield(compress_data, c_3);
    //             }
    //             else {
    //                 PolynomialModel* polynomialModel = new PolynomialModel(degree);
    //                 polynomialModel->fit(bound, segment);
    //                 length = segment.size();

    //                 __yield(compress_data, polynomialModel);
    //             }

    //             buffer.insert(buffer.begin(), p.y);
    //             for (int i=segment.size()-1; i>=length; i--) {
    //                 buffer.insert(buffer.begin(), segment[i].y);
    //             }

    //             segment.clear(); 

    //             delete c_1; f_c_1 = true; c_1 = new ConstantModel(); 
    //             delete c_2; f_c_2 = false; c_2 = new ConstantModel();
    //             delete c_3; f_c_3 = false; c_3 = new ConstantModel();
    //             delete l_1; f_l_1 = true; l_1 = new LinearModel();
    //             delete l_2; f_l_2 = false; l_2 = new LinearModel();
    //             delete l_3; f_l_3 = false; l_3 = new LinearModel();

    //             s_index = -1; s_val = -1; e_val = -1;
    //             f_p = false; degree = 2; direction = 0; 
    //             flag = false; length = 0; scenario = -1;

    //             continue;
    //         }

    //         segment.push_back(p);
            
    //     }
    //     clock.tick();

    //     delete c_1; f_c_1 = true; c_1 = new ConstantModel(); 
    //     delete c_2; f_c_2 = false; c_2 = new ConstantModel();
    //     delete c_3; f_c_3 = false; c_3 = new ConstantModel();
    //     delete l_1; f_l_1 = true; l_1 = new LinearModel();
    //     delete l_2; f_l_2 = false; l_2 = new LinearModel();
    //     delete l_3; f_l_3 = false; l_3 = new LinearModel();

    //     outputFile.writeBin(compress_data);
    //     outputFile.close();
    //     delete compress_data;
        
    //     // Profile average latency
    //     std::cout << std::fixed << "Time taken for each data point (ns): " << clock.getAvgDuration() << "\n";
    //     IterIO timeFile(output+".time", false);
    //     timeFile.write("Time taken for each data point (ns): " + std::to_string(clock.getAvgDuration()));
    //     timeFile.close();
    // }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        clock.start();
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        std::vector<float> buffer = {d->get_value()};
        std::vector<Point2D> segment;

        int phase = 1;
        LinearModel* l_1 = new LinearModel();
        LinearModel* l_2 = new LinearModel();
        float s_index = -1; float s_val = -1; float e_val = -1;

        int degree = 1; int direction = 0; // -1 decrease, 1 increase
        int index = 0; bool flag = false; int scenario = -1;

        while (!buffer.empty()) {
            if (timeseries.hasNext()) {
                buffer.push_back(((Univariate*) timeseries.next())->get_value());
            }
            
            Point2D p(segment.size(), buffer.front());
            buffer.erase(buffer.begin());

            if (phase == 1) {
                if (!l_1->isComplete) l_1->fit(bound, p);

                if (l_1->isComplete) {
                    phase = 2;
                    s_val = p.y;
                    s_index = p.x;
                    e_val = segment.back().y;
                }
            }
            if (phase == 2) {
                if (!l_2->isComplete) l_2->fit(bound, p);
            }

            if (l_1->isComplete && l_2->isComplete) {
                Line* line_1 = l_1->getLine();
                Line* line_2 = l_2->getLine();

                bool f_p = (line_1->get_slope() > line_2->get_slope() && line_1->subs(s_index) > s_val+bound && e_val - bound < s_val + bound)
                            || (line_1->get_slope() < line_2->get_slope() && line_1->subs(s_index) < s_val-bound && e_val + bound > s_val - bound);

                if (f_p) {
                    scenario = 2;
                    int n_direction = line_1->get_slope() > line_2->get_slope() ? -1 : 1;
                    if (direction != n_direction) {
                        direction = n_direction;
                        degree += 1;
                    }

                    if (degree > 3) {
                        degree--;
                        scenario = 0;
                        flag = true;
                    }
                    else {
                        delete l_1; l_1 = l_2;
                        l_2 = new LinearModel();
                        l_2->fit(bound, p);

                        s_val = p.y;
                        s_index = p.x;
                        e_val = segment.back().y;
                    }
                }
                else if (scenario == 2) flag = true;
                else { scenario = 1; flag = true; }
            }

            if (flag) {
                if (scenario == 0) {
                    PolynomialModel* polynomialModel = new PolynomialModel(degree);
                    polynomialModel->fit(bound, segment, l_2->length);

                    __yield(compress_data, polynomialModel);
                    
                    segment = { segment.end() - l_2->length, segment.end() };
                    for (int i=0; i<segment.size(); i++) segment[i].x = i; 
                    
                    phase = 2; p = Point2D(segment.size(), p.y);
                    delete l_1; l_1 = l_2; l_1->translation(s_index);
                    l_2 = new LinearModel(); l_2->fit(bound, p);
                    
                    s_index = p.x; s_val = p.y; e_val = segment.back().y;
                    degree = 1; direction = 0;
                    flag = false; scenario = -1;
                }
                else if (scenario == 1) {
                    __yield(compress_data, l_1);

                    segment = { segment.begin() + l_1->length, segment.end() };
                    for (int i=0; i<segment.size(); i++) segment[i].x = i; 

                    phase = 2; p = Point2D(segment.size(), p.y);
                    delete l_1; l_1 = l_2; l_1->translation(s_index);
                    l_2 = new LinearModel(); l_2->fit(bound, p);

                    s_index = p.x; s_val = p.y; e_val = segment.back().y;
                    degree = 1; direction = 0;
                    flag = false; scenario = -1;
                }
                else if (scenario == 2) {
                    PolynomialModel* polynomialModel = new PolynomialModel(degree);
                    polynomialModel->fit(bound, segment);

                    __yield(compress_data, polynomialModel);
                    segment.clear(); 

                    phase = 1; 
                    delete l_1; l_1 = new LinearModel(); 
                    delete l_2; l_2 = new LinearModel();

                    p = Point2D(0, p.y); l_1->fit(bound, p);

                    s_index = -1; s_val = -1; e_val = -1;
                    degree = 1; direction = 0; 
                    flag = false; scenario = -1;
                }
            }

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
    
};