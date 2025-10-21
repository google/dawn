#version 310 es


struct str {
  ivec4 i;
};

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[4];
} v;
str v_1(uint start_byte_offset) {
  return str(ivec4(v.inner[(start_byte_offset / 16u)]));
}
str func(uint pointer_indices[1]) {
  return v_1((16u * pointer_indices[0u]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str r = func(uint[1](2u));
}
