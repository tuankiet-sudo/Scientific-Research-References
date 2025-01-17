#include "piecewise-approximation/linear.h"

namespace OptimalPLA {
    
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

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        int length = 0;
        time_t basetime = -1;
        UpperHull u_cvx; LowerHull l_cvx;
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
                u_cvx.append(Point2D(p1->x, p1->y-bound));
                l_cvx.append(Point2D(p1->x, p1->y+bound));
            }
            else if (length == 1) {
                p2 = new Point2D(1, p.y);
                u_line = Line::line(Point2D(p1->x, p1->y-bound), Point2D(p2->x, p2->y+bound));
                l_line = Line::line(Point2D(p1->x, p1->y+bound), Point2D(p2->x, p2->y-bound));
                u_cvx.append(Point2D(p2->x, p2->y-bound));
                l_cvx.append(Point2D(p2->x, p2->y+bound));
            }
            else {
                if (l_line->subs(p.x) > p.y + bound || p.y - bound > u_line->subs(p.x)) {
                    Line* l = new Line (
                        (u_line->get_slope() + l_line->get_slope()) / 2, 
                        (u_line->get_intercept() + l_line->get_intercept()) / 2
                    );
                    __yield(compress_data, basetime, length, l);

                    // std::cout << "length: " << length << "\n";
                    // std::cout << "line: " << l->get_slope() << " " << l->get_intercept() << "\n";
                    // std::cout << "---------------\n";
                    
                    delete p1; delete p2;
                    delete l; delete l_line; delete u_line;
                    u_cvx.clear(); l_cvx.clear();

                    length = 0;
                    basetime = data->get_time();
                    p1 = new Point2D(0, p.y);
                    u_cvx.append(Point2D(p1->x, p1->y-bound));
                    l_cvx.append(Point2D(p1->x, p1->y+bound));
                }
                else {
                    bool update_u = p.y + bound < u_line->subs(p.x);
                    bool update_l = p.y - bound > l_line->subs(p.x);

                    if (update_u) {
                        int index = 0;
                        float min_slp = INFINITY;

                        for (int i=0; i<u_cvx.size(); i++) {
                            Line* l = Line::line(u_cvx.at(i), Point2D(p.x, p.y+bound));
                            if (l->get_slope() < min_slp) {
                                min_slp = l->get_slope();
                                index = i;

                                delete u_line;
                                u_line = l;
                            }
                            else delete l;
                        }
                        u_cvx.erase_from_begin(index);
                    }
                    if (update_l) {
                        int index = 0;
                        float max_slp = -INFINITY;

                        for (int i=0; i<l_cvx.size(); i++) {
                            Line* l = Line::line(l_cvx.at(i), Point2D(p.x, p.y-bound));
                            if (l->get_slope() > max_slp) {
                                max_slp = l->get_slope();
                                index = i;
                                
                                delete l_line;
                                l_line = l;
                            }
                            else delete l;
                        }
                        l_cvx.erase_from_begin(index);
                    }

                    if (update_u) l_cvx.append(Point2D(p.x, p.y+bound));
                    if (update_l) u_cvx.append(Point2D(p.x, p.y-bound));
                }
            }

            length++;
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

        int count = 0;
        while (compress_data->getSize() != 0) {
            clock.start();

            time_t basetime = compress_data->getLong();
            int length = compress_data->getInt();
            float slope = compress_data->getFloat();
            float intercept = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
            
            clock.stop();
            count++;
        }

        std::cout << "number of segment " << count << "----\n";

        delete compress_data;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
    
};