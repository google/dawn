#version 310 es

void tint_symbol() {
  float tint_symbol_2 = 0.625f;
  int tint_symbol_3 = 1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
