#include "mydnn.h"

void sigmoid_f(MatrixXd &X) {
  X = (1 / (1 + exp(-X.array()))).matrix();
}

void sigmoid_b(MatrixXd &XE, MatrixXd &X) {
  XE = (X.array() * (1 - X.array()) * XE.array()).matrix();
}

void relu_f(MatrixXd &X) {
  X = ((X.array() > 0).cast<double>() * X.array()).matrix();
}

void relu_b(MatrixXd &XE, MatrixXd &X) {
  XE = ((X.array() > 0).cast<double>() * XE.array()).matrix();
}

void softmax_f(MatrixXd &X) {
  double xMax = X.maxCoeff();
  X.array() -= xMax;
  double sumX = exp(X.array()).matrix().sum();
  X.array() = exp(X.array()) / sumX;
}

void softmax_b(MatrixXd &XE, MatrixXd &X) {
  (void)XE;
  (void)X;
}

void initialize_weight_He(MatrixXd &W) {
  W.setRandom();
  W = W * sqrt(6.0 / W.rows());
}

void initialize_weight_Le(MatrixXd &W) {
  W.setRandom();
  W = W * sqrt(3.0 / W.rows());
}

double calculate_MSE(MatrixXd &Y, MatrixXd &YT) {
  MatrixXd E = (Y - YT) * (Y - YT).transpose() / 2;
  return E(0, 0);
}

double calculate_CEE(MatrixXd &Y, MatrixXd &YT) {
  const double epsilon = 1e-12;
  return -(YT.array() * log(Y.array() + epsilon)).matrix().sum();
}
