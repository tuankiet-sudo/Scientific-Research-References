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
    Matrix* inverse() {
        Matrix* matrix = new Matrix(this->num_row, this->num_col);
        Matrix* identity = new Matrix(this->num_row, this->num_col);

        for (int i=0; i<this->num_row; i++) {
            for (int j=0; j<this->num_col; j++) {
                if (i==j) identity[i][j] = 1;
                else identity[i][j] = 0;
            }
        }

        for (int i=0; i<this->num_row; i++) {
            if (matrix[i][i]==0) {
                throw "Mathematical Error!";
                exit(-1);
            }
            for (int j=0; j<this->num_col; j++) {
                if (i!=j) {
                    float ratio = matrix[j][i]/matrix[i][i];
                    for (int k=0; k<2*n; k++) {
                        matrix[j][k] = matrix[j][k] - ratio*matrix[i][k];
                    }
                }
            }
        }

        for (int i=0; i<this->num_col; i++) {
            for (int j=this->num_col; j<2*this->num_col; j++) {
                matrix[i][j] = matrix[i][j]/matrix[i][i];
            }
        }

        return matrix;
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