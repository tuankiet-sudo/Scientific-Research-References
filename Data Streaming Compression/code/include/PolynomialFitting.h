#ifndef STD_LIBS
#define STD_LIBS
// Begin: define libraries
#include<vector>
#include<math.h>
// End: define libraries
#endif

class Function {
    private:
        int degree;
        float* coefficients;    // coefficient degree starts from 0 

    public:
        Function(int k, const float* coeffs) {
            this->degree = k;
            this->coefficients = new float[k+1];
            for (int i=0; i<k+1; i++) {
                this->coefficients[i] = coeffs[i];
            }
        }

        ~Function() {
            delete[] this->coefficients;
        }

        float substitute(const int vars) {
            float result = this->coefficients[0];
            for (int i=1; i<this->degree+1; i++) {
                result += this->coefficients[i]*pow(vars, i);
            }

            return result;
        }

        std::vector<float> substitute(std::vector<int> vars) {
            std::vector<float> result;
            for (int var : vars) {
                result.push_back(this->substitute(var));
            }
            
            return result;
        }
};

// Begin: Class of constant fitting
Function* PMCMidRange();
// End: Class of constant fitting


// Begin: Class of linear fitting

// End: Class of linear fitting


// Begin: Class of polynomial fitting

// End: Class of polynomial fitting