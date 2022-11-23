struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};
modf_result_vec2_f32 tint_modf(float2 param_0) {
  modf_result_vec2_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_2d50da() {
  float2 arg_0 = (-1.5f).xx;
  modf_result_vec2_f32 res = tint_modf(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_2d50da();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_2d50da();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_2d50da();
  return;
}
