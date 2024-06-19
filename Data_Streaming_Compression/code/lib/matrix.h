#ifndef MATRIX
#define MATRIX

#include "dependencies.h"

namespace HPCLab {
    // Source for manipulating matrix
    template <class T>
    class Matrix {
      private:
        int height;
        int width; 

      public:
        T** cell;
        
        Matrix(int height, int width) {
            this->height = height;
            this->width = width;
            
            this->cell = new T*[height];
            for (int i=0; i<height; i++) {
                this->cell[i] = new T[width];
            }
        }

        ~Matrix() {
            for (int i=0; i<this->height; i++) {
                delete[] this->cell[i];
            }
            delete[] this->cell;
        }

        int get_height() const {
            return this->height;
        }

        int get_width() const {
            return this->width;
        }

        T* toVec() const {
            if (this->width == 1) {
                T* vec = new T[this->height];
                for (int i=0; i<this->height; i++) {
                    vec[i] = this->cell[i][0];
                }

                return vec;
            }
            else if (this->height == 1){
                T* vec = new T[this->width];
                for (int i=0; i<this->width; i++) {
                    vec[i] = this->cell[0][1];
                }

                return vec;
            }
            
            std::cout << "Can not convert this matrix to vector !!" << std::endl;
            exit(-1);
        }

        Matrix<T>* transpose() const {
            Matrix<T>* matrix = new Matrix<T>(this->width, this->height);
            for (int i=0; i<this->width; i++) {
                for (int j=0; j<this->height; j++) {
                    matrix->cell[i][j] = this->cell[j][i];
                }
            }
            
            return matrix;
        }

        // Gauss Jordan method
        // Required square matrix
        Matrix<float>* inverse() const {
            if (this->height != this->width) {
                std::cout << "Can not inverse non square matrix !!" << std::endl;
                exit(-1);
            }

            Matrix<float>* matrix = new Matrix<float>(this->height, this->width*2);
            for (int i=0; i<this->height; i++) {
                for (int j=0; j<this->width; j++) {
                    if (i==j) matrix->cell[i][j+this->width] = 1;
                    else matrix->cell[i][j+this->width] = 0;

                    matrix->cell[i][j] = this->cell[i][j];
                }
            }

            // Swap rows
            for (int i=this->height-1; i>0; i--) {
                if(matrix->cell[i-1][0] < matrix->cell[i][0]) {
                    float* temp = matrix->cell[i];
                    matrix->cell[i] = matrix->cell[i-1];
                    matrix->cell[i-1] = temp;
                }
            }

            // Apply Gauss Jordan method
            for (int i=0; i<this->height; i++) {
                for (int j=0; j<this->width; j++) {
                    if (j!=i) {
                        float ratio = matrix->cell[j][i]/matrix->cell[i][i];
                        for (int k=0; k<this->width*2; k++) {
                            matrix->cell[j][k] -= ratio*matrix->cell[i][k];
                        }
                    }
                }
            }

            for (int i=0; i<this->width; i++) {
                float temp = matrix->cell[i][i];
                for (int j=0; j<this->width*2; j++) {
                    matrix->cell[i][j] = matrix->cell[i][j]/temp;
                }
            }

            // Retrieve inverse matrix
            Matrix<float>* result = new Matrix<float>(this->height, this->width);
            for (int i=0; i<this->height; i++) {
                for (int j=0; j<this->width; j++) {
                    result->cell[i][j] = matrix->cell[i][j+this->width];
                }
            }

            delete matrix;
            return result;
        }
    };

    template <typename T>
    Matrix<T>* matrix_outter_product(const Matrix<T>* A, const Matrix<T>* B) {
        if (A->get_width() != B->get_height()) {
            std::cout << "Can not multiply matrix size " << A->get_height() << "x" << A->get_width()
                << " with " << B->get_height() << "x" << B->get_width() << " !!" << std::endl;
            exit(-1);
        }

        Matrix<T>* matrix = new Matrix<T>(A->get_height(), B->get_width());
        for (int i=0; i<A->get_height(); i++) {
            for (int j=0; j<B->get_width(); j++) {
                matrix->cell[i][j] = 0;
                for (int k=0; k<A->get_width(); k++) {
                    matrix->cell[i][j] += A->cell[i][k]*B->cell[k][j];
                }
            }
        }

        return matrix;
    }

    template <typename T>
    Matrix<T>* matrix_inner_product(const Matrix<T>* A, const Matrix<T>* B) {
        if (A->get_width() != B->get_width() || A->get_height() != B->get_height()) {
            std::cout << "Can not perform element-wise product of matries with different size !!";
            exit(-1);
        }

        Matrix<T>* matrix = new Matrix<T>(A->get_height(), B->get_width());
        for (int i=0; i<A->get_height(); i++) {
            for (int j=0; j<B->get_width(); j++) {
                matrix->cell[i][j] = A->cell[i][j] * B->cell[i][j];
            }
        }

        return matrix;
    }

}

#endif