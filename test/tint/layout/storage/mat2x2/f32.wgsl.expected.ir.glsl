#version 310 es


struct SSBO {
  mat2 m;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  SSBO tint_symbol;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 v = v_1.tint_symbol.m;
  v_1.tint_symbol.m = v;
}
