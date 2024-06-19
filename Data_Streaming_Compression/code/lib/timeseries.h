#ifndef TIMESERIES
#define TIMESERIES

#include "dependencies.h"

namespace HPCLab {
    // Source for manipulating time series data
    class Data {
      private:
        std::time_t time;
        double value;

      public:
        Data(std::time_t time, double val) {
            this->value = val;
            this->time = time;
        }

        std::time_t unix_time() const {
            return this->time;
        }

        std::tm* timestamp() const {
            return std::localtime(&this->time);
        }

        double get_data() const {
            return this->value;
        }
    };

    class TimeSeries {
    private:
        std::vector<Data*> series;  // FIFO queue

    public:

        // Default dimension = 1
        ~TimeSeries() {
            while (!this->series.empty()) {
                delete this->series.back();
                this->series.pop_back();
            }
        }

        bool empty() const {
            return this->series.empty();
        }

        void push(std::time_t time, const double value) {
            this->series.push_back(new Data(time, value));
        }

        std::vector<Data*> get() {
            return this->series;
        }

        Data* get(int i) {
            return this->series[i];
        }

        void reverse() {
            std::reverse(this->series.begin(), this->series.end());
        }

        void print() {
            for (Data* data : this->series) {
                std::cout << data->unix_time() << " " << data->get_data() << std::endl;
            }
        }
    };
}

extern HPCLab::TimeSeries timeseries;

#endif