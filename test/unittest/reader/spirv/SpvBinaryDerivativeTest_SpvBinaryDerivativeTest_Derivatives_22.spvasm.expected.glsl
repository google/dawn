SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  vec2 x_1 = vec2(50.0f, 60.0f);
  vec2 x_2 = ddy_coarse(x_1);
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
ERROR: 0:6: 'ddy_coarse' : no matching overloaded function found 
ERROR: 0:6: '=' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



