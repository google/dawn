SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdxFine_f92fb6() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdxdFdxFine(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdxFine_f92fb6();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'dFdxdFdxFine' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of float'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
