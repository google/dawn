struct modf_result_vec2 {
  float2 fract;
  float2 whole;
};
[numthreads(1, 1, 1)]
void main() {
  const modf_result_vec2 tint_symbol_1 = {float2(0.230000019f, 0.450000048f), float2(1.0f, 3.0f)};
  const float2 fract = tint_symbol_1.fract;
  const modf_result_vec2 tint_symbol_2 = {float2(0.230000019f, 0.450000048f), float2(1.0f, 3.0f)};
  const float2 whole = tint_symbol_2.whole;
  return;
}
