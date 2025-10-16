#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat3x4 inner;
} v;
mat3x4 p = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.inner;
  p[1u] = v.inner[0u];
  p[1u] = v.inner[0u].ywxz;
  vec4 v_1 = v.inner[1u];
  p[0u].y = v_1.x;
}
