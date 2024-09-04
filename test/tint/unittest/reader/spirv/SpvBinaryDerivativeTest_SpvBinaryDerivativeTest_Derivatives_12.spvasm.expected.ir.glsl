SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


void main_1() {
  float x_1 = 50.0f;
  float x_2 = dFdydFdyFine(x_1);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdydFdyFine' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
