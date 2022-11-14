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
  const frexp_result res = tint_frexp(1.230000019f);
  const int exp = res.exp;
  const float fract = res.fract;
  return;
}
