#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  uint tint_symbol_2[2];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  if (false) {
    v.tint_symbol_2[0] = 1u;
  }
  if (false) {
    v.tint_symbol_2[1] = 1u;
  }
}
