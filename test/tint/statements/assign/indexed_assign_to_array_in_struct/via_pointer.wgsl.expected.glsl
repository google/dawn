#version 310 es


struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[8];
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  OuterS s1 = OuterS(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0)));
  uvec4 v_2 = v_1.inner[0u];
  s1.a1[min(v_2.x, 7u)] = v;
}
