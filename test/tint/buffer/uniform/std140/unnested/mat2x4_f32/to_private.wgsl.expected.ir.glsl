#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat2x4 tint_symbol;
} v;
mat2x4 p = mat2x4(vec4(0.0f), vec4(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.tint_symbol;
  p[1] = v.tint_symbol[0];
  p[1] = v.tint_symbol[0].ywxz;
  p[0][1] = v.tint_symbol[1].x;
}
