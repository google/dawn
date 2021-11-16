SKIP: FAILED

#version 310 es
precision mediump float;

struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  vec4 loc3;
};
struct tint_symbol_1 {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs tint_symbol_inner() {
  FragmentOutputs tint_symbol_2 = FragmentOutputs(1, 2.0f, 1u, 1.0f, 2u, vec4(1.0f, 2.0f, 3.0f, 4.0f));
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  FragmentOutputs inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(0, 0u, 0.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0u);
  wrapper_result.loc0 = inner_result.loc0;
  wrapper_result.frag_depth = inner_result.frag_depth;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.sample_mask = inner_result.sample_mask;
  wrapper_result.loc3 = inner_result.loc3;
  return wrapper_result;
}
out int loc0;
out uint loc1;
out float loc2;
out vec4 loc3;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  loc0 = outputs.loc0;
  loc1 = outputs.loc1;
  loc2 = outputs.loc2;
  loc3 = outputs.loc3;
  gl_FragDepth = outputs.frag_depth;
  gl_SampleMask = outputs.sample_mask;
}


Error parsing GLSL shader:
ERROR: 0:49: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:49: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



