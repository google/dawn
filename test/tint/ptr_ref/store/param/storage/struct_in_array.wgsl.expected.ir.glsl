#version 310 es


struct str {
  int i;
};

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  str tint_symbol_1[4];
} v;
void func(uint pointer_indices[1]) {
  v.tint_symbol_1[pointer_indices[0u]] = str(0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func(uint[1](uint(2)));
}
