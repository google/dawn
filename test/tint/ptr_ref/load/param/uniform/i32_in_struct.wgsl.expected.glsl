#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[1];
} v;
int func() {
  uvec4 v_1 = v.inner[0u];
  return int(v_1.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int r = func();
}
