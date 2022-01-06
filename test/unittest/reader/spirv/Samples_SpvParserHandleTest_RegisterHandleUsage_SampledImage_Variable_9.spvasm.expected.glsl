SKIP: FAILED

#version 310 es
precision mediump float;


uniform highp sampler2D x_20;

void main_1() {
  float x_131 = texture(x_20, (vec3(0.0f, 0.0f, 0.0f).xy / vec3(0.0f, 0.0f, 0.0f).z), 0.200000003f);
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
ERROR: 0:8: '=' :  cannot convert from ' global highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



