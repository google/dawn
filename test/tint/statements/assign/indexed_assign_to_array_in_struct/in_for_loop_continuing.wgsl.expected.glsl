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
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((i < 4)) {
      } else {
        break;
      }
      uint v_2 = uint(i);
      i = int((v_2 + uint(1)));
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        uvec4 v_3 = v_1.inner[0u];
        s1.a1[min(v_3.x, 7u)] = v;
      }
      continue;
    }
  }
}
