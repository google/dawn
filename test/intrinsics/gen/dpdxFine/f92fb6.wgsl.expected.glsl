SKIP: FAILED

#version 310 es
precision mediump float;

void dpdxFine_f92fb6() {
  vec3 res = ddx_fine(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdxFine_f92fb6();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx_fine' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp mediump 3-component vector of float'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



