float tint_degrees(float param_0) {
  return param_0 * 57.295779513082323;
}

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  const float x_11 = a;
  b = tint_degrees(x_11);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
