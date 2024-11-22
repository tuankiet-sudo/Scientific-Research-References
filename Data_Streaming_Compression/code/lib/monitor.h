#ifndef MONITOR
#define MONITOR

#include "dependencies.h"
#include "io.h"

using namespace std::chrono;

class Monitor {
    private:
        static bool flag;
        static long page_size;
        static high_resolution_clock::time_point clock;
        static int counter;
        static double latency;  // average latency in nano second
    
    public:
        static void monitor(std::string output) {
            rusage usage;
            IterIO outputFile(output, false);

            {
                CSVObj header;
                header.pushData("user_cpu_time");
                header.pushData("system_cpu_time");
                header.pushData("vsz");
                header.pushData("rss");
                outputFile.writeStr(&header);
            }

            while (Monitor::flag) {
                CSVObj obj;
                {
                    std::string ignore;
                    std::string vsz; long rss;
                    std::string utime; std::string stime;      
                    std::ifstream ifs("/proc/self/stat", std::ios_base::in);    

                    ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                        >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                        >> utime >> stime >> ignore >> ignore >> ignore >> ignore
                        >> ignore >> ignore >> ignore >> vsz >> rss;
                
                    obj.pushData(utime);
                    obj.pushData(stime);
                    obj.pushData(vsz);
                    obj.pushData(std::to_string(rss*Monitor::page_size));

                    ifs.close();
                }
                outputFile.writeStr(&obj);
                //std::this_thread::sleep_for(milliseconds(10));
            }

            outputFile.close();
        }

        static void start() {
            Monitor::page_size = sysconf(_SC_PAGE_SIZE);
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