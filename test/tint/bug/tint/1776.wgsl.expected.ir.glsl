SKIP: FAILED

#version 310 es

struct S {
  vec4 a;
  int b;
};

S sb[];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S x = sb[1];
}
error: Error parsing GLSL shader:
ERROR: 0:8: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1
