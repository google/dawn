#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[1];
} v;
vec2 func(uint pointer_indices[1]) {
  uvec4 v_1 = v.inner[((pointer_indices[0u] * 8u) / 16u)];
  return uintBitsToFloat(mix(v_1.xy, v_1.zw, bvec2(((((pointer_indices[0u] * 8u) & 15u) >> 2u) == 2u))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 r = func(uint[1](1u));
}
