#include "piecewise-approximation/linear.hpp"

namespace SwingFilter {

    Clock clock;

    void __yield(BinObj* obj, short length, Line& line) {
        obj->put(length);
        obj->put(line.get_slope());
        obj->put(line.get_intercept());
    }

    Line __fit(std::vector<Point2D>& segment, Line& u, Line& l, Point2D& p) {
        double A_num = 0, A_den = 0;

        for (Point2D& data : segment) {
            A_num += (data.y-p.y)*(data.x-p.x);
            A_den += (data.x-p.x)*(data.x-p.x);
        }

        double A_ig = A_num / A_den;
        double temp = A_ig > u.get_slope() ? u.get_slope() : A_ig;
        double a_ig = temp > l.get_slope() ? temp : l.get_slope();
        double b_ig = p.y - a_ig * p.x;

        return Line(a_ig, b_ig);
    }

    void compress(TimeSeries& timeseries, float bound, std::string output) {
        clock.start();
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        Univariate* d1 = (Univariate*) timeseries.next();
        compress_data->put(d1->get_time());
        Point2D p1(0, d1->get_value());

        Univariate* d2 = (Univariate*) timeseries.next();
        Point2D p2(1, d2->get_value());
        Line u_line = Line::line(p1, Point2D(p2.x, p2.y+bound));
        Line l_line = Line::line(p1, Point2D(p2.x, p2.y-bound));

        unsigned short length = 2;
        std::vector<Point2D> segment = {p1, p2};
        while (timeseries.hasNext()) {
            Point2D p(length, ((Univariate*) timeseries.next())->get_value());

            if (length > 65000 || l_line.subs(p.x) > p.y + bound || p.y - bound > u_line.subs(p.x)) {
                Line l = __fit(segment, u_line, l_line, p1);
                __yield(compress_data, length, l);

                p1 = Point2D(0, l.subs(length-1));
                p2 = Point2D(1, p.y);
                u_line = Line::line(p1, Point2D(p2.x, p2.y+bound));
                l_line = Line::line(p1, Point2D(p2.x, p2.y-bound));

                length = 2;
                segment = {p1, p2};
            }
            else {
                if (p.y + bound < u_line.subs(p.x)) {
                    u_line = Line::line(p1, Point2D(p.x, p.y+bound));
                }
                
                if (p.y - bound > l_line.subs(p.x)) {
                    l_line = Line::line(p1, Point2D(p.x, p.y-bound));
                }

                length++;
                segment.push_back(p);
            }
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

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept) {
        for (int i=0; i<length-1; i++) {
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
        clock.start();
        while (compress_data->getSize() != 0) {
            unsigned short length = compress_data->getShort();
            float slope = compress_data->getFloat();
            float intercept = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, slope, intercept);
            
            basetime += (length - 1) * interval;
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