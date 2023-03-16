struct frexp_result_f32 {
  float fract;
  int exp;
};
frexp_result_f32 tint_frexp(float param_0) {
  float exp;
  float fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_f32 result = {fract, int(exp)};
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const float tint_symbol = 1.25f;
  const frexp_result_f32 res = tint_frexp(tint_symbol);
  const float fract = res.fract;
  const int exp = res.exp;
  return;
}
