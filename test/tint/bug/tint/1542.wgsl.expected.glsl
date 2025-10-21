#version 310 es

layout(binding = 0, std140)
uniform u_input_block_1_ubo {
  uvec4 inner[1];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 temp = ivec3((uvec3(ivec3(v.inner[0u].xyz)) << (uvec3(0u) & uvec3(31u))));
}
