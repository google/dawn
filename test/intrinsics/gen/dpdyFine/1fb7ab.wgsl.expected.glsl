SKIP: FAILED

#version 310 es
precision mediump float;

void dpdyFine_1fb7ab() {
  vec3 res = ddy_fine(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdyFine_1fb7ab();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy_fine' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp mediump 3-component vector of float'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



