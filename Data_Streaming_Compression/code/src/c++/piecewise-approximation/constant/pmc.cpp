#include "piecewise-approximation/constant.hpp"

namespace PMC {

    Clock clock;

    void __yield(BinObj* obj, short length, float value) {
        obj->put(length);
        obj->put(value);
    }

    BinObj* __midrange(TimeSeries& timeseries, float bound) {
        BinObj* obj = new BinObj;

        Univariate* d = (Univariate*) timeseries.next();
        obj->put(d->get_time());
        float min = d->get_value();
        float max = d->get_value();
        float value = d->get_value();

        unsigned short length = 1;

        clock.start();    
        while (timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();

            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
            
            if (length > 65000 || max - min > 2 * bound) {
                __yield(obj, length, value);
                min = data->get_value();
                max = data->get_value();
                value = data->get_value();
                length = 1;
            }
            else {
                value = (max + min) / 2;
                length++;
            }
        }

        return obj;
    }

    BinObj* __mean(TimeSeries& timeseries, float bound) {
        BinObj* obj = new BinObj;

        Univariate* d = (Univariate*) timeseries.next();
        obj->put(d->get_time());
        float min = d->get_value();
        float max = d->get_value();
        float value = d->get_value();
        
        unsigned short length = 1;
        clock.start();
        while (timeseries.hasNext()) {
            Univariate* data = (Univariate*) timeseries.next();
            float n_value = (value * length + data->get_value()) / (length+1);

            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
        
            if (length > 65000 || max - n_value > bound || n_value - min > bound) {
                __yield(obj, length, value);
                min = data->get_value();
                max = data->get_value();
                value = data->get_value();
                length = 1;
            }
            else {
                value = n_value;
                length++;
            }
        }

        return obj;
    }

    void compress(TimeSeries& timeseries, std::string mode, float bound, std::string output) {
        clock.start();
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

        clock.tick();
        double avg_time = clock.getAvgDuration() / timeseries.size();

        // Profile average latency
        std::cout << std::fixed << "Time taken for each data point (ns): " << avg_time << "\n";
        IterIO timeFile(output+".time", false);
        timeFile.write("Time taken for each data point (ns): " + std::to_string(avg_time));
        timeFile.close();
    }

    void __decompress_segment(IterIO& file, int interval, time_t basetime, int length, float value) {
        for (int i=0; i<length; i++) {
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

            basetime += interval * length;
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