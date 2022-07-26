SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler1D x_20_1;
void main_1() {
  vec4 x_125 = texelFetch(x_20_1, 0, 0);
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
ERROR: 0:4: 'sampler1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



