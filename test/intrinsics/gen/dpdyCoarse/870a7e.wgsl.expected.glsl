SKIP: FAILED

#version 310 es
precision mediump float;

void dpdyCoarse_870a7e() {
  float res = ddy_coarse(1.0f);
}

void fragment_main() {
  dpdyCoarse_870a7e();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy_coarse' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



