SKIP: FAILED

#version 310 es
precision mediump float;

void dpdxFine_f401a2() {
  float res = ddx_fine(1.0f);
}

void fragment_main() {
  dpdxFine_f401a2();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx_fine' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



