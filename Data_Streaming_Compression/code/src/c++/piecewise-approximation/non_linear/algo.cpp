#include "piecewise-approximation/polynomial.h"

namespace Algo {

    Clock clock;

    struct OptimalPLA {
        int length = 0;
        Line* u_line;
        Line* l_line;
        UpperHull u_cvx;
        LowerHull l_cvx;

        OptimalPLA(Point2D* p1, Point2D* p2, float bound) {
            this->length = 2;
            this->u_line = Line::line(Point2D(p1->x, p1->y-bound), Point2D(p2->x, p2->y+bound));
            this->l_line = Line::line(Point2D(p1->x, p1->y+bound), Point2D(p2->x, p2->y-bound));

            this->u_cvx.append(Point2D(p1->x, p1->y-bound));
            this->l_cvx.append(Point2D(p1->x, p1->y+bound));
            this->u_cvx.append(Point2D(p2->x, p2->y-bound));
            this->l_cvx.append(Point2D(p2->x, p2->y+bound));    
        }

        ~OptimalPLA() {
            delete this->u_line;
            delete this->l_line;

            this->u_cvx.clear();
            this->l_cvx.clear();
        }

        bool fit(Point2D* p, float bound) {
            if (this->l_line->subs(p->x) > p->y + bound || p->y - bound > this->u_line->subs(p->x)) {
                return true;
            }
            else {
                bool update_u = p->y + bound < this->u_line->subs(p->x);
                bool update_l = p->y - bound > this->l_line->subs(p->x);

                if (update_u) {
                    int index = 0;
                    float min_slp = INFINITY;

                    for (int i=0; i<this->u_cvx.size(); i++) {
                        Line* l = Line::line(this->u_cvx.at(i), Point2D(p->x, p->y+bound));
                        if (l->get_slope() < min_slp) {
                            min_slp = l->get_slope();
                            index = i;

                            delete this->u_line;
                            this->u_line = l;
                        }
                        else delete l;
                    }
                    this->u_cvx.erase_from_begin(index);
                }
                if (update_l) {
                    int index = 0;
                    float max_slp = -INFINITY;

                    for (int i=0; i<this->l_cvx.size(); i++) {
                        Line* l = Line::line(this->l_cvx.at(i), Point2D(p->x, p->y-bound));
                        if (l->get_slope() > max_slp) {
                            max_slp = l->get_slope();
                            index = i;
                            
                            delete this->l_line;
                            this->l_line = l;
                        }
                        else delete l;
                    }
                    this->l_cvx.erase_from_begin(index);
                }

                if (update_u) this->l_cvx.append(Point2D(p->x, p->y+bound));
                if (update_l) this->u_cvx.append(Point2D(p->x, p->y-bound));
            }
        
            this->length++;
            return false;
        }
    };

    void __yield(BinObj* obj, time_t basetime, int length, Line* line) {
        obj->put(basetime);
        obj->put(length);
        obj->put(line->get_slope());
        obj->put(line->get_intercept());
    }

    void __yield(BinObj* obj, time_t basetime, int length, float a, float b, float c) {
        obj->put(basetime);
        obj->put(-length);
        obj->put(a);
        obj->put(b);
        obj->put(c);
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        time_t basetime = -1;
        Point2D* p1 = nullptr; Point2D* p2 = nullptr;

        OptimalPLA* pla = nullptr;
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            Point2D p(data->get_time() - basetime, data->get_value());

            if (p1 == nullptr) {
                p1 = new Point2D(0, p.y);
            }
            else if (p2 == nullptr) {
                p2 = new Point2D(1, p.y);
                pla = new OptimalPLA(p1, p2, bound);
            }
            else {
                if (pla->fit(&p, bound)) {
                    Line* l = new Line (
                        (pla->u_line->get_slope() + pla->l_line->get_slope()) / 2, 
                        (pla->u_line->get_intercept() + pla->l_line->get_intercept()) / 2
                    );
                    __yield(compress_data, basetime, pla->length, l);

                    delete pla; delete l;
                    delete p1; delete p2;
                    p1 = new Point2D(0, p.y);
                    p2 = nullptr;
                    basetime = data->get_time();
                }
            }

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

        while (compress_data->getSize() != 0) {
            clock.start();

            time_t basetime = compress_data->getLong();
            int length = compress_data->getInt();

            if (length > 0) {
                float slope = compress_data->getFloat();
                float intercept = compress_data->getFloat();
                __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
            }
            else {
                float a = compress_data->getFloat();
                float b = compress_data->getFloat();
                float c = compress_data->getFloat();
                __decompress_segment(outputFile, interval, basetime, -length, a, b, c);
            }
            
            clock.stop();
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
};