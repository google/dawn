float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const float a = 1.0f;
  const float b = 2.0f;
  const float r = tint_float_mod(a, b);
  return;
}
