#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat4 p[4] = mat4[4](mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.inner;
  p[1] = v.inner[2];
  p[1][0] = v.inner[0][1].ywxz;
  p[1][0][0u] = v.inner[0][1].x;
  v_1.inner = p[1][0].x;
}
