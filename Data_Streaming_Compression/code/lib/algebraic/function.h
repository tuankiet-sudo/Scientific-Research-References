#ifndef ALGEBRAIC_FUNCTION_H
#define ALGEBRAIC_FUNCTION_H

#include <string>
#include <iostream>

struct Point2D {
    float x;
    float y;

    Point2D(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

// Geometry function support timeseries analysis
class Line {
    private:
        float slope;
        float intercept;

    public:
        Line(float slope, float intercept) {
            this->slope = slope;
            this->intercept = intercept;
        }

        float subs(float x) {
            return this->slope * x + this->intercept;
        }

        float get_slope() {
            return this->slope;
        }

        float get_intercept() {
            return this->intercept;
        }

        static Line* line(Point2D p1, Point2D p2) {
            float slope = (p1.y - p2.y) / (float) (p1.x-p2.x);
            float intercept = p1.y - slope*p1.x;

            return new Line(slope, intercept);
        }

        static Line* line(float slope, Point2D p) {
            return new Line(slope, p.y - slope*p.x);
        }

        static Point2D* intersection(Line* l1, Line* l2) {
            float x = (l1->intercept - l2->intercept) / (l2->slope - l1->slope);
            float y = l1->intercept + l1->slope*x;

            return new Point2D(x, y); 
        }

};

// Source for manipulating polynomial function
class Polynomial {
    public:
    int degree;
    float* coefficients;    // coefficient degree starts from 0 

    public:
    Polynomial(int k, const float* coeffs) {
        this->degree = k;
        this->coefficients = new float[k+1];
        for (int i=0; i<k+1; i++) {
            this->coefficients[i] = coeffs[i];
        }
    }

    Polynomial(float coeff) {
        this->degree = 0;
        this->coefficients = new float[1];
        this->coefficients[0] = coeff;
    }

    ~Polynomial() {
        delete[] this->coefficients;
    }

    template <typename T>
    float subs(const T vars) const {
        float result = this->coefficients[0];
        for (int i=1; i<this->degree+1; i++) {
            result += this->coefficients[i]*pow(vars, i);
        }

        return result;
    }

    std::string str() const {
        std::string s = "";
        for (int i=this->degree; i>0; i--) {
            s += std::to_string(this->coefficients[i]) + "x^" + std::to_string(i) + " ";
        }

        return s + std::to_string(this->coefficients[0]);
    }
};

#endif