#version 310 es

struct S {
  float f;
};

S tint_symbol;
S tint_symbol_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1 = tint_symbol;
}
