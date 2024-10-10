#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat2x4 inner;
} v;
mat2x4 p = mat2x4(vec4(0.0f), vec4(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.inner;
  p[1] = v.inner[0];
  p[1] = v.inner[0].ywxz;
  p[0][1] = v.inner[1].x;
}
