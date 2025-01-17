#include "piecewise-approximation/polynomial.h"

namespace Algo {

    Clock clock;

    struct OptimalPLA {
        Point2D* p1;
        Point2D* pn;
        Line* u_line;
        Line* l_line;

        OptimalPLA(Point2D* p1, Point2D* p2, float bound) {
            this->pn = new Point2D(p2->x, p2->y);
            this->p1 = new Point2D(p1->x, p1->y);
            this->u_line = Line::line(Point2D(p1->x, p1->y), Point2D(p2->x, p2->y+bound));
            this->l_line = Line::line(Point2D(p1->x, p1->y), Point2D(p2->x, p2->y-bound));   
        }

        ~OptimalPLA() {
            delete this->p1;
            delete this->pn;
            delete this->u_line;
            delete this->l_line;
        }

        bool fit(Point2D* p, float bound) {
            if (this->l_line->subs(p->x) > p->y + bound || p->y - bound > this->u_line->subs(p->x)) {
                return true;
            }
            else {
                if (p->y + bound < this->u_line->subs(p->x)) {
                    delete this->u_line;
                    this->u_line = Line::line(*p1, Point2D(p->x, p->y+bound));
                }
                
                if (p->y - bound > this->l_line->subs(p->x)) {
                    delete this->l_line;
                    this->l_line = Line::line(*p1, Point2D(p->x, p->y-bound));
                }
            }

            delete this->pn;
            this->pn = new Point2D(p->x, p->y);
            return false;
        }
    };

    void __yield(BinObj* obj, time_t basetime, int length, float slope, float intercept) {
        obj->put(basetime);
        obj->put(length);
        obj->put(slope);
        obj->put(intercept);
    }

    void __yield(BinObj* obj, time_t basetime, int length, float a, float b, float c) {
        obj->put(basetime);
        obj->put(-length);
        obj->put(a);
        obj->put(b);
        obj->put(c);
    }

    bool is_intersect(float u1, float l1, float u2, float l2) {
        if (l1 > u1 || l2 > u2 || l1 > u2 || l2 > u1) {
            return false;
        }

        return true;
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        time_t basetime = -1;
        Point2D* p1 = nullptr; Point2D* p2 = nullptr;

        int direction = 0;
        int length = 0;
        OptimalPLA* pla = nullptr;
        float a = 0, b = 0, c = 0;
        float a_u = INFINITY, a_l = -INFINITY;

        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            Point2D p(length, data->get_value());

            if (p1 == nullptr) {
                p1 = new Point2D(0, p.y);
            }
            else if (p2 == nullptr) {
                p2 = new Point2D(1, p.y);
                pla = new OptimalPLA(p1, p2, bound);
            }
            else if (direction == 0) {
                if (pla->fit(&p, bound)) {
                    
                    direction = 1;
                    if (p.y - bound > pla->u_line->subs(p.x)) {
                        // a > 0
                        b = pla->l_line->get_slope();
                        c = pla->l_line->get_intercept();

                        float n_a_u = (pla->u_line->subs(pla->pn->x) - b * pla->pn->x - c) / (pla->pn->x * pla->pn->x);
                        a_u = a_u > n_a_u ? n_a_u : a_u;
                        // a_l = 0;
                    }
                    else if (pla->l_line->subs(p.x) > p.y + bound) {
                        // a < 0
                        b = pla->u_line->get_slope();
                        c = pla->u_line->get_intercept();

                        float n_a_l = (pla->l_line->subs(pla->pn->x) - b * pla->pn->x - c) / (pla->pn->x * pla->pn->x);
                        a_l = a_l < n_a_l ? n_a_l : a_l;
                        // a_u = 0;
                    }

                }
            }

            if (direction != 0) {
                float n_a_u = (p.y + bound - b * p.x - c) / (p.x * p.x);
                float n_a_l = (p.y - bound - b * p.x - c) / (p.x * p.x);

                if (is_intersect(a_u, a_l, n_a_u, n_a_l)) {
                    a_u = a_u > n_a_u ? n_a_u : a_u;
                    a_l = a_l < n_a_l ? n_a_l : a_l;
                    a = (a_u + a_l) / 2;
                }
                else {
                    if (a_u == INFINITY || a_l == -INFINITY) {
                        __yield(compress_data, basetime, length, b, c);  
                    }
                    else {
                        __yield(compress_data, basetime, length, a, b, c);  
                    }
                    
                    delete pla; delete p1; delete p2;
                    length = 0;
                    direction = 0;
                    a_u = INFINITY; a_l = -INFINITY;
                    p1 = new Point2D(0, p.y);
                    p2 = nullptr;
                    basetime = data->get_time();
                }
                
            }

            length++;
            clock.stop();
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;
        delete p1; delete p2; delete pla;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(slope * i + intercept));
            file.writeStr(&obj);
        }
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float a, float b, float c) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(a * i * i + b * i + c));
            file.writeStr(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        int count_q = 0;
        int count_l = 0;
        while (compress_data->getSize() != 0) {
            clock.start();

            time_t basetime = compress_data->getLong();
            int length = compress_data->getInt();

            if (length > 0) {
                float slope = compress_data->getFloat();
                float intercept = compress_data->getFloat();
                __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
                count_l++;
            }
            else {
                float a = compress_data->getFloat();
                float b = compress_data->getFloat();
                float c = compress_data->getFloat();
                __decompress_segment(outputFile, interval, basetime, -length, a, b, c);
                count_q++;
            }
            
            clock.stop();
        }

        std::cout << count_l << " " << count_q << "----\n";

        delete compress_data;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
};