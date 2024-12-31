#include "piecewise-approximation/linear.h"

namespace MixPiece {

    struct Interval {
        float a_u;
        float a_l;
        time_t t;
        int length;

        Interval(float a_u, float a_l, int length, time_t t) {
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
            this->length = length;
        }
    };

    struct B_Block {

        struct Block {
            float a_u = INFINITY;
            float a_l = -INFINITY;
            std::vector<time_t> t;
            std::vector<int> n;

            Block(float a_u, float a_l, time_t t, int n) {
                this->a_u = a_u;
                this->a_l = a_l;
                this->t.push_back(t);
                this->n.push_back(n);
            }
        };

        float b;
        std::vector<Block> blocks;

        B_Block(float b) {
            this->b = b;
        }

        B_Block(float b, float a_u, float a_l, time_t t, int n) {
            this->b = b;
            this->blocks.push_back(Block(a_u, a_l, t, n));
        }

        bool is_intersect(float a_u, float a_l) {
            return a_l <= this->blocks.back().a_u && a_u >= this->blocks.back().a_l;
        } 

        void intersect(float a_u, float a_l, time_t t, int n) {
            if (this->blocks.back().a_u > a_u) this->blocks.back().a_u = a_u;
            if (this->blocks.back().a_l < a_l) this->blocks.back().a_l = a_l;

            this->blocks.back().t.push_back(t);
            this->blocks.back().n.push_back(n);
        }
    };

    struct A_Block {

        struct Block {
            float b;
            time_t t;
            int n;

            Block(float b, time_t t, int n) {
                this->b = b;
                this->t = t;
                this->n = n;
            }
        };

        float a_u;
        float a_l;
        std::vector<Block> blocks;

        A_Block() {
            this->a_u = INFINITY;
            this->a_l = -INFINITY;
        }

        A_Block(float b, float a_u, float a_l, time_t t, int n) {
            this->a_u = a_u;
            this->a_l = a_l;

            this->blocks.push_back(Block(b, t, n));
        }

        bool is_intersect(float a_u, float a_l) {
            return a_l <= this->a_u && a_u >= this->a_l;
        } 

        void intersect(float b, float a_u, float a_l, time_t t, int n) {
            if (this->a_u > a_u) this->a_u = a_u;
            if (this->a_l < a_l) this->a_l = a_l;

            this->blocks.push_back(Block(b, t, n));
        }
    };

    struct R_Block {
        float a_u;
        float a_l;
        float b;
        time_t t;
        int n;

        R_Block(float b, float a_u, float a_l, time_t t, int n) {
            this->b = b;
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
            this->n = n;
        }

        float a() {
            return (a_u + a_l) / 2;
        }
    };

    Clock clock;

    void __yield(BinObj* obj, std::vector<B_Block> b_blocks) {
        obj->put((int) b_blocks.size());
        for (B_Block b_block : b_blocks) {
            float b = b_block.b;
            obj->put(b);
            obj->put((int) b_block.blocks.size());

            for (B_Block::Block block : b_block.blocks) {
                float a = (block.a_l + block.a_u) / 2;
                obj->put(a);
                obj->put((int) block.t.size());

                for (int i=0; i<block.t.size(); i++) {
                    obj->put(block.t[i]);
                    obj->put(block.n[i]);
                }
            }
        }
    }

    void __yield(BinObj* obj, std::vector<A_Block> a_blocks) {
        obj->put((int) a_blocks.size());
        for (A_Block a_block : a_blocks) {
            float a = (a_block.a_u + a_block.a_l) / 2;
            obj->put(a);
            obj->put((int) a_block.blocks.size());

            for (A_Block::Block block : a_block.blocks) {
                obj->put(block.b);
                obj->put(block.t);
                obj->put(block.n);
            }
        }
    }

    void __yield(BinObj* obj, std::vector<R_Block> r_blocks) {
        obj->put((int) r_blocks.size());
        for (R_Block r_block : r_blocks) {
            obj->put(r_block.t);
            obj->put(r_block.n);
            obj->put(r_block.a());
            obj->put(r_block.b);
        }
    }

    void __group(BinObj* compress_data, std::map<float, std::vector<Interval>>& b_intervals) {
        std::vector<B_Block> b_blocks;
        std::vector<A_Block> a_blocks;
        std::vector<R_Block> r_blocks;

        std::vector<std::pair<float, Interval>> ungrouped;
        for (std::pair<float, std::vector<Interval>> it : b_intervals) {
            float b = it.first;
            std::vector<Interval> intervals = it.second;
            std::sort(intervals.begin(), intervals.end(), 
                [](const Interval& a, const Interval& b){ return a.a_l < b.a_l; });

            B_Block group(b);
            for (Interval& interval : intervals) {
                if (group.is_intersect(interval.a_u, interval.a_l)) {
                    group.intersect(interval.a_u, interval.a_l, interval.t, interval.length);
                }
                else if (group.blocks.back().t.size() > 1) {
                    b_blocks.push_back(group);
                    group = B_Block(b, interval.a_u, interval.a_l, interval.t, interval.length);
                }
                else {
                    ungrouped.push_back(std::make_pair(b, Interval(interval.a_u, interval.a_l, interval.length, interval.t)));
                    group = B_Block(b, interval.a_u, interval.a_l, interval.t, interval.length);
                }
            }

            if (group.blocks.back().t.size() > 1) {
                b_blocks.push_back(group);
            }
            else {
                ungrouped.push_back(std::make_pair(b, Interval(
                    group.blocks.back().a_u, group.blocks.back().a_l, 
                    group.blocks.back().n[0], group.blocks.back().t[0]))
                );
            }
        }

        std::sort(ungrouped.begin(), ungrouped.end(), 
            [](const std::pair<float, Interval>& a, const std::pair<float, Interval>& b)
            { return a.second.a_l < b.second.a_l; });

        A_Block group;
        for (std::pair<float, Interval>& entry : ungrouped) {
            if (group.is_intersect(entry.second.a_u, entry.second.a_l)) {
                group.intersect(entry.first, entry.second.a_u, entry.second.a_l, 
                    entry.second.t, entry.second.length);
            }
            else if (group.blocks.size() > 1) {
                a_blocks.push_back(group);
                group = A_Block(entry.first, entry.second.a_u, entry.second.a_l, 
                    entry.second.t, entry.second.length); 
            }
            else {
                r_blocks.push_back(R_Block(group.blocks[0].b, group.a_u, group.a_l, 
                    group.blocks[0].t, group.blocks[0].n));
                group = A_Block(entry.first, entry.second.a_u, entry.second.a_l, 
                    entry.second.t, entry.second.length); 
            }
        }

        if (group.blocks.size() > 1) {
            a_blocks.push_back(group);
        }
        else {
            r_blocks.push_back(R_Block(group.blocks[0].b, group.a_u, 
                group.a_l, group.blocks[0].t, group.blocks[0].n));
        }

        __yield(compress_data, b_blocks);
        __yield(compress_data, a_blocks);
        __yield(compress_data, r_blocks);
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