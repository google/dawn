#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 runtime_in = vec2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = frexp_result_vec2_f32(vec2(0.625f, 0.9375f), ivec2(1, 2));
  frexp_result_vec2_f32 v = frexp_result_vec2_f32(vec2(0.0f), ivec2(0));
  v.fract = frexp(runtime_in, v.exp);
  res = v;
  res = frexp_result_vec2_f32(vec2(0.625f, 0.9375f), ivec2(1, 2));
  vec2 tint_symbol_1 = res.fract;
  ivec2 tint_symbol_2 = res.exp;
}
