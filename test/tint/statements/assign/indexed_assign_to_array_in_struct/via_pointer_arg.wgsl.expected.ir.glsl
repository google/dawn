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
void f(inout OuterS p) {
  InnerS v = InnerS(0);
  uint v_2 = v_1.tint_symbol_1.i;
  p.a1[v_2] = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0)));
  f(s1);
}
