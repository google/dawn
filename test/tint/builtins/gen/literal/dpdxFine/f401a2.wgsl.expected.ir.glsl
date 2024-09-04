SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdxFine_f401a2() {
  float res = dFdxdFdxFine(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdxFine_f401a2();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdxdFdxFine' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
