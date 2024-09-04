SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdyFine_1fb7ab() {
  vec3 res = dFdydFdyFine(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdyFine_1fb7ab();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdydFdyFine' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
