#include "piecewise-approximation/linear.h"

namespace OptimalPLA {
    
    Clock clock;

    void _yield(BinObj* obj, int length, Line* line) {
        obj->put(length);
        obj->put(line->get_slope());
        obj->put(line->get_intercept());
    }

    void _approximate(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept, float u_slope, float u_intercept, float l_slope, float l_intercept) {
        for (int i=0; i<length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + interval*i));
            obj.pushData(std::to_string((basetime + interval*i)*slope + intercept));
            obj.pushData(std::to_string((basetime + interval*i)*u_slope + u_intercept));
            obj.pushData(std::to_string((basetime + interval*i)*l_slope + l_intercept));

            file.writeStr(&obj);
        }
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* obj = new BinObj;

        int length = 0;
        time_t time = -1;
        UpperConvexHull u_cvx;
        LowerConvexHull l_cvx;
        Point2D* p1 = nullptr;
        Point2D* p2 = nullptr;
        Line* u_line = nullptr;
        Line* l_line = nullptr;

        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (time == -1) {
                time = data->get_time();
                obj->put(time);
            }

            Point2D p(data->get_time() - time, data->get_value());
            Point2D l_p(p.x, p.y-bound);
            Point2D u_p(p.x, p.y+bound);

            if (length == 0) {
                p1 = new Point2D(p.x, p.y);
            }
            else if (length == 1) {
                p2 = new Point2D(p.x, p.y);
                Point2D u_p1(p1->x, p1->y+bound);
                Point2D l_p1(p1->x, p1->y-bound);
                Point2D u_p2(p2->x, p2->y+bound);
                Point2D l_p2(p2->x, p2->y-bound);

                u_line = Line::line(&l_p1, &u_p2);
                l_line = Line::line(&u_p1, &l_p2);
                u_cvx.append(l_p1); u_cvx.append(l_p2);
                l_cvx.append(u_p1); l_cvx.append(u_p2);
            }
            else {
                if (l_line->substitute(p.x) > p.y + bound || p.y - bound > u_line->substitute(p.x)) {
                    Line* l1 = Line::line(u_line->get_slope(), l_cvx.at(l_cvx.size()-1));
                    Line* l2 = Line::line(l_line->get_slope(), u_cvx.at(u_cvx.size()-1));
                    
                    Point2D* p0 = Line::intersection(l1, l2);
                    Line* l = Line::line((u_line->get_slope()+l_line->get_slope())/2, p0);
                    _yield(obj, length, l);
                    _yield(obj, length, l_line);
                    _yield(obj, length, u_line);
                    
                    delete p0; delete p1; delete p2;
                    delete l1; delete l2; delete l;
                    delete l_line; delete u_line;
                    u_cvx.clear();
                    l_cvx.clear();

                    length = 0;
                    p1 = new Point2D(p.x, p.y);
                }
                else {
                    bool update_u = p.y + bound < u_line->substitute(p.x);
                    bool update_l = p.y - bound > l_line->substitute(p.x);

                    if (update_u) {
                        int index = 0;
                        float min_slp = INFINITY;

                        for (int i=0; i<u_cvx.size(); i++) {
                            Point2D* q = u_cvx.at(i);
                            Line* l = Line::line(q, &u_p);
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
                            Point2D* q = l_cvx.at(i);
                            Line* l = Line::line(q, &l_p);
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

                    if (update_u) l_cvx.append(u_p);
                    if (update_l) u_cvx.append(l_p);
                }
            }

            length = length + 1;
            clock.stop();
        }

        outputFile.writeBin(obj);
        outputFile.close();
        delete obj;
        delete p1; delete p2;
        delete u_line; delete l_line;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }

    void decompress(std::string input, std::string output, int interval, float bound) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* r_obj = inputFile.readBin();

        // float prev = 0;
        // float now = 0;

        // int count = 0;
        // int seg = 0;

        // float prev_slope = -100;
        // int direct = -1;    // 0 is -a 1 is a

        time_t time = r_obj->getLong();
        while (r_obj->getSize() != 0) {
            clock.start();
            int length = r_obj->getInt();
            float slope = r_obj->getFloat();
            float intercept = r_obj->getFloat();
            int u_length = r_obj->getInt();
            float u_slope = r_obj->getFloat();
            float u_intercept = r_obj->getFloat();
            int l_length = r_obj->getInt();
            float l_slope = r_obj->getFloat();
            float l_intercept = r_obj->getFloat();
            
            _approximate(outputFile, interval, time, length, slope, intercept, u_slope, u_intercept, l_slope, l_intercept);
            
            std::cout << length << " " << slope << "x " << intercept << "\n";
            // now = (time-1)*slope + intercept;
            // if (direct == -1 && prev_slope != -100) {
            //     if (slope > prev_slope) direct = 1;
            //     else if (slope < prev_slope) direct = 0;
            // }

            // if (std::abs(now-prev) < 2*bound) {
            //     if (direct == 0 && slope < prev_slope) count++;
            //     else if (direct == 1 && slope > prev_slope) count++; 
            //     else direct = -1;
            // } 
            // else {
            //     direct = -1;
            // }
            
            // prev = (time+interval*length-1)*slope + intercept;
            // prev_slope = slope;
            // seg++;

            time += length * interval;
            clock.stop();
        }

        delete r_obj;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
    
};