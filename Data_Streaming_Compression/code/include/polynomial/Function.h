#ifndef STD_LIBS
#define STD_LIBS
// Begin: define std libraries
#include <cmath>
// End: define std libraries
#endif

#ifndef CUSTOM_LIBS
#define CUSTOM_LIBS
// Begin: define custom libraries
#include "TimeSeries.h"
#include "Matrix.h"
// End: define custom libraries
#endif

class PolynomialFunction {
    private:
        int degree;
        float* coefficients;    // coefficient degree starts from 0 

    public:
        PolynomialFunction(int k, const float* coeffs) {
            this->degree = k;
            this->coefficients = new float[k+1];
            for (int i=0; i<k+1; i++) {
                this->coefficients[i] = coeffs[i];
            }
        }

        ~PolynomialFunction() {
            delete[] this->coefficients;
        }

        float substitute(const int vars) {
            float result = this->coefficients[0];
            for (int i=1; i<this->degree+1; i++) {
                result += this->coefficients[i]*pow(vars, i);
            }

            return result;
        }
};