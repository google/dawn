#version 310 es

struct modf_result_vec2 {
  vec2 fract;
  vec2 whole;
};


void tint_symbol() {
  modf_result_vec2 tint_symbol_3 = modf_result_vec2(vec2(0.25f, 0.75f), vec2(1.0f, 3.0f));
  vec2 tint_symbol_2 = tint_symbol_3.fract;
  modf_result_vec2 tint_symbol_4 = modf_result_vec2(vec2(0.25f, 0.75f), vec2(1.0f, 3.0f));
  vec2 whole = tint_symbol_4.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
