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
  const float runtime_in = 1.25f;
  frexp_result res = {0.625f, 1};
  res = tint_frexp(runtime_in);
  const frexp_result c = {0.625f, 1};
  res = c;
  const float fract = res.fract;
  const int exp = res.exp;
  return;
}
