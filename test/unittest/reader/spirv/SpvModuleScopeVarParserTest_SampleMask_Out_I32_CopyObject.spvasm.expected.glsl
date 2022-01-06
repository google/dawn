SKIP: FAILED

#version 310 es
precision mediump float;

int x_1[1] = int[1](0);

void main_1() {
  x_1[0] = 12;
  return;
}

struct main_out {
  uint x_1_1;
};
struct tint_symbol_1 {
  uint x_1_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(uint(x_1[0]));
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(0u);
  wrapper_result.x_1_1 = inner_result.x_1_1;
  return wrapper_result;
}
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  gl_SampleMask = outputs.x_1_1;
}


Error parsing GLSL shader:
ERROR: 0:33: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:33: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



