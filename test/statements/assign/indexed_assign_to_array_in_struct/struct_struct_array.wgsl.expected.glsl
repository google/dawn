#version 310 es
precision mediump float;

struct Uniforms {
  uint i;
};
struct InnerS {
  int v;
};
struct S1 {
  InnerS a[8];
};
struct OuterS {
  S1 s2;
};

layout (binding = 4) uniform Uniforms_1 {
  uint i;
} uniforms;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  InnerS v = InnerS(0);
  OuterS s1 = OuterS(S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))));
  s1.s2.a[uniforms.i] = v;
  return;
}
void main() {
  tint_symbol();
}


