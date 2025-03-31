#include <iostream>
#include <math.h>
#include <eigen/Eigen>

using namespace std;

struct Point2D {
    double x;
    double y;

    Point2D(double x, double y) {
        this->x = x;
        this->y = y;
    }
};

int main() {
    Point2D p1(0, 13.0766);
    Point2D p2(6, 21.6365);
    Point2D p3(14, 11.2126);
    Point2D p4(27, 48.2568);
    Point2D p5(33, 5.25507);

    int SIZE = 5;
    Eigen::MatrixXd A(SIZE, SIZE);
    A << pow(p1.x, 4), pow(p1.x, 3), pow(p1.x, 2), p1.x, 1,
        pow(p2.x, 4), pow(p2.x, 3), pow(p2.x, 2), p2.x, 1,
        pow(p3.x, 4), pow(p3.x, 3), pow(p3.x, 2), p3.x, 1,
        pow(p4.x, 4), pow(p4.x, 3), pow(p4.x, 2), p4.x, 1,
        pow(p5.x, 4), pow(p5.x, 3), pow(p5.x, 2), p5.x, 1;

    Eigen::VectorXd b(SIZE);
    b << p1.y, p2.y, p3.y, p4.y, p5.y;

    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    cout << x.transpose() << endl;
}