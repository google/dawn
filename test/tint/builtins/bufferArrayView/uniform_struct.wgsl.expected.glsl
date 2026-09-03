#version 310 es

layout(binding = 0, std140)
uniform v_block_1_ubo {
  uvec4 inner[8];
} v_1;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  vec2 inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  min(0u, (2u - 1u));
  v_2.inner = uintBitsToFloat(v_1.inner[2u].xy);
}
