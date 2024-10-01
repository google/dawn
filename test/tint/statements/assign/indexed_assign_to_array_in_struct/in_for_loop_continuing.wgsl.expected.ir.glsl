#version 310 es


struct Uniforms {
  uint i;
};

struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[8];
};

layout(binding = 4, std140)
uniform tint_symbol_2_1_ubo {
  Uniforms tint_symbol_1;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  OuterS s1 = OuterS(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0)));
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      i = (i + 1);
      {
        uint v_2 = v_1.tint_symbol_1.i;
        s1.a1[v_2] = v;
      }
      continue;
    }
  }
}
