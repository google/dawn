#version 310 es


struct S {
  int a;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  S tint_symbol_1;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int u = (v_1.tint_symbol_1.a + 1);
}
