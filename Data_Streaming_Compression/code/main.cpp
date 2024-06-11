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

int main() {
    
    return 0;
}