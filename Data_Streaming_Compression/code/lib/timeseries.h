#ifndef TIMESERIES
#define TIMESERIES

#include "dependencies.h"

namespace HPCLab {
    // Source for manipulating time series data
    template <class T>
    class Data {
      private:
        std::time_t time;
        T value;

      public:
        Data(std::time_t time, T val) {
            this->value = val;
            this->time = time;
        }

        std::time_t unix_time() const {
            return this->time;
        }

        std::tm* timestamp() const {
            return std::localtime(&this->time);
        }

        T get_data() const {
            return this->value;
        }
    };

    template <class T>
    class TimeSeries {
    private:
        std::vector<Data<T>*> series;  // FIFO queue

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

        void push(std::time_t time, const T value) {
            this->series.push_back(new Data<T>(time, value));
        }

        std::vector<Data<T>*> get() {
            return this->series;
        }

        Data<T>* get(int i) {
            return this->series[i];
        }

        void reverse() {
            std::reverse(this->series.begin(), this->series.end());
        }

        void print() {
            for (Data<T>* data : this->series) {
                std::cout << data->unix_time() << " " << data->get_data() << std::endl;
            }
        }
    };
}

#endif