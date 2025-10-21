#version 310 es

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 m1 = mat2x4(vec4(0.0f), vec4(0.0f));
  uvec4 v_1 = v.inner[0u];
  uvec4 v_2 = v.inner[0u];
  m1[min(v_1.x, 1u)][min(v_2.y, 3u)] = 1.0f;
}
