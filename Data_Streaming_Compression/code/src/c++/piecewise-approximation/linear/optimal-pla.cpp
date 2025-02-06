#include "piecewise-approximation/linear.h"

namespace OptimalPLA {
    
    Clock clock;

    void __yield(BinObj* obj, short length, float slope, float intercept) {
        obj->put(length);
        obj->put(slope);
        obj->put(intercept);
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        Line u_line(0, 0); Line l_line(0, 0);
        UpperHull u_cvx; LowerHull l_cvx;

        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        Point2D p1(0, d->get_value());
        u_cvx.append(Point2D(0, p1.y-bound));
        l_cvx.append(Point2D(0, p1.y+bound));
        
        unsigned short length = 1;
        clock.start();
        while (timeseries.hasNext()) {
            Point2D p(length, ((Univariate*) timeseries.next())->get_value());

            if (length == 1) {
                u_line = Line::line(Point2D(p1.x, p1.y-bound), Point2D(1, p.y+bound));
                l_line = Line::line(Point2D(p1.x, p1.y+bound), Point2D(1, p.y-bound));
                u_cvx.append(Point2D(1, p.y-bound));
                l_cvx.append(Point2D(1, p.y+bound));
            }
            else {
                if (length > 65000 || l_line.subs(p.x) > p.y + bound || p.y - bound > u_line.subs(p.x)) {
                    std::cout << u_line.get_slope() << " " << l_line.get_slope() << "\n";
                    __yield(compress_data, length, (u_line.get_slope()+l_line.get_slope())/2, (u_line.get_intercept()+l_line.get_intercept())/2);

                    p1 = Point2D(0, p.y);
                    u_cvx.clear(); l_cvx.clear();
                    u_cvx.append(Point2D(0, p1.y-bound));
                    l_cvx.append(Point2D(0, p1.y+bound));
                    length = 0;
                }
                else {
                    bool update_u = p.y + bound < u_line.subs(p.x);
                    bool update_l = p.y - bound > l_line.subs(p.x);

                    if (update_u) {
                        int index = 0;
                        float min_slp = INFINITY;

                        for (int i=0; i<u_cvx.size(); i++) {
                            Line l = Line::line(u_cvx.at(i), Point2D(p.x, p.y+bound));
                            if (l.get_slope() < min_slp) {
                                min_slp = l.get_slope();
                                index = i;
                                u_line = Line(l.get_slope(), l.get_intercept());
                            }
                        }
                        u_cvx.erase_from_begin(index);
                    }
                    if (update_l) {
                        int index = 0;
                        float max_slp = -INFINITY;

                        for (int i=0; i<l_cvx.size(); i++) {
                            Line l = Line::line(l_cvx.at(i), Point2D(p.x, p.y-bound));
                            if (l.get_slope() > max_slp) {
                                max_slp = l.get_slope();
                                index = i;
                                l_line = Line(l.get_slope(), l.get_intercept());
                            }
                        }
                        l_cvx.erase_from_begin(index);
                    }

                    if (update_u) l_cvx.append(Point2D(p.x, p.y+bound));
                    if (update_l) u_cvx.append(Point2D(p.x, p.y-bound));
                }
            }

            length++;
            clock.tick();
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;

        // Profile average latency
        std::cout << "Time taken for each data point (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept) {
        for (int i=0; i<length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + interval * i));
            obj.pushData(std::to_string(i * slope + intercept));
            file.write(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        time_t basetime = compress_data->getLong();
        while (compress_data->getSize() != 0) {
            clock.start();
            
            unsigned short length = compress_data->getShort();
            float slope = compress_data->getFloat();
            float intercept = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
            
            basetime += length * interval;
            clock.stop();
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();

        // Profile average latency
        std::cout << "Time taken for each segment (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each segment (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }
    
};