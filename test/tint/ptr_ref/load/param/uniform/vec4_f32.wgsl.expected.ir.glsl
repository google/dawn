#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  vec4 tint_symbol_1;
} v;
vec4 func() {
  return v.tint_symbol_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func();
}
