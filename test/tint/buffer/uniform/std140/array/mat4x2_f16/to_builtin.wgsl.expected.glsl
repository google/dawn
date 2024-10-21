#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x2_f16_std140 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat4x2_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x4 t = transpose(f16mat4x2(v.inner[2].col0, v.inner[2].col1, v.inner[2].col2, v.inner[2].col3));
  float16_t l = length(v.inner[0].col1.yx);
  float16_t a = abs(v.inner[0].col1.yx[0u]);
  float16_t v_2 = (t[0][0u] + float16_t(l));
  v_1.inner = (v_2 + float16_t(a));
}
