#version 310 es
precision mediump float;

struct Uniforms {
  uint i;
  uint j;
};
struct InnerS {
  int v;
};
struct S1 {
  InnerS a2[8];
};
struct OuterS {
  S1 a1[8];
};

uint nextIndex = 0u;

uint getNextIndex() {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

layout (binding = 4) uniform Uniforms_1 {
  uint i;
  uint j;
} uniforms;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  InnerS v = InnerS(0);
  OuterS s = OuterS(S1[8](S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0))), S1(InnerS[8](InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0), InnerS(0)))));
  s.a1[getNextIndex()].a2[uniforms.j] = v;
  return;
}
void main() {
  tint_symbol();
}


