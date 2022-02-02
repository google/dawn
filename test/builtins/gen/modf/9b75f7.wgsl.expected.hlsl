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

void modf_9b75f7() {
  modf_result_vec3 res = tint_modf(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_9b75f7();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_9b75f7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_9b75f7();
  return;
}
