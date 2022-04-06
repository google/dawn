SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DMS x_20_1;
void main_1() {
  uint x_125 = uint(textureSamples(x_20_1));
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
ERROR: 0:6: 'textureSamples' : no matching overloaded function found 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



