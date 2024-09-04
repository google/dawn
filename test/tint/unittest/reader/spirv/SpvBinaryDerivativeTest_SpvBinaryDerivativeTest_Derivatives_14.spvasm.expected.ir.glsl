SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


void main_1() {
  vec3 x_1 = vec3(50.0f, 60.0f, 70.0f);
  vec3 x_2 = dFdydFdyFine(x_1);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'dFdydFdyFine' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
