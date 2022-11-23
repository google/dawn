struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
modf_result_vec4_f32 tint_modf(float4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_4bfced();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_4bfced();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_4bfced();
  return;
}
