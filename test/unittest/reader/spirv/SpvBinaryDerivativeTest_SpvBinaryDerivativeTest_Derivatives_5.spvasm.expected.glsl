SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  vec3 x_1 = vec3(50.0f, 60.0f, 70.0f);
  vec3 x_2 = ddy(x_1);
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
ERROR: 0:6: 'ddy' : no matching overloaded function found 
ERROR: 0:6: '=' :  cannot convert from ' const float' to ' temp mediump 3-component vector of float'
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



