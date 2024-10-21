#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat4 inner[4];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat4 a_1[4]) {
  return a_1[0][0][0u];
}
float b(mat4 m) {
  return m[0][0u];
}
float c(vec4 v) {
  return v[0u];
}
float d(float f_1) {
  return f_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_3 = a(v_1.inner);
  float v_4 = (v_3 + b(v_1.inner[1]));
  float v_5 = (v_4 + c(v_1.inner[1][0].ywxz));
  v_2.inner = (v_5 + d(v_1.inner[1][0].ywxz[0u]));
}
