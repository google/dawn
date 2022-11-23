#version 310 es

struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};


void tint_symbol() {
  frexp_result_vec2_f32 tint_symbol_4 = frexp_result_vec2_f32(vec2(0.625f, 0.9375f), ivec2(1, 2));
  vec2 tint_symbol_2 = tint_symbol_4.fract;
  frexp_result_vec2_f32 tint_symbol_5 = frexp_result_vec2_f32(vec2(0.625f, 0.9375f), ivec2(1, 2));
  ivec2 tint_symbol_3 = tint_symbol_5.exp;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
