struct modf_result {
  float fract;
  float whole;
};
[numthreads(1, 1, 1)]
void main() {
  const modf_result tint_symbol_1 = {0.25f, 1.0f};
  const float fract = tint_symbol_1.fract;
  const modf_result tint_symbol_2 = {0.25f, 1.0f};
  const float whole = tint_symbol_2.whole;
  return;
}
