#ifndef MY_DNN_H
#define MY_DNN_H

#include <ArduinoEigen.h>

using namespace Eigen;

void sigmoid_f(MatrixXd &X);
void sigmoid_b(MatrixXd &XE, MatrixXd &X);
void relu_f(MatrixXd &X);
void relu_b(MatrixXd &XE, MatrixXd &X);
void softmax_f(MatrixXd &X);
void softmax_b(MatrixXd &XE, MatrixXd &X);
void initialize_weight_He(MatrixXd &W);
void initialize_weight_Le(MatrixXd &W);
double calculate_MSE(MatrixXd &Y, MatrixXd &YT);
double calculate_CEE(MatrixXd &Y, MatrixXd &YT);

#endif
