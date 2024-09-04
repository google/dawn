SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdxCoarse_029152() {
  float res = dFdxdFdxCoarse(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_029152();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdxdFdxCoarse' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
