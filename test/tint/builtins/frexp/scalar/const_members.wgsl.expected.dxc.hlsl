struct frexp_result {
  float fract;
  int exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result tint_symbol_1 = {0.625f, 1};
  const float fract = tint_symbol_1.fract;
  const frexp_result tint_symbol_2 = {0.625f, 1};
  const int exp = tint_symbol_2.exp;
  return;
}
