#version 310 es


struct str {
  int i;
};

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  str tint_symbol_1[4];
} v;
str func(uint pointer_indices[1]) {
  return v.tint_symbol_1[pointer_indices[0u]];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str r = func(uint[1](uint(2)));
}
