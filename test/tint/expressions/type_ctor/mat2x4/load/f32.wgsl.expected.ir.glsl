#version 310 es

mat2x4 tint_symbol;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 m = mat2x4(vec4(0.0f), vec4(0.0f));
  tint_symbol = mat2x4(m);
}
