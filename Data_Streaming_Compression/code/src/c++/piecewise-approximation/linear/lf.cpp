// #include "dependencies.h"
// #include "polynomial/linear.h"

// // LF: linear filter
// namespace LF {

//     struct Point {
//         double x;
//         double y;

//         Point(double x, double y) {
//             this->x = x;
//             this->y = y;
//         }
//     };

//     // y = ax + b
//     struct Line {
//         double a;
//         double b;

//         Line(double a, double b) {
//             this->a = a;
//             this->b = b;
//         }
//     };

//     Line line(const Point& p1, const Point& p2) {
//         // return line y = ax + b from 2 points
//         double a = (p1.y-p2.y) / (p1.x-p2.x);
//         double b = p1.y - a*p1.x;

//         return Line(a, b);
//     }

//     Point intersection(Line& l1, Line& l2) {
//         // return intersection of 2 lines
//         double x = (l1.b-l2.b) / (l2.a-l1.a);
//         double y = l1.a*x + l1.b;

//         return Point(x, y);
//     }

//     double project(Line& l, double x) {
//         return l.a*x + l.b;
//     }

//     Line fitting(std::vector<Point>& segment, Line& u, Line& l) {
//         Point p = intersection(u, l);
//         double A_num = 0, A_den = 0;

//         for (Point& data : segment) {
//             A_num += (data.y-p.y)*(data.x-p.x);
//             A_den += (data.x-p.x)*(data.x-p.x);
//         }

//         double A_ig = A_num / A_den;
//         double temp = A_ig > u.a ? u.a : A_ig;
//         double a_ig = temp > l.a ? temp : l.a;
//         double b_ig = p.y - a_ig*p.x;

//         return Line(a_ig, b_ig);
//     }

//     void finalize(std::vector<std::vector<Point>>& results, std::string out_appro, std::string out_coeff) {
//         // Storing approximation result
//         std::fstream file;  
//         file.open(out_appro, std::ofstream::out | std::ofstream::trunc);

//         for (auto& line_segment : results) {
//             for (int i=0; i<line_segment.size()-1; i++) {
//                 Point s = line_segment[i];
//                 Point e = line_segment[i+1];
//                 Line f = line(s, e);

//                 if (i==0) file << (int)s.x << "," << project(f, s.x) << std::endl;
//                 for (int j=s.x+1; j<=e.x; j++) {
//                     file << j << "," << project(f, j) << std::endl;
//                 }
//             }
//         }
        
//         file.close();

//         // Storing coefficients of method
//         file.open(out_coeff, std::ofstream::out | std::ofstream::trunc);

//         for (auto& line_segment : results) {
//             for (Point& p : line_segment) {
//                 file << "(" << p.x << ", " << p.y << ") ";
//             }
//             file << std::endl;
//         }

//         file.close();
//     }

//     // Slide Filter
//     int side(Point& a, Point& b, Point& c) {
//         double cross_product = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);

//         if (cross_product == 0) return 0;       // colinear
//         else if (cross_product > 0) return 1;   // left
//         else return -1;                         // right
//     }

//     void incremental_hull(std::vector<Point>& upper_hull, std::vector<Point>& lower_hull, Point& p) {
//         int u = upper_hull.size() - 1; 
//         int l = lower_hull.size() - 1;
        
//         for (int i=upper_hull.size()-2; i>=0; i--) {
//             Point a(upper_hull[i+1].x, upper_hull[i+1].y);
//             Point b(upper_hull[i].x, upper_hull[i].y);
            
//             if (side(a, b, p) <= 0) u = i;
//             else break;
//         }

//         for (int i=lower_hull.size()-2; i>=0; i--) {
//             Point a(lower_hull[i].x, lower_hull[i].y);
//             Point b(lower_hull[i+1].x, lower_hull[i+1].y);
        
//             if (side(a, b, p) <= 0) l = i;
//             else break;
//         }

//         upper_hull = {upper_hull.begin(), upper_hull.begin() + u + 1};
//         lower_hull = {lower_hull.begin(), lower_hull.begin() + l + 1};

