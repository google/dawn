struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};
modf_result_vec2_f16 tint_modf(vector<float16_t, 2> param_0) {
  modf_result_vec2_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_a545b9() {
  vector<float16_t, 2> arg_0 = (float16_t(-1.5h)).xx;
  modf_result_vec2_f16 res = tint_modf(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_a545b9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_a545b9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_a545b9();
  return;
}
