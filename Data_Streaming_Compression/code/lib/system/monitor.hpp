#ifndef SYSTEM_MONITOR_HPP
#define SYSTEM_MONITOR_HPP

#include <string>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std::chrono;

class Monitor {
    private:
        bool flag;
        long page_size;
        std::thread task;

        Monitor() {
            this->flag = false;
            this->page_size = sysconf(_SC_PAGE_SIZE);
        }

        void __monitor(std::string output) {
            std::fstream file(output, std::ios::out);
            file << "user_cpu_time,system_cpu_time,vsz,rss\n";

            while (this->flag) {
                // dont know why but monitoring cpu time with ruuage seem more accurate
                rusage usage;
                getrusage(RUSAGE_SELF, &usage);  

                std::string data;
                std::ifstream ifs("/proc/self/stat", std::ios_base::in);

                // ignore
                ifs >> data >> data >> data >> data >> data >> data >> data >> data >> data >> data >> data 
                >> data >> data >> data >> data >> data >> data >> data >> data >> data >> data >> data; 
                
                // get necessary data
                ifs >> data; std::string vsz = data;
                ifs >> data; std::string rss = std::to_string(std::stol(data)*this->page_size);
                std::string utime = std::to_string(usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec);
                std::string stime = std::to_string(usage.ru_stime.tv_sec*1000000 + usage.ru_stime.tv_usec);

                file << utime << "," << stime << "," << vsz << "," << rss + "\n";
            }

            file.close();
        }
    
    public:
        static Monitor instance;
        
        void start(std::string output) {
            this->flag = true;
            this->task = std::thread(&Monitor::__monitor, this, output);
        }

        void stop() {
            this->flag = false;
            this->task.join();
        }

};

class Clock {
    private:
        int _counter;
        high_resolution_clock::time_point _pivot;
        double _avg_duration;  // average duration in nano second

    public:
        void start() {
            this->stop();
            this->_pivot = high_resolution_clock::now();
        }

        // Stop the clock and return duration from start in nanoseconds
        long tick() {
            this->_counter++;
            long duration = duration_cast<nanoseconds>(high_resolution_clock::now() - this->_pivot).count();
            this->_avg_duration = ((this->_counter - 1) * this->_avg_duration + duration) / (double) this->_counter;
            
            return duration;
        }

        double getAvgDuration() {
            return this->_avg_duration;
        }

        void stop() {
            this->_counter = 0;
            this->_avg_duration = 0;
        }
};

#endif