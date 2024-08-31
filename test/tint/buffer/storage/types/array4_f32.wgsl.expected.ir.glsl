#version 310 es

float tint_symbol[4];
float tint_symbol_1[4];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1 = tint_symbol;
}