//         upper_hull.push_back(p); lower_hull.push_back(p);
//     }

//     // check if [a, b] interval exists and return [a, b]
//     std::pair<double, double> interval_exist(Line& prev_g, Line& prev_u, Line& prev_l, Line& u, Line& l, double t_j_k) {
//         std::pair<double, double> interval = std::make_pair(-1, -1);
//         Point p = intersection(u, l);

//         if (project(prev_g, p.x)-p.y>0) {
//             // point below g^{k-1}
//             double f_i_k = intersection(l, prev_g).x;
//             if (f_i_k<t_j_k) {
//                 if (project(l, t_j_k)-project(prev_l, t_j_k)>0) {
//                     // find hyperplane s
//                     Point s1(t_j_k, project(prev_l, t_j_k));
//                     Point s2 = intersection(u, l);
//                     Line s = line(s1, s2);

//                     double c_i = intersection(prev_g, u).x;     // c_i^k
//                     double d_i = intersection(prev_g, s).x;     // d_i^k

//                     if (c_i > t_j_k) {
//                         interval = std::make_pair(d_i, f_i_k);
//                     }
//                     else {
//                         interval = std::make_pair((c_i > d_i) ? c_i : d_i, f_i_k);
//                     }
//                 }
//             }
//         }
//         else if (p.y-project(prev_g, p.x)>0) {
//             // point above g^{k-1}
//             double f_i_k = intersection(u, prev_g).x;
//             if (f_i_k<t_j_k) {
//                 if (project(prev_u, t_j_k)-project(u, t_j_k)>0) {
//                     // find hyperplane q
//                     Point q1(t_j_k, project(prev_u, t_j_k));
//                     Point q2 = intersection(u, l);
//                     Line q = line(q1, q2);

//                     double c_i = intersection(prev_g, l).x;   // c_i^k'
//                     double d_i = intersection(prev_g, q).x;   // d_i^k'
                    
//                     if (c_i > t_j_k) {
//                         interval = std::make_pair(d_i, f_i_k);
//                     }
//                     else {
//                         interval = std::make_pair((c_i > d_i) ? c_i : d_i, f_i_k);
//                     }
//                 }
//             }
//         }
        
//         return interval;
//     }

//     void slide_filter(float bound, std::string out_appro, std::string out_coeff) {
//         std::vector<std::vector<Point>> results;
//         std::vector<Point> segment;
//         std::vector<Point> line_segment;
//         std::vector<Point> upper_hull, lower_hull;

//         segment.push_back(Point(0, timeseries.get()[0]->get_data()));
//         segment.push_back(Point(1, timeseries.get()[1]->get_data()));
//         upper_hull = segment, lower_hull = segment;
        
//         Line u = line(Point(0, segment[0].y-bound), Point(1, segment[1].y+bound));
//         Line l = line(Point(0, segment[0].y+bound), Point(1, segment[1].y-bound));
//         Line prev_u(0, 0); Line prev_l(0, 0); Line prev_g(0, 0);

//         bool first = true; int t = 0, prev_t = 0;
//         for (int i=2; i<timeseries.get().size(); i++) {
//             Point data(i, timeseries.get()[i]->get_data());

//             if (data.y-project(u, data.x)>bound || project(l, data.x)-data.y>bound || i==timeseries.get().size()-1) {
//                 if (!first) {
//                     std::pair<double, double> interval = interval_exist(prev_g, prev_u, prev_l, u, l, prev_t);
//                     if (interval.first != -1) {
//                         Point z = intersection(u, l);

//                         if (project(prev_g, z.x)-z.y>0) {
//                             u = line(z, Point(interval.first, project(prev_g, interval.first)));
//                             l = line(z, Point(interval.second, project(prev_g, interval.second)));
//                         }
//                         else if(z.y-project(prev_g, z.x)>0) {
//                             u = line(z, Point(interval.second, project(prev_g, interval.second)));
//                             l = line(z, Point(interval.first, project(prev_g, interval.first)));
//                         }
//                     }
//                     Line g_k = fitting(segment, u, l);
//                     if (interval.first != -1) {
//                         Point p = intersection(g_k, prev_g);
//                         line_segment.push_back(p);
//                     }
//                     else {
//                         Point p1 = Point(prev_t, project(prev_g, prev_t));
//                         Point p2 = Point(t, project(g_k, t));

