#version 310 es

struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

frexp_result_vec2_f32 tint_frexp(vec2 param_0) {
  frexp_result_vec2_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void tint_symbol() {
  vec2 tint_symbol_1 = vec2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = tint_frexp(tint_symbol_1);
  vec2 tint_symbol_2 = res.fract;
  ivec2 tint_symbol_3 = res.exp;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
