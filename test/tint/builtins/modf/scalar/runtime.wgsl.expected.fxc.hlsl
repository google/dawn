struct modf_result_f32 {
  float fract;
  float whole;
};
modf_result_f32 tint_modf(float param_0) {
  modf_result_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const float tint_symbol = 1.25f;
  const modf_result_f32 res = tint_modf(tint_symbol);
  const float fract = res.fract;
  const float whole = res.whole;
  return;
}
