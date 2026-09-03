#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  float16_t inner[];
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  float16_t inner[];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = min(0u, (uint(v_1.inner.length()) - 1u));
  uint v_3 = min(0u, (uint(v.inner.length()) - 1u));
  v_1.inner[v_2] = v.inner[v_3];
}
