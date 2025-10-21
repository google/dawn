#version 310 es


struct S {
  mat2 m;
};

layout(binding = 0, std430)
buffer SSBO_block_1_ssbo {
  S inner;
} v;
layout(binding = 0, std140)
uniform UBO_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
