SKIP: FAILED

#version 310 es

struct S {
  vec4 a;
  int b;
};

struct sb_block {
  S inner[];
};

sb_block sb;
void main_1() {
  S x_18 = sb.inner[1];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:9: '' : array size required 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
