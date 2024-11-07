#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stack>
#include <map>
#include <algorithm>
#include <cmath>
#include <vector>
#include <queue>
#include <ctime>
#include <utility>

using namespace std;

float cosine(double x, double y) {
    return x / sqrt(x*x+y*y);
}

int orientation(int px, double py, int qx, double qy) {
    int val = py * qx - px * qy;

    if (val == 0) return val;
    else return (val > 0) ? 1 : 2; 
}

std::vector<int> graham_scan(std::vector<double>& segment) {
    std::vector<int> convex_hull;
    // find the smallest left most point
    int pos = 0;
    double min = segment[pos];
    for (int i=1; i<segment.size(); i++) {
        if (segment[i] < min) {
            min = segment[i];
            pos = i;
        }
    }

    // sorting based on polar angle
    std::vector<int> index;
    for (int i=0; i<segment.size(); i++) index.push_back(i);
    index[0] = pos; index[pos] = 0;

    std::sort(index.begin()+1, index.end(), [=](double i1, double i2) {
        return cosine(i1-pos, segment[i1]-min) > cosine(i2-pos, segment[i2]-min);
    });

    // retain only farthest point if angles are equal
    int size = 1;
    for (int i=1; i<index.size(); i++) {
        while (i < index.size()-1 
            && orientation(index[i]-pos, segment[index[i]]-min, index[i+1]-index[i], segment[index[i+1]]-segment[index[i]]) == 0) i++;

        index[size++] = index[i];
    }

    // finding convex hull
    std::stack<int> stack;
    stack.push(index[0]);
    stack.push(index[1]);
    stack.push(index[2]);

    for (int i=3; i<size; i++) {
        while (stack.size()>1) {
            int top = stack.top();
            stack.pop();
            int next2top = stack.top();
            stack.push(top);

            if (orientation(top-next2top, segment[top]-segment[next2top], index[i] - top, segment[index[i]]-segment[top]) != 2) stack.pop();
            else break;
        }
        stack.push(index[i]);
    }

    while (!stack.empty()) {
        convex_hull.push_back(stack.top());
        stack.pop();
    }

    return convex_hull;
}

double distance(double a, double b, int x, double y) {
    return abs(a*x-y+b)/sqrt(a*a+1);
}

int x_external(int x1, int x2, int x) {
    // 0:  x_external
    // 1:  x_external to the right
    // -1: x_external to the left

    if (x >= x1 && x >= x2) return 1;
    else if (x <= x1 && x <= x2) return -1;
    else return 0;
}

int search(int p, int m, std::vector<int>& ccw_hull, std::vector<double>& segment) {
    int j = (p+1)%ccw_hull.size();
    double a = (segment[ccw_hull[p]]-segment[ccw_hull[j]])/(ccw_hull[p]-ccw_hull[j]);
    double b = segment[ccw_hull[j]] - a*ccw_hull[j];
    
    // search in ccw order
    // if pj(i+1) < pj(i) return i
    int v = -1; double dis = -INFINITY;
    for (int i=0; i<ccw_hull.size(); i++) {
        double temp = distance(a, b, ccw_hull[(m+i)%ccw_hull.size()], segment[ccw_hull[(m+i)%ccw_hull.size()]]);
        if (dis < temp) {
            dis = temp;
            v = (m+i)%ccw_hull.size();
        }
    }

    return v;
}

bool isSameDirection(double y1, int x1, double y2, int x2) {
    double norm_y1 = y1 / sqrt(y1*y1+x1*x1);
    double norm_x1 = x1 / sqrt(y1*y1+x1*x1);
    double norm_y2 = y2 / sqrt(y2*y2+x2*x2);
    double norm_x2 = x2 / sqrt(y2*y2+x2*x2);

    return (norm_y1 == norm_y2) && (norm_x1 == norm_y1);
}

struct Point {
    int x;
    double y;

    Point(int x, double y) {
        this->x = x;
        this->y = y;
    }
};

