// #include "dependencies.h"
// #include "polynomial/non_linear.h"

// // SGC: smart grid compression
// namespace SGC {

//     struct SGC {
//         int segment_length;
//         HPCLab::Polynomial* model;

//         SGC(int segment_length, HPCLab::Polynomial* model) {
//             this->segment_length = segment_length;
//             this->model = model;
//         }
//     };

//     void finalize(std::vector<SGC>& result, std::string out_appro, std::string out_coeff) {
//         // Storing approximation result
//         std::fstream file;
//         file.open(out_appro, std::ofstream::out | std::ofstream::trunc);

//         for (SGC& ele : result) {
//             for (int i=0; i<ele.segment_length; i++) {
//                 file << i << "," << ele.model->substitute<int>(i) << std::endl;
//             }
//         }
        
//         file.close();

//         // Storing coefficients of method
//         file.open(out_coeff, std::ofstream::out | std::ofstream::trunc);
//         int i = 0;
//         for (SGC& ele : result) {
//             file << i << "-" << ele.segment_length+i-1 << " " << ele.model->str() << std::endl;
//             i += ele.segment_length;
//         }

//         file.close();

//         // Cleaning
//         for (SGC& ele : result) {
//             delete ele.model;
//         }
//     }

//     // Begin: section of algorithm described in paper
//     // Begin: linear approximation

//     // Original src from: https://www.geeksforgeeks.org/convex-hull-using-graham-scan/
//     float cosine(double x, double y) {
//         return x / sqrt(x*x+y*y);
//     }

//     int orientation(int px, double py, int qx, double qy) {
//         int val = py * qx - px * qy;

//         if (val == 0) return val;
//         else return (val > 0) ? 1 : 2; 
//     }

//     std::vector<int> graham_scan(const std::vector<double>& segment) {
//         std::vector<int> convex_hull;
//         // find the smallest left most point
//         int pos = 0;
//         double min = segment[pos];
//         for (int i=1; i<segment.size(); i++) {
//             if (segment[i] < min) {
//                 min = segment[i];
//                 pos = i;
//             }
//         }

//         // sorting based on polar angle
//         std::vector<int> index;
//         for (int i=0; i<segment.size(); i++) index.push_back(i);
//         index[0] = pos; index[pos] = 0;

//         std::sort(index.begin()+1, index.end(), [=](double i1, double i2) {
//             return cosine(i1-pos, segment[i1]-min) > cosine(i2-pos, segment[i2]-min);
//         });

//         // retain only farthest point if angles are equal
//         int size = 1;
//         for (int i=1; i<index.size(); i++) {
//             while (i < index.size()-1 
//                 && orientation(index[i]-pos, segment[index[i]]-min, index[i+1]-index[i], segment[index[i+1]]-segment[index[i]]) == 0) i++;

//             index[size++] = index[i];
//         }

//         // finding convex hull
//         std::stack<int> stack;
//         stack.push(index[0]);
//         stack.push(index[1]);
//         stack.push(index[2]);

//         for (int i=3; i<size; i++) {
//             while (stack.size()>1) {
//                 int top = stack.top();
//                 stack.pop();
//                 int next2top = stack.top();
//                 stack.push(top);

//                 if (orientation(top-next2top, segment[top]-segment[next2top], index[i] - top, segment[index[i]]-segment[top]) != 2) stack.pop();
//                 else break;
//             }
//             stack.push(index[i]);
//         }

//         while (!stack.empty()) {
//             convex_hull.push_back(stack.top());
//             stack.pop();
//         }

//         return convex_hull;
//     }

//     double distance(double a, double b, int x, double y) {
//         return abs(a*x-y+b)/sqrt(a*a+1);
//     }

//     int x_external(int x1, int x2, int x) {
//         // 0:  x_external
//         // 1:  x_external to the right
//         // -1: x_external to the left

//         if (x >= x1 && x >= x2) return 1;
//         else if (x <= x1 && x <= x2) return -1;
//         else return 0;
//     }

//     // int search(int p, int m, std::vector<int>& ccw_hull, std::vector<double>& segment) {
//     //     int j = (p+1)%ccw_hull.size();
//     //     double a = (segment[ccw_hull[p]]-segment[ccw_hull[j]])/(ccw_hull[p]-ccw_hull[j]);
//     //     double b = segment[ccw_hull[j]] - a*ccw_hull[j];
        
