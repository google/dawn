#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  uint inner[];
} v;
uint tint_f16_to_u32(float16_t value) {
  return mix(4294967295u, mix(0u, uint(value), (value >= 0.0hf)), (value <= 65504.0hf));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint q = 0u;
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  v.inner[v_2] = tint_f16_to_u32(f16mat3x2[2](f16mat3x2(f16vec2(0.0hf, 1.0hf), f16vec2(2.0hf, 3.0hf), f16vec2(2.0hf, 3.0hf)), f16mat3x2(f16vec2(0.0hf, 1.0hf), f16vec2(2.0hf, 3.0hf), f16vec2(2.0hf, 3.0hf)))[min(q, 1u)][0u].x);
}
