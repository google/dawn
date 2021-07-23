struct modf_result_vec3 {
  float3 fract;
  float3 whole;
};
modf_result_vec3 tint_modf(float3 param_0) {
  float3 whole;
  float3 fract = modf(param_0, whole);
  modf_result_vec3 result = {fract, whole};
  return result;
}

void modf_2199f1() {
  modf_result_vec3 res = tint_modf(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_2199f1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_2199f1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_2199f1();
  return;
}
