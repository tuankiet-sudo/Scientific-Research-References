#ifndef ALGEBRAIC_CONVEX_HPP
#define ALGEBRAIC_CONVEX_HPP

#include <vector>
#include "algebraic/function.hpp"

class UpperHull {
    private:
        std::vector<Point2D> points;

    public:
        void append(Point2D point) {
            this->points.push_back(point);

            while (this->points.size() > 2) {
                int size = this->points.size();
                Point2D p1 = this->points.at(size-1);
                Point2D p2 = this->points.at(size-2);
                Point2D p3 = this->points.at(size-3);

                Line line = Line::line(p1, p3);
                if (line.subs(p2.x) >= p2.y) {
                    this->points.erase(this->points.begin()+size-2);
                }
                else {
                    break;
                }
            }
        
        }

        Point2D at(int i) {
            return this->points.at(i);
        }

        int size() {
            return this->points.size();
        }

        void clear() {
            this->points.clear();
        }

        void erase_from_begin(int length) {
            this->points.erase(this->points.begin(), this->points.begin() + length);
        }
};

class LowerHull {
    private:
        std::vector<Point2D> points;

    public:
        void append(Point2D point) {
            this->points.push_back(point);

            while (this->points.size() > 2) {
                int size = this->points.size();
                Point2D p1 = this->points.at(size-1);
                Point2D p2 = this->points.at(size-2);
                Point2D p3 = this->points.at(size-3);

                Line line = Line::line(p1, p3);
                if (line.subs(p2.x) <= p2.y) {
                    this->points.erase(this->points.begin()+size-2);
                }
                else {
                    break;
                }
            }
        
        }

        Point2D at(int i) {
            return this->points.at(i);
        }

        int size() {
            return this->points.size();
        }

        void clear() {
            this->points.clear();
        }

        void erase_from_begin(int length) {
            this->points.erase(this->points.begin(), this->points.begin() + length);
        }
};

class ConvexHull {
    private:
        LowerHull lower;
        UpperHull upper;

    public:
        void append(Point2D point) {
            this->lower.append(point);
            this->upper.append(point);
        }

        Point2D at(int i) {
            if (i < this->lower.size()) {
                return this->lower.at(i);
            }
            else {
                return this->upper.at(i-this->lower.size()+1);
            }
        }

        int size() {
            return this->lower.size() + this->upper.size() - 2;
        }

        void clear() {
            this->upper.clear();
            this->lower.clear();
        }
};

#endif