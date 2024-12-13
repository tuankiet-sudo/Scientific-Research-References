#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <string>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std::chrono;

class Monitor {
    private:
        static bool flag;
        static long page_size;
    
    public:
        static void monitor(std::string output) {
            std::fstream file(output, std::ios::out);
            file << "user_cpu_time,system_cpu_time,vsz,rss\n";

            while (Monitor::flag) {
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
                ifs >> data; std::string rss = std::to_string(std::stol(data)*Monitor::page_size);
                std::string utime = std::to_string(usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec);
                std::string stime = std::to_string(usage.ru_stime.tv_sec*1000000 + usage.ru_stime.tv_usec);

                file << utime << "," << stime << "," << vsz << "," << rss + "\n";
            }

            file.close();
        }

        static void start() {
            Monitor::page_size = sysconf(_SC_PAGE_SIZE);
            Monitor::flag = true;
        }

        static void stop() {
            Monitor::flag = false;
        }

};

class Clock {
    private:
        int _counter;
        high_resolution_clock::time_point _clock;
        double _duration;  // average duration in nano second

    public:
        Clock() {
            this->reset();
        }

        void start() {
            this->_counter += 1;
            this->_clock = high_resolution_clock::now();
        }

        // Stop the clock and return duration from start in nanoseconds
        long stop() {
            long duration = duration_cast<nanoseconds>(high_resolution_clock::now() - this->_clock).count();
            this->_duration = ((this->_counter - 1) * this->_duration + duration) / (double) this->_counter;
            
            return duration;
        }

        double getAvgDuration() {
            return this->_duration;
        }

        void reset() {
            this->_counter = 0;
            this->_duration = 0;
        }
};

#endif