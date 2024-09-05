#version 310 es


struct tint_symbol {
  int tint_symbol_1;
};

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol c = tint_symbol(0);
  int d = c.tint_symbol_1;
  v.tint_symbol_2 = (c.tint_symbol_1 + d);
}
