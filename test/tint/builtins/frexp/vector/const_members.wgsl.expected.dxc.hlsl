struct frexp_result_vec2 {
  float2 fract;
  int2 exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result_vec2 tint_symbol_1 = {float2(0.625f, 0.9375f), int2(1, 2)};
  const float2 fract = tint_symbol_1.fract;
  const frexp_result_vec2 tint_symbol_2 = {float2(0.625f, 0.9375f), int2(1, 2)};
  const int2 exp = tint_symbol_2.exp;
  return;
}
