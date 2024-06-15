#include "polynomial/non_linear.h"

using namespace std;

enum Methods {
    SWINGFILTER,
    SLIDEFILTER,
    PNE, APNE
};

int main(int argc, char** argv) {
    string input = argv[1];
    string output = argv[2];
    vector<string> methods(argv+3, argv+argc);
    

    cout << "input: " << input << endl;
    cout << "output: " << output << endl;
    cout << "methods: ";
    for (string method : methods) {
        cout << method << " ";
    }
    cout << endl;
    
    // fstream myFile("data/input/Synthesis/quadratic.csv", ios_base::in);
    // string line;
    // float val;

    // HPCLab::TimeSeries<float> timeseries;
    // while(std::getline(myFile, line)) {
    //     std::stringstream ss(line);
    //     vector<float> data;
    //     while (ss >> val) {
    //         data.push_back(val);
    //         if (ss.peek() == ',') ss.ignore();
    //     }
    //     timeseries.push((time_t) data[0], data[1]);
    // }

    // myFile.close();

    // timeseries.print();

    return 0;
}