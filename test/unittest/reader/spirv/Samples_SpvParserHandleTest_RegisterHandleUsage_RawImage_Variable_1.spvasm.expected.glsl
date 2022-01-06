SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp writeonly image1D x_20;

void main_1() {
  imageStore(x_20, int(1u), vec4(0.0f, 0.0f, 0.0f, 0.0f));
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
ERROR: 0:4: 'image1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



