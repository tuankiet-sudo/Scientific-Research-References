#include "piecewise-approximation/constant.h"

void HybridPMC::_yield(BinObj* obj, int length, float value) {
    obj->put(length);
    obj->put(value);
}

void HybridPMC::_approximate(IterIO& file, int interval, time_t basetime, int prev_point, int length, float value) {
    for (int i = 0; i < length; i++) {
        CSVObj obj;
        obj.pushData(std::to_string(basetime + prev_point));
        obj.pushData(std::to_string(value));

        file.writeStr(&obj);
        prev_point += interval;
    }
}

void HybridPMC::_pmc(BinObj* obj, std::vector<float>& window, float bound) {
    float min = INFINITY;
    float max = -INFINITY;
    float value = 0;
    int length = 0;

    for (int i = 0; i < window.size(); i++) {
        length = length + 1;
        min = min < window[i] ? min : window[i];
        max = max > window[i] ? max : window[i];

        if (max - min > 2 * bound) {
            HybridPMC::_yield(obj, length - 1, value);
            min = window[i];
            max = window[i];
            length = 1;
        }
        value = (max + min) / 2;
    }
    HybridPMC::_yield(obj, length, value);
}

void HybridPMC::compress(TimeSeries& timeseries, int w_size, int m_window, float bound, std::string output) {
    IterIO outputFile(output, false);
    BinObj* obj = new BinObj;

    time_t time = -1;
    float min = INFINITY;
    float max = -INFINITY;
    std::vector<float> buffer;
    
    Monitor::clockReset();
    int count = 0;
    while (timeseries.hasNext()) {
        Univariate<float>* data = (Univariate<float>*) timeseries.next();
        Monitor::startClock();

        if (time == -1) {
            time = data->get_time();
            obj->put(time);
        }

        min = min < data->get_value() ? min : data->get_value();
        max = max > data->get_value() ? max : data->get_value();

        buffer.push_back(data->get_value());
        if (buffer.size() % w_size == 0) {
            if (max - min <= 2 * bound) {
                if (buffer.size() == w_size*m_window) {
                    HybridPMC::_yield(obj, buffer.size(), (max + min) / 2);
                    buffer.clear();
                    min = INFINITY;
                    max = INFINITY;
                }
            }
            else {
                std::vector<float> window(buffer.end() - w_size, buffer.end());
                float w_min = *std::min_element(window.begin(), window.end());
                float w_max = *std::max_element(window.begin(), window.end());

                if (buffer.size() > w_size) {
                    buffer.resize(buffer.size() - w_size);
                    float b_min = *std::min_element(buffer.begin(), buffer.end());
                    float b_max = *std::max_element(buffer.begin(), buffer.end());
                    HybridPMC::_yield(obj, buffer.size(), (b_max + b_min) / 2);
                }
                buffer.clear();
                
                if (w_max - w_min <= 2 * bound) {
                    buffer = window;
                    min = w_min;
                    max = w_max;
                }
                else {
                    HybridPMC::_pmc(obj, window, bound);
                    min = INFINITY;
                    max = -INFINITY;
                }
            }
        }

        count += 1;
        Monitor::endClock();
    }

    if (!buffer.empty()) {
        if (max - min <= 2*bound) {
            HybridPMC::_yield(obj, buffer.size(), (max + min) / 2);
        }
        else {
            std::vector<float> window(buffer.begin() + buffer.size() / w_size * w_size, buffer.end());
            buffer.resize(buffer.size() - window.size());
            float b_min = *std::min_element(buffer.begin(), buffer.end());
            float b_max = *std::max_element(buffer.begin(), buffer.end());

            HybridPMC::_yield(obj, buffer.size(), (b_max + b_min) / 2);
            HybridPMC::_pmc(obj, window, bound);
        }
    }

    outputFile.writeBin(obj);
    outputFile.close();
    delete obj;
}

void HybridPMC::decompress(std::string input, std::string output, int interval) {
    IterIO inputFile(input, true, true);
    IterIO outputFile(output, false);
    BinObj* r_obj = inputFile.readBin();

    int prev_point = 0;
    time_t time = r_obj->getLong();
    while (r_obj->getSize() != 0) {
        int end_point = r_obj->getInt();
        float value = r_obj->getFloat();
        
        HybridPMC::_approximate(outputFile, interval, time, prev_point, end_point, value);
        prev_point += end_point * interval;
    }

    delete r_obj;
    inputFile.close();
    outputFile.close();
}