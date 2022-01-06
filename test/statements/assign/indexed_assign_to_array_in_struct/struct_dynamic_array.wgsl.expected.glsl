#version 310 es
precision mediump float;

struct Uniforms {
  uint i;
};
struct InnerS {
  int v;
};

layout (binding = 4) uniform Uniforms_1 {
  uint i;
} uniforms;
layout (binding = 0) buffer OuterS_1 {
  InnerS a1[];
} s1;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  InnerS v = InnerS(0);
  s1.a1[uniforms.i] = v;
  return;
}
void main() {
  tint_symbol();
}


