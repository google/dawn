struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
void modf_f3d1f9() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_f3d1f9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_f3d1f9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_f3d1f9();
  return;
}
