#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[1];
} v;
vec4 func() {
  return uintBitsToFloat(v.inner[0u]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func();
}
