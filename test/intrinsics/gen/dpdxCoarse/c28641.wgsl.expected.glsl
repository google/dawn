SKIP: FAILED

#version 310 es
precision mediump float;

void dpdxCoarse_c28641() {
  vec4 res = ddx_coarse(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdxCoarse_c28641();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddx_coarse' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of float'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



