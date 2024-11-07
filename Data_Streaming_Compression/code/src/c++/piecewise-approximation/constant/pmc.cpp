// #include "dependencies.h"
// #include "polynomial/constant.h"

// // PMC: poor man compression
// namespace PMC {

//     struct PMC {
//         int end_point;
//         double value;

//         PMC(int end_point, double value) {
//             this->end_point = end_point;
//             this->value = value;
//         }
//     };

//     void finalize(std::vector<PMC>& result, std::string out_appro, std::string out_coeff) {
//         // Storing approximation result
//         std::fstream file;
//         file.open(out_appro, std::ofstream::out | std::ofstream::trunc);

//         int end = -1; int index = 0;
//         for (PMC& ele : result) {
//             for (int i=0; i<ele.end_point-end; i++) {
//                 file << index++ << "," << ele.value << std::endl;
//             }
//             end = ele.end_point;
//         }
        
//         file.close();

//         // Storing coefficients of method
//         file.open(out_coeff, std::ofstream::out | std::ofstream::trunc);

//         for (PMC& ele : result) {
//             file << ele.end_point << " " << ele.value << std::endl;
//         }

//         file.close();
//     }

//     // Begin: section of algorithm described in paper
//     void mid_range(float bound, std::string out_appro, std::string out_coeff) {
//         std::vector<PMC> results;
//         int n = 0;
//         double m = INFINITY;
//         double M = -INFINITY;

//         for (int i=0; i<timeseries.get().size(); i++) {
//             double data = timeseries.get()[i]->get_data();
//             double max = M > data ? M : data;
//             double min = m < data ? m : data;
//             if (max - min > 2*bound) {
//                 results.push_back(PMC(n-1, (m+M)/2));
//                 m = data;
//                 M = data;
//             }
//             else {
//                 m = min;
//                 M = max;
//             }
//             n = n + 1;
//         }
//         results.push_back(PMC(n-1, (m+M)/2));

//         finalize(results, out_appro, out_coeff);
//     }

//     void mean(float bound, std::string out_appro, std::string out_coeff) {
//         std::vector<PMC> results;
//         int n = 0;
//         double m = INFINITY;
//         double M = -INFINITY;
//         double mean = 0;
//         int current_length = 1;

//         for (int i=0; i<timeseries.get().size(); i++) {
//             double data = timeseries.get()[i]->get_data();
//             double current_mean = (mean*(current_length-1) + data)/current_length;
//             double max = M > data ? M : data;
//             double min = m < data ? m : data;

//             if (abs(mean-max)>bound || abs(mean-min)>bound) {
//                 results.push_back(PMC(n-1, mean));
//                 m = data;
//                 M = data;
//                 mean = data;
//                 current_length = 1;
//             }
//             else {
//                 m = min;
//                 M = max;
//                 mean = current_mean;
//             }
//             current_length = current_length + 1;
//             n = n + 1;
//         }
//         results.push_back(PMC(n-1, mean));

//         finalize(results, out_appro, out_coeff);
//     }
    
// }