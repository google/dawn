#version 310 es


struct UBO {
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

struct SSBO {
  int data[4];
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  UBO tint_symbol_1;
} v;
layout(binding = 2, std430)
buffer tint_symbol_4_1_ssbo {
  Result tint_symbol_3;
} v_1;
layout(binding = 1, std430)
buffer tint_symbol_6_1_ssbo {
  SSBO tint_symbol_5;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_3 = v.tint_symbol_1.dynamic_idx;
  v_1.tint_symbol_3.tint_symbol = v_2.tint_symbol_5.data[v_3];
}
