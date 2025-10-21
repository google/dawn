#version 310 es


struct InnerS {
  int v;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(binding = 1, std430)
buffer OuterS_1_ssbo {
  InnerS a1[];
} s1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  uvec4 v_2 = v_1.inner[0u];
  uint v_3 = min(v_2.x, (uint(s1.a1.length()) - 1u));
  s1.a1[v_3] = v;
}
