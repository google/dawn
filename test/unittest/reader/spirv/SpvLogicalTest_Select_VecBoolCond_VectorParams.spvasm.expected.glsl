SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  uvec2 x_1 = (bvec2(true, false) ? uvec2(10u, 20u) : uvec2(20u, 10u));
  return;
}

void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:5: '' : boolean expression expected 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



