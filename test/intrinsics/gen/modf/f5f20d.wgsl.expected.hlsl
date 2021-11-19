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

void modf_f5f20d() {
  modf_result_vec2 res = tint_modf(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_f5f20d();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_f5f20d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_f5f20d();
  return;
}
