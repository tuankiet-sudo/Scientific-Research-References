#include "model-selection/polynomial.hpp"

namespace Bounded {

    Clock clock;

    class Model {
        public:
            virtual Polynomial* get() = 0;
            virtual int getDegree () = 0;        
            virtual bool fit(float bound, std::vector<float>& data, int start) = 0;
    };

    class LinearModel : public Model {
        private:
            UpperHull u_cvx; 
            LowerHull l_cvx;
            Line* u_line;
            Line* l_line;
        
        public:        
            LinearModel() {
                this->u_line = nullptr;
                this->l_line = nullptr;
            }

            ~LinearModel() {
                if (this->l_line != nullptr) delete this->u_line;
                if (this->u_line != nullptr) delete this->l_line;
                this->u_cvx.clear();
                this->l_cvx.clear();
            }

            Polynomial* get() override {
                float slope = (this->u_line->get_slope() + this->l_line->get_slope()) / 2;
                float intercept = (this->u_line->get_intercept() + this->l_line->get_intercept()) / 2;
                
                float coefficients[2] = {intercept, slope};
                return new Polynomial(1, coefficients);
            }

            int getDegree() override {
                return 1;
            }

            bool fit(float bound, std::vector<float>& data, int start) override {
                Point2D p(data.size()-start-1, data.back());

                std::cout << p.x << " " << p.y << "\n";

                if (this->u_cvx.size() == 0) {
                    this->u_cvx.append(Point2D(p.x, p.y-bound));
                    this->l_cvx.append(Point2D(p.x, p.y+bound));
                }
                else if (this->u_cvx.size() == 1) {
                    Line u = Line::line(this->u_cvx.at(0), Point2D(p.x, p.y+bound));
                    Line l = Line::line(this->l_cvx.at(0), Point2D(p.x, p.y-bound));

                    u_cvx.append(Point2D(p.x, p.y-bound));
                    l_cvx.append(Point2D(p.x, p.y+bound));
                    this->u_line = new Line(u.get_slope(), u.get_intercept());
                    this->l_line = new Line(l.get_slope(), l.get_intercept());
                }
                else {
                    if (this->l_line->subs(p.x) > p.y + bound || p.y - bound > this->u_line->subs(p.x)) {
                        return false;
                    }
                    else {
                        bool update_u = p.y + bound < this->u_line->subs(p.x);
                        bool update_l = p.y - bound > this->l_line->subs(p.x);
    
                        if (update_u) {
                            int index = 0;
                            float min_slp = INFINITY;
    
                            for (int i=0; i<this->u_cvx.size(); i++) {
                                Line line = Line::line(this->u_cvx.at(i), Point2D(p.x, p.y+bound));
                                if (line.get_slope() < min_slp) {
                                    min_slp = line.get_slope();
                                    index = i;
                                    
                                    delete this->u_line;
                                    this->u_line = new Line(line.get_slope(), line.get_intercept());
                                }
                            }
                            this->u_cvx.erase_from_begin(index);
                        }
                        if (update_l) {
                            int index = 0;
                            float max_slp = -INFINITY;
    
                            for (int i=0; i<this->l_cvx.size(); i++) {
                                Line line = Line::line(this->l_cvx.at(i), Point2D(p.x, p.y-bound));
                                if (line.get_slope() > max_slp) {
                                    max_slp = line.get_slope();
                                    index = i;

                                    delete this->l_line;
                                    this->l_line = new Line(line.get_slope(), line.get_intercept());
                                }
                            }
                            this->l_cvx.erase_from_begin(index);
                        }
    
                        if (update_u) this->l_cvx.append(Point2D(p.x, p.y+bound));
                        if (update_l) this->u_cvx.append(Point2D(p.x, p.y-bound));
                    }
                }

                return true;
            }
    };

    class PolyModel : public Model {
        private:
            int degree;
            Polynomial* func;

        public:
            PolyModel(int degree) {
                this->degree = degree;
                this->func = nullptr;
            }

            ~PolyModel() {
                if (this->func != nullptr) delete this->func;
            }

            Polynomial* get() override {
                return this->func;
            }

            int getDegree() override {
                return this->degree;
            }

            bool fit(float bound, std::vector<float>& data, int start) override {
                if (this->func != nullptr) {
                    if (std::abs(this->func->subs(data.size()-start-1) - data.back()) <= bound) {
                        return true;
                    }
                }

                Eigen::VectorXd x(degree + 2);                    
                Eigen::VectorXd c(degree + 2);                    
                Eigen::MatrixXd A(2*(data.size()-start), degree + 2);  
                Eigen::VectorXd b(2*(data.size()-start));              
        
                for (int i=0; i<degree+2; i++) {
                    if (i != degree + 1) c(i) = 0.0;
                    else c(i) = 1.0;
                }
                
                for (int i=0; i<data.size()-start; i++) {                                                              
                    for (int j=degree; j>=0; j--) {
                        A(2*i, degree-j) = -pow(i, j);
                        A(2*i+1, degree-j) = pow(i, j);
                    }
                    A(2*i, degree+1) = -1.0;
                    A(2*i+1, degree+1) = -1.0;
        
                    b(2*i) = -data[i+start];
                    b(2*i+1) = data[i+start];
                }
        
                double minobj = sdlp::linprog(c, A, b, x); 
                
                if (minobj <= bound) {
                    float* coefficients = new float[degree+1];
                    for (int i = 0; i <= degree; i++) {
                        coefficients[degree-i] = x(i);
                    }
                    
                    if (this->func != nullptr) delete this->func;
                    this->func = new Polynomial(degree, coefficients);
                    delete coefficients;
        
                    return true;
                }

                return false;
            }
    };

