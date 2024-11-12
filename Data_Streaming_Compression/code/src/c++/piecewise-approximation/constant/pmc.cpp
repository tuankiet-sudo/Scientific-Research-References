#include "dependencies.h"
#include "piecewise-approximation/constant.h"

void PMC::_yield(BinObj* obj, int offset, float value) {
    obj->put(offset);
    obj->put(value);
}

void PMC::_approximate(IterIO& file, int interval, time_t basetime, int prev_point, int end_point, float value) {
    while (prev_point <= end_point) {
        CSVObj obj;
        obj.pushData(std::to_string(basetime + prev_point));
        obj.pushData(std::to_string(value));

        file.writeStr(&obj);
        prev_point += interval;
    }
}

void PMC::compress(TimeSeries& timeseries, std::string mode, float bound, std::string output) {
    double min = INFINITY;
    double max = -INFINITY;
    double value = 0; 
    int length = 0;
    time_t time = -1;
    IterIO outputFile(output, false);
    BinObj* obj = new BinObj;

    while (timeseries.hasNext()) {
        Univariate<float>* data = (Univariate<float>*) timeseries.next();
        if (time == -1) {
            time = data->get_time();
            obj->put(time);
        }

        min = min < data->get_value() ? min : data->get_value();
        max = max > data->get_value() ? max : data->get_value();
        if (mode == "midrange") {
            if (max - min > 2 * bound || !timeseries.hasNext()) {
                PMC::_yield(obj, data->get_time() - time, value);
                min = data->get_value();
                max = data->get_value();
            }
            value = (max + min) / 2;
        }
        else if (mode == "mean") {
            if (abs(value - max) > bound || abs(value - min) > bound || !timeseries.hasNext()) {
                PMC::_yield(obj, data->get_time() - time, value);
                min = data->get_value();
                max = data->get_value();
                length = 0;
            }
            value = (value * length + data->get_value()) / (length+1);
            length++;
        }
    }

    outputFile.writeBin(obj);
    outputFile.close();
    delete obj;
}

void PMC::decompress(std::string input, std::string output, int interval) {
    IterIO inputFile(input, true, true);
    IterIO outputFile(output, false);
    BinObj* r_obj = inputFile.readBin();

    int prev_point = 0;
    time_t time = r_obj->getLong();
    while (r_obj->getSize() != 0) {
        int end_point = r_obj->getInt();
        float value = r_obj->getFloat();
        
        PMC::_approximate(outputFile, interval, time, prev_point, end_point, value);
        prev_point = end_point + interval;
    }

    delete r_obj;
    inputFile.close();
    outputFile.close();
}