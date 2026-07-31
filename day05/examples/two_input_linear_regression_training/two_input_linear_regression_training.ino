double x1 = 2;
double x2 = 3;
double w1 = 3;
double w2 = 4;
double b = 1;
double yT = 27;
double learningRate = 0.01;

void setup() {
  Serial.begin(115200);

  for (int epoch = 0; epoch < 200; epoch++) {
    double y = x1 * w1 + x2 * w2 + b;
    double error = (y - yT) * (y - yT) / 2;
    double outputGradient = y - yT;
    double weight1Gradient = outputGradient * x1;
    double weight2Gradient = outputGradient * x2;
    double biasGradient = outputGradient;

    w1 -= learningRate * weight1Gradient;
    w2 -= learningRate * weight2Gradient;
    b -= learningRate * biasGradient;

    Serial.print("epoch: ");
    Serial.print(epoch);
    Serial.print(" | error: ");
    Serial.print(error, 6);
    Serial.print(" | y: ");
    Serial.print(y, 6);
    Serial.print(" | w1: ");
    Serial.print(w1, 6);
    Serial.print(" | w2: ");
    Serial.print(w2, 6);
    Serial.print(" | b: ");
    Serial.println(b, 6);
  }
}

void loop() {
}
