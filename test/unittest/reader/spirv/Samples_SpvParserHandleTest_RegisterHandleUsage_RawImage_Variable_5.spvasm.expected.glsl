SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2D x_20;

void main_1() {
  uint x_125 = uint(textureQueryLevels(x_20););
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
ERROR: 0:7: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



