#ifndef TIMESERIES
#define TIMESERIES

#include "dependencies.h"

namespace HPCLab {
    // Source for manipulating time series data
    template <class T>
    class Data {
      private:
        std::time_t time;
        T* data;

      public:
        Data(int dimension, std::time_t time) {
            this->data = new T[dimension];
            this->time = time;
        }

        ~Data() {
            delete[] this->data;
        }

        void set_data(int index, const T value) {
            this->data[index] = value;
        }

        std::time_t unix_time() const {
            return this->time;
        }

        std::tm* timestamp() const {
            return std::localtime(&this->time);
        }

        T* get_data() const {
            return this->data;
        }
    };

    template <class T>
    class TimeSeries {
    private:
        int dimension;
        std::vector<Data<T>*> series;  // FIFO queue

    public:
        // Default dimension = 1
        TimeSeries() {
            this->dimension = 1;
        }

        TimeSeries(int dimension) {
            this->dimension = dimension;
        }

        ~TimeSeries() {
            while (!this->series.empty()) {
                delete this->series.back();
                this->series.pop_back();
            }
        }

        bool empty() const {
            return this->series.empty();
        }

        // If dimension = 1
        void push(std::time_t time, const T value) {
            if (this->dimension != 1) {
                std::cout << "push(time_t, T) only use for dimension=1 timeseries !!" << std::endl;
                exit(-1);
            }

            Data<T>* data = new Data<T>(this->dimension, time);
            data->set_data(0, value);
            this->series.push_back(data);
        }

        void push(std::time_t time, T* value) {
            Data<T>* data = new Data<T>(this->dimension, time);
            for (int i=0; i<dimension; i++) {
                data->set_data(i, value[i]);
            }

            this->series.push_back(data);
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

    };
}

#endif