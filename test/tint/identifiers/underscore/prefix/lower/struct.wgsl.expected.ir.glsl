#version 310 es


struct _a {
  int _b;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  _a c = _a(0);
  int d = c._b;
  v.tint_symbol = (c._b + d);
}
