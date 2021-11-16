SKIP: FAILED

#version 310 es
precision mediump float;

void dpdyFine_6eb673() {
  float res = ddy_fine(1.0f);
}

void fragment_main() {
  dpdyFine_6eb673();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy_fine' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