//                         line_segment.push_back(p1);
//                         results.push_back(line_segment);
//                         line_segment = {p2};
//                     }
//                     prev_g = g_k;
//                 }
//                 else {
//                     first = false;
//                     Line g_k = fitting(segment, u, l);
//                     prev_g = g_k;

//                     Point p(t, project(prev_g, t));
//                     line_segment.push_back(p);
//                 }

//                 if (i<timeseries.get().size()-1) {
//                     i = i+1;
//                     Point next(i, timeseries.get()[i]->get_data());
                    
//                     prev_u = u, prev_l = l;
//                     prev_t = segment.back().x; t = data.x;

//                     u = line(Point(data.x, data.y-bound), Point(next.x, next.y+bound));
//                     l = line(Point(data.x, data.y+bound), Point(next.x, next.y-bound));
//                     segment = {data, next};
//                     upper_hull = segment; lower_hull = segment;
//                 }
//             }
//             else {
//                 incremental_hull(upper_hull, lower_hull, data);
//                 std::vector<Point> temp = {upper_hull.begin(), upper_hull.end()-1};
//                 temp.insert(temp.end(), lower_hull.begin()+1, lower_hull.end()-1);

//                 if (data.y-project(l, data.x)>bound) {
//                     for (Point& p : temp) {
//                         Point p1(p.x, p.y+bound);
//                         Point p2(data.x, data.y-bound);
                        
//                         Line new_l = line(p1, p2);
//                         l = new_l.a > l.a ? new_l : l;
//                     }
//                 }
//                 if (project(u, data.x)-data.y>bound) {
//                     for (Point& p : temp) {
//                         Point p1(p.x, p.y-bound);
//                         Point p2(data.x, data.y+bound);
                        
//                         Line new_u = line(p1, p2);
//                         u = new_u.a < u.a ? new_u : u;
//                     }
//                 }
//                 segment.push_back(data);
//             }
//         }

//         Point p(timeseries.get().size()-1, project(prev_g, timeseries.get().size()-1));
//         line_segment.push_back(p);
//         results.push_back(line_segment);

//         finalize(results, out_appro, out_coeff);
//     }

//     // Swing Filter
//     void swing_filter(float bound, std::string out_appro, std::string out_coeff) {
//         std::vector<Point> segment;
//         std::vector<Point> line_segment;
//         std::vector<std::vector<Point>> results;

//         segment.push_back(Point(0, timeseries.get()[0]->get_data()));
//         segment.push_back(Point(1, timeseries.get()[1]->get_data()));
        
//         Line u = line(segment[0], Point(1, segment[1].y+bound));
//         Line l = line(segment[0], Point(1, segment[1].y-bound));

//         line_segment.push_back(segment[0]);
//         for (int i=2; i<timeseries.get().size(); i++) {
//             Point data(i, timeseries.get()[i]->get_data());

//             if (data.y-project(u, data.x)>bound || project(l, data.x)-data.y>bound) {
//                 Line g = fitting(segment, u, l);
//                 Point p(segment.back().x, project(g, segment.back().x));

//                 u = line(p, Point(data.x, data.y+bound));
//                 l = line(p, Point(data.x, data.y-bound));

//                 line_segment.push_back(p);
//                 segment = {p};
//             }
//             else {
//                 if (data.y-project(l, data.x)>bound) {
//                     l = line(intersection(u, l), Point(data.x, data.y-bound));
//                 }
//                 if (project(u, data.x)-data.y>bound) {
//                     u = line(intersection(u, l), Point(data.x, data.y+bound));
//                 }
//             }
//             segment.push_back(data);
//         }

//         Line g = fitting(segment, u, l);
//         Point p(segment.back().x, project(g, segment.back().x));

//         line_segment.push_back(p);
//         results.push_back(line_segment);

//         finalize(results, out_appro, out_coeff);
//     }
// }