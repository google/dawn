SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 dpdyCoarse_3e1ab4() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdydFdyCoarse(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdyCoarse_3e1ab4();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'dFdydFdyCoarse' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 2-component vector of float'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