//     //     // search in ccw order
//     //     // if pj(i+1) < pj(i) return i
//     //     int v = -1; double prev_dis = -INFINITY;
//     //     for (int i=0; i<ccw_hull.size(); i++) {
//     //         double dis = distance(a, b, ccw_hull[(m+i)%ccw_hull.size()], segment[ccw_hull[(m+i)%ccw_hull.size()]]);
//     //         if (dis < prev_dis) {
//     //             return (m+i-1)%ccw_hull.size();
//     //         }
//     //         prev_dis = dis;
//     //     }

//     //     return -1;
//     // }

//     int search(int p, int m, std::vector<int>& ccw_hull, const std::vector<double>& segment) {
//         int j = (p+1)%ccw_hull.size();
//         double a = (segment[ccw_hull[p]]-segment[ccw_hull[j]])/(ccw_hull[p]-ccw_hull[j]);
//         double b = segment[ccw_hull[j]] - a*ccw_hull[j];
        
//         // search in ccw order
//         // if pj(i+1) < pj(i) return i
//         int v = -1; double dis = -INFINITY;
//         for (int i=0; i<ccw_hull.size(); i++) {
//             double temp = distance(a, b, ccw_hull[(m+i)%ccw_hull.size()], segment[ccw_hull[(m+i)%ccw_hull.size()]]);
//             if (dis < temp) {
//                 dis = temp;
//                 v = (m+i)%ccw_hull.size();
//             }
//         }

//         return v;
//     }

//     HPCLab::Polynomial* linear_appro(const std::vector<double>& segment) {
//         // find the convex hull index
//         std::vector<int> convex_hull = graham_scan(segment);
//         // find the upper and lower hull
//         int left = 0; int right = segment.size()-1;
//         // line through leftmost and rightmost points y = ax + b
//         double a = (segment[right]-segment[left])/right; 
//         double b = segment[left]; 
//         std::vector<int> ccw_hull = {left}, upper_hull = {right};
        
//         for (int i : convex_hull) {
//             if (i != left && i != right) {
//                 if (segment[i] >= i*a+b) upper_hull.push_back(i);
//                 else ccw_hull.push_back(i);
//             }
//         }
//         right = ccw_hull.size();  // index of rightmost point in ccw convex hull
//         sort(ccw_hull.begin(), ccw_hull.end());
//         sort(upper_hull.begin(), upper_hull.end(), [](int a, int b) {return a>b;});
//         ccw_hull.insert(ccw_hull.end(), upper_hull.begin(), upper_hull.end());

//         // find the A, B and C
//         int A, B, C;
//         int prev_v_l = right;
//         // find the first v(l1)
//         for (int i=0; i<ccw_hull.size(); i++) {
//             int v_l = search(i, prev_v_l, ccw_hull, segment);
//             int external = x_external(ccw_hull[i], ccw_hull[(i+1)%ccw_hull.size()], ccw_hull[v_l]);
//             if (external == 1) {
//                 prev_v_l = v_l;
//             }
//             else if (external == 0) {
//                 A = ccw_hull[i]; 
//                 B = ccw_hull[(i+1)%ccw_hull.size()];
//                 C = ccw_hull[v_l];
//                 break;
//             }
//             else {
//                 int step = v_l > prev_v_l ? v_l - prev_v_l : v_l - prev_v_l + ccw_hull.size();
//                 for (int j=1; j<step; j++) {
//                     if (x_external(ccw_hull[(prev_v_l+j)%ccw_hull.size()], ccw_hull[(prev_v_l+j+1)%ccw_hull.size()], ccw_hull[i]) == 0) {
//                         A = ccw_hull[j];
//                         B = ccw_hull[(j+1)%ccw_hull.size()];
//                         C = ccw_hull[i];
//                         break;
//                     }
//                 }
//             }
//         }

//         // find the optimal approximation line
//         a = (segment[A]-segment[B])/(A-B); 
//         b = segment[A] - a*A;
//         double c = (segment[C] > segment[A]) ? b + distance(a, b, C, segment[C])/2*sqrt(a*a+1)
//                                             : b - distance(a, b, C, segment[C])/2*sqrt(a*a+1);
        
