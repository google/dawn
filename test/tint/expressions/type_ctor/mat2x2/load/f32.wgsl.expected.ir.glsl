#version 310 es

mat2 tint_symbol;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  tint_symbol = mat2(m);
}
