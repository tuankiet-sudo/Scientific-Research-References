#include <iostream>
#include "polynomial/Polynomial.h"

using namespace std;

void print(Matrix<int>* matrix) {
    for (int i=0; i<matrix->num_row; i++) {
        for (int j=0; j<matrix->num_col; j++) {
            cout << matrix->cell[i][j] << " ";
        }
        cout << endl;
    }
}

void print(Matrix<float>* matrix) {
    for (int i=0; i<matrix->num_row; i++) {
        for (int j=0; j<matrix->num_col; j++) {
            cout << matrix->cell[i][j] << " ";
        }
        cout << endl;
    }
}

int main() {
    Matrix<int>* matrix = new Matrix<int>(3, 3);
    matrix->cell[0][0] = 6; matrix->cell[0][1] = 9; matrix->cell[0][2] = 5;
    matrix->cell[1][0] = 8; matrix->cell[1][1] = 3; matrix->cell[1][2] = 2;
    matrix->cell[2][0] = 1; matrix->cell[2][1] = 4; matrix->cell[2][2] = 7;

    Matrix<float>* inverse = matrix->inverse();
    print(matrix);
    cout << endl;
    print(inverse);

    delete matrix;
    delete inverse;
    return 0;
}