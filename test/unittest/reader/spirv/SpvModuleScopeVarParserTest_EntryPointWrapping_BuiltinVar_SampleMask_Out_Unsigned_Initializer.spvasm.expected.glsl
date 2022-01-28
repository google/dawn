SKIP: FAILED

#version 310 es
precision mediump float;

uint x_1[1] = uint[1](0u);
void main_1() {
  return;
}

struct main_out {
  uint x_1_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_1[0]);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  gl_SampleMask[0] = inner_result.x_1_1;
  return;
}
Error parsing GLSL shader:
ERROR: 0:21: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



