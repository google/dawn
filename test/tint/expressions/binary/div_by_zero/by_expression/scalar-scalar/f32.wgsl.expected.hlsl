[numthreads(1, 1, 1)]
void f() {
  float a = 1.0f;
  float b = 0.0f;
  const float r = (a / (b + b));
  return;
}
