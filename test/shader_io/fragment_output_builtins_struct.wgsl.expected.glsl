SKIP: FAILED

#version 310 es
precision mediump float;

struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs tint_symbol() {
  FragmentOutputs tint_symbol_1 = FragmentOutputs(1.0f, 1u);
  return tint_symbol_1;
}

void main() {
  FragmentOutputs inner_result = tint_symbol();
  gl_FragDepth = inner_result.frag_depth;
  gl_SampleMask[0] = inner_result.sample_mask;
  return;
}
Error parsing GLSL shader:
ERROR: 0:17: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



