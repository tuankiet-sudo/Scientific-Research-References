#include <vector>

struct Data {
    long time;
    float value;

    Data(long time, float value) {
        this->time = time;
        this->value = value;
    }
};

class TimeSeries {
    private:
        std::vector<Data*> series;

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