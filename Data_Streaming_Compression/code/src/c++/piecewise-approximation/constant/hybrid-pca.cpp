#include "piecewise-approximation/constant.h"

namespace HybridPCA {

    struct Buffer {
        float min = INFINITY;
        float max = -INFINITY;
        std::vector<Univariate<float>*> data;
        
        void append(Univariate<float>* data_point) {
            this->data.push_back(data_point);
            this->min = this->min > data_point->get_value() ? data_point->get_value() : this->min;
            this->max = this->max < data_point->get_value() ? data_point->get_value() : this->max;
        }

        void assign(std::vector<Univariate<float>*>& data) {
            this->clear();
            for (Univariate<float>* data_point : data) {
                this->append(data_point);
            }
        }

        int get_size() {
            return this->data.size();
        }

        time_t get_basetime() {
            return this->data[0]->get_time();
        }

        std::vector<Univariate<float>*> pop(int size) {
            std::vector<Univariate<float>*> vec(this->data.end() - size, this->data.end());
            this->data.resize(this->data.size()-size);

            this->min = INFINITY;
            this->max = -INFINITY;
            for (Univariate<float>* data_point : this->data) {
                this->min = this->min > data_point->get_value() ? data_point->get_value() : this->min;
                this->max = this->max < data_point->get_value() ? data_point->get_value() : this->max;
            }

            return vec;
        }

        void clear() {
            this->min = INFINITY;
            this->max = -INFINITY;
            this->data.clear();
        }
    };

    Clock clock;

    void __yield(BinObj* obj, time_t basetime, int length, float value) {
        obj->put(basetime);
        obj->put(length);
        obj->put(value);
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float value) {
        for (int i = 0; i < length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(value));
            file.writeStr(&obj);
        }
    }

    void __pmc(BinObj* obj, Buffer& window, float bound) {
        int length = 0;
        time_t basetime = -1;
        float min = INFINITY;
        float max = -INFINITY;
        float value = 0;
        
        for (Univariate<float>* data : window.data) {
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
            
            if (max - min > 2 * bound) {
                __yield(obj, basetime, length, value);
                min = data->get_value();
                max = data->get_value();
                basetime = data->get_time();
                length = 0;
            }
            value = (max + min) / 2;
            
            length++;
            clock.stop();
        }
        __yield(obj, basetime, length, value);
    }

    void compress(TimeSeries& timeseries, int w_size, int n_window, float bound, std::string output) {
        IterIO outputFile(output, false);
        BinObj* compress_data = new BinObj;
        
        Buffer buffer;
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            buffer.append(data);
            if (buffer.get_size() % w_size == 0) {
                if (buffer.max - buffer.min <= 2 * bound) {
                    if (buffer.get_size() == w_size*n_window) {
                        __yield(compress_data, buffer.get_basetime(), buffer.get_size(), (buffer.max + buffer.min) / 2);
                        buffer.clear();
                    }
                }
                else {
                    std::vector<Univariate<float>*> window = buffer.pop(w_size);
                    
                    if (buffer.get_size() > 0) {
                        __yield(compress_data, buffer.get_basetime(), buffer.get_size(), (buffer.max + buffer.min) / 2);
                        buffer.clear();
                    }
                    
                    buffer.assign(window);
                    if (buffer.max - buffer.min > 2 * bound) {
                        __pmc(compress_data, buffer, bound);
                        buffer.clear();
                    }
                }
            }

            clock.stop();
        }

        outputFile.writeBin(compress_data);
        outputFile.close();
        delete compress_data;

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
            float value = compress_data->getFloat();
            __decompress_segment(outputFile, interval, basetime, length, value);

            clock.stop();
        }

        delete compress_data;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
};