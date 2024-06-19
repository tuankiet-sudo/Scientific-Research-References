#include "polynomial/non_linear.h"

using namespace std;

string loadConfig(string path) {
    string config = "";

    fstream inputFile(path, ios_base::in);
    ostringstream ss;
    ss << inputFile.rdbuf();
    config = ss.str();
    inputFile.close();

    return config;
}

void run(HPCLab::TimeSeries& timeseries, string method, map<string, string>& params) {
    if (method == "ne") normal_equation(timeseries, stoi(params["degree"]), stof(params["bound"]), params["mode"], params["output"]);
}

void loadTimeseries(string input, HPCLab::TimeSeries& timeseries) {
    fstream inputFile(input, ios_base::in);
    string line; double val;

    while(getline(inputFile, line)) {
        std::stringstream ss(line);
        vector<double> data;
        while (ss >> val) {
            data.push_back(val);
            if (ss.peek() == ',') ss.ignore();
        }

        timeseries.push((time_t) data[0], data[1]);
    }

    inputFile.close();
}

int main(int argc, char** argv) {

    string config = loadConfig(argv[1]);
    int split = config.find('@');
    
    string line; string input;
    stringstream ss(config.substr(0, split-1));

    // Load Time series data
    while (getline(ss, line)) {
        int deli = line.find(':');
        string key = line.substr(0, deli);
        string val = line.substr(deli+1);

        if (key == "input") input = val;
    }
    
    HPCLab::TimeSeries timeseries;
    loadTimeseries(input, timeseries);
    
    int index; 
    string s = config.substr(split+2);
    while ((index = s.find('!')) != string::npos) {
        stringstream ss(s.substr(0, index-1));
        getline(ss, line);
        string method = line.substr(line.find(':')+1);

        map<string, string> params;
        while (getline(ss, line)) {
            int deli = line.find(':');
            params[line.substr(0, deli)] = line.substr(deli+1);
        }

        run(timeseries, method, params);
        s.erase(0, index + 2);
    }

    return 0;
}