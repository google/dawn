#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 tint_symbol_1 = vec2(1.25f, 3.75f);
  frexp_result_vec2_f32 v = frexp_result_vec2_f32(vec2(0.0f), ivec2(0));
  v.fract = frexp(tint_symbol_1, v.exp);
  frexp_result_vec2_f32 res = v;
  vec2 tint_symbol_2 = res.fract;
  ivec2 tint_symbol_3 = res.exp;
}
