#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[2];
} v;
vec4 func(uint pointer_indices[1]) {
  return uintBitsToFloat(v.inner[((16u * pointer_indices[0u]) / 16u)]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func(uint[1](1u));
}
