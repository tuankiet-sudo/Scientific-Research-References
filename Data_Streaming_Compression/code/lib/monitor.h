#ifndef MONITOR
#define MONITOR

#include "dependencies.h"

using namespace std::chrono;

class Monitor {
    private:
        static bool flag;
    
    public:
        static void start() {
            Monitor::flag = true;
            while (Monitor::flag) {
                auto start = high_resolution_clock::now();
                std::this_thread::sleep_for(milliseconds(2));
                auto end = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(end - start);

                std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;
            }
        }

        static void stop() {
            Monitor::flag = false;
            std::cout << "stopping...\n";
        }

};

#endif