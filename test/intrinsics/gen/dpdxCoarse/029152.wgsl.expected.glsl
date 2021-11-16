SKIP: FAILED

#version 310 es
precision mediump float;

void dpdxCoarse_029152() {
  float res = ddx_coarse(1.0f);
}

void fragment_main() {
  dpdxCoarse_029152();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx_coarse' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



