#include <vector>
#include <cmath>
#include <iostream>
#include <eigen/Dense>

struct Point2D {
    float x;
    float y;

    Point2D(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

void check(std::vector<Point2D>& points) {
    int degree = points.size()-1;

    Eigen::MatrixXd A(degree+1, degree+1);
    Eigen::VectorXd b(degree+1);

    for (int i=0; i<points.size(); i++) {
        Point2D& p = points[i];

        for (int j=0; j<points.size(); j++) {
            A(i, j) = pow(p.x, degree-j);
        }
        b(i) = p.y; 
    }

    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);
    std::cout << "The solution is:\n" << x.transpose() << std::endl;
}

int main() {
    std::vector<Point2D> points;
    points.push_back(Point2D(0, 50));
    points.push_back(Point2D(50, 25));
    points.push_back(Point2D(200, 800-175));
    // points.push_back(Point2D(65, 50)); 

    check(points);
    return 0;
}