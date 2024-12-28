#include "piecewise-approximation/constant.h"

namespace PMC {

    Clock clock;

    void __yield(BinObj* obj, time_t basetime, int length, float value) {
        obj->put(basetime);
        obj->put(length);
        obj->put(value);
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float value) {
        for (int i=0; i<length; i++) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + i * interval));
            obj.pushData(std::to_string(value));
            file.writeStr(&obj);
        }
    }

    BinObj* __midrange(TimeSeries& timeseries, float bound) {
        int length = 0;
        time_t basetime = -1;
        float min = INFINITY;
        float max = -INFINITY;
        float value = 0;
        
        BinObj* obj = new BinObj;
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
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

        return obj;
    }

    BinObj* __mean(TimeSeries& timeseries, float bound) {
        int length = 0;
        time_t basetime = -1;
        float min = INFINITY;
        float max = -INFINITY;
        float value = 0;
        
        BinObj* obj = new BinObj;
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (basetime == -1) {
                basetime = data->get_time();
            }

            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
            value = (value * length + data->get_value()) / (length+1);

            if (max - value > bound || value - min > bound) {
                value = (value * (length + 1) - data->get_value()) / (length);
                __yield(obj, basetime, length, value);
                min = data->get_value();
                max = data->get_value();
                value = data->get_value();
                length = 0;
                basetime = data->get_time();
            }
        
            length++;
            clock.stop();
        }

        return obj;
    }

    void compress(TimeSeries& timeseries, std::string mode, float bound, std::string output) {
        BinObj* compress_data = nullptr;
        IterIO outputFile(output, false);

        if (mode == "midrange") {
            compress_data = __midrange(timeseries, bound);    
        }
        else if (mode == "mean") {
            compress_data = __mean(timeseries, bound);
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