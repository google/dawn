#version 310 es


struct InnerS {
  int v;
};

struct S1 {
  InnerS a2[8];
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(binding = 1, std430)
buffer OuterS_1_ssbo {
  S1 a1[];
} s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  uvec4 v_2 = v_1.inner[0u];
  uvec4 v_3 = v_1.inner[0u];
  uint v_4 = min(v_2.x, (uint(s.a1.length()) - 1u));
  s.a1[v_4].a2[min(v_3.y, 7u)] = v;
}
