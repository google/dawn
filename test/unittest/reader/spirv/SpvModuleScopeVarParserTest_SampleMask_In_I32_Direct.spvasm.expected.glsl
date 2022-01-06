SKIP: FAILED

#version 310 es
precision mediump float;

int x_1[1] = int[1](0);

void main_1() {
  int x_3 = x_1[0];
  return;
}

struct tint_symbol_2 {
  uint x_1_param;
};

void tint_symbol_inner(uint x_1_param) {
  x_1[0] = int(x_1_param);
  main_1();
}

void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.x_1_param);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.x_1_param = uint(gl_SampleMask);
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:26: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:26: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



