template <class T>
struct Matrix {
    int num_row;
    int num_col;
    T** cell;

    Matrix(const int& rows, const int& cols) {
        this->num_row = rows;
        this->num_col = cols;

        this->cell = new T*[rows];
        for (int i=0; i<rows; i++) {
            this->cell[i] = new T[cols];
        }
    }

    ~Matrix() {
        for (int i=0; i<this->num_row; i++) {
            delete[] this->cell[i];
        }
        delete[] this->cell;
    }

    Matrix* transpose() {
        Matrix* matrix = new Matrix(this->num_row, this->num_col);
        for (int i=0; i<this->num_row; i++) {
            for (int j=0; j<this->num_col; j++) {
                matrix->cell[i][j] = this->cell[j][i];
            }
        }
        
        return matrix;
    }

    // Gauss Jordan method
    // Required square matrix
    Matrix<float>* inverse() {
        Matrix<float>* matrix = new Matrix<float>(this->num_row, this->num_col*2);
        for (int i=0; i<this->num_row; i++) {
            for (int j=0; j<this->num_col; j++) {
                if (i==j) matrix->cell[i][j+this->num_col] = 1;
                else matrix->cell[i][j+this->num_col] = 0;

                matrix->cell[i][j] = this->cell[i][j];
            }
        }

        // swap rows
        for (int i=this->num_row-1; i>0; i--) {
            if(matrix->cell[i-1][0] < matrix->cell[i][0]) {
                float* temp = matrix->cell[i];
                matrix->cell[i] = matrix->cell[i-1];
                matrix->cell[i-1] = temp;
            }
        }

        for (int i=0; i<this->num_row; i++) {
            for (int j=0; j<this->num_col; j++) {
                if (j!=i) {
                    float ratio = matrix->cell[j][i]/matrix->cell[i][i];
                    for (int k=0; k<this->num_col*2; k++) {
                        matrix->cell[j][k] -= ratio*matrix->cell[i][k];
                    }
                }
            }
        }

        for (int i=0; i<this->num_col; i++) {
            float temp = matrix->cell[i][i];
            for (int j=0; j<this->num_col*2; j++) {
                matrix->cell[i][j] = matrix->cell[i][j]/temp;
            }
        }

        Matrix<float>* result = new Matrix<float>(this->num_col, this->num_row);
        for (int i=0; i<num_row; i++) {
            for (int j=0; j<num_col; j++) {
                result->cell[i][j] = matrix->cell[i][j+this->num_col];
            }
        }

        delete matrix;
        return result;
    }

    
    template <typename P>
    static Matrix<P>* multiply(Matrix<P>* A, Matrix<P>* B) {
        if (A->num_col != B->num_row) {
            throw "Matrix shape error";
            exit(-1);
        }

        Matrix<P>* matrix = new Matrix<P>(A->num_row, B->num_col);
        for (int i=0; i<A->num_row; i++) {
            for (int j=0; j<B->num_col; j++) {
                matrix->cell[i][j] = 0;
                for (int k=0; k<A->num_col; k++) {
                    matrix->cell[i][j] += A->cell[i][k]*B->cell[k][j];
                }
            }
        }

        return matrix;
    }
};