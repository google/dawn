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
void unused_entry_point() {
  return;
}

void i() {
  const float s = tint_modf(1.0f).whole;
}
