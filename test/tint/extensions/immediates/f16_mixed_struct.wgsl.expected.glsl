#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(location = 0) uniform uint tint_immediates[3];
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  vec4 inner;
} v;
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_1 = uintBitsToFloat(tint_immediates[0u]);
  vec2 v_2 = vec2(tint_bitcast_to_16bit(tint_immediates[1u]));
  v.inner = vec4(v_1, v_2, float(tint_bitcast_to_16bit(tint_immediates[2u]).x));
}
