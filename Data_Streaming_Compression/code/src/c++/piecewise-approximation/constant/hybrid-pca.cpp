#include "piecewise-approximation/constant.h"

namespace HybridPCA {

    struct Window {
        float min;
        float max;
        std::vector<Univariate*> data;

        Window() {
            this->min = INFINITY;
            this->max = -INFINITY;
        }

        int size() {
            return this->data.size();
        }

        void append(Univariate* p) {
            this->data.push_back(p);
            this->min = this->min < p->get_value() ? this->min : p->get_value();
            this->max = this->max > p->get_value() ? this->max : p->get_value();
        }
    };

    struct Buffer {
        float min;
        float max;
        std::vector<Window*> windows;

        Buffer() {
            this->min = INFINITY;
            this->max = -INFINITY;
        }

        int size() {
            return this->windows.size();
        }

        void pop() {
            this->windows.pop_back();
        }

        void append(Window* window) {
            this->windows.push_back(window);   
            this->min = this->min < window->min ? this->min : window->min;
            this->max = this->max > window->max ? this->max : window->max;
        }

        bool is_appendable(Window* window, float bound) {
            float n_min = this->min < window->min ? this->min : window->min;
            float n_max = this->max > window->max ? this->max : window->max;

            return (n_max-n_min)<=2*bound;
        }

        void clear() {
            this->min = INFINITY;
            this->max = -INFINITY;
            for (Window* window : this->windows) 
                delete window;

            this->windows.clear();
        }
    };

    Clock clock;

    void __yield(BinObj* obj, short length, float value) {
        obj->put(length);
        obj->put(value);
    }

    void __pmc(BinObj* obj, Window* window, float bound) {
        float min = INFINITY;
        float max = -INFINITY;
        float value = 0;
        
        unsigned short length = 0;
        for (Univariate* data : window->data) {
            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
            
            if (max - min > 2 * bound) {
                __yield(obj, length, value);
                min = data->get_value();
                max = data->get_value();
                length = 0;
            }
            value = (max + min) / 2;
            
            length++;
        }

        if (length > 0) __yield(obj, length, value);
    }

    void compress(TimeSeries& timeseries, int w_size, int n_window, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;
        
        Buffer buffer;
        Window* window = new Window;
        
        Univariate* d = (Univariate*) timeseries.next();
        compress_data->put(d->get_time());
        window->append(d);

        clock.start();
        while (timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();
            window->append(data);

            if (window->size() == w_size) {
                if (buffer.is_appendable(window, bound)) {
                    buffer.append(window);
                    if (buffer.size() == n_window) {
                        __yield(compress_data, w_size*n_window, (buffer.max+buffer.min)/2);
                        buffer.clear();
                    }
                }
                else {
                    if (buffer.size() > 0) {
                        __yield(compress_data, buffer.size()*w_size, (buffer.max+buffer.min)/2);
                        buffer.clear();
                    }

                    if ((window->max-window->min)>2*bound) {
                        __pmc(compress_data, window, bound);
                        delete window;
                    }
                    else {
                        buffer.append(window);
                    }
                }
                window = new Window;
            }

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

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float value) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(value));
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
            float value = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, value);

            basetime += length * interval;
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