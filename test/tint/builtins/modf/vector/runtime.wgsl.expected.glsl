#version 310 es


struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 tint_symbol_1 = vec2(1.25f, 3.75f);
  modf_result_vec2_f32 v = modf_result_vec2_f32(vec2(0.0f), vec2(0.0f));
  v.fract = modf(tint_symbol_1, v.whole);
  modf_result_vec2_f32 res = v;
  vec2 tint_symbol_2 = res.fract;
  vec2 whole = res.whole;
}
