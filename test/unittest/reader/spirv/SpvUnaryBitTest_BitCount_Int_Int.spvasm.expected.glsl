SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  uint u1 = 10u;
  int i1 = 30;
  uvec2 v2u1 = uvec2(10u, 20u);
  ivec2 v2i1 = ivec2(30, 40);
  int x_1 = countbits(i1);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:9: 'countbits' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



