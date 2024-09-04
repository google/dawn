SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 dpdxFine_9631de() {
  vec2 res = dFdxdFdxFine(vec2(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdxFine_9631de();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdxdFdxFine' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp highp 2-component vector of float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
