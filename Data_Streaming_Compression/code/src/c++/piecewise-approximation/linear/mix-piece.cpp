#include <map>
#include <algorithm>

#include "piecewise-approximation/linear.h"

namespace MixPiece {

    struct Interval {
        float a_u;
        float a_l;
        unsigned int t;

        Interval(float a_u, float a_l, int t) {
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
        }
    };

    struct B_Block {

        struct Block {
            float a_u;
            float a_l;
            std::vector<unsigned int> t;

            Block() {
                this->a_u = INFINITY;
                this->a_l = -INFINITY;
            }

            Block(float a_u, float a_l, int t) {
                this->a_u = a_u;
                this->a_l = a_l;
                this->t.push_back(t);
            }
        };

        float b;
        std::vector<Block> blocks;

        B_Block(float b) {
            this->b = b;
            this->blocks.push_back(Block());
        }

        B_Block(float b, float a_u, float a_l, int t) {
            this->b = b;
            this->blocks.push_back(Block(a_u, a_l, t));
        }

        bool is_intersect(float a_u, float a_l) {
            return a_l <= this->blocks.back().a_u && a_u >= this->blocks.back().a_l;
        }

        void intersect(float a_u, float a_l, int t) {
            if (this->blocks.back().a_u > a_u) this->blocks.back().a_u = a_u;
            if (this->blocks.back().a_l < a_l) this->blocks.back().a_l = a_l;

            this->blocks.back().t.push_back(t);
        }
    };

    struct A_Block {

        struct Block {
            float b;
            unsigned int t;

            Block(float b, int t) {
                this->b = b;
                this->t = t;
            }
        };

        float a_u;
        float a_l;
        std::vector<Block> blocks;

        A_Block() {
            this->a_u = INFINITY;
            this->a_l = -INFINITY;
        }

        A_Block(float b, float a_u, float a_l, int t) {
            this->a_u = a_u;
            this->a_l = a_l;

            this->blocks.push_back(Block(b, t));
        }

        bool is_intersect(float a_u, float a_l) {
            return a_l <= this->a_u && a_u >= this->a_l;
        } 

        void intersect(float b, float a_u, float a_l, int t) {
            if (this->a_u > a_u) this->a_u = a_u;
            if (this->a_l < a_l) this->a_l = a_l;

            this->blocks.push_back(Block(b, t));
        }
    };

    struct R_Block {
        float a_u;
        float a_l;
        float b;
        unsigned int t;

