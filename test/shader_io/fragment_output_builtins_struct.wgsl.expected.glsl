SKIP: FAILED

#version 310 es
precision mediump float;

struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};
struct tint_symbol_1 {
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs tint_symbol_inner() {
  FragmentOutputs tint_symbol_2 = FragmentOutputs(1.0f, 1u);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  FragmentOutputs inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(0.0f, 0u);
  wrapper_result.frag_depth = inner_result.frag_depth;
  wrapper_result.sample_mask = inner_result.sample_mask;
  return wrapper_result;
}
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  gl_FragDepth = outputs.frag_depth;
  gl_SampleMask = outputs.sample_mask;
}


Error parsing GLSL shader:
ERROR: 0:29: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:29: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



