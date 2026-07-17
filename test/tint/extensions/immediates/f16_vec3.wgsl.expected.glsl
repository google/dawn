#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(location = 0) uniform uint tint_immediates[2];
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  vec3 inner;
} v;
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = vec3(tint_bitcast_to_16bit(uvec2(tint_immediates[0u], tint_immediates[1u])).xyz);
}
