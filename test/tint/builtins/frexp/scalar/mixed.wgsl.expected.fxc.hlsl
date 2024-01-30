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

static const frexp_result_f32 c = {0.625f, 1};
[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = {0.625f, 1};
  res = tint_frexp(runtime_in);
  res = c;
  float fract = res.fract;
  int exp = res.exp;
  return;
}
