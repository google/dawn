SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdyCoarse_ae1873() {
  vec3 res = dFdydFdyCoarse(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdyCoarse_ae1873();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdydFdyCoarse' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
