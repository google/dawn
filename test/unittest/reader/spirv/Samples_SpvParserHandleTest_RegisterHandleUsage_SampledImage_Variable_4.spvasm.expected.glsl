SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2D x_20_x_10;

void main_1() {
  float x_131 = texture(x_20_x_10, vec2(0.0f, 0.0f), 0.200000003f);
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: '=' :  cannot convert from ' global highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



