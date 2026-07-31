double x = 2;
double w = 3;
double b = 1;
double yT = 10;
double learningRate = 0.001;

void setup() {
  Serial.begin(115200);

  for (int epoch = 0; epoch < 10000; epoch++) {
    double y = x * w + b;
    double error = (y - yT) * (y - yT) / 2;
    double outputGradient = y - yT;
    double weightGradient = outputGradient * x;
    double biasGradient = outputGradient;

    w -= learningRate * weightGradient;
    b -= learningRate * biasGradient;

    if (epoch % 100 == 0 || epoch == 9999) {
      Serial.print("epoch: ");
      Serial.print(epoch);
      Serial.print(" | error: ");
      Serial.print(error, 6);
      Serial.print(" | y: ");
      Serial.print(y, 6);
      Serial.print(" | w: ");
      Serial.print(w, 6);
      Serial.print(" | b: ");
      Serial.println(b, 6);
    }
  }
}

void loop() {
}
