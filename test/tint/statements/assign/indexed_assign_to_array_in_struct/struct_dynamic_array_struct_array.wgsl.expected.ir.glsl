SKIP: FAILED

#version 310 es

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
  S1 a1[];
};

uniform Uniforms uniforms;
OuterS s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  s.a1[uniforms.i].a2[uniforms.j] = v;
}
error: Error parsing GLSL shader:
ERROR: 0:17: '' : array size required 
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
