#include "piecewise-approximation/linear.h"

namespace MixPiece {

    struct Interval {
        float a_u;
        float a_l;
        int length;
        time_t t;

        Interval(float a_u, float a_l, int length, time_t t) {
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
            this->length = length;
        }
    };

    struct GroupedB {
        float b;
        float a_u;
        float a_l;
        std::vector<time_t> t;
        std::vector<int> length;

        GroupedB(float b) {
            this->b = b;
            this->a_u = INFINITY;
            this->a_l = -INFINITY;
        }

        GroupedB(float b, float a_u, float a_l, time_t t, int length) {
            this->b = b;
            this->a_l = a_l;
            this->a_u = a_u;
            this->t.push_back(t);
            this->length.push_back(length);
        }
    };

    struct GroupedA {
        float a_u;
        float a_l;
        std::vector<float> b;
        std::vector<time_t> t;
        std::vector<int> length;

        GroupedA() {
            this->a_u = INFINITY;
            this->a_l = -INFINITY;
        }

        GroupedA(float b, float a_u, float a_l, time_t t, int length) {
            this->a_l = a_l;
            this->a_u = a_u;
            this->b.push_back(b);
            this->t.push_back(t);
            this->length.push_back(length);
        }
    };

    struct Rest {
        float a_u;
        float a_l;
        float b;
        time_t t;
        int length;

        Rest(float b, float a_u, float a_l, time_t t, int length) {
            this->b = b;
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
            this->length = length;
        }
    };

    Clock clock;

    void __yield(BinObj* obj, std::vector<GroupedB> groups_b) {
        obj->put((int)groups_b.size());
        float b = groups_b[0].b;
        int count = 0;

        BinObj record;
        for (GroupedB group_b : groups_b) {
            if (b == group_b.b) {

            }
            else {
                count = 0;
                record = BinObj();
            }

            count++;
        }
    }

    void __yield(BinObj* obj, std::vector<GroupedA> groups_a) {
        obj->put((int)groups_a.size());
    }

    void __yield(BinObj* obj, std::vector<Rest> rest) {
        obj->put((int)rest.size());
    }

    void __group(BinObj* compress_data, std::map<float, std::vector<Interval>>& b_intervals) {
        std::vector<GroupedB> groups_b;
        std::vector<GroupedA> groups_a;
        std::vector<Rest> rests;

        std::vector<GroupedB> ungrouped;
        for (std::pair<float, std::vector<Interval>> it : b_intervals) {
            float b = it.first;
            std::vector<Interval> intervals = it.second;
            std::sort(intervals.begin(), intervals.end(), 
                [](const Interval& a, const Interval& b){ return a.a_l < b.a_l; });

            GroupedB group(b);
            for (Interval& interval : intervals) {
                if (interval.a_l <= group.a_u && interval.a_u >= group.a_l) {
                    group.a_u = group.a_u > interval.a_u ? interval.a_u : group.a_u;
                    group.a_l = group.a_l < interval.a_l ? interval.a_l : group.a_l;
                    group.length.push_back(interval.length);
                    group.t.push_back(interval.t);
                }
                else if (group.t.size() > 1) {
                    groups_b.push_back(group);
                    group = GroupedB(b, interval.a_u, interval.a_l, interval.t, interval.length);
                }
                else {
                    ungrouped.push_back(group);
                    group = GroupedB(b, interval.a_u, interval.a_l, interval.t, interval.length);
                }
            }

            if (group.t.size() > 1) {
                groups_b.push_back(group);
            }
            else {
                ungrouped.push_back(group);
            }
        }

        std::sort(ungrouped.begin(), ungrouped.end(), 
            [](const GroupedB& a, const GroupedB& b){ return a.a_l < b.a_l; });

        GroupedA group;
        for (GroupedB interval : ungrouped) {
            if (interval.a_l <= group.a_l && interval.a_u >= group.a_l) {
                group.a_u = group.a_u > interval.a_u ? interval.a_u : group.a_u;
                group.a_l = group.a_l < interval.a_l ? interval.a_l : group.a_l;
                
                group.b.push_back(interval.b);
                group.t.push_back(interval.t[0]);
                group.length.push_back(interval.length[0]);
            }
            else if (group.t.size() > 1) {
                groups_a.push_back(group);
                group = GroupedA(interval.b, interval.a_u, interval.a_l, interval.t[0], interval.length[0]); 
            }
            else {
                rests.push_back(Rest(group.b[0], group.a_u, group.a_l, group.t[0], group.length[0]));
                group = GroupedA(interval.b, interval.a_u, interval.a_l, interval.t[0], interval.length[0]); 
            }
        }

        if (group.t.size() > 1) {
            groups_a.push_back(group);
        }
        else {
            rests.push_back(Rest(group.b[0], group.a_u, group.a_l, group.t[0], group.length[0]));
        }

        __yield(compress_data, groups_b);
        __yield(compress_data, groups_a);
        __yield(compress_data, rests);
    }
    
