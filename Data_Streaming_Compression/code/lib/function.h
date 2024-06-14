// Define all necessary mathematics function
#ifndef FUNCTION
#define FUNCTION

#include "dependencies.h"

namespace HPCLab {
    // Source for manipulating polynomial function
    class Polynomial {
      private:
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

        ~Polynomial() {
            delete[] this->coefficients;
        }

        template <typename T>
        float substitute(const T vars) const {
            float result = this->coefficients[0];
            for (int i=1; i<this->degree+1; i++) {
                result += this->coefficients[i]*pow(vars, i);
            }

            return result;
        }
    };
}

#endif