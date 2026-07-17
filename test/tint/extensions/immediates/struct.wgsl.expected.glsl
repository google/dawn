#version 310 es

layout(location = 0) uniform uint tint_immediates[4];
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  vec4 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v_1 = uintBitsToFloat(uvec3(tint_immediates[0u], tint_immediates[1u], tint_immediates[2u]).xyz);
  v.inner = vec4(v_1, float(tint_immediates[3u]));
}
