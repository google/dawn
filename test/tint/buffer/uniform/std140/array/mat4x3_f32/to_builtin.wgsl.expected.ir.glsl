#version 310 es


struct mat4x3_f32_std140 {
  vec3 col0;
  uint tint_pad_0;
  vec3 col1;
  uint tint_pad_1;
  vec3 col2;
  uint tint_pad_2;
  vec3 col3;
  uint tint_pad_3;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat4x3_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x4 t = transpose(mat4x3(v.inner[2].col0, v.inner[2].col1, v.inner[2].col2, v.inner[2].col3));
  float l = length(v.inner[0].col1.zxy);
  float a = abs(v.inner[0].col1.zxy[0u]);
  float v_2 = (t[0][0u] + float(l));
  v_1.inner = (v_2 + float(a));
}
