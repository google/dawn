struct modf_result {
  float fract;
  float whole;
};
modf_result tint_modf(float param_0) {
  float whole;
  float fract = modf(param_0, whole);
  modf_result result = {fract, whole};
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  const modf_result res = tint_modf(1.230000019f);
  const float fract = res.fract;
  const float whole = res.whole;
  return;
}