    struct Segment {
        int start;
        short length;
        int triedDegree;
        int childLength;
        int childCoeffByte;
        bool childTrigger;
        int nextTrigger;
        bool isComplete;
        Model* model;

        Segment(int start, Model* model) {
            this->start = start;
            this->length = 0;
            this->triedDegree = 1;
            this->childLength = 0;
            this->childCoeffByte = 0;
            this->childTrigger = false;
            this->model = model; 
            this->isComplete = false;
        }

        ~Segment() {
            if (this->model != nullptr) delete this->model;
        }

        void updateCRTrigger() {
            this->triedDegree++;
            float curr_cr = (float) (this->childLength + this->length) / (this->childCoeffByte + 6 + this->model->getDegree() * 4);
            this->nextTrigger = std::ceil(curr_cr * (10 + this->triedDegree * 4));
        }

        void setChildTrigger(Segment* child) {
            this->childLength += child->length;
            this->childCoeffByte += 6 + child->model->getDegree() * 4;
            this->updateChildTrigger();
        }

        void updateChildTrigger() {
            this->childTrigger = 
                (this->childCoeffByte + 6 + this->model->getDegree() * 4) > 
                (this->triedDegree * 4 + 10);
        }

        void updateModel(Model* model, short length) {
            this->length = length;
            this->childLength = 0;
            this->childCoeffByte = 0;
            this->isComplete = false;

            delete this->model;
            this->model = model;
        }

    };

    void __yield(BinObj* obj, Model* model, short length) {
        short degree_length = length | ((model->getDegree() - 1) << 14) ;
        Polynomial* polynomial = model->get();

        obj->put(degree_length);
        for (int i = 0; i <= model->getDegree(); i++) {
            obj->put(polynomial->coefficients[i]);
        }
        std::cout << "yield: " << model->getDegree() << " " << length << "\n";
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        clock.start();
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        time_t basetime = 1;
        compress_data->put(basetime);

        std::vector<Segment*> segments = {new Segment(0, new LinearModel())};
        std::vector<float> buffer;

        while (timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();
            // std::cout << data->get_time() << " " << data->get_value() << "\n";
            buffer.push_back(data->get_value());

            for (int i=0; i<segments.size(); i++) {
                Segment* seg = segments[i];
                if(!seg->isComplete) {
                    if (!seg->model->fit(bound, buffer, seg->start)) {
                        seg->isComplete = true;
                        Segment* n_seg = new Segment(buffer.size()-1, new LinearModel());
                        segments.push_back(n_seg);
                        
                        for (int j=0; j<i; j++) segments[j]->setChildTrigger(seg);
                    }
                    else seg->length++;
                }
                
                if ((buffer.size() - seg->start > 16000) || 
                    (seg->triedDegree == 4 && seg->isComplete)) {
                    
                    __yield(compress_data, seg->model, seg->length);
                    for (int j=i+1; j<segments.size(); j++) 
                        segments[j]->start -= seg->length;
                    segments.erase(segments.begin() + i--);
                    buffer.erase(buffer.begin(), buffer.begin() + seg->length);
                }
            }

            // for (Segment* seg : segments) {
            //     std::cout << seg->start << " " << buffer[seg->start] << " " << seg->length << " --- ";
            // }
            // std::cout << "\n";

            for (int i=0; i<segments.size(); i++) {
                Segment* seg = segments[i];
                if (!seg->isComplete) break;

                // Fit because of condition 1
                // buffer.pop_back();
                while (seg->childTrigger && seg->triedDegree < 4) {
                    PolyModel* n_model = new PolyModel(seg->triedDegree + 1);
                    if (n_model->fit(bound, buffer, seg->start)) {
                        // std::cout << "fit ok " << i << " " << segments.size() << "\n";
                        seg->updateModel(n_model, buffer.size() - seg->start);
                        segments.erase(segments.begin() + i + 1, segments.end());
                        // std::cout << "fit ok " << segments.size() << "\n";
                    }

                    seg->updateCRTrigger();
                    seg->updateChildTrigger();
                    // std::cout << "child " << i << ": " << seg->triedDegree << " " << seg->model->getDegree() << " " << seg->childTrigger << "\n";
                }
                // buffer.push_back(data->get_value());

                // Fit success -> break
                if (!seg->isComplete) break;
                
                // Fit because of condition 2
                if (buffer.size() - seg->start > seg->nextTrigger && seg->triedDegree < 4) {
                    PolyModel* n_model = new PolyModel(seg->triedDegree + 1);
                    if (n_model->fit(bound, buffer, seg->start)) {
                        seg->updateModel(n_model, buffer.size() - seg->start);
                        segments.erase(segments.begin() + i + 1, segments.end());
                    }
                    
                    seg->updateCRTrigger();
                    // std::cout << "cr " << i << ": " << seg->triedDegree << " " << seg->model->getDegree() << " " << seg->nextTrigger << "\n";
                }
                // Fit success -> break
                if (!seg->isComplete) break;
            }
        }

        for (Segment* seg : segments) {
            __yield(compress_data, seg->model, seg->length);
        }

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
            unsigned short degree_length = compress_data->getShort();
            int degree = (degree_length >> 14) + 1;
            unsigned short length = degree_length & (0xffff >> 2);

            float* coefficients = new float[degree+1];
            for (int i = 0; i <= degree; i++) {
                coefficients[i] = compress_data->getFloat();
            }

            Polynomial polynomial(degree, coefficients);
            __decompress_segment(outputFile, interval, basetime, length, polynomial);
            delete[] coefficients;
            
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