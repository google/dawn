#version 310 es

layout(binding = 0, std140)
uniform v_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_2 = v_1.inner[0u];
  uint v_3 = uint(int(v_2.x));
  int u = int((v_3 + uint(1)));
}
