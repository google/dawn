SKIP: FAILED

#version 310 es
precision mediump float;

void dpdx_99edb1() {
  vec2 res = ddx(vec2(0.0f, 0.0f));
}

void fragment_main() {
  dpdx_99edb1();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



