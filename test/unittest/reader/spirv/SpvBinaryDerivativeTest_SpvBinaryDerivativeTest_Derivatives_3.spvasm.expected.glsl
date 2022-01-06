SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  float x_2 = ddy(50.0f);
  return;
}

void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



