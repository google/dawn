#version 310 es


struct S {
  uint data[3];
};

layout(binding = 0, std140)
uniform constants_block_1_ubo {
  uvec4 inner[1];
} v;
S s = S(uint[3](0u, 0u, 0u));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_1 = v.inner[0u];
  s.data[min(v_1.x, 2u)] = 0u;
}
