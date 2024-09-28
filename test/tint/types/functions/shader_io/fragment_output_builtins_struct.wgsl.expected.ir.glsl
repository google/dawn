#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs tint_symbol_inner() {
  return FragmentOutputs(1.0f, 1u);
}
void main() {
  FragmentOutputs v = tint_symbol_inner();
  gl_FragDepth = v.frag_depth;
  gl_SampleMask[0u] = int(v.sample_mask);
}
