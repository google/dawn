SKIP: FAILED

#version 310 es
precision mediump float;

void dpdx_e263de() {
  float res = ddx(1.0f);
}

void fragment_main() {
  dpdx_e263de();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