        R_Block(float b, float a_u, float a_l, int t) {
            this->b = b;
            this->a_u = a_u;
            this->a_l = a_l;
            this->t = t;
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
                    obj->put((int) block.t[i]);
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
                obj->put((int) block.t);
            }
        }
    }

    void __yield(BinObj* obj, std::vector<R_Block> r_blocks) {
        obj->put((int) r_blocks.size());
        for (R_Block block : r_blocks) {
            obj->put(block.a());
            obj->put(block.b);
            obj->put((int) block.t);
        }
    }

    void __group(BinObj* compress_data, std::map<float, std::vector<Interval>>& b_intervals) {
        std::vector<B_Block> b_blocks;
        std::vector<A_Block> a_blocks;
        std::vector<R_Block> r_blocks;

        std::vector<std::pair<float, Interval>> ungrouped;
        for (auto it = b_intervals.begin(); it != b_intervals.end(); it++) {
            float b = it->first;
            
            std::vector<Interval> intervals = it->second;
            std::sort(intervals.begin(), intervals.end(), 
                [](const Interval& a, const Interval& b){ return a.a_l < b.a_l; });

            B_Block group(b);
            for (Interval& interval : intervals) {
                if (group.is_intersect(interval.a_u, interval.a_l)) {
                    group.intersect(interval.a_u, interval.a_l, interval.t);
                }
                else if (group.blocks.back().t.size() > 1) {
                    b_blocks.push_back(group);
                    group = B_Block(b, interval.a_u, interval.a_l, interval.t);
                }
                else {
                    ungrouped.push_back(std::make_pair(b, Interval(
                        group.blocks[0].a_u, group.blocks[0].a_l, group.blocks[0].t[0])
                    ));
                    group = B_Block(b, interval.a_u, interval.a_l, interval.t);
                }
            }

            if (group.blocks.back().t.size() > 1) {
                b_blocks.push_back(group);
            }
            else {
                ungrouped.push_back(std::make_pair(b, Interval(
                    group.blocks.back().a_u, group.blocks.back().a_l, group.blocks.back().t[0]))
                );
            }
        }

        std::sort(ungrouped.begin(), ungrouped.end(), 
            [](const std::pair<float, Interval>& a, const std::pair<float, Interval>& b)
            { return a.second.a_l < b.second.a_l; });

        A_Block group;
        for (std::pair<float, Interval>& entry : ungrouped) {
            if (group.is_intersect(entry.second.a_u, entry.second.a_l)) {
                group.intersect(entry.first, entry.second.a_u, entry.second.a_l, entry.second.t);
            }
            else if (group.blocks.size() > 1) {
                a_blocks.push_back(group);
                group = A_Block(entry.first, entry.second.a_u, entry.second.a_l, entry.second.t); 
            }
            else {
                r_blocks.push_back(R_Block(group.blocks[0].b, group.a_u, group.a_l, group.blocks[0].t));
                group = A_Block(entry.first, entry.second.a_u, entry.second.a_l, entry.second.t); 
            }
        }

        if (group.blocks.size() > 1) {
            a_blocks.push_back(group);
        }
        else {
            r_blocks.push_back(R_Block(group.blocks[0].b, group.a_u, group.a_l, group.blocks[0].t));
        }

        __yield(compress_data, b_blocks);
        __yield(compress_data, a_blocks);
        __yield(compress_data, r_blocks);
    }
    
    void compress(TimeSeries& timeseries, int n_segment, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;

        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        float b_1 = floor(d->get_value() / bound) * bound;
        float b_2 = ceil(d->get_value() / bound) * bound;
        
        
        unsigned int index = 1; int length = 1; int count = 0;
        int flag = 0; bool floor_flag = true; bool ceil_flag = true;
        float slp_u_1 = INFINITY; float slp_u_2 = INFINITY;
        float slp_l_1 = -INFINITY; float slp_l_2 = -INFINITY;
        
        std::map<float, std::vector<Interval>> b_intervals;
        clock.start();
        while (timeseries.hasNext()) {
            Point2D p(length, ((Univariate*) timeseries.next())->get_value());

            if (p.y > slp_u_1*(p.x)+b_1+bound || p.y < slp_l_1*(p.x)+b_1-bound) {
                floor_flag = false;
            }
            if (p.y > slp_u_2*(p.x)+b_2+bound || p.y < slp_l_2*(p.x)+b_2-bound) {
                ceil_flag = false;
            }
            if (floor_flag) flag++;
            if (ceil_flag) flag--;

            if (!floor_flag && !ceil_flag) {
                if (flag > 0) {
                    std::vector<Interval> vec = b_intervals[b_1];
                    vec.push_back(Interval(slp_u_1, slp_l_1, index-1));
                    b_intervals[b_1] = vec;
                }
                else {
                    std::vector<Interval> vec = b_intervals[b_2];
                    vec.push_back(Interval(slp_u_2, slp_l_2, index-1));
                    b_intervals[b_2] = vec;
                }

                count++;
                length = 0; flag = 0;
                floor_flag = true; ceil_flag = true;
                
                p.x = 0;
                b_1 = floor(p.y / bound) * bound;
                b_2 = ceil(p.y / bound) * bound;
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
                count = 0; index = 0;
                __group(compress_data, b_intervals);
                b_intervals.clear();
            }
            
            index++;
            length++;
            clock.tick();
        }

        if (flag > 0) {
            std::vector<Interval> vec = b_intervals[b_1];
            vec.push_back(Interval(slp_u_1, slp_l_1, index));
            b_intervals[b_1] = vec;
        }
        else {
            std::vector<Interval> vec = b_intervals[b_2];
            vec.push_back(Interval(slp_u_2, slp_l_2, index));
            b_intervals[b_2] = vec;
        }

        __group(compress_data, b_intervals);
        b_intervals.clear();
        
        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;

        // Profile average latency
        std::cout << "Time taken for each data point (ns): " << clock.getAvgDuration() << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(clock.getAvgDuration()));
        timeFile.close();
    }

    struct Segment {
        float b;
        float a;
        unsigned int t;

        Segment(float b, float a, int t) {
            this->b = b;
            this->a = a;
            this->t = t;
        }
    };

    void __decompress_segment(IterIO& file, int interval, time_t basetime, unsigned int start, Segment& segment) {                
        int i = 0;
        while (start++ <= segment.t) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + interval * start));
            obj.pushData(std::to_string(i++ * segment.a + segment.b));
            file.write(&obj);
        }
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* compress_data = inputFile.readBin();

        time_t basetime = compress_data->getLong();        
        std::vector<Segment> segments;
        clock.start();
        while (compress_data->getSize() != 0) {
            unsigned int start = 0;

            // Decompress part 1
            int b_blocks = compress_data->getInt();
            while (b_blocks-- > 0) {
                float b = compress_data->getFloat();
                int a_blocks = compress_data->getInt();
                while (a_blocks-- > 0) {
                    float a = compress_data->getFloat();
                    int blocks = compress_data->getInt();
                    while (blocks-- > 0) {
                        unsigned int time = compress_data->getInt();
                        segments.push_back(Segment(b, a, time));
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
                    unsigned int time = compress_data->getInt();
                    segments.push_back(Segment(b, a, time));
                }
            }

            // Decompress part 3
            int blocks = compress_data->getInt();
            while (blocks-- > 0) {
                float a = compress_data->getFloat();
                float b = compress_data->getFloat();
                unsigned int time = compress_data->getInt();
                segments.push_back(Segment(b, a, time));
            }

            // Decompress segments
            std::sort(segments.begin(), segments.end(), 
                [](const Segment& a, const Segment& b) { return a.t < b.t; });

            for (Segment& segment : segments) {
                __decompress_segment(outputFile, interval, basetime, start, segment);
                start = segment.t + 1;
            }

            clock.tick();
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