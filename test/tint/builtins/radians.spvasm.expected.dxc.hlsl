float tint_radians(float param_0) {
  return param_0 * 0.01745329251994329547;
}

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  const float x_11 = a;
  b = tint_radians(x_11);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
