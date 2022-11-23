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
  const float2 runtime_in = float2(1.230000019f, 3.450000048f);
  modf_result_vec2 res = {float2(0.230000019f, 0.450000048f), float2(1.0f, 3.0f)};
  res = tint_modf(runtime_in);
  const modf_result_vec2 c = {float2(0.230000019f, 0.450000048f), float2(1.0f, 3.0f)};
  res = c;
  const float2 fract = res.fract;
  const float2 whole = res.whole;
  return;
}
