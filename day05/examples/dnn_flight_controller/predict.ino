#include <ArduinoEigen.h>

#include "model_data.h"
#include "mydnn.h"

using namespace Eigen;

const int NUM_X = 6;
const int NUM_H = 16;
const int NUM_M = 16;
const int NUM_Y = 3;

MatrixXd X;
MatrixXd H;
MatrixXd M;
MatrixXd Y;

MatrixXd WH(NUM_X, NUM_H);
MatrixXd BH(1, NUM_H);
MatrixXd WM(NUM_H, NUM_M);
MatrixXd BM(1, NUM_M);
MatrixXd WY(NUM_M, NUM_Y);
MatrixXd BY(1, NUM_Y);

void dnn_initialize() {
  WH = Map<Matrix<double, NUM_X, NUM_H, RowMajor>>(_WH);
  BH = Map<MatrixXd>(_BH, 1, NUM_H);
  WM = Map<Matrix<double, NUM_H, NUM_M, RowMajor>>(_WM);
  BM = Map<MatrixXd>(_BM, 1, NUM_M);
  WY = Map<Matrix<double, NUM_M, NUM_Y, RowMajor>>(_WY);
  BY = Map<MatrixXd>(_BY, 1, NUM_Y);
}

void dnn_predict(double x[], const int numX, double y[], const int numY) {
  if (numX != NUM_X || numY != NUM_Y) {
    for (int i = 0; i < numY; i++) {
      y[i] = 0.0;
    }
    return;
  }

  X = Map<MatrixXd>(x, 1, NUM_X);

  H = X * WH + BH;
  relu_f(H);
  M = H * WM + BM;
  relu_f(M);
  Y = M * WY + BY;

  Map<MatrixXd>(y, 1, NUM_Y) = Y;
}
