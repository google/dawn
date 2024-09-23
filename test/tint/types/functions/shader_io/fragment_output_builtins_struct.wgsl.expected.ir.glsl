SKIP: FAILED

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
  gl_SampleMask = v.sample_mask;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'assign' :  cannot convert from ' global highp uint' to ' out unsized 1-element array of highp int SampleMaskIn'
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
