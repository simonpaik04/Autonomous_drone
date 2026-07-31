double x = 2;
double w = 3;
double b = 1;
double yT = 10;

void setup() {
  Serial.begin(115200);

  double y = x * w + b;
  double error = (y - yT) * (y - yT) / 2;

  Serial.print("y: ");
  Serial.println(y, 6);
  Serial.print("error: ");
  Serial.println(error, 6);
}

void loop() {
}
