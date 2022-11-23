struct modf_result {
  float fract;
  float whole;
};
modf_result tint_modf(float param_0) {
  modf_result result;
  result.fract = modf(param_0, result.whole);
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const float runtime_in = 1.230000019f;
  modf_result res = {0.230000019f, 1.0f};
  res = tint_modf(runtime_in);
  const modf_result c = {0.230000019f, 1.0f};
  res = c;
  const float fract = res.fract;
  const float whole = res.whole;
  return;
}
