// Define all necessary libraries
#ifndef DEPENDENCY
#define DEPENDENCY

// Including standard libraries
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

// Define custom classes
class Matrix {

};

template <class T>
class TimeSeries {
  private:
    std::vector<long> time;
    std::vector<T> value;

  public:
    ~TimeSeries() {
        while (!this->series.empty()) {
            delete this->series.back();
            this->series.pop_back();
        }
    }

    void push(long time, float value) {
        this->series.push_back(new Data(time, value));
    }

    Data* get(int i = 0) {
        return this->series[i];
    }
};

#endif