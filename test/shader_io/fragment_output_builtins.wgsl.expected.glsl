SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_symbol {
  float value;
};

float main1_inner() {
  return 1.0f;
}

struct tint_symbol_1 {
  uint value;
};

tint_symbol main1() {
  float inner_result = main1_inner();
  tint_symbol wrapper_result = tint_symbol(0.0f);
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol outputs;
  outputs = main1();
  gl_FragDepth = outputs.value;
}


#version 310 es
precision mediump float;

struct tint_symbol {
  float value;
};
struct tint_symbol_1 {
  uint value;
};

uint main2_inner() {
  return 1u;
}

tint_symbol_1 main2() {
  uint inner_result_1 = main2_inner();
  tint_symbol_1 wrapper_result_1 = tint_symbol_1(0u);
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
void main() {
  tint_symbol_1 outputs;
  outputs = main2();
  gl_SampleMask = outputs.value;
}


Error parsing GLSL shader:
ERROR: 0:24: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:24: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



