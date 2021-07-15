float tint_frexp(float param_0, inout int param_1) {
  float float_exp;
  float significand = frexp(param_0, float_exp);
  param_1 = int(float_exp);
  return significand;
}

[numthreads(1, 1, 1)]
void main() {
  int exponent = 0;
  const float significand = tint_frexp(1.230000019f, exponent);
  return;
}
