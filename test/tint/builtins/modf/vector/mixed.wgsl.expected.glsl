#version 310 es

struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};

modf_result_vec2_f32 tint_modf(vec2 param_0) {
  modf_result_vec2_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void tint_symbol() {
  vec2 runtime_in = vec2(1.25f, 3.75f);
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(0.25f, 0.75f), vec2(1.0f, 3.0f));
  res = tint_modf(runtime_in);
  res = modf_result_vec2_f32(vec2(0.25f, 0.75f), vec2(1.0f, 3.0f));
  vec2 tint_symbol_1 = res.fract;
  vec2 whole = res.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
