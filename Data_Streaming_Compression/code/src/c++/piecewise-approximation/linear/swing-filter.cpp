#include "piecewise-approximation/linear.h"

namespace SwingFilter {

    Clock clock;

    void __yield(BinObj* obj, time_t basetime, int length, Line* line) {
        obj->put(basetime);
        obj->put(length);
        obj->put(line->get_slope());
        obj->put(line->get_intercept());
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept) {
        for (int i=0; i<length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + interval * i));
            obj.pushData(std::to_string(i * slope + intercept));
            file.writeStr(&obj);
        }
    }

    Line* __fit(std::vector<Point2D>& segment, Line* u, Line* l, Point2D* p) {
        double A_num = 0, A_den = 0;

        for (Point2D& data : segment) {
            A_num += (data.y-p->y)*(data.x-p->x);
            A_den += (data.x-p->x)*(data.x-p->x);
        }

        double A_ig = A_num / A_den;
        double temp = A_ig > u->get_slope() ? u->get_slope() : A_ig;
        double a_ig = temp > l->get_slope() ? temp : l->get_slope();
        double b_ig = p->y - a_ig * p->x;

        return new Line(a_ig, b_ig);
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        int length = 0;
        time_t basetime = -1;
        std::vector<Point2D> segment;
        Point2D* p1 = nullptr; Point2D* p2 = nullptr;
        Line* u_line = nullptr; Line* l_line = nullptr;

        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            Point2D p(length, data->get_value());

            if (length == 0) {
                p1 = new Point2D(0, p.y);
            }
            else if (length == 1) {
                p2 = new Point2D(1, p.y);
                u_line = Line::line(*p1, Point2D(p2->x, p2->y+bound));
                l_line = Line::line(*p1, Point2D(p2->x, p2->y-bound));
            }
            else {
                if (l_line->subs(p.x) > p.y + bound || p.y - bound > u_line->subs(p.x)) {
                    Line* l = __fit(segment, u_line, l_line, p1);
                    __yield(compress_data, basetime, length, l);
                    
                    delete p1; delete p2;
                    delete l; delete l_line; delete u_line;
                    segment.clear();

                    length = 0;
                    basetime = data->get_time();
                    p1 = new Point2D(0, p.y);
                    p.x = 0;
                }
                else {
                    if (p.y + bound < u_line->subs(p.x)) {
                        delete u_line;
                        u_line = Line::line(*p1, Point2D(p.x, p.y+bound));
                    }
                    
                    if (p.y - bound > l_line->subs(p.x)) {
                        delete l_line;
                        l_line = Line::line(*p1, Point2D(p.x, p.y-bound));
                    }
                }
            }

            length++;
            segment.push_back(p);
            clock.stop();
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;
        delete p1; delete p2;
        delete u_line; delete l_line;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        while (compress_data->getSize() != 0) {
            clock.start();

            time_t basetime = compress_data->getLong();
            int length = compress_data->getInt();
            float slope = compress_data->getFloat();
            float intercept = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
            
            clock.stop();
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }

};