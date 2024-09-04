SKIP: FAILED

#version 310 es

struct Uniforms {
  uint i;
};

struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[];
};

uniform Uniforms uniforms;
OuterS s1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  s1.a1[uniforms.i] = v;
}
error: Error parsing GLSL shader:
ERROR: 0:12: '' : array size required 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
