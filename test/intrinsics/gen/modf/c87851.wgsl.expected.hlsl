struct modf_result_vec2 {
  float2 fract;
  float2 whole;
};
modf_result_vec2 tint_modf(float2 param_0) {
  float2 whole;
  float2 fract = modf(param_0, whole);
  modf_result_vec2 result = {fract, whole};
  return result;
}

void modf_c87851() {
  modf_result_vec2 res = tint_modf(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_c87851();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_c87851();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_c87851();
  return;
}
