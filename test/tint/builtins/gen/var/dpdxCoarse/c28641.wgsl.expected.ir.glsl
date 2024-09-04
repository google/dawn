SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdxCoarse_c28641() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdxdFdxCoarse(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_c28641();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'dFdxdFdxCoarse' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
