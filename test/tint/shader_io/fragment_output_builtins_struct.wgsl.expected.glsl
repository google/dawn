#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;

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
  gl_SampleMask[0] = int(inner_result.sample_mask);
  return;
}
