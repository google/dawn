[numthreads(1, 1, 1)]
void main() {
  int exponent = 0;
  float tint_tmp;
  float tint_tmp_1 = frexp(1.230000019f, tint_tmp);
  exponent = int(tint_tmp);
  const float significand = tint_tmp_1;
  return;
}
