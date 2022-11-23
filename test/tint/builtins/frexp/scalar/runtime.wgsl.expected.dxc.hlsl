struct frexp_result {
  float fract;
  int exp;
};
frexp_result tint_frexp(float param_0) {
  float exp;
  float fract = frexp(param_0, exp);
  frexp_result result = {fract, int(exp)};
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const float tint_symbol = 1.25f;
  const frexp_result res = tint_frexp(tint_symbol);
  const float fract = res.fract;
  const int exp = res.exp;
  return;
}
