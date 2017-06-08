#include "dummy.h"

int dummy(int a, int b){
    return a + b;
}

Eigen::MatrixXd dummyM(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B){
    return A + B;
}