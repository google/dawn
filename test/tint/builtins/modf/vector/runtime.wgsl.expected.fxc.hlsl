struct modf_result_vec2 {
  float2 fract;
  float2 whole;
};
modf_result_vec2 tint_modf(float2 param_0) {
  modf_result_vec2 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const float2 tint_symbol = float2(1.230000019f, 3.450000048f);
  const modf_result_vec2 res = tint_modf(tint_symbol);
  const float2 fract = res.fract;
  const float2 whole = res.whole;
  return;
}
