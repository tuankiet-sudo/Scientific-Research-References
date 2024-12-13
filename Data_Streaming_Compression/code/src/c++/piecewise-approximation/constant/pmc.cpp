#include "piecewise-approximation/constant.h"

namespace PMC {

    Clock clock;

    void _yield(BinObj* obj, int offset, float value) {
        obj->put(offset);
        obj->put(value);
    }

    void _approximate(IterIO& file, int interval, time_t basetime, int prev_point, int end_point, float value) {
        while (prev_point <= end_point) {
            CSVObj obj;
            obj.pushData(std::to_string(basetime + prev_point));
            obj.pushData(std::to_string(value));

            file.writeStr(&obj);
            prev_point += interval;
        }
    }

    void compress(TimeSeries& timeseries, std::string mode, float bound, std::string output) {
        float min = INFINITY;
        float max = -INFINITY;
        float value = 0; 
        int length = 0;
        time_t time = -1;
        IterIO outputFile(output, false);
        BinObj* obj = new BinObj;
        Univariate<float>* prev_data = nullptr;
        
        while (timeseries.hasNext()) {
            Univariate<float>* data = (Univariate<float>*) timeseries.next();
            clock.start();

            if (time == -1) {
                time = data->get_time();
                obj->put(time);
            }

            min = min < data->get_value() ? min : data->get_value();
            max = max > data->get_value() ? max : data->get_value();
            if (mode == "midrange") {
                length++;
                if (max - min > 2 * bound) {
                    _yield(obj, prev_data->get_time() - time, value);
                    min = data->get_value();
                    max = data->get_value();
                }
                value = (max + min) / 2;
            }
            else if (mode == "mean") {
                value = (value * length + data->get_value()) / (length+1);
                length++;

                if (max - value > bound || value - min > bound) {
                    value = (value * length - data->get_value()) / (length - 1);
                    _yield(obj, prev_data->get_time() - time, value);
                    min = data->get_value();
                    max = data->get_value();
                    length = 1;
                    value = data->get_value();
                }
            }

            prev_data = data;
            clock.stop();
        }
        _yield(obj, prev_data->get_time(), value);

        outputFile.writeBin(obj);
        outputFile.close();
        delete obj;

        std::cout << std::fixed << "Time taken for each data points: " 
        << clock.getAvgDuration() << " nanoseconds \n";
    }

    void decompress(std::string input, std::string output, int interval) {
        IterIO inputFile(input, true, true);
        IterIO outputFile(output, false);
        BinObj* r_obj = inputFile.readBin();

        int prev_point = 0;
        time_t time = r_obj->getLong();

        while (r_obj->getSize() != 0) {
            clock.start();
            int end_point = r_obj->getInt();
            float value = r_obj->getFloat();
            _approximate(outputFile, interval, time, prev_point, end_point, value);
            prev_point = end_point + interval;
            clock.stop();
        }

        delete r_obj;
        inputFile.close();
        outputFile.close();

        std::cout << std::fixed << "Time taken to decompress each segment: " 
        << clock.getAvgDuration() << " nanoseconds\n";
    }
};