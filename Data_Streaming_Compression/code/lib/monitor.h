#ifndef MONITOR
#define MONITOR

#include "dependencies.h"
#include "io.h"

using namespace std::chrono;

class Monitor {
    private:
        static bool flag;
        static high_resolution_clock::time_point clock;
        static int counter;
        static double latency;  // average latency in nano second
    
    public:
        static void monitor(std::string output) {
            rusage usage;
            IterIO outputFile(output, false);

            CSVObj header;
            header.pushData("user_cpu_time");
            header.pushData("system_cpu_time");
            header.pushData("max_rss");
            header.pushData("block_input");
            header.pushData("block_output");
            outputFile.writeStr(&header);

            while (Monitor::flag) {
                CSVObj obj;
                getrusage(RUSAGE_SELF, &usage);
                obj.pushData(std::to_string(usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec));
                obj.pushData(std::to_string(usage.ru_stime.tv_sec*1000000 + usage.ru_stime.tv_usec));
                obj.pushData(std::to_string(usage.ru_maxrss));
                obj.pushData(std::to_string(usage.ru_inblock));
                obj.pushData(std::to_string(usage.ru_oublock));
                
                outputFile.writeStr(&obj);
                // std::this_thread::sleep_for(milliseconds(10));
            }

            outputFile.close();
        }

        static void start() {
            Monitor::flag = true;
        }

        static void stop() {
            Monitor::flag = false;
        }

        static void startClock() {
            Monitor::counter = Monitor::counter + 1;
            Monitor::clock = high_resolution_clock::now();
        }

        static void endClock() {
            long duration = duration_cast<nanoseconds>(high_resolution_clock::now() - Monitor::clock).count();
            Monitor::latency = ((Monitor::counter - 1) * Monitor::latency + duration) / (double) Monitor::counter;
        }

        static double getLatency() {
            return Monitor::latency;
        }

        static void clockReset() {
            Monitor::latency = 0;
            Monitor::counter = 0;
        }

};

#endif