//         // for (auto val : segment) std::cout << val << " ";
//         // std::cout << std::endl;
//         // std::cout << "y = " << a << "x + " <<c<<std::endl;
//         float coeffs[2] = {(float)c, (float)a};
//         return new HPCLab::Polynomial(1, coeffs);
//     }
//     // End: polynomial approximation

//     // Begin: polynomial approximation
//     HPCLab::Polynomial* polynomial_appro(const std::vector<double>& segment, int degree) {
//         return NULL;
//     }
//     // End: polynomial approximation

//     float calError(const HPCLab::Polynomial* function, const std::vector<double>& data) {
//         float max = 0;
//         for (int i=0; i<data.size(); i++) {
//             float error = abs(function->substitute<int>(i) - data[i]);
//             max = error > max ? error : max;
//         }

//         return max;
//     }

//     int choose_best_model(std::vector<SGC>& temp) {
//         float compress_ratio = 0;
//         int res = -1;

//         for (int i=0; i<temp.size(); i++) {
//             if (compress_ratio < temp[i].segment_length/(temp[i].model->degree+1)) {
//                 compress_ratio = temp[i].segment_length/(temp[i].model->degree+1);
//                 res = i;
//             }
//         }

//         for (int i=0; i<temp.size(); i++) {
//             if (i != res) delete temp[i].model;
//         }

//         return res;
//     }

//     HPCLab::Polynomial* approx_succeeded(int k, float bound, const std::vector<double>& segment) {
//         if (k == 0) {
//             // PMC-Midrange
//             static double m = INFINITY;
//             static double M = -INFINITY;

//             if (M == -INFINITY || m == INFINITY) {
//                 for (double data : segment) {
//                     m = m > data ? data : m;
//                     M = M < data ? data : M;
//                 }
//             }
//             else {
//                 m = m > segment.back() ? segment.back() : m;
//                 M = M < segment.back() ? segment.back() : M;
//             }

//             if (M - m > 2 * bound) {
//                 m = INFINITY;
//                 M = -INFINITY;
//                 return NULL;
//             }
//             else {
//                 return new HPCLab::Polynomial((float)(m+M)/2);
//             }
//         }
//         else {
//             HPCLab::Polynomial* model = (k == 1) ? linear_appro(segment) : polynomial_appro(segment, k);
//             if (calError(model, segment) > bound) {
//                 delete model;
//                 model = NULL;
//             }
            
//             return model;
//         }
//     }

//     void smart_grid_compression(int degree, float bound, std::string out_appro, std::string out_coeff) {
//         std::vector<double> segment;
//         std::vector<SGC> results;
        
//         int pivot = 0;
//         for (int i=0; i<timeseries.get().size(); i++) {
//             if (segment.empty()) pivot = i;
//             segment.push_back(timeseries.get()[i]->get_data());
//             if (segment.size() < 3) continue;

//             std::vector<SGC> temp;
//             HPCLab::Polynomial* prev_model = NULL;
//             for (int k=0; k<=degree; k++) {
//                 HPCLab::Polynomial* model = approx_succeeded(k, bound, segment);
//                 while (model != NULL) {
//                     if (prev_model != NULL) {
//                         delete prev_model;
//                         prev_model = NULL;
//                     }
//                     prev_model = model;
//                     segment.push_back(timeseries.get()[++i]->get_data());
//                     model = approx_succeeded(k, bound, segment);
//                 }
//                 if (prev_model != NULL) {
//                     temp.push_back(SGC(segment.size()-1, prev_model));
//                     prev_model = NULL;
//                 }
//             } 

//             if (temp.empty()) {
//                 for (double val : segment) {
//                     results.push_back(SGC(1, new HPCLab::Polynomial(val)));
//                 }
//             }
//             else {
//                 int res = choose_best_model(temp);
//                 results.push_back(temp[res]);
//                 i = pivot + temp[res].segment_length - 1;
//             }
            
//             segment.clear();
//             temp.clear();
//         }

//         finalize(results, out_appro, out_coeff);

//     }
// }