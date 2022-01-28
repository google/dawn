SKIP: FAILED

#version 310 es
precision mediump float;

uint x_1[1] = uint[1](0u);
void main_1() {
  uint x_3 = x_1[0];
  return;
}

void tint_symbol(uint x_1_param) {
  x_1[0] = x_1_param;
  main_1();
}

void main() {
  tint_symbol(uint(gl_SampleMask[0]));
  return;
}
Error parsing GLSL shader:
ERROR: 0:16: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