// Slide Filter
    int side(Point& a, Point& b, Point& c) {
        double cross_product = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);

        if (cross_product == 0) return 0;       // colinear
        else if (cross_product > 0) return 1;   // left
        else return -1;                         // right
    }

    void incremental_hull(std::vector<Point>& upper_hull, std::vector<Point>& lower_hull, Point& p) {
        int u = upper_hull.size() - 1; 
        int l = lower_hull.size() - 1;
        
        for (int i=upper_hull.size()-2; i>=0; i--) {
            Point a(upper_hull[i+1].x, upper_hull[i+1].y);
            Point b(upper_hull[i].x, upper_hull[i].y);
            
            if (side(a, b, p) <= 0) u = i;
            else break;
        }

        for (int i=lower_hull.size()-2; i>=0; i--) {
            Point a(lower_hull[i].x, lower_hull[i].y);
            Point b(lower_hull[i+1].x, lower_hull[i+1].y);
        
            if (side(a, b, p) <= 0) l = i;
            else break;
        }

        upper_hull = {upper_hull.begin(), upper_hull.begin() + u + 1};
        lower_hull = {lower_hull.begin(), lower_hull.begin() + l + 1};

        upper_hull.push_back(p); lower_hull.push_back(p);
    }

int main()
{
    vector<double> segment = {3, 1, 2, 4, 9, 3, 5, 7, 6, 2, 8};
    // find the convex hull index
    std::vector<int> convex_hull = graham_scan(segment);
    // find the upper and lower hull
    int left = 0; int right = segment.size()-1;
    // line through leftmost and rightmost points y = ax + b
    double a = (segment[right]-segment[left])/right; 
    double b = segment[left]; 
    std::vector<int> ccw_hull = {left}, upper_hull = {right};
    
    for (int i : convex_hull) {
        if (i != left && i != right) {
            if (segment[i] >= i*a+b) upper_hull.push_back(i);
            else ccw_hull.push_back(i);
        }
    }
    right = ccw_hull.size();  // index of rightmost point in ccw convex hull
    sort(upper_hull.begin(), upper_hull.end(), [](int a, int b) {return a>b;});
    sort(ccw_hull.begin(), ccw_hull.end());
    ccw_hull.insert(ccw_hull.end(), upper_hull.begin(), upper_hull.end());
    
    for (int i=0; i<ccw_hull.size(); i++) {
        cout << "(" << ccw_hull[i] << ", " << segment[ccw_hull[i]] << ")\n";
    }

    cout << "----------------\n";
    {   
        vector<Point> segment;
        segment.push_back(Point(0, 3));
        segment.push_back(Point(1, 1));
        segment.push_back(Point(2, 2));
        segment.push_back(Point(3, 4));
        segment.push_back(Point(4, 9));
        segment.push_back(Point(5, 3));
        segment.push_back(Point(6, 5));
        segment.push_back(Point(7, 7));
        segment.push_back(Point(8, 6));
        segment.push_back(Point(9, 2));
        segment.push_back(Point(10, 8));
        

        vector<Point> upper_hull = {Point(0, 3), Point(1, 1)}; 
        vector<Point> lower_hull = {Point(0, 3), Point(1, 1)};
        vector<Point> temp = {segment.begin(), segment.begin() + 2};

        for (int i=2; i<segment.size(); i++) {
            temp.push_back(segment[i]);
            incremental_hull(upper_hull, lower_hull, segment[i]);    
        }

        cout << "lower hull:\n";
        for (Point& p : lower_hull) cout << "(" << p.x << ", " << p.y <<")\n";
        cout << "upper hull:\n";
        for (Point& p : upper_hull) cout << "(" << p.x << ", " << p.y <<")\n";

        vector<Point> index = {upper_hull.begin(), upper_hull.end()-1};
        index.insert(index.end(), lower_hull.begin()+1, lower_hull.end()-1);
        for (Point& p : index) cout << "(" << p.x << ", " << p.y <<")\n";
    }

    return 0;
}