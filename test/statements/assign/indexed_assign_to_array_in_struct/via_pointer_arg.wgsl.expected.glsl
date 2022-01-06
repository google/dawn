#version 310 es
precision mediump float;

struct Uniforms {
  uint i;
};
struct InnerS {
  int v;
};
struct OuterS {
  InnerS a1[8];
};

layout (binding = 4) uniform Uniforms_1 {
  uint i;
} uniforms;

void f(inout OuterS p) {
  InnerS v = InnerS(0);
  p.a1[uniforms.i] = v;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  OuterS s1 = OuterS(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0)));
  f(s1);
  return;
}
void main() {
  tint_symbol();
}


