float tint_float_mod(float lhs, float rhs) {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float a = 1.0f;
  float b = 0.0f;
  const float r = tint_float_mod(a, b);
  return;
}
