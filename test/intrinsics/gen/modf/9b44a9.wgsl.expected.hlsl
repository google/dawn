struct modf_result_vec4 {
  float4 fract;
  float4 whole;
};
modf_result_vec4 tint_modf(float4 param_0) {
  float4 whole;
  float4 fract = modf(param_0, whole);
  modf_result_vec4 result = {fract, whole};
  return result;
}

void modf_9b44a9() {
  modf_result_vec4 res = tint_modf(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_9b44a9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_9b44a9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_9b44a9();
  return;
}