    void compress(TimeSeries& timeseries, int n_segment, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        int count = 0;
        time_t basetime = -1;
        float b_1; float b_2;
        int length = 0; int flag = 0;
        bool floor_flag = true; bool ceil_flag = true;
        float slp_u_1 = INFINITY; float slp_u_2 = INFINITY;
        float slp_l_1 = -INFINITY; float slp_l_2 = -INFINITY;
        
        std::map<float, std::vector<Interval>> b_intervals;
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            Point2D p(length, data->get_value());

            if (length == 0) {
                b_1 = floor(p.y / bound) * bound;
                b_2 = ceil(p.y / bound) * bound;
            }
            else {
                if (p.y > slp_u_1*(p.x)+b_1+bound || p.y < slp_l_1*(p.x)+b_1-bound) {
                    floor_flag = false;
                }
                if (p.y > slp_u_2*(p.x)+b_2+bound || p.y < slp_l_2*(p.x)+b_2-bound) {
                    ceil_flag = false;
                }
                if (floor_flag) flag++;
                if (ceil_flag) flag--;

                if (!floor_flag and !ceil_flag) {
                    if (flag > 0) {
                        std::vector<Interval> vec = b_intervals.at(b_1);
                        vec.push_back(Interval(slp_u_1, slp_l_1, length, basetime));
                        b_intervals[b_1] = vec;
                    }
                    else {
                        std::vector<Interval> vec = b_intervals.at(b_2);
                        vec.push_back(Interval(slp_u_2, slp_l_2, length, basetime));
                        b_intervals[b_2] = vec;
                    }

                    count++;
                    length = 0; flag = 0;
                    floor_flag = true; ceil_flag = true;
                    slp_u_1 = INFINITY; slp_u_2 = INFINITY;
                    slp_l_1 = -INFINITY; slp_l_2 = -INFINITY;
                }

                if (p.y < slp_u_1 * p.x + b_1 - bound) {
                    slp_u_1 = (p.y + bound - b_1) / p.x;
                }
                if (p.y > slp_l_1 * p.x + b_1 + bound) {
                    slp_l_1 = (p.y - bound - b_1) / p.x;
                }
                if (p.y < slp_u_2 * p.x + b_2 - bound) {
                    slp_u_2 = (p.y + bound - b_2) / p.x;
                }
                if (p.y > slp_l_2 * p.x + b_2 + bound) {
                    slp_l_2 = (p.y - bound - b_2) / p.x;
                }

                if (count == n_segment) {
                    count = 0;
                    __group(compress_data, b_intervals);
                    b_intervals.clear();
                }
            }

            clock.stop();
        }

        if (!floor_flag and !ceil_flag) {
            if (flag > 0) {
                std::vector<Interval> vec = b_intervals.at(b_1);
                vec.push_back(Interval(slp_u_1, slp_l_1, length, basetime));
                b_intervals[b_1] = vec;
            }
            else {
                std::vector<Interval> vec = b_intervals.at(b_2);
                vec.push_back(Interval(slp_u_2, slp_l_2, length, basetime));
                b_intervals[b_2] = vec;
            }

            __group(compress_data, b_intervals);
            b_intervals.clear();
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }


    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float slope, float intercept) {
        for (int i=0; i<length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + interval * i));
            obj.pushData(std::to_string(i * slope + intercept));
            file.writeStr(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        while (compress_data->getSize() != 0) {
            clock.start();

            // Decompress part 1
            int b_blocks = compress_data->getInt();
            while (b_blocks-- > 0) {
                float b = compress_data->getFloat();
                int a_blocks = compress_data->getInt();
                while (a_blocks-- > 0) {
                    float a = compress_data->getFloat();
                    int blocks = compress_data->getInt();
                    while (blocks-- > 0) {
                        time_t basetime = compress_data->getLong();
                        int length = compress_data->getInt();
                        __decompress_segment(outputFile, interval, basetime, length, a, b);
                    }
                }
            }

            // Decompress part 2
            int a_blocks = compress_data->getInt();
            while (a_blocks-- > 0) {
                float a = compress_data->getFloat();
                int blocks = compress_data->getInt();
                while (blocks-- > 0) {
                    float b = compress_data->getFloat();
                    time_t basetime = compress_data->getLong();
                    int length = compress_data->getInt();
                    __decompress_segment(outputFile, interval, basetime, length, a, b);
                }
            }

            // Decompress part 3
            int blocks = compress_data->getInt();
            while (blocks-- > 0) {
                float a = compress_data->getFloat();
                float b = compress_data->getFloat();
                time_t basetime = compress_data->getLong();
                int length = compress_data->getInt();
                __decompress_segment(outputFile, interval, basetime, length, a, b);
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