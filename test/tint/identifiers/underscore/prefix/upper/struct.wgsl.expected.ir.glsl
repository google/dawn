#version 310 es


struct _A {
  int _B;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  _A c = _A(0);
  int d = c._B;
  v.tint_symbol = (c._B + d);
}
