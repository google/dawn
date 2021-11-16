SKIP: FAILED

#version 310 es
precision mediump float;

void dpdy_7f8d84() {
  float res = ddy(1.0f);
}

void fragment_main() {
  dpdy_7f8d84();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



