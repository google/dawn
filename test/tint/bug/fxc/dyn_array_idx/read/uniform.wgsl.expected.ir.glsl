#version 310 es


struct UBO {
  ivec4 data[4];
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  UBO tint_symbol_1;
} v;
layout(binding = 2, std430)
buffer tint_symbol_4_1_ssbo {
  Result tint_symbol_3;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_2 = v.tint_symbol_1.dynamic_idx;
  v_1.tint_symbol_3.tint_symbol = v.tint_symbol_1.data[v_2].x;
}
