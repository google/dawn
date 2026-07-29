#version 310 es

layout(location = 0) uniform uint tint_immediates[2];
layout(binding = 0, std430)
buffer out_block_1_ssbo {
  float inner;
} v;
layout(binding = 1, std140)
uniform idx_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_2 = v_1.inner[0u];
  v.inner = uintBitsToFloat(tint_immediates[((min(v_2.x, 1u) * 4u) / 4u)]);
}
