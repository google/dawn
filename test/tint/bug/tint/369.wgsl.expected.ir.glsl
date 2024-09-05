#version 310 es


struct S {
  mat2 m;
};

struct S_std140 {
  vec2 m_col0;
  vec2 m_col1;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v;
layout(binding = 0, std140)
uniform tint_symbol_3_std140_1_ubo {
  S_std140 tint_symbol_2;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
